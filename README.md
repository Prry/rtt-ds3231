# DS3231 驱动软件包


<br>


## 1 介绍

ds3231软件包是针对RT-Thread驱动框架实现的实时时钟驱动，遵循RT-Thread RTC框架，可以从芯片内置RTC无缝切换使用外置高精度ds3231 RTC。软件包支持实时时钟和闹钟功能。


### 1.1 目录结构

| 名称     | 说明       |
| -------- | ---------- |
| ds3231.h  | 头文件|
| ds3231.c  |源文件 |
| README.md  | 软件包使用说明|
|ds3231_datasheet.pdf| 芯片中文手册|
|SConscript|RT-Thread默认构建脚本|
|LICENSE|许可证文件|



### 1.2 许可证

ds3231 软件包遵循 Apache license v2.0 许可，详见 `LICENSE` 文件。


### 1.3 依赖
- RT-Thread 3.0+
- RT-Thread I2C设备驱动框架
- RT-Thread RTC设备驱动框架

<br>

## 2 获取 ds3231软件包

使用 ds3231 package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages --->
    peripheral libraries and drivers --->
        [*] extren rtc drivers  --->
            [*] ds3231：External RTC drivers fo ds3231
                 Version (latest)  --->
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

<br>

## 3 使用 ds3231 软件包

### 3.1 初始化
首先需初始化ds3231驱动，可以手动在初始化线程调用“rt_hw_ds3231_init()”初始化，也可以直接使用“INIT_DEVICE_EXPORT”自动初始化。

> 注意：如果开启了芯片内部RTC，需先禁止内部RTC


### 3.2 使用方式
**获取当前时间**

```
time_t time(time_t *t)
time_t now;      
now = time(RT_NULL);
```

**设置时间**

```
 #define RT_DEVICE_CTRL_RTC_SET_TIME     0x11        /**< set time */
 rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)；
```

**获取闹钟时间**

```
 #define RT_DEVICE_CTRL_RTC_GET_ALARM    0x12        /**< get alarm */
 rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)；
```

**设置闹钟时间**

```
 #define RT_DEVICE_CTRL_RTC_SET_ALARM    0x13      /**< set alarm */
 rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)；
```

**提供一个获取内部温度接口**
```
float ds3231_get_temperature(void)；
```

详细用法可以参考“/components/drivers/rtc.c”和“/components/drivers/alarm.c”源码。


### 3.3 msh/finsh测试

```
/*msh获取时间*/
msh >date
Sun Mar  1 12:01:13 2020
........
/*msh设置时间*/
msh >date 2020 03 01 20 00 00 
msh >date
Sun Mar  1 20:00:03 2020
........
/*finsh获取时间*/
finsh >list_date()
Sun Mar  1 20:17:13 2020
........
/*finsh设置时间*/
finsh >set_date(2020,3,2)
        0, 0x00000000
finsh >set_time(12,0,0)  
        0, 0x00000000
finsh >list_date()
Mon Mar  2 12:00:04 2020
........
/*msh打印温度*/
msh >list_ds31_temp
ds3231 temperature: [29.0C]
........
/*finsh打印温度*/
finsh >list_ds31_temp()
ds3231 temperature: [29.0C] 
```


<br>

## 4 注意事项

使用RT-Thread的RTC框架，RTC设备注册名称为“rtc”，注意需先屏蔽内置芯片RTC驱动。

<br>

## 5 联系方式

- 维护：[Acuity](https://github.com/Prry)
- 主页：<https://github.com/Prry/rtt-ds3231>       




