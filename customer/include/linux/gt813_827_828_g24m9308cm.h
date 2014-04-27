/* drivers/input/touchscreen/gt813_827_828.h
 * 
 * 2010 - 2012 Goodix Technology.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be a reference 
 * to you, when you are integrating the GOODiX's CTP IC into your system, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 * 
 * Version:1.2
 *        V1.0:2012/05/01,create file.
 *        V1.2:2012/06/08,add some macro define.
 *
 */

#ifndef _LINUX_GOODIX_TOUCH_H
#define	_LINUX_GOODIX_TOUCH_H

#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <mach/gpio_data.h>
#include <linux/earlysuspend.h>

struct goodix_i2c_rmi_platform_data {
    uint32_t version;	/* Use this entry for panels with */
    unsigned gpio_pwr;
    unsigned gpio_rst;
    unsigned gpio_irq;
    bool irq_edge; /* 0:rising edge, 1:falling edge */
    bool swap_xy;
    bool xpol;
    bool ypol;
    int xmax;
    int ymax; 
//    int config_info_len;
//    u8 *config_info;
};

struct goodix_ts_data {
    spinlock_t irq_lock;
    struct i2c_client *client;
    struct input_dev  *input_dev;
    struct hrtimer timer;
    struct work_struct  work;
    struct early_suspend early_suspend;
    struct goodix_i2c_rmi_platform_data *pdata;
    s32 irq_is_disable;
    s32 use_irq;
    u16 abs_x_max;
    u16 abs_y_max;
    u8  max_touch_num;
    u8  int_trigger_type;
    u8  green_wake_mode;
    u8  chip_type;
    u8  enter_update;
    u8  gtp_is_suspend;
};
extern u16 show_len;
extern u16 total_len;

//***************************PART1:ON/OFF define*******************************
#define GTP_CUSTOM_CFG        1
#define GTP_DRIVER_SEND_CFG   1
#define GTP_HAVE_TOUCH_KEY    0
#define GTP_POWER_CTRL_SLEEP  0
#define GTP_AUTO_UPDATE       0
#define GTP_CHANGE_X2Y        1
#define GTP_ESD_PROTECT       0
#define GTP_CREATE_WR_NODE    0
#define GTP_ICS_SLOT_REPORT   1

#define GUP_USE_HEADER_FILE   0
#if GUP_USE_HEADER_FILE
    #define GT828       //Choose you IC type.GT813,GT827,GT828
#endif

#define GTP_DEBUG_ON          0
#define GTP_DEBUG_ARRAY_ON    0
#define GTP_DEBUG_FUNC_ON     0

//***************************PART2:TODO define**********************************
//STEP_1(REQUIRED):Change config table.
/*TODO: puts the config info corresponded to your TP here, the following is just 
a sample config, send this config should cause the chip cannot work normally*/
//default or float
//#define CTP_CFG_GROUP1 {0x1D,0x0E,0x1C,0x0D,0x1B,0x0C,0x1A,0x0B,0x19,0x0A,0x18,0x09,0x17,0x08,0x16,0x07,0x15,0x06,0x14,0x05,0x13,0x04,0x12,0x03,0x11,0x02,0xFF,0x02,0x01,0x00,0x00,0x0A,0x01,0x0B,0x02,0x0C,0x03,0x0D,0x04,0x0E,0x05,0x0F,0x06,0x10,0x07,0x11,0x08,0x12,0x09,0xFF,0x1F,0x03,0x50,0x10,0x10,0x12,0x00,0x00,0x04,0x00,0x00,0x02,0x50,0x30,0x35,0x03,0x00,0x05,0x00,0x02,0x58,0x03,0x20,0x5B,0x5E,0x55,0x58,0x05,0x00,0x08,0x20,0x00,0x10,0x10,0x03,0xAC,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}
//following is used by 8316
#define CTP_CFG_GROUP1   {0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,0x06,0x15,0x07,0x16,0x08,0x17,0x09,0x18,0x0A,0x19,0x0B,0x1A,0xFF,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x13,0x09,0x12,0x08,0x11,0x07,0x10,0x06,0x0F,0x05,0x0E,0x04,0x0D,0x03,0xFF,0x0E,0x0F,0x10,0x11,0x12,0x09,0x03,0x88,0x88,0x88,0x2A,0x00,0x00,0x08,0x00,0x00,0x02,0x3C,0x28,0x1C,0x03,0x00,0x05,0x00,0x03,0x00,0x04,0x00,0x33,0x38,0x30,0x35,0x25,0x00,0x25,0x19,0x05,0x14,0x10,0x02,0x30,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}
//following is used by 7316
#define CTP_CFG_GROUP11     {0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,0x06,0x15,0x07,0x16,0x08,0x17,0x09,0x18,0x0A,0x19,0x0B,0x1A,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0x0D,0x04,0x0E,0x05,0x0F,0x06,0x10,0x07,0x11,0x08,0x12,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x4F,0x03,0x80,0x88,0x88,0x1F,0x00,0x00,0x05,0x00,0x00,0x0E,0x50,0x37,0x50,0x03,0x00,0x05,0x00,0x02,0x58,0x04,0x00,0x3C,0x3B,0x3F,0x3D,0x25,0x00,0x05,0x19,0x05,0x14,0x10,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}
//following is used by 9308
#define CTP_CFG_GROUP_9308 {0x00,0x0F,0x01,0x10,0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,0x06,0x15,0x07,0x16,0x08,0x17,0x09,0x18,0x0A,0x19,0x0B,0x1A,0x0C,0x1B,0x0D,0x1C,0xFF,0x1D,0x13,0x09,0x12,0x08,0x11,0x07,0x10,0x06,0x0F,0x05,0x0E,0x04,0x0D,0x03,0x0C,0x02,0xFF,0xFF,0xFF,0xFF,0x1F,0x03,0x48,0x10,0x10,0x29,0x00,0x00,0x06,0x00,0x00,0x02,0x50,0x30,0x55,0x03,0x00,0x05,0x00,0x01,0xE0,0x03,0x20,0x49,0x49,0x44,0x44,0x28,0x00,0x25,0x19,0x02,0x14,0x10,0x02,0xDA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}

//TODO puts your group2 config info here,if need.
//VDDIO
#define CTP_CFG_GROUP2 {\
    }
//TODO puts your group3 config info here,if need.
//GND
#define CTP_CFG_GROUP3 {\
    }

//STEP_2(REQUIRED):Change I/O define & I/O operation mode.
#define GTP_RST_PORT    PAD_GPIOC_3
#define GTP_INT_PORT    PAD_GPIOA_16
#define GTP_INT_IRQ     INT_GPIO_0
#define GTP_INT_CFG     GPIO_IRQ(GTP_INT_IRQ-INT_GPIO_0, GPIO_IRQ_FALLING) 


#define GTP_GPIO_AS_INPUT(pin)          do{\
                                            gpio_set_status(pin,gpio_status_in);\
                                        }while(0)
#define GTP_GPIO_AS_OUTPUT(pin)          do{\
                                            gpio_set_status(pin,gpio_status_out);\
                                        }while(0)
#define GTP_GPIO_AS_INT(pin)            do{\
                                            gpio_set_status(pin,gpio_status_in);\
                                            gpio_irq_set(pin, GTP_INT_CFG);\
                                        }while(0)
#define GTP_GPIO_GET_VALUE(pin)         gpio_get_status(pin)
#define GTP_GPIO_OUTPUT(pin,level)      gpio_out(pin,level)
#define GTP_GPIO_REQUEST(pin, label)    gpio_request(pin, label)
#define GTP_GPIO_FREE(pin)              gpio_free(pin)
#define GTP_IRQ_TAB                     {IRQ_TYPE_EDGE_FALLING,IRQ_TYPE_EDGE_RISING}

//STEP_3(optional):Custom set some config by themself,if need.
#if GTP_CUSTOM_CFG
  #define GTP_MAX_HEIGHT   1024		
  #define GTP_MAX_WIDTH    768
  #define GTP_INT_TRIGGER  0    //0:Falling 1:Rising
#else
  #define GTP_MAX_HEIGHT   4096
  #define GTP_MAX_WIDTH    4096
  #define GTP_INT_TRIGGER  1
#endif
#define GTP_MAX_TOUCH      5
#define GTP_ESD_CHECK_CIRCLE  2000

//STEP_4(optional):If this project have touch key,Set touch key config.                                    
#if GTP_HAVE_TOUCH_KEY
    #define GTP_KEY_TAB	 {KEY_MENU, KEY_HOME, KEY_SEND}
#endif

//***************************PART3:OTHER define*********************************
#define GTP_DRIVER_VERSION    "V1.2<2012/06/08>"
#define GTP_I2C_NAME          "Goodix-TS"
#define GTP_I2C_ADDR 	  	  0x5D
#define GTP_POLL_TIME	      10
#define GTP_ADDR_LENGTH       2
#define GTP_CONFIG_LENGTH     112
#define FAIL                  0
#define SUCCESS               1

//Register define
#define GTP_READ_COOR_ADDR    0x0F40
#define GTP_REG_SLEEP         0x0FF2
#define GTP_REG_SENSOR_ID     0x0FF5
#define GTP_REG_CONFIG_DATA   0x0F80
#define GTP_REG_VERSION       0x0F7D

#define RESOLUTION_LOC        71
#define TRIGGER_LOC           66

//Log define
#define GTP_INFO(fmt,arg...)           printk("<<-GTP-INFO->>[%d]"fmt"\n", __LINE__, ##arg)
#define GTP_ERROR(fmt,arg...)          printk("<<-GTP-ERROR->>[%d]"fmt"\n", __LINE__, ##arg)
#define GTP_DEBUG(fmt,arg...)          do{\
                                         if(GTP_DEBUG_ON)\
                                         printk("<<-GTP-DEBUG->>[%d]"fmt"\n",__LINE__, ##arg);\
                                       }while(0)
#define GTP_DEBUG_ARRAY(array, num)    do{\
                                         s32 i;\
                                         u8* a = array;\
                                         if(GTP_DEBUG_ARRAY_ON)\
                                         {\
                                            printk("<<-GTP-DEBUG-ARRAY->>\n");\
                                            for (i = 0; i < (num); i++)\
                                            {\
                                                printk("%02x   ", (a)[i]);\
                                                if ((i + 1 ) %10 == 0)\
                                                {\
                                                    printk("\n");\
                                                }\
                                            }\
                                            printk("\n");\
                                        }\
                                       }while(0)
#define GTP_DEBUG_FUNC()               do{\
                                         if(GTP_DEBUG_FUNC_ON)\
                                         printk("<<-GTP-FUNC->>[%d]Func:%s\n", __LINE__, __func__);\
                                       }while(0)
#define GTP_SWAP(x, y)                 do{\
                                         typeof(x) z = x;\
                                         x = y;\
                                         y = z;\
                                       }while (0)

//****************************PART4:UPDATE define*******************************
#define PACK_SIZE            64                    //update file package size
#define SEARCH_FILE_TIMES    50
#define UPDATE_FILE_PATH_2   "/data/goodix/_goodix_update_.bin"
#define UPDATE_FILE_PATH_1   "/sdcard/goodix/_goodix_update_.bin"

//Error no
#define ERROR_NO_FILE           2   //ENOENT
#define ERROR_FILE_READ         23  //ENFILE
#define ERROR_FILE_TYPE         21  //EISDIR
#define ERROR_GPIO_REQUEST      4   //EINTR
#define ERROR_I2C_TRANSFER      5   //EIO
#define ERROR_NO_RESPONSE       16  //EBUSY
#define ERROR_TIMEOUT           110 //ETIMEDOUT

//*****************************End of Part III********************************

#endif /* _LINUX_GOODIX_TOUCH_H */
