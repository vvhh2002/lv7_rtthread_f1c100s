#ifndef __ADC_APP_H
#define __ADC_APP_H

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

typedef struct adc_app {
	uint8_t start;
}APD_APP_T;

 
lv_obj_t * app_adc_create(lv_obj_t *parent, void *user_data);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif