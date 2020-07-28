#ifndef __MAIN_PAGE_H
#define __MAIN_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

#include "titlepage.h"

typedef struct main_page{

	uint8_t		 cur_page_num;

	lv_obj_t     *p_app;	/* 存储当前APP的obj指针,NULL表示当前在桌面 */
	lv_obj_t     *p_tv;
	lv_task_t	 *p_task;
	TITLE_PAGE_T *p_title_page;


} MAIN_PAGE_T;

MAIN_PAGE_T * main_page_create(void);


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif