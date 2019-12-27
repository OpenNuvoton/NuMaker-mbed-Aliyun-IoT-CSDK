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
#if COMPONENT_MBEDBL_UCP
#include "update-client-metadata-header/arm_uc_metadata_header_v2.h"
#include "update-client-paal/arm_uc_paal_update_api.h"
#include "mbedbl-ucp.h"
#include "eng/ota/ota_api.h"
#endif
#include <inttypes.h>

/* Stringize */
#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

void *HAL_Malloc(uint32_t size)
{
	return new uint8_t[size];
}

void HAL_Free(void *ptr)
{
    uint8_t *ptr_ = static_cast<uint8_t *>(ptr);
	delete [] ptr_;
}

uint64_t HAL_UptimeMs(void)
{
    /* NOTE: This requires MBED_CPU_STATS_ENABLED and DEVICE_LPTICKER. */
    return mbed_uptime() / 1000;
}

void HAL_SleepMs(uint32_t ms)
{
	ThisThread::sleep_for(ms);
}

void HAL_Srandom(uint32_t seed)
{
	srand(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    uint32_t random_ = (uint32_t) rand();
	return (region > 0) ? (random_ % region) : random_;
}

int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
	va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
	return vsnprintf(str, len, format, ap);
}

void HAL_Reboot(void)
{
   /* FIXME: Unmount file system and then reset */

    system_reset();
}

#define FIRMWARE_VERSION_UNKNOWN    00000000

int HAL_GetFirmwareVersion(char *version)
{
    int n = snprintf(version, IOTX_FIRMWARE_VER_LEN, STR(FIRMWARE_VERSION_UNKNOWN));

#if COMPONENT_MBEDBL_UCP
    /* We don't call ARM_UCP_GetActiveFirmwareDetails to get active firmware version because
     * it requires UCP sequence ARM_UCP_SetPAALUpdate/ARM_UCP_Initialize/ARM_UCP_Finalize. */
    do {
        const uint8_t *int_hdr =(const uint8_t *) MBED_CONF_UPDATE_CLIENT_APPLICATION_DETAILS;

        /* Check magic of internal header */
        uint32_t headerMagic = arm_uc_parse_uint32(int_hdr);
        if (headerMagic != ARM_UC_INTERNAL_HEADER_MAGIC_V2) {
            hal_err("Invalid magic of internal header: 0x%08x", headerMagic);
            break;
        }

        /* Check version of internal header */
        uint32_t headerVersion = arm_uc_parse_uint32(int_hdr + 4);
        if (headerVersion != ARM_UC_INTERNAL_HEADER_VERSION_V2) {
            hal_err("Invalid version of internal header: 0x%08x", headerVersion);
            break;
        }

        arm_uc_error_t result = { .code = ERR_INVALID_PARAMETER };
        arm_uc_firmware_details_t details;
        result = arm_uc_parse_internal_header_v2(int_hdr, &details);
        if (result.error != ERR_NONE) {
            hal_err("Failed to parse v2 header");
            break;
        }

        n = snprintf(version, IOTX_FIRMWARE_VER_LEN + 1, "%" PRIu64, details.version);
    } while (0);

    /* Handle exception case from here */

#endif

    return n;
}

void *HAL_SemaphoreCreate(void)
{
    Semaphore *sem = new Semaphore(0);
    if (!sem) {
        hal_err("Create Semaphore failed");
        return NULL;
    }
    
    return static_cast<void *>(sem);
}

void HAL_SemaphoreDestroy(void *sem)
{
    if (sem == NULL) {
        hal_err("NULL semaphore");
        return;
    }

    Semaphore *sem_ = static_cast<Semaphore *>(sem);
    delete sem_;
}

void HAL_SemaphorePost(void *sem)
{
    if (sem == NULL) {
        hal_err("NULL semaphore");
        return;
    }

    Semaphore *sem_ = static_cast<Semaphore *>(sem);
    osStatus rc = sem_->release();
    if (rc != osOK) {
        hal_err("Semaphore::release() failed: %d", rc);
    }
}

int HAL_SemaphoreWait(_IN_ void *sem, _IN_ uint32_t timeout_ms)
{
    if (sem == NULL) {
        hal_err("NULL semaphore");
        return -1;
    }

    Semaphore *sem_ = static_cast<Semaphore *>(sem);
    /* Semaphore::wait returns number of available tokens, before taking one; 
     * or -1 in case of incorrect parameters */
    int32_t rc = sem_->try_acquire_for((timeout_ms == ((uint32_t) PLATFORM_WAIT_INFINITE)) ? osWaitForever : timeout_ms);
    if (rc < 1) {
        hal_err("Semaphore::wait(%d) failed: %d", timeout_ms, rc);
        return -1;
    } else {
        return 0;
    }
}

int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used)
{
    /* Default arguments for constructing Thread */
    osPriority priority = osPriorityNormal;
    uint32_t stack_size = OS_STACK_SIZE;
    unsigned char *stack_mem = NULL;
    const char *name = NULL;

    /* Specify arguments for constructing Thread */
    if (hal_os_thread_param) {
        /* Thread priority */
        switch (hal_os_thread_param->priority) {
            case os_thread_priority_idle:
                priority = osPriorityIdle;
                break;

            case os_thread_priority_low:
                priority = osPriorityLow;
                break;

            case os_thread_priority_belowNormal:
                priority = osPriorityBelowNormal;
                break;

            case os_thread_priority_normal:
                priority = osPriorityNormal;
                break;

            case os_thread_priority_aboveNormal:
                priority = osPriorityAboveNormal;
                break;

            case os_thread_priority_high:
                priority = osPriorityHigh;
                break;

            case os_thread_priority_realtime:
                priority = osPriorityRealtime;
                break;
                
            default:
                priority = osPriorityNormal;
        }
        
        /* Thread stack address/size */
        if (hal_os_thread_param->stack_size) {
            stack_size = hal_os_thread_param->stack_size;

            if (hal_os_thread_param->stack_addr) {
                stack_mem = static_cast<unsigned char *>(hal_os_thread_param->stack_addr);                
            }
        }

        /* Thread state detached or not */
        if (hal_os_thread_param->detach_state) {
            hal_warning("HAL_ThreadCreate() with detach state not supported");
        }

        /* Thread name */
        name = hal_os_thread_param->name;
    }

    osStatus status = osOK;

    /* Construct Thread */
    Thread *the = new Thread(priority, stack_size, stack_mem, name);
    if (!the) {
        hal_err("Create Thread failed");
        return -1;
    }

    /* Enter do-while one loop with support for catching exception case
     *
     * Run 'return' to exit from the function without handling exception case
     * Run 'break' to exit from the loop to handle exception case
     */
    do {

        /* Thread::start() */
        status = the->start(callback(reinterpret_cast<void (*)(void *)>(work_routine), arg));
        if (status != osOK) {
            hal_err("Thread::start(%p, %p) failed", work_routine, arg);
            break;
        }

        MBED_ASSERT(thread_handle);
        *thread_handle = static_cast<void *>(the);

        /* FIXME: 'stack_used' is not documented clearly
        *
        * Assume that 'stack_used' indicates if specified stack is used. */
        if (stack_used) {
            *stack_used = stack_mem ? 1 : 0;
        }

        return 0;

    /* Exit do-while one-loop */
    } while (0);

    /* Handle exception case from here */

    /* Destruct Thread */
    delete the;

    return -1;
}

/* Implement Aliyun IoT OTA HAL with mbed-bootloader UCP
 *
 * On Mbed OS, Arm has designed mbed-bootloader which is general-purpose enough for
 * OTA application. To make use of mbed-bootloader, we must save downloaded firmware
 * to storage in a mbed-bootloader compatible format. This is what mbed-bootloader UCP
 * does. Notes on using mbed-bootloader UCP to implement Aliyun IoT OTA HAL:
 *
 * 1. Enable real HAL implementation only when mbed-bootloader UCP is enabled
 * 2. IoT OTA HAL I/F doesn't pass along firmware size/version which are required by
 *    mbed-bootloader UCP. We fetch them by calling internal OTA functions in device SDK.
 * 3. Tracing device SDK IoT code, firmware size/version are not ready on the
 *    HAL_Firmware_Persistence_Start(...) call but are on the HAL_Firmware_Persistence_Write(...)
 *    call. We implement most logic there.
 * 4. To be compatible with mbed-bootloader, firmware version must be UNIX timestamp
 *    and in a format like "1576222734" acquired by running 'date +%s' in POSIX-like
 *    environment.
 */

#if COMPONENT_MBEDBL_UCP
extern "C" {
    int dm_ota_get_ota_handle(void **handle);
}

/* Flags for mbed-bootloader compatible UCP layer */
#define MBEDBL_UCP_WRITE_PREPARED   (1 << 0)      // UCP write prepared
#define MBEDBL_UCP_WRITE_FAILED     (1 << 1)      // UCP write failed

static uint16_t mbedbl_status = 0;
static mbedbl_ucp_wrtctx_t ucp_ctx;

void HAL_Firmware_Persistence_Start(void)
{
    mbedbl_status = 0;
}

int HAL_Firmware_Persistence_Write(char *buffer, uint32_t length)
{
    /* Prepare UCP write for the first write */
    if (!(mbedbl_status & MBEDBL_UCP_WRITE_PREPARED)) {
        /* Get OTA handle */
        void *ota_handle = NULL;
        int res = dm_ota_get_ota_handle(&ota_handle);
        if (res != SUCCESS_RETURN || !ota_handle) {
            hal_err("dm_ota_get_ota_handle(...) failed with %d, ota_handle=0x%08x", res, ota_handle);
            return -1;
        }

        /* Fetch firmware size/version required by mbed-bootloader UCP */
        int firmware_size = 0;
        char firmware_version[IOTX_FIRMWARE_VER_LEN + 1] = { 0 };
        if (IOT_OTA_Ioctl(ota_handle, IOT_OTAG_FILE_SIZE, &firmware_size, 4) != 0) {
            hal_err("IOT_OTA_Ioctl(IOT_OTAG_FILE_SIZE) failed");
            return -1;
        }
        if (IOT_OTA_Ioctl(ota_handle, IOT_OTAG_VERSION, firmware_version, sizeof(firmware_version)) != 0) {
            hal_err("IOT_OTA_Ioctl(IOT_OTAG_VERSION) failed");
            return -1;
        }
        hal_info("Firmware size: %d bytes, version: %s", firmware_size, firmware_version);

        /* Prepare UCP write */
        if (MBEDBL_UCP_PrepareWrite(&ucp_ctx, 0, firmware_version, firmware_size)) {
            hal_err("MBEDBL_UCP_PrepareWrite(...) failed");
            return -1;
        }
        /* Mark we have prepared UCP write */
        mbedbl_status |= MBEDBL_UCP_WRITE_PREPARED;
    }

    /* UCP write */
    if (MBEDBL_UCP_Write(&ucp_ctx, (uint8_t *) buffer, &length) != 0) {
        mbedbl_status |= MBEDBL_UCP_WRITE_FAILED;
        hal_err("MBEDBL_UCP_Write(...) failed");
        return -1;
    }

    return 0;
}

int HAL_Firmware_Persistence_Stop(void)
{
    /* Have prepared UCP write? */
    if (!(mbedbl_status & MBEDBL_UCP_WRITE_PREPARED)) {
        hal_err("UCP write not prepared");
        return -1;
    }

    /* No UCP write failed? */
    if (mbedbl_status & MBEDBL_UCP_WRITE_FAILED) {
        hal_err("UCP write failed ever, so not finalize UCP write");
        return -1;
    }

    /* Finalize UCP write
     *
     * This commits firmware update to storage. We must address it very carefully to avoid
     * updating to corrupted firmware.
     */
    if (MBEDBL_UCP_FinalizeWrite(&ucp_ctx) != 0) {
        hal_err("MBEDBL_UCP_FinalizeWrite(...) failed");
        return -1;
    }

    return 0;
}

#else

void HAL_Firmware_Persistence_Start(void)
{
	return;
}

int HAL_Firmware_Persistence_Write(char *buffer, uint32_t length)
{
	return -1;
}

int HAL_Firmware_Persistence_Stop(void)
{
	return -1;
}

#endif

void *HAL_MutexCreate(void)
{
	Mutex *mutex_ = new Mutex;
    if (!mutex_) {
        hal_err("Create mutex failed");
        return NULL;
    }

    return static_cast<void *>(mutex_);
}

void HAL_MutexDestroy(void *mutex)
{
	if (mutex == NULL) {
        hal_err("NULL mutex");
        return;
    }

    Mutex *mutex_ = static_cast<Mutex *>(mutex);
    delete mutex_;
}

void HAL_MutexLock(void *mutex)
{
	if (mutex == NULL) {
        hal_err("NULL mutex");
        return;
    }

    Mutex *mutex_ = static_cast<Mutex *>(mutex);
    mutex_->lock();
}

void HAL_MutexUnlock(void *mutex)
{
	if (mutex == NULL) {
        hal_err("NULL mutex");
        return;
    }

    Mutex *mutex_ = static_cast<Mutex *>(mutex);
    mutex_->unlock();
}
