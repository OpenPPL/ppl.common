// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "binary.h"
#include "crypto.h"
#include "ppl/common/log.h"

#include <atomic>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

#ifdef _WIN32
extern const __declspec(selectany) unsigned char __ppl3core_ocl_binary_archive[];
extern const __declspec(selectany) size_t sizeof___ppl3core_ocl_binary_archive;
#else
extern const unsigned char __ppl3core_ocl_binary_archive[] __attribute__((weak));
extern const size_t sizeof___ppl3core_ocl_binary_archive __attribute__((weak));
#endif

namespace ppl { namespace common { namespace ocl {

namespace {
class ChunkGroupDB {
public:
    explicit ChunkGroupDB(int which);
    ~ChunkGroupDB();

    void InitFromArchive(const unsigned char* data, size_t size, int flags);
    void InsertChunkGroup(const ChunkGroup* group, int flags);
    int kernelsize() {
        return chunkMap_.size();
    }
    int clearChunkMap();
    void Lock() {
        this->locker_.lock();
    }
    void Unlock() {
        this->locker_.unlock();
    }
    int FindChunk(const char* ns, const char* name, const char* archIOInfo, const uint8_t** buffer, size_t* size) const;
    int FindChunkInGroup(const ChunkGroup* group, const char* name, const char* archIOInfo, const uint8_t** buffer,
                         size_t* size) const;

private:
    std::mutex locker_;
    std::map<std::string, std::vector<std::pair<int, const ChunkGroup*>>> chunkMap_;
};

static inline int Compare_KernelBin(const Chunk* kb, const char* name, const char* archIOInfo) {
    int rc = strcmp(kb->name0, name);
    if (rc != 0) {
        return rc;
    }
    if (archIOInfo == NULL)
        return 0;
    return strcmp(kb->name1, archIOInfo);
}

int ChunkGroupDB::FindChunkInGroup(const ChunkGroup* group, const char* name, const char* archIOInfo,
                                   const uint8_t** buffer, size_t* size) const {
    const Chunk* kb = (const Chunk*)(group + 1);
    uint32_t begin = 0, end = group->count;
    // LOG(INFO) << "Chunk numbers of Group[" << group->ns << "/" << group->device << "]: "<< end;

    while (begin < end) {
        uint32_t mid = (begin + end) / 2;
        int rc = Compare_KernelBin(kb + mid, name, archIOInfo);
        if (rc == 0) {
            *buffer = (const uint8_t*)group + kb[mid].offset;
            *size = kb[mid].size;
            return 0;
        } else if (rc < 0) {
            begin = mid + 1;
        } else if (rc > 0) {
            end = mid;
        } else {
            break;
        }
    }
    return -1;
}

int ChunkGroupDB::FindChunk(const char* ns, const char* name, const char* archIOInfo, const uint8_t** buffer,
                            size_t* size) const {
    auto i = this->chunkMap_.find(ns);
    if (i == this->chunkMap_.end()) {
        return -1;
    }

    for (size_t j = 0; j < i->second.size(); ++j) {
        const ChunkGroup* group = i->second[j].second;
        if (0 == this->FindChunkInGroup(group, name, archIOInfo, buffer, size)) {
            return 0;
        }
    }
    LOG(ERROR) << "name: " << name << " " << archIOInfo << " is not exist!";
    return -1;
}

ChunkGroupDB::ChunkGroupDB(int which) {
    if (which == 0) {
        // way1: compiled binary file as cpp file
        if (&sizeof___ppl3core_ocl_binary_archive && &__ppl3core_ocl_binary_archive) {
            InitFromArchive(__ppl3core_ocl_binary_archive, sizeof___ppl3core_ocl_binary_archive, ChunkGroup_No_Copy);
        }

        // TODO way2: load ocl binaries under the table temporarily.
        // std::string file_name = std::string("./ppl_ocl_binary.bin");
        // FILE* fp = fopen(file_name.c_str(), "rb");
        // if(nullptr == fp) {
        //     LOG(ERROR) << "Open file failed with name: " << file_name.c_str();
        //     return;
        // }
        // fseek(fp, 0, SEEK_END);
        // int size = ftell(fp);
        // fseek(fp, 0, SEEK_SET);
        // uint8_t *data = (uint8_t *)malloc(size);
        // fread(data, size, sizeof(uint8_t), fp);
        // InitFromArchive(data, size, 0);
        // free(data);
        // fclose(fp);
    }
    // else if (which == 1) {
    //    if (&sizeof___ppl3core_ocl_source_archive && &__ppl3core_ocl_source_archive) {
    //        InitFromArchive(__ppl3core_ocl_source_archive, sizeof___ppl3core_ocl_source_archive, ChunkGroup_No_Copy);
    //    }
    //}
}

int ChunkGroupDB::clearChunkMap() {
    std::lock_guard<std::mutex> locker_guard(this->locker_);
    for (auto& i : this->chunkMap_) {
        for (auto& g : i.second) {
            if (g.first == 1) {
                if (g.second != nullptr) {
                    free(const_cast<void*>((const void*)g.second));
                    g.second = nullptr;
                }
            }
        }
        // clear all vector content
        if (!(i.second).empty()) {
            i.second.clear();
            std::vector<std::pair<int, const ChunkGroup*>>().swap(i.second);
        }
    }
    this->chunkMap_.clear();
    return 0;
}

ChunkGroupDB::~ChunkGroupDB() {
    for (auto& i : this->chunkMap_) {
        for (auto& g : i.second) {
            if (g.first == 1) {
                if (g.second != nullptr) {
                    free(const_cast<void*>((const void*)g.second));
                    g.second = nullptr;
                    // LOG(ERROR) << "Not all kernel binary are released!";
                }
            }
        }
    }
}

void ChunkGroupDB::InitFromArchive(const unsigned char* data, size_t size, int flags) {
    while (size >= sizeof(ChunkGroup)) {
        const ChunkGroup* group = (const ChunkGroup*)data;
        if (group->size <= size) {
            this->InsertChunkGroup(group, flags);
            // print(2)
        } else {
            break;
        }
        data += group->size;
        size -= group->size;
    }
}

void ChunkGroupDB::InsertChunkGroup(const ChunkGroup* group, int flags) {
    if (group->size < sizeof(ChunkGroup)) {
        LOG(WARNING) << "group->size: " << group->size << " is less than sizeof(Group)";
        return;
    }
    //! TODO(yaodingjie) replace "CURRENT-DEVICE-ID" with runtime check
    // TODO(wangyingrui) just use Adreno_750 now.
    // if (group->device[0] != 0 && 0 != strncmp(group->device, GetDeviceDescription().c_str(), sizeof(group->device)))
    // {
    //    LOG(INFO) << GetDeviceDescription().c_str() << " vs " << group->device;
    //    LOG(ERROR) << "DEVICE ID Mismatched";
    //    return;
    //}
    // TODO(wangyingrui) not support encrypt yet
    if (group->flags & ChunkGroup_Encrypted) {
        uint32_t inputSize = group->size - sizeof(ChunkGroup);
        const ChunkGroup* inputData = (ChunkGroup*)malloc(group->size);
        vector<char> dst;
        DecryptContent((const char*)(group + 1), inputSize, &dst);
        memcpy((void*)inputData, group, sizeof(ChunkGroup));
        memcpy((void*)(inputData + 1), &dst[0], inputSize);
        group = inputData;
    }
    if (group->flags & ChunkGroup_Compressed) {
        LOG(WARNING) << "Compressed chunk group is not supported!";
        return;
    }
    size_t kb_size = group->size - sizeof(ChunkGroup); // sizeof(headers) + data
    size_t kb_hdr_size = sizeof(Chunk) * group->count; // sizeof(headers)

    if (kb_hdr_size > kb_size) {
        LOG(WARNING) << "kb_size is less than " << kb_hdr_size;
        return;
    }
    size_t kb_data_size = kb_size - kb_hdr_size; // data size
    const Chunk* kb = (const Chunk*)(group + 1);
    size_t kb_data_size_calc = 0;
    for (uint32_t i = 0; i < group->count; ++i) {
        kb_data_size_calc += kb[i].size;
        if (kb[i].offset + kb[i].size > group->size) {
            LOG(ERROR) << "chunk data (offset, size): " << kb[i].offset << ", " << kb[i].size
                       << "group->size: " << group->size;
            return;
        }
    }
    if (kb_data_size_calc != kb_data_size) {
        LOG(ERROR) << "Data specified in group header does not equal to data size accumulated by chunk headers";
        return;
    }
    std::lock_guard<std::mutex> locker_guard(this->locker_);

    if (group->flags & ChunkGroup_Encrypted) {
        this->chunkMap_[group->ns].push_back(std::make_pair(1, group));
    } else if (flags & ChunkGroup_No_Copy) {
        this->chunkMap_[group->ns].push_back(std::make_pair(0, group));
    } else {
        ChunkGroup* buffer = (ChunkGroup*)malloc(group->size);
        if (buffer == nullptr) {
            LOG(ERROR) << "malloc " << group->size << " bytes failed!";
            return;
        }
        memcpy(buffer, group, group->size);
        this->chunkMap_[group->ns].push_back(std::make_pair(1, buffer));
    }
}
} // namespace

static ChunkGroupDB* GetKernelBinaryDB() {
    static ChunkGroupDB sInstance(0);
    return &sInstance;
}
static ChunkGroupDB* GetKernelSourceDB() {
    static ChunkGroupDB sInstance(1);
    return &sInstance;
}

int InsertKernelBinaryGroup(const uint8_t* buffer, size_t size, int flags) {
    GetKernelBinaryDB()->InitFromArchive(buffer, size, flags);
    return 0;
}
void LockKernelBinaryDB() {
    GetKernelBinaryDB()->Lock();
}
void UnlockKernelBinaryDB() {
    GetKernelBinaryDB()->Unlock();
}
int FindKernelBinary(const char* ns, const char* name0, const char* name1, const uint8_t** buffer, size_t* size) {
    return GetKernelBinaryDB()->FindChunk(ns, name0, name1, buffer, size);
}

//---------------------------------------------//

int InsertKernelSourceGroup(const uint8_t* buffer, size_t size, int flags) {
    GetKernelSourceDB()->InitFromArchive(buffer, size, flags);
    return 0;
}
void LockKernelSourceDB() {
    GetKernelSourceDB()->Lock();
}
void UnlockKernelSourceDB() {
    GetKernelSourceDB()->Unlock();
}
int FindKernelSource(const char* ns, const char* name0, const char* name1, const uint8_t** buffer, size_t* size) {
    return GetKernelSourceDB()->FindChunk(ns, name0, name1, buffer, size);
}
int KernelBinarySize() {
    return GetKernelBinaryDB()->kernelsize();
}

int ReleaseAllKernelBinaryGroup() {
    GetKernelBinaryDB()->clearChunkMap();
    return 0;
}

//-----------------------------------------------//
/*save kernel binary*/
namespace {
struct BinNode {
    uint8_t* data;
    uint32_t size;
    BinNode* next;
};

struct BinHeader {
    char ns[64];
    uint32_t count;
    uint32_t size;
    BinNode* next;
};

class Binary {
public:
    Binary();
    ~Binary();
    void InsertNode(const char* ns, uint8_t* data, size_t size);
    void SaveKernelBinToFile(const char* dir);
    void Lock() {
        this->locker_.lock();
    }
    void Unlock() {
        this->locker_.unlock();
    }

private:
    std::mutex locker_;
    std::vector<BinHeader*> group;
    void CreateBinHeader(const char* ns, uint8_t* data, size_t size);
    int FindBinHeader(const char* ns);
};

Binary::Binary() {}

Binary::~Binary() {
    for (size_t k = 0; k < this->group.size(); k++) {
        BinHeader* header = this->group[k];
        BinNode* curr = header->next;
        while (nullptr != curr) {
            BinNode* tmp = curr->next;
            free(curr->data);
            free(curr);
            curr = tmp;
        }
        free(header);
    }
}

static inline void insert(BinHeader* header, uint8_t* data, size_t size) {
    BinNode* curr = header->next;
    uint32_t count = header->count;
    Chunk* chunk = (Chunk*)data;
    int cmp = Compare_KernelBin(((Chunk*)curr->data), chunk->name0, chunk->name1);

    while (--count) {
        BinNode* tmp = curr->next;
        if (cmp < 0) {
            cmp = Compare_KernelBin(((Chunk*)tmp->data), chunk->name0, chunk->name1);
            if (cmp > 0) {
                BinNode* bn = (BinNode*)malloc(sizeof(BinNode));
                bn->next = tmp;
                bn->data = (uint8_t*)malloc(size);
                bn->size = size;
                memcpy(bn->data, data, size);
                curr->next = bn;

                header->size += size;
                header->count++;
                break;
            }
        }
        curr = tmp;
    }
    if (nullptr == curr->next && cmp < 0) {
        BinNode* bn = (BinNode*)malloc(sizeof(BinNode));
        bn->data = (uint8_t*)malloc(size);
        memcpy(bn->data, data, size);
        bn->next = nullptr;
        bn->size = size;
        curr->next = bn;

        header->size += size;
        header->count++;
    }

    if (nullptr == curr->next && cmp > 0) {
        BinNode* bn = (BinNode*)malloc(sizeof(BinNode));
        bn->next = header->next;
        bn->data = (uint8_t*)malloc(size);
        memcpy(bn->data, data, size);
        bn->size = size;
        header->next = bn;
        header->size += size;
        header->count++;
    }
}

void Binary::CreateBinHeader(const char* ns, uint8_t* data, size_t size) {
    BinHeader* header = (BinHeader*)malloc(sizeof(BinHeader));
    memset(header, 0, sizeof(BinHeader));
    strcpy(header->ns, ns);

    BinNode* node = (BinNode*)malloc(sizeof(BinNode));
    node->next = nullptr;
    node->data = (uint8_t*)malloc(size);
    memcpy(node->data, data, size);

    header->next = node;
    header->size += size;
    header->count++;
    this->group.push_back(header);
    return;
}

int Binary::FindBinHeader(const char* ns) {
    int rc = -1;
    if (0 == this->group.size())
        return rc;

    for (size_t i = 0; i < this->group.size(); i++) {
        rc = strcmp(this->group[i]->ns, ns);
        if (rc == 0)
            return i;
    }
    return -1;
}

void Binary::InsertNode(const char* ns, uint8_t* data, size_t size) {
    this->Lock();
    int res = FindBinHeader(ns);
    if (res < 0) {
        CreateBinHeader(ns, data, size);
    } else {
        insert(group[res], data, size);
    }
    this->Unlock();
}

void Binary::SaveKernelBinToFile(const char* dir) {
    // std::string file_name = std::string("__ppl_ocl_binary_") + GetDeviceDescription();
    // file_name += std::string(".bin");
    std::string file_name = std::string("ppl_ocl_binary.bin");

    if (dir)
        file_name = std::string(dir) + std::string("/") + file_name;
    // const char* name = getenv("__PPL3_OCL_BINARY_FILE_NAME");
    // file_name = (nullptr == name) ? file_name : std::string(name);

    FILE* fp = fopen(file_name.c_str(), "wb");
    if (nullptr == fp) {
        LOG(ERROR) << "Create file failed with name: " << file_name.c_str();
        return;
    }

    for (uint32_t k = 0; k < this->group.size(); k++) {
        size_t total_size = this->group[k]->size + sizeof(ChunkGroup);
        ChunkGroup* chunk_group = (ChunkGroup*)malloc(this->group[k]->size + sizeof(ChunkGroup));
        memset(chunk_group, 0, total_size);

        strcpy(chunk_group->ns, this->group[k]->ns);
        // strcpy(chunk_group->device, GetDeviceDescription().c_str());
        strcpy(chunk_group->device, "Adreno_750");

        chunk_group->size = total_size;
        chunk_group->count = group[k]->count;

        Chunk* chunk = (Chunk*)(chunk_group + 1);
        uint32_t chunk_data_offset = sizeof(ChunkGroup) + sizeof(Chunk) * group[k]->count;
        uint8_t* data = (uint8_t*)((uint8_t*)chunk_group + chunk_data_offset);

        BinNode* curr = group[k]->next;

        for (uint32_t i = 0; i < group[k]->count; i++) {
            // memcopy Chunk
            memcpy(chunk, curr->data, sizeof(Chunk));
            // update chunk param
            chunk->offset = chunk_data_offset;
            // memcopy chunk data
            memcpy(data, curr->data + sizeof(Chunk), chunk->size);
            data += chunk->size;
            chunk_data_offset += chunk->size;
            chunk++;

            curr = curr->next;
        }

        fseek(fp, 0, SEEK_END);
        uint32_t count = fwrite((void*)((char*)chunk_group), sizeof(char), chunk_group->size, fp);
        if (chunk_group->size != count) {
            LOG(ERROR) << "Write Chunk Group failed[" << group[k]->ns << "]";
        }
    }
    fclose(fp);
}

static Binary* GetBinary() {
    static Binary sInstance;
    return &sInstance;
}

} // namespace

void InsertBinaryChunk(const char* ns, uint8_t* data, size_t size) {
    GetBinary()->InsertNode(ns, data, size);
}

void SaveKernelBinToFile(const char* dir) {
    GetBinary()->SaveKernelBinToFile(dir);
}

}}} // namespace ppl::common::ocl
