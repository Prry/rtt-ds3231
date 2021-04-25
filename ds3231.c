
/*
 * Copyright (c) 2020 panrui <https://github.com/Prry/rtt-ds3231>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-01     panrui      the first version
 */
 
#include <rtthread.h>
#include <rtdevice.h>
#include "ds3231.h"

#ifdef PKG_USING_DS3231

#define DBG_TAG "ds3231"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define 	DS3231_ARRD			0x68	/* slave address */

#define		REG_SEC				0x00
#define		REG_MIN				0x01
#define		REG_HOUR			0x02
#define		REG_DAY				0x03
#define		REG_WEEK			0x04
#define		REG_MON				0x05
#define		REG_YEAR			0x06
#define 	REG_ALM1_SEC  		0x07
#define 	REG_ALM1_MIN 	  	0x08
#define 	REG_ALM1_HOUR     	0x09
#define 	REG_ALM1_DAY_DATE 	0x0A
#define 	REG_ALM2_MIN  		0x0B
#define 	REG_ALM2_HOUR     	0x0C
#define 	REG_ALM2_DAY_DATE 	0x0D
#define 	REG_CONTROL         0x0E
#define 	REG_STATUS          0x0F
#define 	REG_AGING_OFFSET    0x10
#define 	REG_TEMP_MSB 		0x11
#define 	REG_TEMP_LSB 		0x12

#define	DS3231_I2C_BUS		"i2c1"		/* i2c linked */
#define	DS3231_DEVICE_NAME	"rtc"		/* register device name */

static struct rt_device ds3231_dev;	/* ds3231 device */

static unsigned char bcd_to_hex(unsigned char data)
{
    unsigned char temp;

    temp = ((data>>4)*10 + (data&0x0f));
    return temp;
}

static unsigned char hex_to_bcd(unsigned char data)
{
    unsigned char temp;

    temp = (((data/10)<<4) + (data%10));
    return temp;
}

static rt_err_t  ds3231_read_reg(rt_device_t dev, rt_uint8_t reg,rt_uint8_t *data,rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[2];
	struct rt_i2c_bus_device *i2c_bus = RT_NULL;
	
	RT_ASSERT(dev != RT_NULL);
	 
	i2c_bus = (struct rt_i2c_bus_device*)dev->user_data;
    msg[0].addr  = DS3231_ARRD;
    msg[0].flags = RT_I2C_WR;
    msg[0].len   = 1;
    msg[0].buf   = &reg;
    msg[1].addr  = DS3231_ARRD;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = data_size;
    msg[1].buf   = data;

    if(rt_i2c_transfer(i2c_bus, msg, 2) == 2)
	{
        return RT_EOK;
    }
    else
    {
	  	LOG_E("i2c bus read failed!\r\n");
        return -RT_ERROR;
    }
}

static rt_err_t  ds3231_write_reg(rt_device_t dev, rt_uint8_t reg, rt_uint8_t *data, rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[2];
	struct rt_i2c_bus_device *i2c_bus = RT_NULL;
	
	RT_ASSERT(dev != RT_NULL);
	 
	i2c_bus = (struct rt_i2c_bus_device*)dev->user_data;
    msg[0].addr		= DS3231_ARRD;
    msg[0].flags	= RT_I2C_WR;
    msg[0].len   	= 1;
    msg[0].buf   	= &reg;
    msg[1].addr  	= DS3231_ARRD;
    msg[1].flags	= RT_I2C_WR | RT_I2C_NO_START;
    msg[1].len   	= data_size;
    msg[1].buf   	= data;
    if(rt_i2c_transfer(i2c_bus, msg, 2) == 2)
	{
        return RT_EOK;
    }
    else
    {
	  	LOG_E("i2c bus write failed!\r\n");
        return -RT_ERROR;
    }
}

static rt_err_t rt_ds3231_open(rt_device_t dev, rt_uint16_t flag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* open interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_ds3231_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return RT_EOK;
}

static rt_err_t rt_ds3231_control(rt_device_t dev, int cmd, void *args)
{
	rt_err_t	ret = RT_EOK;
    time_t 		*time;
    struct tm 	time_temp;	
    rt_uint8_t 	buff[7];
	
    RT_ASSERT(dev != RT_NULL);
    rt_memset(&time_temp, 0, sizeof(struct tm));

    switch (cmd)
    {
    	/* read time */
        case RT_DEVICE_CTRL_RTC_GET_TIME:
	        time = (time_t *)args;
	        ret = ds3231_read_reg(dev, REG_SEC,buff,7);
			if(ret == RT_EOK)
			{
				time_temp.tm_year  = bcd_to_hex(buff[6]) + 2000 - 1900;
				time_temp.tm_mon   = bcd_to_hex(buff[5]&0x7f) - 1;
				time_temp.tm_mday  = bcd_to_hex(buff[4]);
				time_temp.tm_hour  = bcd_to_hex(buff[2]);
				time_temp.tm_min   = bcd_to_hex(buff[1]);
				time_temp.tm_sec   = bcd_to_hex(buff[0]);
	        	*time = mktime(&time_temp);
			}
        break;

		/* set time */
        case RT_DEVICE_CTRL_RTC_SET_TIME:
        {
        	struct tm *time_new;
					
            time = (time_t *)args;
            time_new = localtime(time);
            buff[6] = hex_to_bcd(time_new->tm_year + 1900 - 2000);
            buff[5] = hex_to_bcd(time_new->tm_mon + 1);
            buff[4] = hex_to_bcd(time_new->tm_mday);
            buff[3] = hex_to_bcd(time_new->tm_wday+1);
            buff[2] = hex_to_bcd(time_new->tm_hour);
            buff[1] = hex_to_bcd(time_new->tm_min);
            buff[0] = hex_to_bcd(time_new->tm_sec);
            ret = ds3231_write_reg(dev, REG_SEC, buff, 7);
        }
        break;
	#ifdef RT_USING_ALARM
		/* get alarm time */
		case RT_DEVICE_CTRL_RTC_GET_ALARM:
		{ 	
		  	struct rt_rtc_wkalarm *alm_time;
					
		  	ret = ds3231_read_reg(dev, REG_ALM1_SEC, buff, 4);
			if(ret == RT_EOK)
			{
			  	alm_time = (struct rt_rtc_wkalarm *)args;
				alm_time->tm_hour  = bcd_to_hex(buff[2]);
				alm_time->tm_min   = bcd_to_hex(buff[1]);
				alm_time->tm_sec   = bcd_to_hex(buff[0]);
			}
		}
		break;
		
		/* set alarm time */
		case RT_DEVICE_CTRL_RTC_SET_ALARM:
		{
			struct rt_rtc_wkalarm *alm_time;
					
            alm_time = (struct rt_rtc_wkalarm *)args;
            buff[3] = 0x80;	/* enable, alarm when hours, minutes, and seconds match */
            buff[2] = hex_to_bcd(alm_time->tm_hour);
            buff[1] = hex_to_bcd(alm_time->tm_min);
            buff[0] = hex_to_bcd(alm_time->tm_sec);
            ret = ds3231_write_reg(dev, REG_ALM1_SEC, buff, 4);
		}
		break;
	#endif
        default:
        break;
	}
    return ret;
}

float ds3231_get_temperature(void)
{
 	rt_int8_t buff[2];
	float temp = 0.0f;
	
	ds3231_read_reg(&ds3231_dev, REG_TEMP_MSB, (rt_uint8_t*)buff, 2);
	if(buff[0]&0x80)
	{/* negative temperature */
		temp = buff[0];
		temp -= (buff[1]>>6)*0.25;	/* 0.25C resolution */
	}
	else
	{/* positive temperature */
		temp = buff[0];
		temp += ((buff[1]>>6)&0x03)*0.25;
	}	
	
	return temp;
}

int rt_hw_ds3231_init(void)
{		
    struct rt_i2c_bus_device *i2c_device;
    uint8_t data;
	
    i2c_device = rt_i2c_bus_device_find(DS3231_I2C_BUS);
    if (i2c_device == RT_NULL)
    {
        LOG_E("i2c bus device %s not found!\r\n", DS3231_I2C_BUS);
        return -RT_ERROR;
    }				 	

    /* register rtc device */
    ds3231_dev.type   		= RT_Device_Class_RTC;
    ds3231_dev.init    		= RT_NULL;
    ds3231_dev.open    		= rt_ds3231_open;
    ds3231_dev.close   		= RT_NULL;
    ds3231_dev.read   		= rt_ds3231_read;
    ds3231_dev.write  	 	= RT_NULL;
    ds3231_dev.control 		= rt_ds3231_control;
    ds3231_dev.user_data 	= (void*)i2c_device;	/* save i2cbus */;		
    rt_device_register(&ds3231_dev, DS3231_DEVICE_NAME, RT_DEVICE_FLAG_RDWR);
		
    /* init ds3231 */
    data = 0x04;	/* close clock out */
    ds3231_write_reg(&ds3231_dev, REG_CONTROL, &data, 1);
	LOG_D("the rtc of ds3231 init succeed!");
	
    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_ds3231_init);

#ifdef RT_USING_FINSH
#include <finsh.h>

void list_ds31_temp(void)
{
	float temp = 0.0f;
	
	temp = ds3231_get_temperature();
	
	rt_kprintf("ds3231 temperature: [%d.%dC] \n", (int)temp, (int)(temp * 10) % 10);
}
FINSH_FUNCTION_EXPORT(list_ds31_temp, list ds3231 temperature.)
#endif /* RT_USING_FINSH */

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
MSH_CMD_EXPORT(list_ds31_temp, list ds3231 temperature.);
#endif /* RT_USING_FINSH & FINSH_USING_MSH */

#endif /* PKG_USING_DS3231 */
