/*
 *  my-account
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "myaccount_ug.h"
#include "myaccount_ug_common.h"
#include "myaccount_ug_addaccount.h"
#include "myaccount_ug_account_list.h"
#include <app_control_internal.h>
#include <account_internal.h>

static myaccount_appdata *g_myaccount_appdata = NULL;

#define MYACCOUNT_LAUNCH_TIME_OUT 0.7
#define MYACCOUNT_RELAUNCH_INTERVAL 0.3
#define MYACCOUNT_MAX_TOUCH_BLOCK_INTERVAL 2.5


void *myaccount_init_appdata()
{
	if (g_myaccount_appdata == NULL) {
		g_myaccount_appdata = calloc(1, sizeof(myaccount_appdata));
		g_myaccount_appdata->item_selected_state = false;
	}
	return g_myaccount_appdata;
}

void myaccount_destroy_appdata()
{
	if (g_myaccount_appdata == NULL) {
		MYACCOUNT_ERROR("myaccount_destroy_appdata: appdata NULL \n");
		return;
	}

	if (g_myaccount_appdata->base) {
		evas_object_del(g_myaccount_appdata->base);
		g_myaccount_appdata->base = NULL;
	}

	MA_MEMFREE(g_myaccount_appdata->capability_filter)

	/* For accounts list */
	if (g_myaccount_appdata->ug_called) {
		ug_destroy(g_myaccount_appdata->ug_called);
		g_myaccount_appdata->ug_called = NULL;
	}
	if (g_myaccount_appdata->layout_main) {
		evas_object_del(g_myaccount_appdata->layout_main);
		g_myaccount_appdata->layout_main = NULL;
	}
	if (g_myaccount_appdata->layout_addaccount) {
		evas_object_del(g_myaccount_appdata->layout_addaccount);
		g_myaccount_appdata->layout_addaccount = NULL;
	}
	if (g_myaccount_appdata->navi_bar) {
		evas_object_del(g_myaccount_appdata->navi_bar);
		g_myaccount_appdata->navi_bar = NULL;
	}
	if (g_myaccount_appdata->help_label) {
		evas_object_del(g_myaccount_appdata->help_label);
		g_myaccount_appdata->help_label = NULL;
	}
	if (g_myaccount_appdata->refresh_btn) {
		evas_object_del(g_myaccount_appdata->refresh_btn);
		g_myaccount_appdata->refresh_btn = NULL;
	}
	if (g_myaccount_appdata->refresh_icon) {
		evas_object_del(g_myaccount_appdata->refresh_icon);
		g_myaccount_appdata->refresh_icon = NULL;
	}
	if (g_myaccount_appdata->cancel_sync_icon) {
		evas_object_del(g_myaccount_appdata->cancel_sync_icon);
		g_myaccount_appdata->cancel_sync_icon = NULL;
	}
	if (g_myaccount_appdata->auto_sync_check) {
		evas_object_del(g_myaccount_appdata->auto_sync_check);
		g_myaccount_appdata->auto_sync_check = NULL;
	}
	if (g_myaccount_appdata->bg) {
		evas_object_del(g_myaccount_appdata->bg);
		g_myaccount_appdata->bg = NULL;
	}
	if (g_myaccount_appdata->ly) {
		elm_object_part_content_unset(g_myaccount_appdata->ly, "elm.swallow.content");
		evas_object_del(g_myaccount_appdata->ly);
		g_myaccount_appdata->ly = NULL;
	}
	if (g_myaccount_appdata->modal_popup) {
		evas_object_del(g_myaccount_appdata->modal_popup);
		g_myaccount_appdata->modal_popup = NULL;
	}
	if (g_myaccount_appdata->r_title_sk) {
		evas_object_del(g_myaccount_appdata->r_title_sk);
		g_myaccount_appdata->r_title_sk = NULL;
	}

	if (g_myaccount_appdata->progress_timer) {
		ecore_timer_del(g_myaccount_appdata->progress_timer);
		g_myaccount_appdata->progress_timer = NULL;
	}
	if (g_myaccount_appdata->relaunch_timer) {
		ecore_timer_del(g_myaccount_appdata->relaunch_timer);
		g_myaccount_appdata->relaunch_timer = NULL;
	}
	if (g_myaccount_appdata->pc) {
		pkgmgr_client_free(g_myaccount_appdata->pc);
		g_myaccount_appdata->pc = NULL;
	}

	if(g_myaccount_appdata->theme) {
		elm_theme_free(g_myaccount_appdata->theme);
		g_myaccount_appdata->theme = NULL;
	}

	if(g_myaccount_appdata->clickblock_timer){
		ecore_timer_del(g_myaccount_appdata->clickblock_timer);
		g_myaccount_appdata->clickblock_timer = NULL;
	}

	g_myaccount_appdata->sorted_sp_list= NULL;
	g_myaccount_appdata->sorted_account_list= NULL;

	MA_MEMFREE(g_myaccount_appdata->caller_userdata);
	MA_MEMFREE(g_myaccount_appdata->called_from);
	MA_MEMFREE(g_myaccount_appdata);

}

void * myaccount_get_appdata()
{
	return g_myaccount_appdata;
}

static Eina_Bool _myaccount_common_click_block_timer_cb(void *data)
{
	myaccount_appdata *ad = (myaccount_appdata *)data;

	if(!ad){
		return ECORE_CALLBACK_CANCEL;
	}

	if(ad->clickblock_timer){
		ecore_timer_del(ad->clickblock_timer);
		ad->clickblock_timer = NULL;
	}

	MYACCOUNT_DBUG("_myaccount_common_click_block_timer_cb: ad->item_selected_state=%d\n", 	myaccount_common_get_item_selected_state());
	myaccount_common_set_item_selected_state(FALSE);

	return ECORE_CALLBACK_CANCEL;
}

void myaccount_common_set_item_selected_state(bool val)
{
	myaccount_appdata *appdata = (myaccount_appdata*)myaccount_get_appdata();

	MYACCOUNT_DBUG("myaccount_common_set_item_selected_state[%s]", val ? "true":"false");
	if (!appdata) {
		MYACCOUNT_ERROR("appdata is NULL");
		return;
	}

	appdata->item_selected_state = val;

	if(appdata->clickblock_timer){
		ecore_timer_del(appdata->clickblock_timer);
		appdata->clickblock_timer = NULL;
	}

	if( val == true ) {
		appdata->clickblock_timer = ecore_timer_add(MYACCOUNT_MAX_TOUCH_BLOCK_INTERVAL, _myaccount_common_click_block_timer_cb, appdata);
	}
}

bool myaccount_common_get_item_selected_state()
{
	myaccount_appdata *appdata = (myaccount_appdata*)myaccount_get_appdata();

	return (appdata->item_selected_state);
}

void myaccount_common_get_icon_by_name(char *domain_name,
											char *icon_path)
{
	if (!domain_name) {
		MYACCOUNT_ERROR("\n !!! domain_name is NULL \n");
		return;
	}
	char tmp_domain_name[64] = {0,};
	myaccount_common_lowercase(domain_name, tmp_domain_name);

	if (!icon_path) {
		MYACCOUNT_ERROR("\n !!! icon_path is NULL \n");
		return;
	}

	MA_SNPRINTF(icon_path, BUFSIZE, "%s", "A01_2_Icon_default.png");
}

void myaccount_common_lowercase(char *src, char *dest)
{
	int i;
	int src_len=0;

	if (!src || !dest) {
		MYACCOUNT_ERROR("myaccount_common_lowercase src=%p, dest=%p\n",
						src, dest);
		return;
	}
	src_len = strlen(src);
	for(i=0;i<src_len;i++) {
		dest[i] = tolower(src[i]);
	}
	dest[i] = '\0';
}

void myaccount_common_handle_notification(const char* event_type)
{
	myaccount_appdata *appdata = (myaccount_appdata*)myaccount_get_appdata();

	if (!appdata) {
		MYACCOUNT_ERROR("App data NULL!!! \n");
		return;
	}

	int error_code = -1;
	int count = -1;

	myaccount_common_set_item_selected_state(false);

	MYACCOUNT_DBUG("called_from[%s],eMode(%d),event_type(%s), capability_filter(%s)\n", appdata->called_from, appdata->eMode,event_type, appdata->capability_filter);
	if(appdata->capability_filter && strlen(appdata->capability_filter)) {
		error_code = myaccount_common_get_account_cnt_by_capability(appdata->capability_filter, &count);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_DBUG(": myaccount_common_get_account_cnt_by_capability code(%d)count(%d) \n", error_code, count);
		}

	} else {
		error_code = myaccount_common_get_all_account_cnt(&count, false);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("myaccount_common_handle_notification: myaccount_common_get_all_account_cnt code(%d) \n", error_code);
		}
	}
	MYACCOUNT_DBUG("count[%d],appdata->add_genlist[%p],appdata->account_genlist[%p]", count,appdata->add_genlist,appdata->account_genlist);

	if (appdata->eMode == eMYACCOUNT_ACCOUNT_LIST) {
		if (count > 0) {
			if (appdata->add_genlist) {
				if( strcmp(event_type, ACCOUNT_NOTI_NAME_SYNC_UPDATE)){	// not sync update -> insert, update, delete
					MYACCOUNT_DBUG("#  recved noti-event(%s) -> elm_naviframe_item_pop / quit_cb should be called.", (event_type != NULL) ? event_type : "null");
					elm_naviframe_item_pop(appdata->navi_bar);
					appdata->add_genlist = NULL;
					MYACCOUNT_DBUG("elm_naviframe_item_pop func finished.\n");
				} else {
					// if sync update received, do nothing.
					MYACCOUNT_DBUG("recved noti-event(%s) -> do nothing.", (event_type != NULL) ? event_type : "null");
				}
			}

			if (appdata->account_genlist) {
				myaccount_list_refresh_item_list(appdata);
			} else {
				myaccount_ug_account_list_create((void*)appdata);
			}
		} else {
			if (appdata->add_genlist) {
				evas_object_show(appdata->add_genlist);
			} else {
				MYACCOUNT_DBUG("eMYACCOUNT_ACCOUNT_LIST mode, count=0, add_genlist=NULL, event_type must be delete.");
				myaccount_addaccount_create_view(appdata);	// event_type=delete, fix duplicate elm_naviframe_item_push happening.
			}
		}
	} else if (appdata->eMode == eMYACCOUNT_ADD_ACCOUNT) {
		MYACCOUNT_SLOGD("myaccount_common_handle_notification: ADD_ACCOUNT, event_type = %s\n", event_type);
		if(event_type){
			if (appdata->ug && count > 0) {
				if (!g_strcmp0(event_type, ACCOUNT_NOTI_NAME_INSERT) || !g_strcmp0(event_type, ACCOUNT_NOTI_NAME_SYNC_UPDATE)) {
					app_control_h app_control = NULL;
					app_control_create(&app_control);
					app_control_add_extra_data(app_control, "account_noti", event_type);
					error_code = ug_send_result(appdata->ug, app_control);
					ui_gadget_h ug = appdata->ug;
					error_code = ug_destroy_me(ug);
					app_control_destroy(app_control);
/*
					service_h service;
					service_create(&service);
					service_add_extra_data(service, "account_noti", event_type);
					error_code = ug_send_result(appdata->ug, service);
					ui_gadget_h ug = appdata->ug;
					error_code = ug_destroy_me(ug);
					service_destroy(service);
*/
				}
			}
		}
	}
}

static void _myaccount_common_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	myaccount_appdata *ad = (myaccount_appdata *)data;

	if(ad->modal_popup)
		evas_object_del(ad->modal_popup);
	ad->modal_popup = NULL;
}

Evas_Object* _myaccount_common_add_popup(Evas_Object* parent, const char* text, const char* title)
{
	myaccount_appdata *ad = (myaccount_appdata*)myaccount_get_appdata();

	MYACCOUNT_DBUG("multi window not enabled.\n");
	ad->modal_popup = elm_popup_add(parent);

	evas_object_size_hint_weight_set(ad->modal_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(ad->modal_popup, text);
	if(!title)
		elm_object_part_text_set(ad->modal_popup, "title,text", title);
	evas_object_smart_callback_add(ad->modal_popup, "block,clicked", _myaccount_common_popup_cb, (void*)ad);

    Evas_Object *ok_btn = elm_button_add(ad->modal_popup);
    elm_object_text_set(ok_btn, dgettext(MA_UG_NAME, "IDS_MA_BUTTON_OK"));
    elm_object_part_content_set(ad->modal_popup, "button1", ok_btn);
    evas_object_smart_callback_add(ok_btn, "clicked", _myaccount_common_popup_cb, (void*)ad);
    elm_object_style_set(ok_btn, "popup_button/default");

	evas_object_show(ad->modal_popup);
	return ad->modal_popup;
}

static Eina_Bool _myaccount_common_relaunch_application_timer_cb(void *data)
{
	myaccount_appdata *ad = (myaccount_appdata *)data;

	if(!ad){
		return ECORE_CALLBACK_CANCEL;
	}

	if(ad->relaunch_timer){
		ecore_timer_del(ad->relaunch_timer);
		ad->relaunch_timer = NULL;
	}

	app_control_send_launch_request(ad->appsvc_handle, NULL, NULL);

	MYACCOUNT_DBUG("ad->item_selected_state=%d\n", 	myaccount_common_get_item_selected_state());
	myaccount_common_set_item_selected_state(FALSE);

	app_control_destroy(ad->appsvc_handle);
	ad->appsvc_handle = NULL;

	return ECORE_CALLBACK_CANCEL;
}

static void _myaccount_common_try_relaunch(myaccount_appdata* ad, app_control_h app_control)
{
	if(ad->relaunch_timer){
		ecore_timer_del(ad->relaunch_timer);
		ad->relaunch_timer = NULL;
	}

	ad->appsvc_handle = app_control;

	ad->relaunch_timer = ecore_timer_add(MYACCOUNT_RELAUNCH_INTERVAL, _myaccount_common_relaunch_application_timer_cb, ad);
}


static Eina_Bool _myaccount_common_launch_application_timer_cb(void *data)
{
	myaccount_appdata *ad = (myaccount_appdata *)data;

	if(!ad){
		return ECORE_CALLBACK_CANCEL;
	}

	if(ad->progress_timer){
		ecore_timer_del(ad->progress_timer);
		ad->progress_timer = NULL;
	}

	MYACCOUNT_DBUG("ad->item_selected_state=%d\n", 	myaccount_common_get_item_selected_state());

	return ECORE_CALLBACK_CANCEL;
}

static void _myaccount_common_launch_timer_start(myaccount_appdata *ad)
{
	if(ad->progress_timer){
		ecore_timer_del(ad->progress_timer);
		ad->progress_timer = NULL;
	}

	ad->progress_timer = ecore_timer_add(MYACCOUNT_LAUNCH_TIME_OUT, _myaccount_common_launch_application_timer_cb, ad);
}

int  myaccount_common_launch_application(MyAccountRequestViewType
														request_type,
														char *package_name,
														char *username,
														int account_id,
														void *data )
{
	int ret = APP_CONTROL_ERROR_NONE;
	app_control_h app_control = NULL;
	myaccount_appdata *ad = (myaccount_appdata *) data;
	char id_buf[64] = {0,};

	if (!ad || !package_name) {
		MYACCOUNT_SLOGE("myaccount_common_launch_application: App data=%p, package_name=%p\n",
							ad, package_name);
		return 0;
	}

	MYACCOUNT_SLOGD("trying to launch (%s)\n", package_name);

	ret = app_control_create(&app_control);
	if ( ret != APP_CONTROL_ERROR_NONE ) {
		MYACCOUNT_SLOGE("app_control_create() fail, ret = %d\n", ret);
	}

	ret = app_control_set_app_id(app_control, package_name);
	if ( ret != APP_CONTROL_ERROR_NONE ) {
		MYACCOUNT_SLOGE("app_control_set_app_id() fail, ret = %d\n", ret);
	}

	if(ad->caller_userdata) {
		ret = app_control_add_extra_data(app_control, "myaccount_userdata", ad->caller_userdata);
		if ( ret != APP_CONTROL_ERROR_NONE ) {
			MYACCOUNT_SLOGE("app_control_add_extra_data() fail, ret = %d\n", ret);
		}

	}

	if (ret != APP_CONTROL_ERROR_NONE)
		MYACCOUNT_ERROR("myaccount_common_launch_application: service_set_package failed\n");

	switch (request_type) {
		case MYACCOUNT_REQUEST_SIGNIN :
				app_control_set_operation(app_control, ACCOUNT_OPERATION_SIGNIN);

			break;
		case MYACCOUNT_REQUEST_VIEW :
			if(!username) {
				MYACCOUNT_ERROR("myaccount_common_launch_application: username=%p !!!\n",
								username);
				MA_MEMFREE(package_name);
				if(app_control)
					app_control_destroy(app_control);
				return 0;
			}

			MA_SNPRINTF(id_buf, sizeof(id_buf), "%d", account_id);
			ret = app_control_add_extra_data(app_control, ACCOUNT_DATA_ID, id_buf);
			if ( ret != APP_CONTROL_ERROR_NONE ) {
				MYACCOUNT_ERROR("app_control_add_extra_data fail, ret = %d\n", ret);
			}

			ret = app_control_set_operation(app_control, ACCOUNT_OPERATION_VIEW);
			if ( ret != APP_CONTROL_ERROR_NONE ) {
				MYACCOUNT_ERROR("app_control_set_operation fail, ret = %d\n", ret);
			}

			ret = app_control_add_extra_data(app_control, ACCOUNT_DATA_USERNAME, username);
			if ( ret != APP_CONTROL_ERROR_NONE ) {
				MYACCOUNT_ERROR("app_control_add_extra_data fail, ret = %d\n", ret);
			}
			break;
		default :
			MA_MEMFREE(username);
			MA_MEMFREE(package_name);
			app_control_destroy(app_control);
			return 0;
	}
	MYACCOUNT_SLOGD("myaccount_common_launch_application request type=%d, package name = %s\n",
							request_type, package_name);

	ret = app_control_send_launch_request(app_control, NULL, NULL);
	if ( ret != APP_CONTROL_ERROR_NONE ) {
		MYACCOUNT_ERROR("app_control_send_launch_request fail, ret = %d\n", ret);
	}

	MA_MEMFREE(username);
	MA_MEMFREE(package_name);

	if(ret != APP_CONTROL_ERROR_NONE) {
		MYACCOUNT_SLOGE("(%s) launch fail. ret(%x)\n", package_name, ret);
		_myaccount_common_try_relaunch(ad, app_control);
		return 0;
	}

	app_control_destroy(app_control);
	_myaccount_common_launch_timer_start(ad);

	return ret;
}

void myaccount_common_delete_modal_popup(Evas_Object *popup)
{
	if (!popup){
		MYACCOUNT_ERROR("myaccount_common_delete_modal_popup: popup is NULL \n");
		return;
	}
	evas_object_del(popup);
}

Eina_Bool myaccount_common_modal_popup_timer_cb(gpointer data)
{
	if (!data) {
		MYACCOUNT_ERROR("myaccount_common_modal_popup_timer_cb: data is NULL \n");
		return false;
	}
	myaccount_appdata *global_data = NULL;
	global_data = (myaccount_appdata*)data;
	if (global_data->modal_popup) {
		myaccount_common_delete_modal_popup(global_data->modal_popup);
		global_data->modal_popup = NULL;
	}
	if (global_data->progress_timer) {
		ecore_timer_del(global_data->progress_timer);
		global_data->progress_timer = NULL;
	}
	return false;
}

int myaccount_common_get_all_account_cnt(int *count, bool include_secret)
{
	int error_code = -1;
	int account_cnt = 0;
	if (!count) {
		MYACCOUNT_ERROR("myaccount_common_get_all_account_cnt: count ptr is NULL \n");
		return error_code;
	}
	if( include_secret ) {
		error_code = account_get_total_count_from_db(&account_cnt);
	}else {
		error_code = account_get_total_count_from_db_ex(&account_cnt);
	}
	if (error_code != ACCOUNT_ERROR_NONE) {
		return error_code;
	}
	*count = account_cnt;

	return ACCOUNT_ERROR_NONE;
}

static bool _myaccount_common_query_cb(account_h account, void *user_data)
{
	int* tmp_count = (int*)user_data;

	*tmp_count = *tmp_count + 1;

	return TRUE;
}

int myaccount_common_get_account_cnt_by_capability(const char* capability_type, int *count)
{
	int error_code = -1;
	int account_cnt = 0;
	if (!count) {
		MYACCOUNT_ERROR("myaccount_common_get_account_cnt_by_capability: count ptr is NULL \n");
		return error_code;
	}

	error_code = account_query_account_by_capability_type(_myaccount_common_query_cb, capability_type, &account_cnt);
	if (error_code != ACCOUNT_ERROR_NONE) {

		*count = 0;

		return error_code;
	}
	*count = account_cnt;

	return ACCOUNT_ERROR_NONE;
}

int myaccount_common_get_account_cnt_by_appid(const char* appid, int *count)
{
	int error_code = -1;
	int account_cnt = 0;
	if (!count) {
		MYACCOUNT_ERROR("myaccount_common_get_account_cnt_by_appid: count ptr is NULL \n");
		return error_code;
	}

	error_code = account_query_account_by_package_name(_myaccount_common_query_cb, appid, &account_cnt);

	if (error_code != ACCOUNT_ERROR_NONE) {

		*count = 0;

		return error_code;
	}
	*count = account_cnt;

	return ACCOUNT_ERROR_NONE;
}

static int _myaccount_common_pkmgr_return_cb(uid_t target_uid, int req_id, const char *pkg_type,
					const char *pkg_name,
					const char *key,
					const char *val,
					const void *pmsg,
					void *data)
{
	MYACCOUNT_SLOGD(" _myaccount_common_pkmgr_return_cb: pkg_type = %s, pkg_name = %s, key = %s, val = %s\n", pkg_type, pkg_name,key,val);

	int error_code = -1;
	int count = -1;
	if( strcmp(key, "end") == 0 && strcmp(val, "ok") == 0) {

		myaccount_appdata *appdata = (myaccount_appdata*)myaccount_get_appdata();

		if (!appdata) {
			MYACCOUNT_ERROR("App data NULL!!! \n");
			return -1;
		}
		if(appdata->capability_filter && strlen(appdata->capability_filter)){
			error_code = myaccount_common_get_account_cnt_by_capability(appdata->capability_filter, &count);
			if (error_code != ACCOUNT_ERROR_NONE) {
				MYACCOUNT_ERROR("__myaccount_addaccount_quit_cb: myaccount_common_get_all_account_cnt fail(%d) \n",
											error_code);
			}
		} else {
			error_code = myaccount_common_get_all_account_cnt(&count, false);
			if (error_code != ACCOUNT_ERROR_NONE) {
				MYACCOUNT_ERROR("__myaccount_addaccount_quit_cb: myaccount_common_get_all_account_cnt fail(%d) \n",
											error_code);
			}
		}

		if (appdata->eMode == eMYACCOUNT_ACCOUNT_LIST) {
			if (count > 0) {
				if (appdata->add_genlist) {
					evas_object_del(appdata->add_genlist);
					appdata->add_genlist = NULL;
					elm_naviframe_item_pop(appdata->navi_bar);
				}
				if (appdata->account_genlist) {
					myaccount_list_refresh_item_list(appdata);
				} else {
					myaccount_ug_account_list_create((void*)appdata);
				}
			} else {
				if (appdata->add_genlist) {
					evas_object_show(appdata->add_genlist);
				} else {
					myaccount_addaccount_create_view(appdata);
				}
			}
		} else if (appdata->eMode == eMYACCOUNT_ADD_ACCOUNT) {
			if (appdata->ug) {
				ui_gadget_h ug = appdata->ug;
				error_code = ug_destroy_me(ug);
			}
		} else {
			//MYACCOUNT_WARNING("Unknown mode !!!");
		}
		return 0;
	}
	return 0;
}

void myaccount_common_listen_pkgmgr_for_pkg_uninstall()
{
	myaccount_appdata *appdata = (myaccount_appdata*)myaccount_get_appdata();

	if (!appdata) {
		MYACCOUNT_ERROR("App data NULL!!! \n");
		return;
	}

	int result = 0;
	pkgmgr_client *pc = NULL;
	pc = pkgmgr_client_new(PC_LISTENING);
	if(pc == NULL) {
		MYACCOUNT_ERROR("pc is NULL\n");
		return;
	}
	appdata->pc = pc;
	result = pkgmgr_client_listen_status(pc, _myaccount_common_pkmgr_return_cb, pc);
	if(result < 0)
	{
		MYACCOUNT_ERROR("status listen failed!\n");
		return;
	}
}


bool myaccount_common_account_by_package_name_cb(account_h account,
													void *user_data)
{
	return false;
}

static bool _myaccount_common_get_account_type_count_cb(account_type_h account_type, void *user_data)
{
	int *count = (int*)user_data;
	char* type_buf = NULL;
	int type_int = -1;

	if(account_type == NULL) {
		MYACCOUNT_ERROR(" account type handle is NULL \n");
		return FALSE;
	}

	account_type_get_app_id(account_type, &type_buf);

	if(!type_buf) return FALSE;


	account_type_get_multiple_account_support(account_type, &type_int);

	if(type_int == FALSE
		&& account_query_account_by_package_name(myaccount_common_account_by_package_name_cb, type_buf, NULL) == ACCOUNT_ERROR_NONE) {
		MYACCOUNT_SLOGD("Multiple account not support (%s)\n", type_buf);
		MA_MEMFREE(type_buf);
		return TRUE;
	}

	*count = *count+1;

	MA_MEMFREE(type_buf);

	return TRUE;
}


int myaccount_common_get_account_type_count(const char* capability_filter)
{
	int count = 0;

	if(capability_filter) {
		account_type_query_by_provider_feature(_myaccount_common_get_account_type_count_cb, capability_filter, (void*)&count);
	}
	else {
		account_type_foreach_account_type_from_db(_myaccount_common_get_account_type_count_cb, (void*)&count);
	}

	MYACCOUNT_DBUG("count %d\n", count);

	return count;
}

