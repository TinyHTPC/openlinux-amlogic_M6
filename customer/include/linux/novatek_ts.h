#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>

//#include <linux/51boardpcb.h>
//*************************TouchScreen Work Part*****************************



#define NOVATEK_I2C_NAME "novatek-ts"
#define	NOVATEK_I2C_SCL	200*1000
/*******************Step 1: define resolution *****************/
//define default resolution of the touchscreen
#define TOUCH_MAX_HEIGHT 		600			
#define TOUCH_MAX_WIDTH			1024
#define MAX_FINGER_NUM	10
#define READ_COOR_ADDR	0x00
//define default resolution of LCM
#define SCREEN_MAX_HEIGHT   LCD_SCREEN_HIGHT//600
#define SCREEN_MAX_WIDTH    LCD_SCREEN_WIDTH//1024
/**********************Step 2: Setting Interrupt******************************/
#define BABBAGE_NOVATEK_TS_RST1   (1*32 + 7)
#define BABBAGE_NOVATEK_TS_INT1   (1*32 + 6)

#define INT_PORT BABBAGE_NOVATEK_TS_INT1
#define TS_INT gpio_to_irq(INT_PORT)
/******************Step 3: Setting Reset option**************************/
#define Novatek_HWRST_LowLeval()    (gpio_set_value(BABBAGE_NOVATEK_TS_RST1, 0))
#define Nvoatek_HWRST_HighLeval()   (gpio_set_value(BABBAGE_NOVATEK_TS_RST1, 1);)

/*********************************************************************/

//*************************Touchkey Surpport Part*****************************
//#define HAVE_TOUCH_KEY
#ifdef HAVE_TOUCH_KEY
	#define READ_COOR_ADDR 0x00
	const uint16_t touch_key_array[]={
									  KEY_MENU,				//MENU
									  KEY_HOME,				//HOME
									  KEY_BACK				//BACK
									  
									 }; 
	#define MAX_KEY_NUM	 (sizeof(touch_key_array)/sizeof(touch_key_array[0]))
#else
	#define READ_COOR_ADDR 0x00
#endif


struct tp_event {
	u16	x;
	u16	y;
    	s16 id;
	u16	pressure;
	u8  touch_point;
	u8  flag;
};
#define DRIVER_SEND_CFG
#define NT11002   0
#define NT11003   1
#define NT11004   2
#define IC_DEFINE   NT11003
#if IC_DEFINE == NT11002
#define IIC_BYTENUM    4
#elif IC_DEFINE == NT11003
#define IIC_BYTENUM    6
#elif IC_DEFINE == NT11004
#define IIC_BYTENUM    2
#endif
//Set Interrupt Type
#define EDGE_INT        0    //must handle andriod 3.0 below 
#define EDGE_TIMER_INT  1    //All 
#define INT_TYPE        EDGE_INT
//set trigger mode
#define INT_TRIGGER		0

//set polling mode timer
#define POLL_TIME		10	//actual query spacing interval:POLL_TIME+6
#define _NOVATEK_CAPACITANCEPANEL_BOOTLOADER_FUNCTION_
#ifdef _NOVATEK_CAPACITANCEPANEL_BOOTLOADER_FUNCTION_
enum
{
  RS_OK         = 0,
  RS_INIT_ER    = 8,
  RS_ERAS_ER    = 9,
  RS_FLCS_ER    = 10,
  RS_WD_ER      = 11
} ;
#endif
struct novatek_ts_data {
	uint8_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	char phys[32];
	int use_irq;
	struct hrtimer timer;
	struct work_struct work;
	uint16_t abs_x_max;
	uint16_t abs_y_max;
	uint8_t max_touch_num;
	uint8_t int_trigger_type;
	struct early_suspend early_suspend;
//	int (*power)(struct rk_ts_data * ts, int on);
	
};

static const char *novatek_ts_name = "Novatek Capacitive TouchScreen";
static struct workqueue_struct *novatek_wq;

//============================================================
//emdoor
struct novatek_i2c_platform_data {
	uint32_t version;	/* Use this entry for panels with */
    int mode;
    int irq;
    int (*init_irq)(void);
    int (*get_irq_level)(void);
    int poll_period;
    bool cache_enable;
    void *data;
    void (*power)(int on);
    //void (*power_off)(void); 
	void (*do_reset)(void);
	void (*platform_sleep)();
};
enum ts_mode {
    TS_MODE_INT_FALLING,
    TS_MODE_INT_RISING,
    TS_MODE_INT_LOW,
    TS_MODE_INT_HIGH,
    TS_MODE_TIMER_LOW,
    TS_MODE_TIMER_HIGH,
    TS_MODE_TIMER_READ,
    TS_MODE_NUM,
};



