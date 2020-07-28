#include "common.h"
#include <stdio.h>
#include <stdlib.h>

/**
* @brief 从文件系统获取bmp图片
* @param image-存储图片信息的指针
* @param file_name-文件名
* @retval	0-加载成功 其它-错误代码
*/
uint8_t common_load_img_bin_from_file(lv_img_dsc_t *image, const char *file_name)
{
#if defined (VS2017_SIM)
	uint8_t res;
	uint32_t br;
	uint8_t *pbuf;		//读取数据缓冲区
	uint8_t *pbmp;
	uint32_t i;
	uint32_t header;
	int file_size;
	uint8_t fname[256];

	if (image == NULL || file_name == NULL)
		return 1;

	sprintf(fname, "./app/%s", file_name + 2);

	printf("fname:%s\n", fname);

	FILE *fp = NULL;

	fp = fopen(fname, "rb");
	if (fp == NULL) {
		printf("open file failed!\r\n");
		return 1;
	}

	//获取文件长度
	fseek(fp, 0, SEEK_END);		/* 定位到文件末 */
	file_size = ftell(fp);		/* 文件长度 */

	fseek(fp, 0, SEEK_SET);		/* 恢复到文件头 */

	if (file_size > 1 * 1024 * 1024) {
		/* 文件过大了 */
		fclose(fp);
		return 2;
	}


	pbuf = lv_mem_alloc(file_size);
	if (pbuf == NULL)
		return 1;

	//printf("file_size:%d\n", file_size);

	if (fread(pbuf, 1, file_size, fp) < file_size) {
		printf("read file failed!\r\n");
		fclose(fp);
		return 3;
	}

	//printf("file:%x", pbuf[0]);

	/* 获取头 */
	header = (uint32_t)pbuf[3] << 24;
	header |= (uint32_t)pbuf[2] << 16;
	header |= (uint32_t)pbuf[1] << 8;
	header |= (uint32_t)pbuf[0];

	//printf("header:0X%X\r\n", header);
	//printf("width:%d height:%d\r\n", (uint16_t)(header >> 10), header >> 21);


	/* 获取图片数据流 */
	pbmp = (uint8_t *)pbuf + 4;

	image->header.cf = pbuf[0];
	image->header.always_zero = 0;
	image->header.w = (uint16_t)(header >> 10);
	image->header.h = (uint16_t)(header >> 21);
	image->data_size = file_size - 4;		//去掉4字节的头,剩余的就是图片数据流长度
	image->data = pbmp;


	fclose(fp);

	return 0;
#else
	// uint8_t res;
	// uint32_t br;
	// uint8_t *pbuf;		//读取数据缓冲区
	// uint8_t *pbmp;
	// uint32_t i;
	// uint32_t header;
	// uint32_t file_size;

	// if (image == NULL)
	// 	return 1;
	// if (file_name == NULL)
	// 	return 1;

	// res = f_open(&SDFile, (const TCHAR*)file_name, FA_OPEN_EXISTING | FA_READ);
	// if (res != FR_OK) {
	// 	return 1;
	// }

	// file_size = SDFile.obj.objsize;

	// if (file_size > 1 * 1024 * 1024) {
	// 	/* 文件过大了 */
	// 	f_close(&SDFile);
	// 	return 2;
	// }

	// /* 为图片申请内存,如果图片需要一直显示,内存不用释放 */
	// pbuf = lv_mem_alloc(file_size);
	// if (pbuf == NULL)
	// 	return 1;

	// res = f_read(&SDFile, pbuf, file_size, &br);
	// if ((res != FR_OK) || (br == 0)) {
	// 	f_close(&SDFile);
	// 	return 3;
	// }


	// /* 获取头 */
	// header = (uint32_t)pbuf[3] << 24;
	// header |= (uint32_t)pbuf[2] << 16;
	// header |= (uint32_t)pbuf[1] << 8;
	// header |= (uint32_t)pbuf[0];

	// //rt_kprintf("header:0X%X\r\n",header);
	// //rt_kprintf("width:%d height:%d\r\n", (uint16_t)(header >> 10), header >> 21);


	// /* 获取图片数据流 */
	// pbmp = (uint8_t *)pbuf + 4;

	// image->header.cf = pbuf[0];
	// image->header.always_zero = 0;
	// image->header.w = (uint16_t)(header >> 10);
	// image->header.h = (uint16_t)(header >> 21);
	// image->data_size = file_size - 4;		//去掉4字节的头,剩余的就是图片数据流长度
	// image->data = pbmp;



	// f_close(&SDFile);

	return 0;

	

#endif

}


/**
* @brief 获取RTOS的消息队列的数据
* @param pBuffer-数据存储区
* @param len-需要获取的长度
* @retval	0-有消息 1-无消息
*/
uint8_t common_message_recv(uint8_t *pbuffer, uint8_t len)
{
#if defined ( VS2017_SIM   )
	static uint8_t mq_buf[MQ_MAX_LEN] = { 0 };
	mq_buf[0] = GUI_MQ_WLAN_STATUS;
	mq_buf[1] = 1;
	memcpy(pbuffer, mq_buf, MQ_MAX_LEN);
	return 0;
#else
	// return rt_mq_recv(gui_mq, pbuffer, MQ_MAX_LEN, RT_WAITING_NO); /* 这里更新消息，查询的方式 RT_WAITING_NO */
	return 0;
#endif
}

/**
* @brief 获取硬件RTC时间
* @param None
* @retval	时间结构体
* @note WIN32获取的是真实时间
*/
void common_get_time(RTC_DATE_TIME_T *date_time)
{
#if defined ( VS2017_SIM   )

	/* WIN32端获取的是真实时间 */
	RTC_DATE_TIME_T current_time;

	time_t nowtime;
	nowtime = time(NULL); //获取日历时间  
	struct tm *local;
	local = localtime(&nowtime);  //获取当前系统时间  

	date_time->Year = (local->tm_year - 100); //tm_year是从1900开始的 Yeay是从2000年开始的
	date_time->Month = local->tm_mon + 1;
	date_time->Date = local->tm_mday;	//这里读到的日期比实际大1天，原因未知
	date_time->Hours = local->tm_hour;
	date_time->Minutes = local->tm_min;
	date_time->Seconds = local->tm_sec;


#else
//	static RTC_DateTypeDef current_date = { 0 };
//	static RTC_TimeTypeDef current_time = { 0 };

//	rtc_current(&current_date, &current_time);

	// RTC_DATE_TIME_T current;
	// date_time->Year = current_date.Year;
	// date_time->Month = current_date.Month;
	// date_time->Date = current_date.Date;
	// date_time->Hours = current_time.Hours;
	// date_time->Minutes = current_time.Minutes;
	// date_time->Seconds = current_time.Seconds;

	


#endif
}



/**
* @brief APP公用初始化
* @param parent-父对象
* @param title-APP标题
* @retval	创建的APP的对象
*/
lv_obj_t * common_app_int(lv_obj_t *parent, const char *title)
{
	lv_obj_t *obj = lv_obj_create(parent, NULL);
	lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);

	//lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 50);	    // 设置透明
	lv_obj_set_style_local_border_width(obj, LV_PAGE_PART_BG, LV_STATE_DEFAULT, 0);	// 设置边框线宽为0
	lv_obj_set_style_local_radius(obj, LV_PAGE_PART_BG, LV_STATE_DEFAULT, 0);			// 去除圆角

	/*
	 *  设置app默认背景
	 */
	lv_obj_t *p_img_bg = lv_img_create(obj, NULL);
	lv_img_set_src(p_img_bg, &BackgroundImage);
	lv_obj_align(p_img_bg, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

	/*
	 *  标题
	 */
	lv_obj_t *p_label_title = lv_label_create(obj, NULL);
	lv_label_set_text(p_label_title, title);
	lv_obj_align(p_label_title, obj, LV_ALIGN_IN_TOP_MID, 0, 16);


	return obj;
}