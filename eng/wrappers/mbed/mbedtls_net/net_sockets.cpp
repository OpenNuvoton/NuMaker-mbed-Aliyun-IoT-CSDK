#include "mbed.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls_net/net_sockets.h"
#include "platform/plat_oride.h"
#include "misc/cast_timeout.h"
#include "misc/hal_log.h"
#include <limits.h>

void aliyun_iotkit::mbedtls_net::mbedtls_net_init_alt(mbedtls_net_context_alt *ctx)
{
    memset(ctx, 0x00, sizeof (*ctx));

    /* Default to blocking */
    ctx->mbed_timeout = -1;
}

int aliyun_iotkit::mbedtls_net::mbedtls_net_connect_alt(mbedtls_net_context_alt *ctx, const char *host, const char *port, int proto)
{
    int rc = 0;
    bool do_conn = host ? true : false;

    /* Set 'host':'port' into 'sockaddr': */
    SocketAddress sockaddr;
    /* Translate 'host' to IP address */
    if (host) {
        rc = net_prepare()->gethostbyname(host, &sockaddr);
        if (rc != 0) {
            hal_err("Network interface gethostbyname(%s) failed with %d", host, rc);
            rc = MBEDTLS_ERR_NET_UNKNOWN_HOST;
            goto cleanup;
        }
    }
    /* Set port into 'sockaddr' */
    if (port) {
        sockaddr.set_port(atoi(port));
    }

    /* Establish socket dependent on protocol */
    switch (proto) {
        case MBEDTLS_NET_PROTO_TCP:
            ctx->proto = MBEDTLS_NET_PROTO_TCP;
            ctx->tcp.sock = new TCPSocket();
            /* Check if socket establish succeeds */
            if (ctx->tcp.sock == NULL) {
                hal_err("Construct TCP socket OOM");
                rc = MBEDTLS_ERR_NET_SOCKET_FAILED;
                goto cleanup;
            }
            ctx->sock = ctx->tcp.sock;

            /* Open network socket */
            rc = ctx->tcp.sock->open(net_prepare());
            if (rc != 0) {
                hal_err("TCP socket open failed with %d", rc);
                rc = MBEDTLS_ERR_NET_SOCKET_FAILED;
                break;
            }

            /* Connect to remote peer */
            if (do_conn) {
                rc = ctx->tcp.sock->connect(sockaddr);
                if (rc != 0) {
                    hal_err("TCP socket connect failed with %d", rc);
                    rc = MBEDTLS_ERR_NET_CONNECT_FAILED;
                    break;
                }
            } else {
                hal_warning("mbedtls_net_connect_alt() with NULL host in TCP connection");
            }
            break;

        case MBEDTLS_NET_PROTO_UDP:
            ctx->proto = MBEDTLS_NET_PROTO_UDP;
            ctx->udp.sock = new UDPSocket();
            /* Check if socket establish succeeds */
            if (ctx->udp.sock == NULL) {
                hal_err("Construct UDP socket OOM");
                rc = MBEDTLS_ERR_NET_SOCKET_FAILED;
                goto cleanup;
            }
            ctx->sock = ctx->udp.sock;

            /* Open network socket */
            rc = ctx->udp.sock->open(net_prepare());
            if (rc != 0) {
                hal_err("UDP socket open failed with %d", rc);
                rc = MBEDTLS_ERR_NET_SOCKET_FAILED;
                break;
            }

            /* Set remote peer address for next send() call and filtering incoming packets */
            if (do_conn) {
                rc = ctx->udp.sock->connect(sockaddr);
                if (rc != 0) {
                    hal_err("UDP socket connect failed with %d", rc);
                    rc = MBEDTLS_ERR_NET_CONNECT_FAILED;
                    break;
                }
            } else {
                hal_err("mbedtls_net_connect_alt() with NULL host in UDP connection");
            }
            break;
            
        default:
            /* Invalid parameter 'proto' */
            rc = MBEDTLS_ERR_NET_BAD_INPUT_DATA;
            goto cleanup;
    }

cleanup:

    /* Reset 'ctx' on error */
    if (rc != 0) {
        mbedtls_net_free_alt(ctx);
    }

    return rc;
}

int aliyun_iotkit::mbedtls_net::mbedtls_net_send_alt(void *ctx, const unsigned char *buf, size_t len)
{
    /* No change to blocking/timeout setting. Its change must have done outside. */
    return mbedtls_net_send_mbedtimeout_alt(ctx, buf, len, NULL);
}

int aliyun_iotkit::mbedtls_net::mbedtls_net_send_timeout_alt(void *ctx, const unsigned char *buf, size_t len, uint32_t timeout)
{
    /* Cast timeout from mbedtls to mbed */
    int timeout_ = cast_timeout_mbedtls2mbed(timeout);

    return mbedtls_net_send_mbedtimeout_alt(ctx, buf, len, &timeout_);
}

int aliyun_iotkit::mbedtls_net::mbedtls_net_send_mbedtimeout_alt(void *ctx, const unsigned char *buf, size_t len, int *mbed_timeout_p)
{
    /* Cast 'void *ctx' to 'mbedtls_net_context_alt *' for following net socket operations */
    mbedtls_net_context_alt *ctx_ = static_cast<mbedtls_net_context_alt *>(ctx);

    /* Check 'sock' field in context */
    if (ctx_->sock == NULL) {
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }

    /* Configure timeout in ms. Without explicit timeout, use implicit one. */
    int mbed_timeout = mbed_timeout_p ? *mbed_timeout_p : ctx_->mbed_timeout;
    ctx_->sock->set_timeout(mbed_timeout);

    /* Invoke socket send() and then cast return code */
    nsapi_size_or_error_t rc = ctx_->sock->send(buf, len);
    if (rc >= 0) {
        return rc;
    } else if (rc == NSAPI_ERROR_WOULD_BLOCK) {
        if (mbed_timeout) {
            return MBEDTLS_ERR_SSL_TIMEOUT;
        } else {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
    } else {
        hal_err("Socket send(%d), timeout(%d) failed with %d", len, mbed_timeout, rc);
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }
}

int aliyun_iotkit::mbedtls_net::mbedtls_net_recv_alt(void *ctx, unsigned char *buf, size_t len)
{
    /* No change to blocking/timeout setting. Its change must have done outside. */
    return mbedtls_net_recv_mbedtimeout_alt(ctx, buf, len, NULL);
}

int aliyun_iotkit::mbedtls_net::mbedtls_net_recv_timeout_alt(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
    /* Cast timeout from mbedtls to mbed */
    int timeout_ = cast_timeout_mbedtls2mbed(timeout);

    return mbedtls_net_recv_mbedtimeout_alt(ctx, buf, len, &timeout_);
}

int aliyun_iotkit::mbedtls_net::mbedtls_net_recv_mbedtimeout_alt(void *ctx, unsigned char *buf, size_t len, int *mbed_timeout_p)
{
    /* Cast 'void *ctx' to 'mbedtls_net_context_alt *' for following net socket operations */
    mbedtls_net_context_alt *ctx_ = static_cast<mbedtls_net_context_alt *>(ctx);

    /* Check 'sock' field in context */
    if (ctx_->sock == NULL) {
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }

    /* Configure timeout in ms. Without explicit timeout, use implicit one. */
    int mbed_timeout = mbed_timeout_p ? *mbed_timeout_p : ctx_->mbed_timeout;
    ctx_->sock->set_timeout(mbed_timeout);

    /* Invoke socket recv() and then cast return code */
    nsapi_size_or_error_t rc = ctx_->sock->recv(buf, len);
    if (rc >= 0) {
        return rc;
    } else if (rc == NSAPI_ERROR_WOULD_BLOCK) {
        if (mbed_timeout) {
            return MBEDTLS_ERR_SSL_TIMEOUT;
        } else {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
    } else {
        hal_err("Socket recv(%d), timeout(%d) failed with %d", len, mbed_timeout, rc);
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }
}

void aliyun_iotkit::mbedtls_net::mbedtls_net_set_mbedtimeout(void *ctx, int mbed_timeout)
{
    /* Cast 'void *ctx' to 'mbedtls_net_context_alt *' for following net socket operations */
    mbedtls_net_context_alt *ctx_ = static_cast<mbedtls_net_context_alt *>(ctx);

    ctx_->mbed_timeout = mbed_timeout;
}

void aliyun_iotkit::mbedtls_net::mbedtls_net_free_alt(mbedtls_net_context_alt *ctx)
{
    /* Destroy socket dependent on protocol */
    switch (ctx->proto) {
        case MBEDTLS_NET_PROTO_TCP:
            delete ctx->tcp.sock;
            break;

        case MBEDTLS_NET_PROTO_UDP:
            delete ctx->udp.sock;
            break;
    }

    /* Reset mbedtls_net_context_alt 'ctx' */
    mbedtls_net_init_alt(ctx);

    return;
}

void aliyun_iotkit::mbedtls_net::mbedtls_timing_set_delay_alt(void *data, uint32_t int_ms, uint32_t fin_ms)
{
    /* Cast 'void *data' to 'mbedtls_timing_delay_context_alt *ctx' for following
     * timing delay operations */
    mbedtls_timing_delay_context_alt *ctx = reinterpret_cast<mbedtls_timing_delay_context_alt *>(data);

    /* Record intermediate/final delay */
    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;

    /* Not special case 'cancel' if fin_ms != 0 */
    if (fin_ms != 0) {
        ctx->timer.reset();
        ctx->timer.start();
	} else {
        ctx->timer.stop();
    }
}

int aliyun_iotkit::mbedtls_net::mbedtls_timing_get_delay_alt(void *data)
{
    /* Cast 'void *data' to 'mbedtls_timing_delay_context_alt *ctx' for following
     * timing delay operations */
    mbedtls_timing_delay_context_alt *ctx = reinterpret_cast<mbedtls_timing_delay_context_alt *>(data);

    /* Special case 'cancel' if fin_ms == 0 */
    if (ctx->fin_ms == 0) {
        return -1;
    }

    /* Get elapsed time in ms since last set_delay for the same context */
    uint32_t elapsed_ms = ctx->timer.read_high_resolution_us() / 1000;

    /* If using a event-driven style of programming, an event must be generated
     * when the final delay is passed. The event must cause a call to mbedtls_ssl_handshake()
     * with the proper SSL context to be scheduled. Care must be taken to ensure
     * that at most one such call happens at a time. */
    if (elapsed_ms >= ctx->fin_ms) {
        return 2;
    }

    /* The intermediate delay is passed, and we needn't do anymore compared to final delay passed */
    if (elapsed_ms >= ctx->int_ms) {
        return 1;
    }

    /* Neither intermediate delay nor final delay is passed */
    return 0;
}

/* Cast timeout from ali to mbed
 *
 * 1. In mbed, nagative (like -1) is for blocking/indefinitely/forever.
 * 2. By converting large part inaccurately, cast ali timeout value larger than INT_MAX to -1.
 */
int aliyun_iotkit::mbedtls_net::cast_timeout_ali2mbed(uint32_t timeout)
{
    if (timeout > INT_MAX) {
        return -1;
    } else {
        return static_cast<int>(timeout);
    }
}

/* Cast timeout from mbedtls to mbed
 *
 * 1. In mbed, nagative (like -1) is for blocking/indefinitely/forever.
 * 2. In mbedtls, 0 is for blocking/indefinitely/forever.
 * 3. By converting large part inaccurately, cast mbedtls timeout value larger than INT_MAX to -1.
 */
int aliyun_iotkit::mbedtls_net::cast_timeout_mbedtls2mbed(uint32_t timeout)
{
    if (timeout == 0) {
        return -1;
    } else if (timeout > INT_MAX) {
        return -1;
    } else {
        return static_cast<int>(timeout);
    }
}
