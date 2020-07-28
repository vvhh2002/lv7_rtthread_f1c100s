#include "media_app.h"
 

static void media_create(lv_obj_t * parent)
{
	static lv_style_t style_box;

	lv_style_init(&style_box);
	lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_IN_TOP_MID); /* 设置文本位置 */
	lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(30));           /* 设置文本y轴偏移*/
	lv_style_set_value_color(&style_box, LV_STATE_DEFAULT, LV_COLOR_WHITE);        /*字体颜色为白色*/
	//lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(10));             /* 设置box距离上面的偏移*/
	lv_style_set_bg_opa(&style_box, LV_STATE_DEFAULT, 0);						   /* 设置box背景透明度为0 */

 

	lv_obj_t * led_h = lv_cont_create(parent, NULL);
	lv_obj_align(led_h, parent, LV_ALIGN_IN_TOP_MID, -150, 150);
	lv_cont_set_layout(led_h, LV_LAYOUT_PRETTY_MID);
 
    lv_obj_set_width(led_h, 400);
	lv_obj_set_height(led_h, 150);

	lv_obj_add_style(led_h, LV_CONT_PART_MAIN, &style_box);
	lv_obj_set_style_local_value_str(led_h, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "LEDs");

	lv_obj_t * led = lv_led_create(led_h, NULL);
	lv_coord_t led_size = lv_obj_get_height_fit(led_h);
	lv_obj_set_size(led, led_size, led_size);
	lv_led_off(led);

	led = lv_led_create(led_h, led);
	lv_led_set_bright(led, (LV_LED_BRIGHT_MAX - LV_LED_BRIGHT_MIN) / 2 + LV_LED_BRIGHT_MIN);

	led = lv_led_create(led_h, led);
	lv_led_on(led);
 
}


lv_obj_t * app_media_create(lv_obj_t *parent, void *user_data)
{

	lv_obj_t *obj = common_app_int(lv_scr_act(), "TIME APP");
 
	media_create(obj);



	return obj;
}
