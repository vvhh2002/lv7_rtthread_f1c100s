#ifndef __AIR_APP_H
#define __AIR_APP_H

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

typedef struct air_app {
	uint8_t start;
}AIR_APP_T;

 
lv_obj_t * app_air_create(lv_obj_t *parent, void *user_data);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif