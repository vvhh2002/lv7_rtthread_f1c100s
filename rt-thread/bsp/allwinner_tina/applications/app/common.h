#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <time.h>

#include "../lvgl/lvgl.h"
//#define   VS2017_SIM

LV_IMG_DECLARE(BackgroundImage)
LV_IMG_DECLARE(ic_adc)
LV_IMG_DECLARE(ic_backlight)
LV_IMG_DECLARE(ic_beep)
LV_IMG_DECLARE(ic_calc)
LV_IMG_DECLARE(ic_camera)
LV_IMG_DECLARE(ic_car)
LV_IMG_DECLARE(ic_clock)
LV_IMG_DECLARE(ic_ebook)
LV_IMG_DECLARE(ic_file)
LV_IMG_DECLARE(ic_game)
LV_IMG_DECLARE(ic_gyroscope)
LV_IMG_DECLARE(ic_key)
LV_IMG_DECLARE(ic_light)
LV_IMG_DECLARE(ic_music)
LV_IMG_DECLARE(ic_photos)
LV_IMG_DECLARE(ic_record)
LV_IMG_DECLARE(ic_setting)
LV_IMG_DECLARE(ic_temp)
LV_IMG_DECLARE(ic_video)
LV_IMG_DECLARE(ic_voltage)
LV_IMG_DECLARE(ic_weather)
LV_IMG_DECLARE(ic_webview)



/* ICON描述信息结构体 */
typedef struct
{
	lv_img_dsc_t	*icon_img;
	const char		*text_en;
	const char		*text_cn;
	lv_obj_t*		(*app)(lv_obj_t *parent, void *user_data);

}icon_item;

/* 保存icon的对象的链表结构 */
typedef struct icon_obj
{
	lv_obj_t *obj;
	struct icon_obj *next;
}_icon_obj_list;


//时间结构体
typedef struct rtc_date_time{
	//24小时制
	uint8_t Year;		//年份，最小00   最大99  时间范围  2000-2099
	uint8_t Month;
	uint8_t Date;
	uint8_t Hours;
	uint8_t Minutes;
	uint8_t Seconds;
}RTC_DATE_TIME_T;

/* 消息头定义 */
/* 请跟BSP的同类结构体保持一致 */
enum {
	GUI_MQ_TEMP = 0,		   	 /* 温度 */
	GUI_MQ_WLAN_STATUS,          /* WIFI状态更新 */
	GUI_MQ_KEY,                  /* 按键信息 */
	GUI_MQ_WLAN_UPDATA_SET,      /* 更新WIFI设置 */
	GUI_MQ_MPU_UPDATA,           /* MPU6050三轴数据更新 */
};

/* 宏定义 -----------------------------------------------------------*/


#define MQ_MAX_LEN 128


uint8_t common_load_img_bin_from_file(lv_img_dsc_t *image, const char *file_name);
uint8_t common_message_recv(uint8_t *pbuffer, uint8_t len);
void common_get_time(RTC_DATE_TIME_T *date_time);
lv_obj_t * common_app_int(lv_obj_t *parent, const char *title);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif