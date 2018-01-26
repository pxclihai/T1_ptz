# stm32f10x RT-Thread Demo

---

## 1、简介

通过 `app\src\app_task.c` 的 `test_env()` 方法来演示环境变量的读取及修改功能，每次系统启动并且初始化EasyFlash成功后会调用该方法。

在 `test_env()` 方法中，会先读取系统的启动次数，读取后对启动次数加一，再存入到环境变量中，实现记录系统启动（开机）次数的功能。

### 1.1、使用方法

- 1、打开电脑的终端与Demo的串口1进行连接，串口配置 115200 8 1 N，此时在终端中就可以输入《EasyFlash 使用说明》中提到的常用命令（目前只有环境变量操作命令）
- 2、在终端输入 `printenv` 查看当前的系统启动次数，即 `boot_times` 的value
- 3、输入 `reboot` 命令，或者直接重启Demo平台
- 4、等待系统重启完成后，再次输入 `printenv` ，即可查看到启动次数有所增加

> 注意：对于无法连接终端的用户，也可以使用仿真器与Demo平台进行连接，来观察启动次数的变化

## 2、文件（夹）说明

`components\easyflash\port\ef_port.c` 移植参考文件

`RVMDK` 下为Keil工程文件

`EWARM` 下为IAR工程文件# T1_ptz
