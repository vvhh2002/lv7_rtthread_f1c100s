#include "adc_app.h"
 

 
static void slider_event_cb(lv_obj_t * slider, lv_event_t e)
{
	if (e == LV_EVENT_VALUE_CHANGED) {
		if (lv_slider_get_type(slider) == LV_SLIDER_TYPE_NORMAL) {
			static char buf[16];
			lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
			lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, buf);
		} else {
			static char buf[32];
			lv_snprintf(buf, sizeof(buf), "%d-%d", lv_slider_get_left_value(slider), lv_slider_get_value(slider));
			lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, buf);
		}

	}

}


static void controls_create(lv_obj_t * parent)
{
	
	lv_disp_size_t disp_size	= lv_disp_get_size_category(NULL);
	lv_coord_t grid_w			= lv_obj_get_width_grid(parent, 2, 1);// 划分两列，返回一列的宽度
	static lv_style_t style_box;

	lv_style_init(&style_box);
	lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT); /* 设置文本位置 */
	lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(10));           /* 设置文本y轴偏移*/
	lv_style_set_value_color(&style_box, LV_STATE_DEFAULT, LV_COLOR_WHITE);        /*字体颜色为白色*/
	lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(10));             /* 设置box距离上面的偏移*/
	lv_style_set_bg_opa(&style_box, LV_STATE_DEFAULT, 0);						   /* 设置box背景透明度为0 */
	           

	lv_obj_t * h = lv_cont_create(parent, NULL);			// 创建容器
	lv_obj_align(h, NULL, LV_ALIGN_CENTER,-100,-100);
	
	lv_cont_set_layout(h, LV_LAYOUT_PRETTY_MID);			// 设置容器布局
	lv_obj_add_style(h, LV_CONT_PART_MAIN, &style_box);		// 设置容器风格
	 
    //lv_obj_set_style_local_bg_opa(h, LV_CONT_PART_MAIN,LV_STATE_DEFAULT,0);// 容器北京透明
	lv_obj_set_style_local_value_str(h, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Basics");// 容器标题

	lv_cont_set_fit2(h, LV_FIT_NONE, LV_FIT_TIGHT);			// 设置容器对齐策略，包裹子对象
	lv_obj_set_width(h, grid_w);							// 设置容器宽度

	/*
	 *  在容器Basics里面创建两个按钮
	 */
	lv_obj_t * btn = lv_btn_create(h, NULL);
	lv_btn_set_fit2(btn, LV_FIT_NONE, LV_FIT_TIGHT); /* 这句话没有必要感觉*/
	lv_obj_set_width(btn, lv_obj_get_width_grid(h, 2, 1)); //把容器分成两列，返回每列的宽度作为按钮的宽度
	lv_obj_t * label = lv_label_create(btn, NULL);
	lv_label_set_text(label, "Button");

	btn = lv_btn_create(h, btn);
	lv_btn_toggle(btn);
	label = lv_label_create(btn, NULL);
	lv_label_set_text(label, "Button");


	/*
	 *  在容器Basics里面创建 个sw开关和checkbox复选框
	 */
	lv_switch_create(h, NULL);

	lv_checkbox_create(h, NULL);

	/*
	 *  在容器Basics里面创建 个滑动条slider
	 */
	lv_coord_t fit_w = lv_obj_get_width_fit(h); /* 返回容器的宽度，减去左右边界了*/

	lv_obj_t * slider = lv_slider_create(h, NULL);
	lv_slider_set_value(slider, 40, LV_ANIM_OFF);  //设置单项滑动条初试值40，关闭动画
	lv_obj_set_event_cb(slider, slider_event_cb);
	lv_obj_set_width_margin(slider, fit_w);        // 调整间距 实际上slider的宽度obj_w = fit_w - 默认margin_left - 默认margin_right

	/*Use the knobs style value the display the current value in focused state*/
	lv_obj_set_style_local_margin_top(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(10)); // 距离上面的位置
	lv_obj_set_style_local_value_font(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
	lv_obj_set_style_local_value_ofs_y(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25)); // 设置聚焦时候数值的位置，这里设置向上偏移25
	lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);  // 设置默认情况透明，即不显示数值
	lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);   // 设置聚焦时候不透明，显示数值
	lv_obj_set_style_local_transition_time(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);      // 设置弹出数值的时间
	lv_obj_set_style_local_transition_prop_5(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
	lv_obj_set_style_local_transition_prop_6(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

	slider = lv_slider_create(h, slider);
	lv_slider_set_type(slider, LV_SLIDER_TYPE_RANGE);
	lv_slider_set_value(slider, 70, LV_ANIM_OFF);
	lv_slider_set_left_value(slider, 30, LV_ANIM_OFF);
	lv_obj_set_style_local_value_ofs_y(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, -LV_DPX(25));
	lv_obj_set_style_local_value_font(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_theme_get_font_small());
	lv_obj_set_style_local_value_opa(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_event_cb(slider, slider_event_cb);
	lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);      /*To refresh the text*/
	if (lv_obj_get_width(slider) > fit_w) lv_obj_set_width(slider, fit_w);

}

 

lv_obj_t * app_adc_create(lv_obj_t *parent, void *user_data)
{


	lv_obj_t *obj = common_app_int(lv_scr_act(), "ADC APP");
 

	controls_create(obj);



	return obj;
}
