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
#include "platform/plat_oride.h"
#include "misc/hal_log.h"
#include "misc/cast_timeout.h"
#include "mbed.h"

int HAL_TCP_Destroy(uintptr_t fd);

uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    uintptr_t fd = static_cast<uintptr_t>(-1);

    /* Enter do-while one loop with support for catching exception case
     *
     * Run 'return' to exit from the function without handling exception case
     * Run 'break' to exit from the loop to handle exception case
     */
    do {
        /* Create 'TCPSocket' for TCP socket */
        TCPSocket *tcpsock = new TCPSocket();
        fd = reinterpret_cast<uintptr_t>(tcpsock);

        /* Open a network socket on the network stack of the given network interface */
        int rc = tcpsock->open(net_prepare());
        if (rc != NSAPI_ERROR_OK) {
            hal_err("Open network socket failed with %d", rc);
            break;
        }

        /* Set 'host':'port' into 'sockaddr' */
        SocketAddress sockaddr;
        if (host) {
            /* Translate 'host' to IP address */
            int rc = net_prepare()->gethostbyname(host, &sockaddr);
            if (rc != 0) {
                hal_err("DNS failed with %s", host);
                break;
            }
        } else {
            hal_err("HAL_TCP_Establish failed with NULL host");
            break;
        }
        /* Set port into 'sockaddr' */
        sockaddr.set_port(port);

        rc = tcpsock->connect(sockaddr);
        if (rc != 0) {
            hal_err("TCP connection failed with %d", rc);
            break;
        }

        /* Successful return */
        return fd;

    /* Exit do-while one-loop */
    } while (0);

    /* Handle exception case from here */
    HAL_TCP_Destroy(fd);
    return static_cast<uintptr_t>(-1);
}

int HAL_TCP_Destroy(uintptr_t fd)
{
    /* Check 'fd' parameter */
    if (fd == static_cast<uintptr_t>(-1)) {
        return -1;
    }

    /* Cast 'fd' to 'TCPSocket *' */
    TCPSocket *tcpsocket = reinterpret_cast<TCPSocket *>(fd);

    delete tcpsocket;

    return 0;
}

int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    /* Check 'fd' parameter */
    if (fd == static_cast<uintptr_t>(-1)) {
        hal_err("Invalid fd: %d", fd);
        return -1;
    }

    /* Cast 'fd' to 'TCPSocket *' */
    TCPSocket *tcpsocket = reinterpret_cast<TCPSocket *>(fd);

    uint32_t rmn_ms = timeout_ms;
    uint32_t len_xfer = 0;
    Timer t;
    t.start();

    do {
        tcpsocket->set_timeout(cast_timeout_ali2mbed(rmn_ms));
        nsapi_size_or_error_t rc = tcpsocket->send(reinterpret_cast<const unsigned char *>(buf) + len_xfer, len - len_xfer);

        if (rc >= 0) {
            len_xfer += rc;
        } else if (rc == NSAPI_ERROR_WOULD_BLOCK) {
            break;
        } else {
            hal_err("HAL_TCP_Write failed");
            break;
        }

        /* Calculate remaining time */
        uint32_t elapsed_ms = t.read_ms();
        rmn_ms = (timeout_ms >= elapsed_ms) ? (timeout_ms - elapsed_ms) : 0;

    } while ((len_xfer < len) && rmn_ms);

    return len_xfer;
}

int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    /* Check 'fd' parameter */
    if (fd == static_cast<uintptr_t>(-1)) {
        hal_err("Invalid fd: %d", fd);
        return -1;
    }

    /* Cast 'fd' to 'TCPSocket *' */
    TCPSocket *tcpsocket = reinterpret_cast<TCPSocket *>(fd);

    uint32_t rmn_ms = timeout_ms;
    uint32_t len_xfer = 0;
    Timer t;
    t.start();

    do {
        tcpsocket->set_timeout(cast_timeout_ali2mbed(rmn_ms));
        nsapi_size_or_error_t rc = tcpsocket->recv(reinterpret_cast<char *>(buf) + len_xfer, len - len_xfer);

        if (rc >= 0) {
            len_xfer += rc;
        } else if (rc == NSAPI_ERROR_WOULD_BLOCK) {
            break;
        } else {
            hal_err("HAL_TCP_Read failed");
            break;
        }

        /* Calculate remaining time */
        uint32_t elapsed_ms = t.read_ms();
        rmn_ms = (timeout_ms >= elapsed_ms) ? (timeout_ms - elapsed_ms) : 0;

    } while ((len_xfer < len) && rmn_ms);

    return len_xfer;
}
