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

#if defined(COAP_CLIENT) || defined(COAP_SERVER)

int HAL_UDP_close(intptr_t p_socket);

intptr_t HAL_UDP_create(char *host, unsigned short port)
{
    intptr_t fd = static_cast<intptr_t>(-1);

    /* Enter do-while one loop with support for catching exception case
     *
     * Run 'return' to exit from the function without handling exception case
     * Run 'break' to exit from the loop to handle exception case
     */
    do {
        /* Create 'UDPSocket' for UDP socket */
        UDPSocket *udpsock = new UDPSocket();
        fd = reinterpret_cast<intptr_t>(udpsock);

        /* Open a network socket on the network stack of the given network interface */
        int rc = udpsock->open(net_prepare());
        if (rc != NSAPI_ERROR_OK) {
            hal_err("Open network socket failed with %d", rc);
            break;
        }

        /* Set 'host':'port' into 'sockaddr': */
        SocketAddress sockaddr;
        if (host) {
            /* Translate 'host' to IP address */
            int rc = net_prepare()->gethostbyname(host, &sockaddr);
            if (rc != 0) {
                hal_err("DNS failed with %s", host);
                break;
            }
        } else {
            hal_err("HAL_UDP_create failed with NULL host");
            break;
        }
        /* Set port into 'sockaddr' */
        sockaddr.set_port(port);

        /* Set remote peer address for next send() call and filtering incoming packets */
        rc = udpsock->connect(sockaddr);
        if (rc != 0) {
            hal_err("UDP connection failed with %d", rc);
            break;
        }

        /* Successful return */
        return fd;

    /* Exit do-while one-loop */
    } while (0);

    /* Handle exception case from here */
    HAL_UDP_close(fd);
    return static_cast<intptr_t>(-1);
}

int HAL_UDP_close(intptr_t p_socket)
{
    /* Check 'p_socket' parameter */
    if (p_socket == static_cast<intptr_t>(-1)) {
        return -1;
    }

    /* Cast 'p_socket' to 'UDPSocket *' */
    UDPSocket *udpsocket = reinterpret_cast<UDPSocket *>(p_socket);

    delete udpsocket;

    return 0;
}

int HAL_UDP_write(intptr_t p_socket,
                  const unsigned char *p_data,
                  unsigned int datalen)
{
    return HAL_UDP_sendto(p_socket, NULL, p_data, datalen, static_cast<unsigned int>(-1));
}

int HAL_UDP_readTimeout(intptr_t p_socket,
                        unsigned char *p_data,
                        unsigned int datalen,
                        unsigned int timeout)
{
    return HAL_UDP_recvfrom(p_socket, NULL, p_data, datalen, timeout);
}

intptr_t HAL_UDP_create_without_connect(const char *host, unsigned short port)
{
    /* 'host' here means local address to bind to, not remote address to connect to. */

    intptr_t fd = static_cast<intptr_t>(-1);

    /* Enter do-while one loop with support for catching exception case
     *
     * Run 'return' to exit from the function without handling exception case
     * Run 'break' to exit from the loop to handle exception case
     */
    do {
        /* Create 'UDPSocket' for UDP socket */
        UDPSocket *udpsock = new UDPSocket();
        fd = reinterpret_cast<intptr_t>(udpsock);

        /* Set 'host':'port' into 'sockaddr': */
        SocketAddress sockaddr;
        if (host) {
            /* Translate 'host' to IP address */
            int rc = net_prepare()->gethostbyname(host, &sockaddr);
            if (rc != 0) {
                hal_err("DNS failed with %s", host);
                break;
            }
        } else {
            /* Bind with 'NULL' host
             *
             * On NULL 'host', regard it as 'INADDR_ANY' which means bind to all available interfaces.
             * Check 'INADDR_ANY' at the link below for its meaning in different situations.
             *
             * http://man7.org/linux/man-pages/man7/ip.7.html
             */
            sockaddr.set_ip_address(NULL);
        }
        /* Set port into 'sockaddr' */
        sockaddr.set_port(port);

        /* Bind to 'sockaddr' */
        int rc = udpsock->bind(sockaddr);
        if (rc != 0) {
            hal_err("UDPSocket::bind(%s:%d) failed: %d", host ? host : "NULL", port, rc);
            break;
        }

        /* TODO: Follow reference implementation for multi-cast configuration */

        return fd;

    /* Exit do-while one-loop */
    } while (0);

    /* Handle exception case from here */
    HAL_UDP_close_without_connect(fd);
    return static_cast<intptr_t>(-1);
}

int HAL_UDP_close_without_connect(intptr_t sockfd)
{
    return HAL_UDP_close(sockfd);
}

int HAL_UDP_joinmulticast(intptr_t sockfd,
                          char *p_group)
{
    /* Check 'sockfd' parameter */
    if (sockfd == static_cast<intptr_t>(-1)) {
        hal_err("Invalid sockfd: %p", sockfd);
        return -1;
    }

    if (p_group == NULL) {
        hal_err("Invalid p_group: %p", p_group);
        return -1;
    }

    /* Cast 'p_socket' to 'UDPSocket *' */
    UDPSocket *udpsocket = reinterpret_cast<UDPSocket *>(sockfd);

     /* Prepare multicast group address */
    SocketAddress sockaddr;
    if (!sockaddr.set_ip_address(p_group)) {
        hal_err("SocketAddress.set_ip_address(%s) failed", p_group);
        return -1;
    }

    int rc = udpsocket->join_multicast_group(sockaddr);
    if (rc != 0) {
        hal_err("UDPSocket::join_multicast_group(%s) failed: %d", p_group, rc);
        return -1;
    }

    return 0;
}

int HAL_UDP_recvfrom(intptr_t sockfd,
                     NetworkAddr *p_remote,
                     unsigned char *p_data,
                     unsigned int datalen,
                     unsigned int timeout_ms)
{
    /* Check 'sockfd' parameter */
    if (sockfd == static_cast<intptr_t>(-1)) {
        hal_err("Invalid sockfd: %p", sockfd);
        return -1;
    }

    /* Cast 'sockfd' to 'UDPSocket *' */
    UDPSocket *udpsocket = reinterpret_cast<UDPSocket *>(sockfd);

    /* Configure timeout in ms */
    udpsocket->set_timeout(cast_timeout_ali2mbed(timeout_ms));

    /* Remote address */
    SocketAddress sockaddr;

    /* Invoke socket recvfrom() */
    nsapi_size_or_error_t rc = udpsocket->recvfrom(&sockaddr, p_data, datalen);

    /* Return remote address */
    if (p_remote) {
        strncpy(reinterpret_cast<char *>(p_remote->addr), sockaddr.get_ip_address(), NETWORK_ADDR_LEN);
        p_remote->addr[NETWORK_ADDR_LEN - 1] = '\0';    // null character manually added
        p_remote->port = sockaddr.get_port();
    }

    /* FIXME: Error code mapping doesn't follow spec strictly. */
    if (rc >= 0) {
        return rc;
    } else if (rc == NSAPI_ERROR_WOULD_BLOCK) {
        return 0;
    } else {
        hal_err("UDPSocket::recv() failed: %d", rc);
        return -1;
    }
}

int HAL_UDP_sendto(intptr_t sockfd,
                   const NetworkAddr *p_remote,
                   const unsigned char *p_data,
                   unsigned int datalen,
                   unsigned int timeout_ms)
{
    /* Check 'p_socket' parameter */
    if (sockfd == static_cast<intptr_t>(-1)) {
        hal_err("Invalid sockfd: %p", sockfd);
        return -1;
    }

    /* Cast 'sockfd' to 'UDPSocket *' */
    UDPSocket *udpsocket = reinterpret_cast<UDPSocket *>(sockfd);

    /* Configure timeout in ms */
    udpsocket->set_timeout(cast_timeout_ali2mbed(timeout_ms));

    /* Prepare remote address */
    SocketAddress sockaddr;
    /* If 'p_remote' is NULL, remote address should have been specified through HAL_UDP_create()/HAL_UDP_connect(). */
    if (p_remote) {
        if (!sockaddr.set_ip_address(reinterpret_cast<const char *>(p_remote->addr))) {
            hal_err("SocketAddress.set_ip_address(%s) failed", p_remote->addr);
            return -1;
        }
        sockaddr.set_port(p_remote->port);
    }

    /* Invoke socket send/sendto */
    nsapi_size_or_error_t rc = 0;
    if (p_remote) {
        rc = udpsocket->sendto(sockaddr, p_data, datalen);
    } else {
        rc = udpsocket->send(p_data, datalen);
    }

    /* FIXME: Error code mapping doesn't follow spec strictly. */
    if (rc >= 0) {
        return rc;
    } else if (rc == NSAPI_ERROR_WOULD_BLOCK) {
        return 0;
    } else {
        hal_err("UDPSocket::send()/sendto() failed: %d", rc);
        return -1;
    }
}

uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
    return -1;
}

#endif
