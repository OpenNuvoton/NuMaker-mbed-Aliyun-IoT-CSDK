# Port of Alibaba Cloud IoT C-SDK onto Mbed OS

This library is port of [Alibaba Cloud IoT C-SDK](https://github.com/aliyun/iotkit-embedded) onto [Mbed OS](https://github.com/ARMmbed/mbed-os), especially on [Nuvoton's Mbed Enabled boards](https://os.mbed.com/teams/Nuvoton/).

## Version

Based on IoT C-SDK 3.1.0

## Bugfix

1.  Fix memory leak with `wrapper_http_deinit(...)` call:

    https://github.com/aliyun/iotkit-embedded/issues/162

1.  Fix stack overrun (needed 18KiB+ stack) in http2 examples: `http2_example_stream.c`, `http2_example_uploadfile.c`, and `http2_example_break_resume.c`.
    This is done by changing `HTTP2_RECV_BUFFER_LENGHT` to smaller e.g. 2KiB from 16KiB.

## Known issue or limitation

1.  Meet OOM (needed 128KiB+ heap) in http2 examples: `http2_example_stream`, `http2_example_uploadfile`, and `http2_example_break_resume`.
    These examples can run only on e.g. **NUMAKER_PFM_NUC472** target with 1MiB XRAM.
1.  Support protocol: MQTT/Mbed TLS

## Example

-   [Nuvoton's Alibaba Cloud IoT C-SDK simple example](https://github.com/OpenNuvoton/NuMaker-mbed-Aliyun-IoT-CSDK-example)
-   [Nuvoton's Alibaba Cloud IoT C-SDK firmware OTA example](https://github.com/OpenNuvoton/NuMaker-mbed-Aliyun-IoT-CSDK-OTA-example)
