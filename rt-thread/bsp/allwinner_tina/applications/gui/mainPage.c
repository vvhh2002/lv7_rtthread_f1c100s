#include <rtthread.h> 
#include <rtdevice.h>  

#include "../../littlevGL7.1.0/lv_conf.h"
#include "../../littlevGL7.1.0/lvgl/src/lvgl.h"
#include "../../littlevGL7.1.0/lvgl/src/lvgl.h"
//#include "../lvgl/src/lv_themes/lv_theme_default.h"

//extern const lv_img_t wallpaper1;
//extern const lv_img_t wallpaper2;
void main_page_demo(void)
{
lv_demo_widgets();

}


static void main_page_run(void *p)
{
    main_page_demo();

    #ifdef  RT_USING_WDT
    rt_uint16_t wdt_tick=0;
    rt_device_t wdg_dev=rt_device_find("wdt");
    #endif
    while (1){
        rt_thread_delay(RT_TICK_PER_SECOND / 100);
        lv_task_handler();
        #ifdef  RT_USING_WDT
        if(wdt_tick++>300){
            rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
            wdt_tick=0;        }
        #endif


        
    }
}

int main_page_init(void)
{
    rt_err_t ret       = RT_EOK;
    rt_thread_t thread = RT_NULL;

    /* init littlevGL */
    ret = lv_rtt_port_init("lcd");
    if (ret != RT_EOK){
        return ret;
    }

    /* littleGL demo gui thread */
    thread = rt_thread_create("lv_demo", main_page_run, RT_NULL, 4096, 5, 10);
    if (thread == RT_NULL){
        return RT_ERROR;
    }
    rt_thread_startup(thread);

    return RT_EOK;
}
INIT_APP_EXPORT(main_page_init);