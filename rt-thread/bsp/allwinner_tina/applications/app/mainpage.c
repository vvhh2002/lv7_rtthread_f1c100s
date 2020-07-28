#include "mainpage.h"
#include "adc_app.h"
#include "time_app.h"
#include "air_app.h"
#include "media_app.h"
#include "messagebox.h"

/* 桌面的ICON信息定义 */
static lv_obj_t * _main_page_app_null(lv_obj_t *parent, void *user_data);

static const icon_item icon_desktop_page1[] =
{
	{ &ic_adc,		"weather",	"天气", app_adc_create },
	{ &ic_backlight,"TIME",		"时间",	app_time_create },
	{ &ic_beep,		"AIR",		"空调",	app_air_create },
	{ &ic_calc,		"KEY",		"蜂鸣", app_media_create },
	{ &ic_camera,	"MEDIA",	"音频",	_main_page_app_null },
	{ &ic_car,		"CALENDAR", "日历", _main_page_app_null },
	{ &ic_clock,	"MEDIA",	"音频",	_main_page_app_null },
	{ &ic_ebook,	"CALENDAR", "日历", _main_page_app_null },

};



/* 桌面的ICON信息定义 */
static const icon_item icon_desktop_page2[] =
{
	{ &ic_file,		"weather",	"天气", _main_page_app_null },
	{ &ic_game,		"TIME",		"时间",	_main_page_app_null },
	{ &ic_gyroscope,"AIR",		"空调",	_main_page_app_null },
	{ &ic_key,		"KEY",		"蜂鸣", _main_page_app_null },
	{ &ic_light,	"MEDIA",	"音频",	_main_page_app_null },
	{ &ic_music,	"CALENDAR", "日历", _main_page_app_null },
	{ &ic_photos,	"MEDIA",	"音频",	_main_page_app_null },
	{ &ic_record,	"CALENDAR", "日历", _main_page_app_null },

};



/* 桌面的ICON信息定义 */
static const icon_item icon_desktop_page3[] =
{
	{ &ic_setting,	"Setting",	"天气", _main_page_app_null },
	{ &ic_temp,		"TIME",		"时间",	_main_page_app_null },
	{ &ic_video,	"AIR",		"空调",	_main_page_app_null },
	{ &ic_voltage,	"KEY",		"蜂鸣", _main_page_app_null },
	{ &ic_weather,	"MEDIA",	"音频",	_main_page_app_null },
	{ &ic_webview,	"CALENDAR", "日历", _main_page_app_null },

};


static _icon_obj_list *_g_icon_obj_list = NULL;/* 链表表头定义 */


/**
* @brief APP空函数
* @param user_data-自定义数据
* @retval	None
*/
static lv_obj_t * _main_page_app_null(lv_obj_t *parent, void *user_data)
{

	(void)user_data;
	(void)parent;
 
	messagebox_create("app is null \r\n");

	return NULL;
}

/**
* @brief 追加list
* @param obj-list对象
* @retval	None
* @note 在链表结尾添加
*/
void icon_obj_list_append(lv_obj_t *obj)
{
	if (_g_icon_obj_list == NULL) {
		_g_icon_obj_list = (_icon_obj_list*)lv_mem_alloc(sizeof(_icon_obj_list));
		_g_icon_obj_list->next = NULL;
		_g_icon_obj_list->obj = obj;
	}
	else {
		_icon_obj_list *p = _g_icon_obj_list;
		_icon_obj_list *node;
		node = (_icon_obj_list*)lv_mem_alloc(sizeof(_icon_obj_list));
		while (p) {
			if (p->next == NULL) {
				p->next = node;
				node->next = NULL;
				node->obj = obj;
			}
			p = p->next;
		}
	}
}

static void _main_page_msg_hander(MAIN_PAGE_T *p_main_page, uint8_t *buffer)
{
	switch (buffer[0]) {
	case GUI_MQ_KEY:
	//printf("key:%d",buffer[1]);
	if (p_main_page->p_app != NULL) {
		lv_event_send(p_main_page->p_app, LV_EVENT_KEY, (const void *)buffer[1]);/* 向当前打开的app 发送按键值 */
	}

	break;
	
	default:
	break;
	}
}
static void _main_page_update_task_cb(lv_task_t *t)
{
	static uint8_t mq_rec_bufffer[MQ_MAX_LEN] = { 0 };
	if (common_message_recv(mq_rec_bufffer, MQ_MAX_LEN) == 0) { /* 接收消息 */
		_main_page_msg_hander((MAIN_PAGE_T*)t->user_data,mq_rec_bufffer);					/* 处理消息 */
	}
}
/**
* @brief 创建ICON
* @param parent-父对象
* @param item-保存ICON信息的数组
* @param icon_num-数量
* @param icon_event_cb-事件回调函数
* @retval	None
* @note 所有ICON公用一个事件回调函数
*/
static void _main_page_create_icon_desktop(lv_obj_t *parent, icon_item *item, uint16_t icon_num, lv_event_cb_t icon_event_cb,void *p_pars)
{

	uint16_t i = 0;
	uint16_t x_pos = 0, y_pos = 0;

	static lv_style_t style_label;

	x_pos = 32;
	y_pos = 40;
	uint16_t x_pad		 = 100;
	uint16_t icon_width  = 80;
	uint16_t y_pad       = 50;
	uint16_t icon_height = 80;
	static lv_style_t style;
	lv_style_init(&style);

	for (i = 0; i < icon_num; ++i)
	{
		x_pos = ((i % 4) * (icon_width+80)) + x_pad;
		y_pos = ((i / 4) * (icon_height+80)) + y_pad;

		lv_obj_t *btn = lv_btn_create(parent, NULL);
		lv_obj_t *image = lv_img_create(btn, NULL);
		lv_obj_t *label = lv_label_create(btn, NULL);
		btn->user_data = (void*)p_pars;
		lv_obj_set_event_cb(btn, icon_event_cb);
		icon_obj_list_append(btn);

		lv_obj_set_pos(btn, x_pos, y_pos);
		lv_obj_set_size(btn, 100, 120);
		lv_obj_set_drag_parent(btn, true); // 使能允许拖拽btn的时候带动父对象

		lv_style_set_image_recolor_opa(&style,	LV_STATE_PRESSED, LV_OPA_30);
		lv_style_set_text_color(&style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
		lv_style_set_image_recolor(&style,		LV_STATE_PRESSED, LV_COLOR_BLACK);
		lv_style_set_radius(&style,				LV_STATE_DEFAULT, 5);
		lv_style_set_border_width(&style,		LV_STATE_DEFAULT, 0);
		lv_style_set_bg_opa(&style,				LV_STATE_DEFAULT, 0);

		lv_obj_add_style(btn, LV_IMGBTN_PART_MAIN, &style);

		lv_img_set_src(image, item[i].icon_img);

		lv_label_set_text(label, item[i].text_en);
		lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

	}

}



/**
* @brief ICON的事件回调函数
* @param obj-触发事件的对象
* @param event-事件类型
* @retval	None
* @note 根据对象找链表中的索引,根据索引执行APP
*/

static void icon_obj_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) /* 如果没有拖动且释放了就触发（无论是否长按）*/
	{
		uint16_t index = 0;
		_icon_obj_list *p = _g_icon_obj_list;
		if (obj->user_data==NULL) {
			return;
		}
		MAIN_PAGE_T *p_main_page = (MAIN_PAGE_T *)(obj->user_data);

		while (p){
			if (p->obj == obj){
				break;
			}
			index++;
			p = p->next;
		}
 
		uint8_t icon_num_page1, icon_num_page2;
		char *p_app_name =NULL;
		icon_num_page1 = sizeof(icon_desktop_page1) / sizeof(icon_desktop_page1[0]);
		icon_num_page2 = sizeof(icon_desktop_page2) / sizeof(icon_desktop_page2[0]);
		if (index < icon_num_page1) {
			p_main_page->p_app = icon_desktop_page1[index].app(lv_scr_act(), NULL);	/* 第一个页面 */
			p_app_name = icon_desktop_page1[index].text_en;
		} else if ((index < icon_num_page1 + icon_num_page2)) {
			p_main_page->p_app = icon_desktop_page2[index - icon_num_page1].app(lv_scr_act(), NULL);/* 第二个页面 */
			p_app_name = icon_desktop_page2[index - icon_num_page1].text_en;
		} else {
			p_main_page->p_app = icon_desktop_page3[index - icon_num_page1 - icon_num_page2].app(lv_scr_act(), NULL);//printf("page3\n");
			p_app_name = icon_desktop_page3[index - icon_num_page1 - icon_num_page2].text_en;
		}

		printf("app open :%s\r\n", p_app_name);
	
	}
}

MAIN_PAGE_T * main_page_create(void)
{  
	MAIN_PAGE_T *p_main_page = lv_mem_alloc(sizeof(MAIN_PAGE_T));

	/*
	 *  设置桌面背景
	 */
	lv_obj_t *p_img_bg	= lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(p_img_bg, &BackgroundImage);
	lv_obj_align(p_img_bg, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

	/*
	 *  初始化滑屏切换界面
	 */
	p_main_page->p_tv = lv_tabview_create(lv_scr_act(), NULL);
	lv_obj_set_pos(p_main_page->p_tv,0, 32);

	
	//lv_obj_set_style_local_bg_opa(p_main_page->p_tv,LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT,);

	lv_tabview_set_btns_pos(p_main_page->p_tv, LV_TABVIEW_TAB_POS_NONE);						// 不显示tab栏
	lv_obj_set_style_local_bg_opa(p_main_page->p_tv, LV_TABVIEW_PART_BG, LV_STATE_DEFAULT, 0);	 
 
	for (int i = 0; i < 3; i++) {
		lv_tabview_add_tab(p_main_page->p_tv, "");												// 创建三个页面
	}
	/*
	 *  创建三个页面的内容
	 */
	_main_page_create_icon_desktop(lv_tabview_get_tab(p_main_page->p_tv,0), (icon_item*)icon_desktop_page1, sizeof(icon_desktop_page1) / sizeof(icon_desktop_page1[0]), icon_obj_event_cb, (void*)p_main_page);
	_main_page_create_icon_desktop(lv_tabview_get_tab(p_main_page->p_tv,1), (icon_item*)icon_desktop_page2, sizeof(icon_desktop_page2) / sizeof(icon_desktop_page2[0]), icon_obj_event_cb, (void*)p_main_page);
	_main_page_create_icon_desktop(lv_tabview_get_tab(p_main_page->p_tv,2), (icon_item*)icon_desktop_page3, sizeof(icon_desktop_page3) / sizeof(icon_desktop_page3[0]), icon_obj_event_cb, (void*)p_main_page);

	p_main_page->p_title_page = title_page_create(lv_scr_act());

	
	p_main_page->p_task=lv_task_create(_main_page_update_task_cb, 10, LV_TASK_PRIO_LOW, (void *)p_main_page);
	
	lv_task_ready(p_main_page->p_task);
	


	return p_main_page;
}

void main_page_delete(MAIN_PAGE_T *p_main_page)
{
	//lv_obj_del();

}



#include <rtthread.h> 
#include <rtdevice.h>  

static void main_page_run(void *p)
{
    main_page_create();

    #ifdef  RT_USING_WDT
    rt_uint16_t wdt_tick=0;
    rt_device_t wdg_dev=rt_device_find("wdt");
    #endif
    while (1){
        //rt_thread_delay(RT_TICK_PER_SECOND / 100);
		rt_thread_mdelay(10);
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
