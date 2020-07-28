#include "air_app.h"
 
static void linemeter_anim(lv_obj_t * linemeter, lv_anim_value_t value)
{
	lv_linemeter_set_value(linemeter, value);

	static char buf[64];
	lv_snprintf(buf, sizeof(buf), "%d", value);
	lv_obj_t * label = lv_obj_get_child(linemeter, NULL);
	lv_label_set_text(label, buf);
	lv_obj_align(label, linemeter, LV_ALIGN_CENTER, 0, 0);
}
static void gauge_anim(lv_obj_t * gauge, lv_anim_value_t value)
{
	lv_gauge_set_value(gauge, 0, value);

	static char buf[64];
	lv_snprintf(buf, sizeof(buf), "%d", value);
	lv_obj_t * label = lv_obj_get_child(gauge, NULL);
	lv_label_set_text(label, buf);
	lv_obj_align(label, gauge, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_y(label));
}
static void arc_anim(lv_obj_t * arc, lv_anim_value_t value)
{
	lv_arc_set_end_angle(arc, value);

	static char buf[64];
	lv_snprintf(buf, sizeof(buf), "%d", value);
	lv_obj_t * label = lv_obj_get_child(arc, NULL);
	lv_label_set_text(label, buf);
	lv_obj_align(label, arc, LV_ALIGN_CENTER, 0, 0);

}

static void bar_anim(lv_task_t * t)
{
	static uint32_t x = 0;
	lv_obj_t * bar = t->user_data;

	static char buf[64];
	lv_snprintf(buf, sizeof(buf), "Copying %d/%d", x, lv_bar_get_max_value(bar));
	lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf);

	lv_bar_set_value(bar, x, LV_ANIM_OFF);
	x++;
	if (x > lv_bar_get_max_value(bar)) x = 0;
}
static void air_create(lv_obj_t * parent)
{
	static lv_style_t style_box;

	lv_style_init(&style_box);
	lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_IN_TOP_MID); /* 设置文本位置 */
	lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(30));           /* 设置文本y轴偏移*/
	lv_style_set_value_color(&style_box, LV_STATE_DEFAULT, LV_COLOR_WHITE);        /*字体颜色为白色*/
	lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(10));             /* 设置box距离上面的偏移*/
	lv_style_set_bg_opa(&style_box, LV_STATE_DEFAULT, 0);						   /* 设置box背景透明度为0 */



 
	 lv_coord_t grid_w_meter = lv_obj_get_width_grid(parent, 3, 1);

	lv_coord_t meter_h	  = lv_obj_get_height_fit(parent);
	lv_coord_t meter_size = LV_MATH_MIN(grid_w_meter, meter_h);


	/*
	 *  第一个 仪表盘Line meter
	 */
	lv_obj_t * lmeter = lv_linemeter_create(parent, NULL);
	lv_obj_align(lmeter,NULL, LV_ALIGN_IN_TOP_LEFT,10,100);
	 
	lv_linemeter_set_value(lmeter, 50);
	lv_obj_set_size(lmeter, meter_size, meter_size);
	lv_obj_add_style(lmeter, LV_LINEMETER_PART_MAIN, &style_box);
	lv_obj_set_style_local_value_str(lmeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, "Line meter");
 
	//数值大小
	lv_obj_t * label = lv_label_create(lmeter, NULL);
	lv_obj_align(label, lmeter, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());

	//动画
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, lmeter);
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)linemeter_anim);
	lv_anim_set_values(&a, 0, 100);
	lv_anim_set_time(&a, 4000);
	lv_anim_set_playback_time(&a, 1000);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);





	/*
	 *  第二个 仪表盘Gauge
	 */

	lv_obj_t * gauge = lv_gauge_create(parent, NULL);
	lv_obj_align(gauge, lmeter, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
	lv_obj_set_drag_parent(gauge, true);
	lv_obj_set_size(gauge, meter_size, meter_size);
	lv_obj_add_style(gauge, LV_GAUGE_PART_MAIN, &style_box);
	lv_obj_set_style_local_value_str(gauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, "Gauge");

	label = lv_label_create(gauge, label);
	lv_obj_align(label, gauge, LV_ALIGN_CENTER, 0, grid_w_meter / 3);

	lv_anim_set_var(&a, gauge);
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)gauge_anim);
	lv_anim_start(&a);



	/*
	 *  第三个个 仪表盘Arc
	 */

	lv_obj_t * arc = lv_arc_create(parent, NULL);
	lv_obj_align(arc, gauge, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
	lv_obj_set_drag_parent(arc, true);
	lv_arc_set_bg_angles(arc, 0, 360);
	lv_arc_set_rotation(arc, 270);
	lv_arc_set_angles(arc, 0, 0);
	lv_obj_set_size(arc, meter_size, meter_size);
	lv_obj_add_style(arc, LV_ARC_PART_BG, &style_box);
	lv_obj_set_style_local_value_str(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, "Arc");

	label = lv_label_create(arc, label);
	lv_obj_align(label, arc, LV_ALIGN_CENTER, 0, 0);

	lv_anim_set_var(&a, arc);
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)arc_anim);
	lv_anim_set_values(&a, 0, 360);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);

 




	///*Create a bar and use the backgrounds value style property to display the current value*/
	//lv_obj_t * bar_h = lv_cont_create(parent, NULL);
	//lv_obj_align(bar_h, arc, LV_ALIGN_OUT_TOP_RIGHT, 50, 0);
	//lv_cont_set_fit2(bar_h, LV_FIT_NONE, LV_FIT_TIGHT);
	//lv_obj_add_style(bar_h, LV_CONT_PART_MAIN, &style_box);
	//lv_obj_set_style_local_value_str(bar_h, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Bar");

 //
	// lv_obj_set_width(bar_h, lv_obj_get_width_grid(parent, 3, 2));

	//lv_obj_t * bar = lv_bar_create(bar_h, NULL);
	//lv_obj_set_width(bar, lv_obj_get_width_fit(bar_h));
	//lv_obj_set_style_local_value_font(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_theme_get_font_small());
	//lv_obj_set_style_local_value_align(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	//lv_obj_set_style_local_value_ofs_y(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 20);
	//lv_obj_set_style_local_margin_bottom(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 7);
	//lv_obj_align(bar, NULL, LV_ALIGN_CENTER, 0, 0);




	//lv_obj_t * led_h = lv_cont_create(parent, NULL);
	//lv_cont_set_layout(led_h, LV_LAYOUT_PRETTY_MID);
 //
 //  lv_obj_set_width(led_h, lv_obj_get_width_grid(parent, 3, 1));

	//lv_obj_set_height(led_h, lv_obj_get_height(bar_h));
	//lv_obj_add_style(led_h, LV_CONT_PART_MAIN, &style_box);
	//lv_obj_set_drag_parent(led_h, true);
	//lv_obj_set_style_local_value_str(led_h, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "LEDs");

	//lv_obj_t * led = lv_led_create(led_h, NULL);
	//lv_coord_t led_size = lv_obj_get_height_fit(led_h);
	//lv_obj_set_size(led, led_size, led_size);
	//lv_led_off(led);

	//led = lv_led_create(led_h, led);
	//lv_led_set_bright(led, (LV_LED_BRIGHT_MAX - LV_LED_BRIGHT_MIN) / 2 + LV_LED_BRIGHT_MIN);

	//led = lv_led_create(led_h, led);
	//lv_led_on(led);

	//if (disp_size == LV_DISP_SIZE_MEDIUM) {
	//	lv_obj_add_protect(led_h, LV_PROTECT_POS);
	//	lv_obj_align(led_h, bar_h, LV_ALIGN_OUT_BOTTOM_MID, 0, lv_obj_get_style_margin_top(led_h, LV_OBJ_PART_MAIN) + lv_obj_get_style_pad_inner(parent, LV_PAGE_PART_SCROLLABLE));
	//}

	/*lv_task_create(bar_anim, 100, LV_TASK_PRIO_LOW, bar);*/
}


lv_obj_t * app_air_create(lv_obj_t *parent, void *user_data)
{

	lv_obj_t *obj = common_app_int(lv_scr_act(), "TIME APP");
 
	air_create(obj);



	return obj;
}
