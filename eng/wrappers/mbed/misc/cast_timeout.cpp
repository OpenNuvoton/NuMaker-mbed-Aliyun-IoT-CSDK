#include "mbed.h"
#include "misc/cast_timeout.h"
#include <limits.h>

/* Cast timeout from ali to mbed
 *
 * 1. In mbed, nagative (like -1) is for blocking/indefinitely/forever.
 * 2. By converting large part inaccurately, cast ali timeout value larger than INT_MAX to -1.
 */
int aliyun_iotkit::cast_timeout::cast_timeout_ali2mbed(uint32_t timeout)
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
int aliyun_iotkit::cast_timeout::cast_timeout_mbedtls2mbed(uint32_t timeout)
{
    if (timeout == 0) {
        return -1;
    } else if (timeout > INT_MAX) {
        return -1;
    } else {
        return static_cast<int>(timeout);
    }
}
