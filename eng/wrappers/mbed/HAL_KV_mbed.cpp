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

#include "wrappers/wrappers.h"
#include "misc/hal_log.h"
#include "mbed.h"
#include "kv_config.h"
#include "KVMap.h"
#include "KVStore.h"
#include <stdio.h>

#if defined(HAL_KV)

/* Stringize */
#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

static KVStore *default_main_kvstore();

int HAL_Kv_Get(const char *key, void *val, int *buffer_len)
{
    /* Check argument validity */
    if (!key || !val || !buffer_len) {
        hal_err("HAL_Kv_Get(%s) failed with invalid argument");
        return -1;
    }

    KVStore *kvstore = default_main_kvstore();
    MBED_ASSERT(kvstore);

    size_t actual_size = 0;
    int ret = kvstore->get(key, val, *buffer_len, &actual_size, 0);
    if (ret != MBED_SUCCESS) {
        hal_err("HAL_Kv_Get(%s) failed", key);
        return -1;
    } else {
        *buffer_len = actual_size;
    }

    return 0;
}

int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    /* Check argument validity */
    if (!key || !val || !len) {
        hal_err("HAL_Kv_Set(%s) failed with invalid argument");
        return -1;
    }

    KVStore *kvstore = default_main_kvstore();
    MBED_ASSERT(kvstore);

    int ret = kvstore->set(key, val, len, 0);
    if (ret != MBED_SUCCESS) {
        hal_err("HAL_Kv_Set(%s) failed", key);
        return -1;
    }

    return 0;
}

static KVStore *default_main_kvstore()
{
    int ret = kv_init_storage_config();
    if (ret != MBED_SUCCESS) {
        MBED_ERROR1(MBED_MAKE_ERROR(MBED_MODULE_HAL, MBED_ERROR_CODE_UNKNOWN), \
                    "kv_init_storage_config failed", ret);
    }

    KVMap &kv_map = KVMap::get_instance();
    KVStore *kv_instance = kv_map.get_main_kv_instance("/" STR(MBED_CONF_STORAGE_DEFAULT_KV) "/");
    if (!kv_instance) {
        MBED_ERROR(MBED_MAKE_ERROR(MBED_MODULE_HAL, MBED_ERROR_CODE_UNKNOWN), \
                   "KVMap::get_main_kv_instance failed");
    }

    return kv_instance;
}

#endif
