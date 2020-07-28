#ifndef __TITLE_PAGE_H
#define __TITLE_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

typedef struct title_page {
	lv_obj_t *p_cont;

	lv_obj_t *p_label_home;
	lv_obj_t *p_label_time;
	lv_obj_t *p_label_signal;
	lv_obj_t *p_label_ele;

	uint8_t  time_buf[6];
	lv_task_t * p_task;
} TITLE_PAGE_T;

TITLE_PAGE_T * title_page_create(lv_obj_t *par);

void title_page_updata_time(TITLE_PAGE_T *p_title, RTC_DATE_TIME_T *p_rtc_time);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif