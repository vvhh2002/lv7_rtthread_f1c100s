#include "time_app.h"
 
static void visuals_create(lv_obj_t * parent)
{
	static lv_style_t style_box;

	lv_style_init(&style_box);
	lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT); /* 设置文本位置 */
	lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(10));           /* 设置文本y轴偏移*/
	lv_style_set_value_color(&style_box, LV_STATE_DEFAULT, LV_COLOR_WHITE);        /*字体颜色为白色*/
	lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(10));             /* 设置box距离上面的偏移*/
	lv_style_set_bg_opa(&style_box, LV_STATE_DEFAULT, 0);						   /* 设置box背景透明度为0 */


	lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);

	lv_coord_t grid_h_chart = lv_obj_get_height_grid(parent, 1, 1);
	lv_coord_t grid_w_chart = lv_obj_get_width_grid(parent, 2, 1);
	printf("grid_h_chart= %d grid_w_chart= %d \n", grid_h_chart, grid_w_chart);

	lv_obj_t * chart = lv_chart_create(parent, NULL);
	lv_obj_align(chart, NULL, LV_ALIGN_IN_TOP_LEFT,50, 100);
	lv_obj_add_style(chart, LV_CHART_PART_BG, &style_box);

	lv_obj_set_drag_parent(chart, true); // 使能子对象拖拽功能

	lv_obj_set_style_local_value_str(chart, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Line chart");// 设置标题
	lv_obj_set_width_margin(chart, grid_w_chart-50);	// 设置宽度和高度，默认减去边界
	lv_obj_set_height_margin(chart, grid_h_chart-100);
	lv_chart_set_div_line_count(chart, 3, 0);		// 设置X轴和y轴的分割线数量
	lv_chart_set_point_count(chart, 8);				// 设置数据线的点数
	lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   // 设置类型

	lv_obj_set_style_local_pad_left(chart,   LV_CHART_PART_BG, LV_STATE_DEFAULT, 4 * (LV_DPI / 10));
	lv_obj_set_style_local_pad_bottom(chart, LV_CHART_PART_BG, LV_STATE_DEFAULT, 3 * (LV_DPI / 10));
	lv_obj_set_style_local_pad_right(chart,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 2 * (LV_DPI / 10));
	lv_obj_set_style_local_pad_top(chart,    LV_CHART_PART_BG, LV_STATE_DEFAULT, 2 * (LV_DPI / 10));
	lv_chart_set_y_tick_length(chart, 0, 0);
	lv_chart_set_x_tick_length(chart, 0, 0);
	lv_chart_set_y_tick_texts(chart, "600\n500\n400\n300\n200", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_x_tick_texts(chart, "Jan\nFeb\nMar\nApr\nMay\nJun\nJul\nAug", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
	
	lv_chart_series_t * s1 = lv_chart_add_series(chart, LV_THEME_DEFAULT_COLOR_PRIMARY);
	lv_chart_series_t * s2 = lv_chart_add_series(chart, LV_THEME_DEFAULT_COLOR_SECONDARY);

	lv_chart_set_next(chart, s1, 10);
	lv_chart_set_next(chart, s1, 90);
	lv_chart_set_next(chart, s1, 30);
	lv_chart_set_next(chart, s1, 60);
	lv_chart_set_next(chart, s1, 10);
	lv_chart_set_next(chart, s1, 90);
	lv_chart_set_next(chart, s1, 30);
	lv_chart_set_next(chart, s1, 60);
	lv_chart_set_next(chart, s1, 10);
	lv_chart_set_next(chart, s1, 90);

	lv_chart_set_next(chart, s2, 32);
	lv_chart_set_next(chart, s2, 66);
	lv_chart_set_next(chart, s2, 5);
	lv_chart_set_next(chart, s2, 47);
	lv_chart_set_next(chart, s2, 32);
	lv_chart_set_next(chart, s2, 32);
	lv_chart_set_next(chart, s2, 66);
	lv_chart_set_next(chart, s2, 5);
	lv_chart_set_next(chart, s2, 47);
	lv_chart_set_next(chart, s2, 66);
	lv_chart_set_next(chart, s2, 5);
	lv_chart_set_next(chart, s2, 47);










	lv_obj_t * chart2 = lv_chart_create(parent, chart);
	lv_obj_align(chart, NULL, LV_ALIGN_IN_TOP_RIGHT, -50, 100);
	lv_chart_set_type(chart2, LV_CHART_TYPE_COLUMN);

	s1 = lv_chart_add_series(chart2, LV_THEME_DEFAULT_COLOR_PRIMARY);
	s2 = lv_chart_add_series(chart2, LV_THEME_DEFAULT_COLOR_SECONDARY);

	lv_chart_set_next(chart2, s1, 10);
	lv_chart_set_next(chart2, s1, 90);
	lv_chart_set_next(chart2, s1, 30);
	lv_chart_set_next(chart2, s1, 60);
	lv_chart_set_next(chart2, s1, 10);
	lv_chart_set_next(chart2, s1, 90);
	lv_chart_set_next(chart2, s1, 30);
	lv_chart_set_next(chart2, s1, 60);
	lv_chart_set_next(chart2, s1, 10);
	lv_chart_set_next(chart2, s1, 90);

	lv_chart_set_next(chart2, s2, 32);
	lv_chart_set_next(chart2, s2, 66);
	lv_chart_set_next(chart2, s2, 5);
	lv_chart_set_next(chart2, s2, 47);
	lv_chart_set_next(chart2, s2, 32);
	lv_chart_set_next(chart2, s2, 32);
	lv_chart_set_next(chart2, s2, 66);
	lv_chart_set_next(chart2, s2, 5);
	lv_chart_set_next(chart2, s2, 47);
	lv_chart_set_next(chart2, s2, 66);
	lv_chart_set_next(chart2, s2, 5);
	lv_chart_set_next(chart2, s2, 47);
 
}


lv_obj_t * app_time_create(lv_obj_t *parent, void *user_data)
{

	lv_obj_t *obj = common_app_int(lv_scr_act(), "TIME APP");
 
	visuals_create(obj);



	return obj;
}
