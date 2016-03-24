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

#include "myaccount_ug_account_list.h"
#include "myaccount_ug_addaccount.h"
#include "myaccount_ug_common.h"
#include <vconf.h>
#include <account_internal.h>

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

static myaccount_appdata *g_appdata = NULL;
static int g_handle_noti = 0;

int myaccount_subscribe_to_account_notification_vconfkey();

static void lang_changed(void * priv)
{
	if (!priv) {
		MYACCOUNT_SLOGE("lang_changed ### priv=%p\n", priv);
		return ;
	}

	myaccount_appdata *ugd;
	ugd = (myaccount_appdata*)priv;

	char* language = NULL;
	language = vconf_get_str(VCONFKEY_LANGSET);

	if(language) {
		char tmp_buf[6]={0,};
		MA_STRNCPY(tmp_buf, language, 6);
		ugd->current_language = strdup(tmp_buf);
	}
	MA_MEMFREE(language);

	elm_object_item_text_set (ugd->nf_it, dgettext(MA_UG_NAME, "IDS_MA_HEADER_ACCOUNTS"));
	myaccount_list_refresh_item_list(ugd);

	if( ugd->add_genlist ) {
		// title text update
		elm_object_item_text_set (ugd->add_nf_it, dgettext(MA_UG_NAME, "IDS_MA_HEADER_ADD_ACCOUNT"));
		myaccount_addaccount_refresh_item_list(ugd);
	} else {
		MYACCOUNT_SLOGE("lang_changed ### ugd->add_genlist is NULL\n");
	}
}


//static gboolean __myaccount_account_list_parse_param(myaccount_mode_e *mode, service_h data, myaccount_appdata *ad)
static gboolean __myaccount_account_list_parse_param(myaccount_mode_e *mode, app_control_h data, myaccount_appdata *ad)
{
	if (!data || !ad) {
		return FALSE;
	} else {
		char *extracted = NULL;
		ad->capability_filter = NULL;
/*
		service_get_extra_data(data, "mode", &extracted);
		service_get_extra_data(data, "capability_filter", &ad->capability_filter);
		service_get_extra_data(data, "myaccount_userdata", &ad->caller_userdata);
		service_get_extra_data(data, "from", &ad->called_from);
*/
		app_control_get_extra_data(data, "mode", &extracted);
		app_control_get_extra_data(data, "capability_filter", &ad->capability_filter);
		app_control_get_extra_data(data, "myaccount_userdata", &ad->caller_userdata);
		app_control_get_extra_data(data, "from", &ad->called_from);
		if (extracted) {
			MYACCOUNT_SLOGD("view_account_list = %s\n", extracted);

			if (!strcmp(extracted, "account_list"))
				*mode = eMYACCOUNT_ACCOUNT_LIST;
			else if (!strcmp(extracted, "add_account"))
				*mode = eMYACCOUNT_ADD_ACCOUNT;
			else
				*mode = eMYACCOUNT_ACCOUNT_LIST;
		} else {
			*mode = eMYACCOUNT_ACCOUNT_LIST;
		}

		MA_MEMFREE(extracted);
	}
	return TRUE;
}

//void *myaccount_account_list_create(ui_gadget_h ug, enum ug_mode mode, service_h data, void *priv)
void *myaccount_account_list_create(ui_gadget_h ug, enum ug_mode mode, app_control_h data, void *priv)
{
	 Evas_Object *parent;
	 myaccount_appdata *ugd;
	 int count = -1;
	 int error_code = -1;

	 if (!ug || !priv) {
		 return NULL;
	 }

	 bindtextdomain("setting-myaccount-efl", "/usr/apps/setting-myaccount-efl/res/locale");
	 ugd = (myaccount_appdata*)priv;
	 ugd->ug = ug;
	 ugd->eMode = 0;
	 parent = (Evas_Object *)ug_get_window();
	 if (!parent) {
		 return NULL;
	 }
	 g_appdata = ugd;

	ugd->win_main = parent;
	Evas_Object *conformant = (Evas_Object *)ug_get_conformant();

	ugd->conformant = conformant;

	 /*sbscribe to vconf noti key which will recevie notification about account update, delete and insert*/
#ifdef ENABLE_NOTI
	 myaccount_subscribe_to_account_notification_vconfkey();
#endif
	/*listen to pkgmgr for uninstall of pkg*/
	myaccount_common_listen_pkgmgr_for_pkg_uninstall();

	 __myaccount_account_list_parse_param(&ugd->eMode, data, ugd);

	if( ugd->capability_filter
		&& strlen(ugd->capability_filter) > 0 ) {

		error_code = myaccount_common_get_account_cnt_by_capability(ugd->capability_filter, &count);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_SLOGE("myaccount_account_list_create: myaccount_common_get_all_account_cnt fail(%d) \n",
								error_code);
		}

	} else {
		error_code = myaccount_common_get_all_account_cnt(&count, false);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_SLOGE("myaccount_account_list_create: myaccount_common_get_all_account_cnt fail(%d) \n",
								error_code);
		}
	}

	char* language = NULL;
	language = vconf_get_str(VCONFKEY_LANGSET);

	if(language) {
		char tmp_buf[6]={0,};

		MA_STRNCPY(tmp_buf, language, 6);

		ugd->current_language = strdup(tmp_buf);
	}
	MA_MEMFREE(language);

	myaccount_list_view_create(ugd);

	myaccount_list_navi_create(ugd);

	if (ugd->eMode == eMYACCOUNT_ACCOUNT_LIST) {
		if (count > 0) {
			myaccount_ug_account_list_create((void*)priv);
		} else if (count == 0) {
			myaccount_addaccount_create_view(ugd);
		} else {
			MYACCOUNT_ERROR("add list count(%d)", count);
		}
	} else if (ugd->eMode == eMYACCOUNT_ADD_ACCOUNT) {
		myaccount_addaccount_create_view(ugd);
	}

	 return ugd->base;
 }

bool myaccount_subscribe_callback(const char* event_type, int account_id, void* user_data)
{
	myaccount_common_handle_notification(event_type);

	return TRUE;
}

int myaccount_subscribe_to_account_notification_vconfkey()
{
	myaccount_appdata* ad = (myaccount_appdata*)myaccount_get_appdata();
	int ret = -1;

	if(!ad) return 0;

	ret = account_subscribe_create(&ad->account_subscribe);
	if(ret != ACCOUNT_ERROR_NONE){
		MYACCOUNT_ERROR("account_subscribe_create fail");
		return 0;
	}
	ret = account_subscribe_notification_ex(ad->account_subscribe, myaccount_subscribe_callback, NULL);
	if(ret != ACCOUNT_ERROR_NONE){
		MYACCOUNT_ERROR("account_subscribe_notification_ex fail");
		return 0;
	}

	return 1;
}

 //static void myaccount_account_list_start(ui_gadget_h ug, service_h data, void *priv)
 static void myaccount_account_list_start(ui_gadget_h ug, app_control_h data, void *priv)
 {
	 MYACCOUNT_ERROR("My account ug goes to start state\n");
 }

// static void myaccount_account_list_pause(ui_gadget_h ug, service_h data, void *priv)
 static void myaccount_account_list_pause(ui_gadget_h ug, app_control_h data, void *priv)
 {
	__attribute__((__unused__)) myaccount_appdata *ugd;
	 ugd = (myaccount_appdata*)priv;

	 MYACCOUNT_ERROR("My account ug goes to pause state\n");
 }

// static void myaccount_account_list_resume(ui_gadget_h ug, service_h data, void *priv)
static void myaccount_account_list_resume(ui_gadget_h ug, app_control_h data, void *priv)
 {
	 myaccount_appdata *ugd;
	 ugd = (myaccount_appdata*)priv;
	 MYACCOUNT_ERROR("My account ug goes to resume state\n");
	myaccount_common_set_item_selected_state(false);
#ifndef ENABLE_NOTI
	 myaccount_common_handle_notification(NULL);
#endif

	char* language = NULL;
	language = vconf_get_str(VCONFKEY_LANGSET);

	if(language && (strcmp(language, ugd->current_language) == 0)) {
		lang_changed(priv);
	} else {
		/* refresh item to update font size */
		myaccount_list_refresh_item_list(ugd);
		if( ugd->add_genlist ) {
			myaccount_addaccount_refresh_item_list(ugd);
		}
	}
	MA_MEMFREE(language);
}

void myaccount_destroy_data()
{
	if (!g_appdata) {
		MYACCOUNT_SLOGE("myaccount_account_ug_destroy ###  g_appdata=%p\n", g_appdata);
		return;
	}

	myaccount_appdata *ugd = g_appdata;
	account_unsubscribe_notification_ex(ugd->account_subscribe);
	ugd->account_subscribe = NULL;

	myaccount_destroy_appdata();

	if (g_handle_noti > 0) {
		g_source_remove(g_handle_noti);
		g_handle_noti = 0;
	}

	MYACCOUNT_DBUG("Terminate Myaccount_ug_main[%s]", __func__);
	g_appdata = NULL;
}

//static void myaccount_account_list_destroy(ui_gadget_h ug, service_h data, void *priv)
static void myaccount_account_list_destroy(ui_gadget_h ug, app_control_h data, void *priv)
{
	myaccount_destroy_data();
}

//static void myaccount_account_list_message(ui_gadget_h ug, service_h msg, service_h data, void *priv)
static void myaccount_account_list_message(ui_gadget_h ug, app_control_h msg, app_control_h data, void *priv)
{

}

//static void myaccount_account_list_event(ui_gadget_h ug, enum ug_event event, service_h data, void *priv)
static void myaccount_account_list_event(ui_gadget_h ug, enum ug_event event, app_control_h data, void *priv)
 {
	switch (event) {
	case UG_EVENT_LOW_MEMORY:
			break;
	case UG_EVENT_LOW_BATTERY:
			break;
	case UG_EVENT_LANG_CHANGE: /* old : SG_BINDTEXTDOMAIN_NOTIFY*/
			lang_changed(priv);
			MYACCOUNT_DBUG("myaccount_account_list_event: UG_EVENT_LANG_CHANGE \n");
			break;
	case UG_EVENT_ROTATE_PORTRAIT:
			break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
			break;
	case UG_EVENT_ROTATE_LANDSCAPE:
			break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
			break;
	default:
			break;
	 }
 }

//static void myaccount_on_key_event(ui_gadget_h ug, enum ug_key_event event, service_h data, void * priv)
static void myaccount_on_key_event(ui_gadget_h ug, enum ug_key_event event, app_control_h data, void * priv)
{
	if(!ug) {
		MYACCOUNT_DBUG("myaccount_on_key_event ug is NULL !!!");
		return;
	}
	switch (event) {
		case UG_KEY_EVENT_END: {
				ui_gadget_h ug = ((myaccount_appdata *)priv)->ug;
				ug_destroy_me(ug);
			}
			break;
		default:
			break;
	}
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	MYACCOUNT_ERROR("UG_MODULE_INIT");
	myaccount_appdata *ugd;
	if (!ops) {
		MYACCOUNT_DBUG("UG_MODULE_INIT ops is NULL !!!");
		return -1;
	}
	ugd = (myaccount_appdata*)myaccount_init_appdata();
	if (!ugd) {
		MYACCOUNT_DBUG("myaccount_init_appdata failed!\n");
		return -1;
	}
	ops->create = myaccount_account_list_create;
	ops->start = myaccount_account_list_start;
	ops->pause = myaccount_account_list_pause;
	ops->resume = myaccount_account_list_resume;
	ops->destroy = myaccount_account_list_destroy;
	ops->message = myaccount_account_list_message;
	ops->event = myaccount_account_list_event;
	ops->key_event= myaccount_on_key_event;/*to handle BACK(END)key Event recently added in UG*/
	ops->priv = ugd;
	ops->opt = UG_OPT_INDICATOR_ENABLE;/*UG_OPT_INDICATOR_ENABLE;*/

	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	MYACCOUNT_ERROR("myaccount UG_MODULE_EXIT!\n");
}

UG_MODULE_API int setting_plugin_reset(app_control_h data, void *priv)
{
	MYACCOUNT_ERROR("setting_plugin_reset called");
	//no plugin to reset
	return 0;
}
