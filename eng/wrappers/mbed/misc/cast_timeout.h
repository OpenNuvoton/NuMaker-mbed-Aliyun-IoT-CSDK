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

#ifndef CAST_TIMEOUT_H
#define CAST_TIMEOUT_H

#include "mbed.h"

namespace aliyun_iotkit { namespace cast_timeout
{
    /* Cast timeout among ali/mbedtls/mbed
     *
     * There are inconsistencies in timeout definition among ali/mbedtls/mbed (rtos/socket):
     * In ali, 0 for non-blocking and 0xFFFFFFFF for maximum
     * In mbedtls, 0 for blocking/indefinitely/forever and 0xFFFFFFFF for maximum
     * In mbed, 0 for non-blocking and negative for blocking/indefinitely/forever
     *
     * TODO: Check timeout 0 in ali means non-blocking or blocking. It is not well-defined.
     */
    int cast_timeout_ali2mbed(uint32_t timeout);
    int cast_timeout_mbedtls2mbed(uint32_t timeout);
}}

using namespace aliyun_iotkit::cast_timeout;

#endif /* cast_timeout.h */
