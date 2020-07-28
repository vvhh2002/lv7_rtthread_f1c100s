 #include "messagebox.h"


 
static const char *_g_p_btns[] = { "Cancel", "Ok", "" };
static lv_obj_t  *_g_p_msgbox = NULL;
static lv_style_t _g_style_modal;

static void _msgbox_event_cb(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_DELETE && obj == _g_p_msgbox) {
		/* ɾ��������,ȥ��ģ̬��Ч�� */
		lv_obj_del_async(lv_obj_get_parent(obj));
		_g_p_msgbox = NULL; /* happens before object is actually deleted! */

	} else if (event == LV_EVENT_VALUE_CHANGED) {

		lv_msgbox_start_auto_close(obj, 0);
		/* ��ȡ�û����µİ�ť���ı�  �Զ��ر� */
		printf("User Choose is %d .\n", lv_msgbox_get_active_btn(obj));
 
	}

}

static void _msgbox_opa_anim_cb(void * bg, lv_anim_value_t v)
{
	lv_obj_set_style_local_bg_opa(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, v);
}

void messagebox_create(uint8_t *p_msg)
{
	if (_g_p_msgbox != NULL) {
		return ;
	}
	lv_style_init(&_g_style_modal);
	lv_style_set_bg_color(&_g_style_modal, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_bg_grad_color(&_g_style_modal, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	//lv_style_set_bg_opa(&style_modal, LV_STATE_DEFAULT, LV_OPA_50);

	/* ����һ����������ģ̬��Ч�� */

	lv_obj_t *obj = lv_obj_create(lv_layer_top(), NULL);/* Create a base object for the modal background */
	lv_obj_add_style(obj, LV_LINEMETER_PART_MAIN, &_g_style_modal);
	lv_obj_set_pos(obj, 0, 0);
	lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);

	_g_p_msgbox = lv_msgbox_create(obj, NULL);
	lv_obj_align_origo(_g_p_msgbox, NULL, LV_ALIGN_CENTER, 0, -50); /* λ�� */
	lv_msgbox_add_btns(_g_p_msgbox, _g_p_btns);
	lv_obj_t * btnm = lv_msgbox_get_btnmatrix(_g_p_msgbox);
	lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECK_STATE);
	lv_msgbox_set_text(_g_p_msgbox, p_msg);
	lv_obj_set_event_cb(_g_p_msgbox, _msgbox_event_cb);


	/* Fade the message box in with an animation */
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, obj);
	lv_anim_set_time(&a, 500);
	lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_50);
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)_msgbox_opa_anim_cb);
	lv_anim_start(&a);

}