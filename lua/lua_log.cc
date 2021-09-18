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

#include "luacpp.h"
#include "ppl/common/log.h"
#include <memory>
using namespace std;
using namespace luacpp;

namespace ppl { namespace common { namespace lua {

void RegisterLog(const shared_ptr<LuaState>& lstate, const shared_ptr<LuaTable>& lmodule) {
    lmodule->SetInteger("LOG_LEVEL_DEBUG", LOG_LEVEL_DEBUG);
    lmodule->SetInteger("LOG_LEVEL_INFO", LOG_LEVEL_INFO);
    lmodule->SetInteger("LOG_LEVEL_WARNING", LOG_LEVEL_WARNING);
    lmodule->SetInteger("LOG_LEVEL_ERROR", LOG_LEVEL_ERROR);
    lmodule->SetInteger("LOG_LEVEL_FATAL", LOG_LEVEL_FATAL);
    lmodule->SetInteger("LOG_LEVEL_MAX", LOG_LEVEL_MAX);

    lmodule->Set("SetLoggingLevel", lstate->CreateFunction([](uint32_t level) -> void {
        GetCurrentLogger()->SetLogLevel(level);
    }));
    lmodule->Set("GetLoggingLevel", lstate->CreateFunction([]() -> uint32_t {
        return GetCurrentLogger()->GetLogLevel();
    }));
}

}}} // namespace ppl::common::lua
