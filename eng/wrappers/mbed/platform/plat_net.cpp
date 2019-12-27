#include "mbed.h"
#include "plat_oride.h"

MBED_WEAK
NetworkInterface *aliyun_iotkit::plat_net::net_prepare(void)
{
    static NetworkInterface *net = NULL;

    if (net) {
        return net;
    }

    NetworkInterface *net_ = NetworkInterface::get_default_instance();
    if (net_ == NULL) {
        MBED_ERROR(MBED_MAKE_ERROR(MBED_MODULE_UNKNOWN, MBED_ERROR_CODE_UNKNOWN), \
                   "No default network interface");
    }

    nsapi_error_t status = net_->connect();
    if (status != NSAPI_ERROR_OK) {
        MBED_ERROR1(MBED_MAKE_ERROR(MBED_MODULE_UNKNOWN, MBED_ERROR_CODE_UNKNOWN), \
                    "Connecting to the network failed", status);
    }
    SocketAddress sockaddr;
    status = net_->get_ip_address(&sockaddr);
    if (status == NSAPI_ERROR_OK) {
        const char *ip_address = sockaddr.get_ip_address();
        printf("Connected to the network successfully. IP address: %s\r\n", ip_address ? ip_address : "null");
    } else {
        printf("Connected to the network successfully. Network interface get_ip_address(...) failed with %d\r\n", status);
    }

    MBED_ASSERT(net_ != NULL);
    net = net_;
    return net;
}
