#ifndef __PPLCUDA_CONV_PARAM_H_
#define __PPLCUDA_CONV_PARAM_H_

#include <string>
#include <vector>
#include <cuda_runtime.h>
#include <cuda.h>
#include "ppl/common/types.h"
#include "ppl/common/retcode.h"

struct conv_param_t {
    int in_height;
    int in_width;
    int in_num;
    int num_grp;
    int num_chl;
    int num_chl_pad;
    int num_flt;
    int num_flt_pad;
    int flt_height;
    int flt_width;
    int out_height;
    int out_width;
    int stride_height;
    int stride_width;
    int pad_height;
    int pad_width;
    int hole_height;
    int hole_width;
    int has_bias; // int4* bias;
};

struct tiles_param_t {
    int m_cta  = -1;
    int n_cta  = -1;
    int k_cta  = -1;
    int m_warp = -1;
    int n_warp = -1;

    int k_per_step   = -1; // for idxn conv
    int k_per_set    = -1; // for 2spk conv
    int flt_size     = -1; // for 2spk conv
    int flt_pad_size = -1; // for idxn conv

    int cta_size_in_thd = -1;
    int smem_size       = 0;
    int buf             = 1;
};

struct algo_param_t {
    std::string algo_type = "";
    std::string algo_name = "";
    std::string conv_type = "";
    std::string mma_shape = "";
    tiles_param_t tiles;
    int kid                    = -1;
    unsigned int splitk        = 1;
    unsigned int splitf        = 1;
    unsigned int gemm_batch    = 1;

    ppl::common::RetCode ParseAlgoName();
    ppl::common::RetCode BuildAlgoName();

    void UseDefaultF1Kernel(const cudaDeviceProp& device_prop)
    {
        if (device_prop.major == 7 && device_prop.minor < 5) {
            algo_name         = "nv2spkSm70Fp16Conv_hmma1688_nhwc_f1_b16x8_w16x8_k8_s8_buf1";
        } else if (device_prop.major == 7 && device_prop.minor >= 5) {
            algo_name         = "nv2spkSm75Fp16Conv_hmma1688_nhwc_f1_b16x8_w16x8_k8_s8_buf1";
        } else if (device_prop.major == 8 && device_prop.minor >= 0) {
            algo_name         = "nv2spkSm80Fp16Conv_hmma1688_nhwc_f1_b16x8_w16x8_k8_s8_buf1";
        }
        algo_type             = "TuringIMMAImpgemm";
        mma_shape             = "hmma1688";
        tiles.m_cta           = 16;
        tiles.n_cta           = 8;
        tiles.m_warp          = 16;
        tiles.n_warp          = 8;
        tiles.k_cta           = 8;
        tiles.k_per_set       = 8;
        tiles.cta_size_in_thd = 32;
        tiles.smem_size       = 384;
        tiles.flt_size        = 1;
        tiles.buf             = 1;
        kid                   = 0;
        gemm_batch            = 1;
    };

    void SetIdxnKernelParam(int m_cta, int n_cta, int k_cta, int m_warp, int n_warp, int k_per_step, int flt_pad_size,
            int cta_size_in_thd, int smem_size, int splitk, int splitf, std::string mma_shape)
    {
        this->tiles.m_cta = m_cta;
        this->tiles.n_cta = n_cta;
        this->tiles.k_cta = k_cta;

        this->tiles.m_warp = m_warp;
        this->tiles.n_warp = n_warp;

        this->tiles.k_per_step = k_per_step;
        this->tiles.flt_pad_size = flt_pad_size;

        this->tiles.cta_size_in_thd = cta_size_in_thd;
        this->tiles.smem_size = smem_size;

        this->splitk = splitk;
        this->splitf = splitf;

        this->mma_shape = mma_shape;
        this->conv_type = "idxn";
        this->BuildAlgoName();
    }

    void Set2spkKernelParam(int m_cta, int n_cta, int k_cta, int m_warp, int n_warp, int k_per_set, int flt_size,
            int buf_num, int cta_size_in_thd, int smem_size, int splitk, int splitf, std::string mma_shape)
    {
        this->tiles.m_cta = m_cta;
        this->tiles.n_cta = n_cta;
        this->tiles.k_cta = k_cta;

        this->tiles.m_warp = m_warp;
        this->tiles.n_warp = n_warp;

        this->tiles.k_per_set = k_per_set;
        this->tiles.flt_size = flt_size;
        this->tiles.buf = buf_num;

        this->tiles.cta_size_in_thd = cta_size_in_thd;
        this->tiles.smem_size = smem_size;

        this->splitk = splitk;
        this->splitf = splitf;

        this->mma_shape = mma_shape;
        this->conv_type = "2spk";
        this->BuildAlgoName();
    }

    void SetSwzlKernelParam(int m_cta, int n_cta, int k_cta, int m_warp, int n_warp, int flt_size,
            int buf_num, int cta_size_in_thd, int smem_size, int splitk, int splitf, std::string mma_shape)
    {
        this->tiles.m_cta = m_cta;
        this->tiles.n_cta = n_cta;
        this->tiles.k_cta = k_cta;

        this->tiles.m_warp = m_warp;
        this->tiles.n_warp = n_warp;

        this->tiles.flt_size = flt_size;
        this->tiles.buf = buf_num;

        this->tiles.cta_size_in_thd = cta_size_in_thd;
        this->tiles.smem_size = smem_size;

        this->splitk = splitk;
        this->splitf = splitf;

        this->mma_shape = mma_shape;
        this->conv_type = "swzl";
        this->BuildAlgoName();
    }
};

struct quant_param_t {
    float in_scale;
    void *d_flt_scale;
    float out_scale;
    float pre_scale;
};

struct fuse_param_t {
    int has_activation = 0; // 1: relu,  2: sigmoid
    bool has_clip      = false;
    float clip_min;
    float clip_max;
    int has_prelu = 0;
    void* prelu;
    float leaky  = 0;
    bool has_elt = false;
    void* pre_data;
    int has_elt_activation = 0;
    bool has_elt_clip      = false;
    float elt_clip_min;
    float elt_clip_max;
    int has_elt_prelu = 0;
    void* elt_prelu;
    float elt_leaky = 0;
    bool has_concat = false;
    int concat_offset;
    int concat_stride;
    void* post_concat;
};

struct fuse_info_t {
    std::vector<std::string> types; // support fuse relu + add + relu right now
    std::vector<int32_t> input_inds; // save fused kernel's input index
    std::vector<void*> fuse_attrs; // save fused kernel's attributes
    int channel_size   = -1; // save total channel size for concat
    int channel_offset = -1; // save output offset if we fuse concat
    int concat_edge_id = -1; // save concat output edge id
};

#endif // __PPLCUDA_CONV_PARAM_H_
