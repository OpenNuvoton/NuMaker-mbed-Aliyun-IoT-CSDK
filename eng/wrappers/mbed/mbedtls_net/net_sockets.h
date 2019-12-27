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

#ifndef NET_SOCKETS_H
#define NET_SOCKETS_H

#include "mbed.h"

namespace aliyun_iotkit { namespace mbedtls_net
{
    /* Context for mbedtls net socket */
    struct mbedtls_net_context_alt
    {
        int32_t                     proto;
        InternetSocket *            sock;
        union {
            struct {
                TCPSocket *         sock;
            } tcp;
            struct {
                UDPSocket *         sock;
            } udp;
        };
        int                         mbed_timeout;
    };

    /* The following APIs are alternate implementations of mbedtls net socket.
     * See mbedtls net_socket.h/net_socket.c for details */
    void mbedtls_net_init_alt(mbedtls_net_context_alt *ctx);
    int mbedtls_net_connect_alt(mbedtls_net_context_alt *ctx, const char *host, const char *port, int proto);
    int mbedtls_net_send_alt(void *ctx, const unsigned char *buf, size_t len);
    int mbedtls_net_send_timeout_alt(void *ctx, const unsigned char *buf, size_t len, uint32_t timeout);
    int mbedtls_net_send_mbedtimeout_alt(void *ctx, const unsigned char *buf, size_t len, int *mbed_timeout_p);
    int mbedtls_net_recv_alt(void *ctx, unsigned char *buf, size_t len);
    int mbedtls_net_recv_timeout_alt(void *ctx, unsigned char *buf, size_t len, uint32_t timeout);
    int mbedtls_net_recv_mbedtimeout_alt(void *ctx, unsigned char *buf, size_t len, int *mbed_timeout_p);
    void mbedtls_net_set_mbedtimeout(void *ctx, int mbed_timeout);
    void mbedtls_net_free_alt(mbedtls_net_context_alt *ctx);

    /* Context for mbedtls timing delay */
    struct mbedtls_timing_delay_context_alt
    {
        LowPowerTimer   timer;
        uint32_t        int_ms;
        uint32_t        fin_ms;
    };

    /* The following APIs are alternate implementations of mbedtls timing delay.
     * See mbedtls timing.h/timing.c for details */
    void mbedtls_timing_set_delay_alt(void *data, uint32_t int_ms, uint32_t fin_ms);
    int mbedtls_timing_get_delay_alt(void *data);

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

using namespace aliyun_iotkit::mbedtls_net;

#endif /* mbedtls_net.h */
