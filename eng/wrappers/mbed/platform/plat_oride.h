/*
 * Copyright (c) 2019-2020, Nuvoton Technology Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLAT_ORIDE_H
#define PLAT_ORIDE_H

#include "mbed.h"

namespace aliyun_iotkit { namespace plat_fs
{
    /* Prepare default file system in mbed */
    FileSystem *fs_prepare(void);
}}

namespace aliyun_iotkit { namespace plat_net
{
    /* Prepare default network interface in mbed */
    NetworkInterface *net_prepare(void);
}}

using namespace aliyun_iotkit::plat_fs;
using namespace aliyun_iotkit::plat_net;

#endif /* plat_oride.h */
