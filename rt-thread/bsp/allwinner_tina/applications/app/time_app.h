#ifndef __TIME_APP_H
#define __TIME_APP_H

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

typedef struct time_app {
	uint8_t start;
}TIME_APP_T;

 
lv_obj_t * app_time_create(lv_obj_t *parent, void *user_data);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif