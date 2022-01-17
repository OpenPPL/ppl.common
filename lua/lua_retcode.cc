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

#include "ppl/common/retcode.h"
#include "luacpp/luacpp.h"
#include <memory>
using namespace std;
using namespace luacpp;

namespace ppl { namespace common { namespace lua {

void RegisterRetCode(const shared_ptr<LuaState>& lstate, const shared_ptr<LuaTable>& lmodule) {
    lmodule->SetInteger("RC_SUCCESS", RC_SUCCESS);
    lmodule->SetInteger("RC_OTHER_ERROR", RC_OTHER_ERROR);
    lmodule->SetInteger("RC_UNSUPPORTED", RC_UNSUPPORTED);
    lmodule->SetInteger("RC_OUT_OF_MEMORY", RC_OUT_OF_MEMORY);
    lmodule->SetInteger("RC_INVALID_VALUE", RC_INVALID_VALUE);
    lmodule->SetInteger("RC_EXISTS", RC_EXISTS);
    lmodule->SetInteger("RC_NOT_FOUND", RC_NOT_FOUND);
    lmodule->SetInteger("RC_PERMISSION_DENIED", RC_PERMISSION_DENIED);
    lmodule->SetInteger("RC_HOST_MEMORY_ERROR", RC_HOST_MEMORY_ERROR);
    lmodule->SetInteger("RC_DEVICE_MEMORY_ERROR", RC_DEVICE_MEMORY_ERROR);
    lmodule->SetInteger("RC_DEVICE_RUNTIME_ERROR", RC_DEVICE_RUNTIME_ERROR);

    lmodule->Set("GetRetCodeStr", lstate->CreateFunction(&GetRetCodeStr));
}

}}} // namespace ppl::common::lua
