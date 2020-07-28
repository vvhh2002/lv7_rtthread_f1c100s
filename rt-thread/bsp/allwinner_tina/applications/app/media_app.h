#ifndef __MEDIA_APP_H
#define __MEDIA_APP_H

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

typedef struct media_app {
	uint8_t start;
}MEDIA_APP_T;

 
lv_obj_t * app_media_create(lv_obj_t *parent, void *user_data);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif