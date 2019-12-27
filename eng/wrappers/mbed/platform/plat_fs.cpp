#include "mbed.h"
#include "plat_oride.h"

MBED_WEAK
FileSystem *aliyun_iotkit::plat_fs::fs_prepare(void)
{
    /* TODO: Need to take into consideration that kvstore has mounted one filesystem
     *       when storage.storage_type is FILESYSTEM. */
    MBED_ASSERT(0);
    return NULL;
}
