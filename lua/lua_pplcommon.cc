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
#include "ppl/common/common.h"
#include <memory>
using namespace std;
using namespace luacpp;

namespace ppl { namespace common { namespace lua {

void RegisterLog(const shared_ptr<LuaState>&, const shared_ptr<LuaTable>&);
void RegisterRetCode(const shared_ptr<LuaState>&, const shared_ptr<LuaTable>&);
void RegisterTypes(const shared_ptr<LuaState>&, const shared_ptr<LuaTable>&);

}}} // namespace ppl::common::lua

using namespace ppl::common::lua;

extern "C" {

int PPLCOMMON_PUBLIC luaopen_luappl_common(lua_State* l) {
    // may be used by module functions outside this function scope
    auto lstate = make_shared<LuaState>(l, false);
    auto lmodule = make_shared<LuaTable>(lstate->CreateTable());

    RegisterLog(lstate, lmodule);
    RegisterRetCode(lstate, lmodule);
    RegisterTypes(lstate, lmodule);

    lmodule->PushSelf();
    return 1;
}
}
