#include "titlepage.h"

/**
* @brief	：状态栏任务函数
* @param	t：任务指针
* @retval	None
*/
static void _title_page_update_task_cb(lv_task_t *t)
{
    RTC_DATE_TIME_T date_time;

	common_get_time(&date_time); // 得到当前时间
	title_page_updata_time((TITLE_PAGE_T *)t->user_data, &date_time);


}
TITLE_PAGE_T * title_page_create(lv_obj_t *par)
{  
 
	TITLE_PAGE_T *p_title = lv_mem_alloc(sizeof(TITLE_PAGE_T));
 
	p_title->p_cont = lv_cont_create(par, NULL);
	lv_obj_set_size(p_title->p_cont, LV_HOR_RES, 32);
	lv_obj_set_style_local_bg_opa(p_title->p_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 80);	// 设置背景透明度
	lv_obj_set_style_local_border_opa(p_title->p_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);	// 设置边框明度
	lv_obj_set_style_local_radius(p_title->p_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);		// 去除圆角


	 /*
	 *  设置状态栏home
	 */
	p_title->p_label_home = lv_label_create(p_title->p_cont, NULL);
	lv_obj_align(p_title->p_label_home, p_title->p_cont, LV_ALIGN_IN_TOP_LEFT, 5, 5);	// 位置
	lv_obj_set_style_local_text_color(p_title->p_label_home, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);/*设置颜色*/
	lv_label_set_text(p_title->p_label_home, LV_SYMBOL_HOME);

	/*
	*  设置状态栏电池图标
	*/
	p_title->p_label_ele = lv_label_create(p_title->p_cont, p_title->p_label_home);
	lv_obj_set_style_local_text_color(p_title->p_label_ele, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_label_set_text(p_title->p_label_ele, "%10 "LV_SYMBOL_BATTERY_2);
	lv_obj_align(p_title->p_label_ele, p_title->p_cont, LV_ALIGN_IN_TOP_RIGHT, -5, 5); // 设置位置
	/*
	*  设置状态栏信号图标
	*/
	p_title->p_label_signal = lv_label_create(p_title->p_cont, NULL);
	lv_obj_set_style_local_text_color(p_title->p_label_signal, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_label_set_text(p_title->p_label_signal, LV_SYMBOL_WIFI);
	lv_obj_align(p_title->p_label_signal, p_title->p_cont, LV_ALIGN_IN_TOP_RIGHT, -65, 5);


	/*
	 *  时间
	 */
	p_title->p_label_time = lv_label_create(p_title->p_cont, NULL);
	lv_obj_set_style_local_text_color(p_title->p_label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_align(p_title->p_label_time, p_title->p_cont, LV_ALIGN_IN_TOP_MID, 0, 6);


	/*
	 *  更新任务
	 */
	p_title->p_task = lv_task_create(_title_page_update_task_cb, 100, LV_TASK_PRIO_LOW, (void*)p_title);
	lv_task_ready(p_title->p_task);// 下次周期调用回调函数

	return p_title;
}

void title_page_updata_time(TITLE_PAGE_T *p_title, RTC_DATE_TIME_T *p_rtc_time)
{
	sprintf((char*)p_title->time_buf, "%02d:%02d", p_rtc_time->Hours, p_rtc_time->Minutes);
	lv_label_set_text(p_title->p_label_time, (const char*)p_title->time_buf);
}

void title_page_delete(TITLE_PAGE_T *p_title)
{


}