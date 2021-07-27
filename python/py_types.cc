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

#include "ppl/common/types.h"
#include "pybind11/pybind11.h"

namespace ppl { namespace common { namespace python {

void RegisterTypes(pybind11::module* m) {
    m->attr("DATATYPE_UNKNOWN") = (uint32_t)DATATYPE_UNKNOWN;
    m->attr("DATATYPE_UINT8") = (uint32_t)DATATYPE_UINT8;
    m->attr("DATATYPE_UINT16") = (uint32_t)DATATYPE_UINT16;
    m->attr("DATATYPE_UINT32") = (uint32_t)DATATYPE_UINT32;
    m->attr("DATATYPE_UINT64") = (uint32_t)DATATYPE_UINT64;
    m->attr("DATATYPE_FLOAT16") = (uint32_t)DATATYPE_FLOAT16;
    m->attr("DATATYPE_FLOAT32") = (uint32_t)DATATYPE_FLOAT32;
    m->attr("DATATYPE_FLOAT64") = (uint32_t)DATATYPE_FLOAT64;
    m->attr("DATATYPE_BFLOAT16") = (uint32_t)DATATYPE_BFLOAT16;
    m->attr("DATATYPE_INT4B") = (uint32_t)DATATYPE_INT4B;
    m->attr("DATATYPE_INT8") = (uint32_t)DATATYPE_INT8;
    m->attr("DATATYPE_INT16") = (uint32_t)DATATYPE_INT16;
    m->attr("DATATYPE_INT32") = (uint32_t)DATATYPE_INT32;
    m->attr("DATATYPE_INT64") = (uint32_t)DATATYPE_INT64;
    m->attr("DATATYPE_BOOL") = (uint32_t)DATATYPE_BOOL;
    m->attr("DATATYPE_COMPLEX64") = (uint32_t)DATATYPE_COMPLEX64;
    m->attr("DATATYPE_COMPLEX128") = (uint32_t)DATATYPE_COMPLEX128;
    m->attr("DATATYPE_MAX") = (uint32_t)DATATYPE_MAX;

    m->attr("DATAFORMAT_UNKNOWN") = (uint32_t)DATAFORMAT_UNKNOWN;
    m->attr("DATAFORMAT_NDARRAY") = (uint32_t)DATAFORMAT_NDARRAY;
    m->attr("DATAFORMAT_NHWC") = (uint32_t)DATAFORMAT_NHWC;
    m->attr("DATAFORMAT_N2CX") = (uint32_t)DATAFORMAT_N2CX;
    m->attr("DATAFORMAT_N4CX") = (uint32_t)DATAFORMAT_N4CX;
    m->attr("DATAFORMAT_N8CX") = (uint32_t)DATAFORMAT_N8CX;
    m->attr("DATAFORMAT_N16CX") = (uint32_t)DATAFORMAT_N16CX;
    m->attr("DATAFORMAT_N32CX") = (uint32_t)DATAFORMAT_N32CX;
    m->attr("DATAFORMAT_MAX") = (uint32_t)DATAFORMAT_MAX;

    m->def("GetDataTypeStr", &GetDataTypeStr);
    m->def("GetDataFormatStr", &GetDataFormatStr);
    m->def("GetSizeOfDataType", &GetSizeOfDataType);
}

}}} // namespace ppl::common::python
