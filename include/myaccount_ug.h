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

#ifndef _MYACCOUNT_H_
#define _MYACCOUNT_H_


#include <glib.h>
#include <stdio.h>
#include <vconf-keys.h>
#include <ui-gadget-module.h>
#include <package-manager.h>
#include <account.h>
#include <efl_extension.h>

#include "Eina.h"


#ifndef LOG_TAG
#define LOG_TAG "MYACCOUNT"
#endif

#include <dlog.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifndef PREFIX
#  define PREFIX "/usr"
#endif

#define MYACCOUNT_VERBOSE(fmt, arg...) \
		LOGD(": [%s (%d)]" fmt ,  __FUNCTION__, __LINE__, ##arg);
#define MYACCOUNT_DBUG(fmt, arg...) \
		LOGD(": [%s (%d)]" fmt ,  __FUNCTION__, __LINE__, ##arg);
#define MYACCOUNT_WARNING(fmt, arg...) \
		LOGW(": [%s (%d)]" fmt ,  __FUNCTION__, __LINE__, ##arg);
#define MYACCOUNT_ERROR(fmt, arg...) \
		LOGE(": [%s (%d)]" fmt ,  __FUNCTION__, __LINE__, ##arg);
#define MYACCOUNT_FATAL(fmt, arg...) \
		LOGF(": [%s (%d)]" fmt ,  __FUNCTION__, __LINE__, ##arg);

#define MYACCOUNT_SLOGD(fmt, arg...) \
		SECURE_LOGD(": [%s (%d)]" fmt ,  __FUNCTION__, __LINE__, ##arg);
#define MYACCOUNT_SLOGI(fmt, arg...) \
		SECURE_LOGD(": [%s (%d)]" fmt ,  __FUNCTION__, __LINE__, ##arg);
#define MYACCOUNT_SLOGE(fmt, arg...) \
		SECURE_LOGD(": [%s (%d)]" fmt ,  __FUNCTION__, __LINE__, ##arg);

#define _EDJ(obj) elm_layout_edje_get(obj)

#define PACKAGE "myaccount"

typedef enum {
	eMYACCOUNT_ACCOUNT_LIST,
	eMYACCOUNT_ADD_ACCOUNT,
} myaccount_mode_e;


typedef struct appdata {
	Evas_Object *base;
	ui_gadget_h ug;

	Evas_Object *navi_bar;
	Evas_Object *win_main;
	Evas_Object *conformant;
	Elm_Object_Item *navi_it;
	myaccount_mode_e eMode;
	Elm_Win_Indicator_Mode indi_mode;
	Elm_Win_Indicator_Opacity_Mode indi_o_mode;
	bool overlap_mode;

	/* For accounts list*/
	Evas_Object *layout_main;
	Evas_Object *account_genlist;
	Evas_Object *help_label;
	Evas_Object *r_title_sk;
	Evas_Object *modal_popup;
	Ecore_Timer *progress_timer;
	Evas_Object *auto_sync_check;
	Evas_Object *refresh_btn;
	Evas_Object *refresh_icon;
	Evas_Object *cancel_sync_icon;
	Elm_Object_Item *nf_it;
	GList* account_info_list;
	GList* sorted_account_list;

	Evas_Object *bg;
	Elm_Theme* theme;
	/* For addaccount view*/
	Evas_Object *layout_addaccount;
	Evas_Object *ly;
	Evas_Object *add_genlist;
	bool item_selected_state;
	GList* sorted_sp_list;
	Elm_Object_Item *add_nf_it;
	int prev_app_cnt;

	/* to hold info about key lock*/
	int lock_priv_state;
	int lock_curr_state;
	ui_gadget_h ug_called;

	/* For application launch */
	Ecore_Timer *relaunch_timer;
	app_control_h appsvc_handle;

	/* to block fast double-click */
	Ecore_Timer *clickblock_timer;

	/* languuage state */
	char* current_language;

	/*package manager client handle */
	pkgmgr_client *pc;

	/* extra data from ug caller */
	char *capability_filter;
	char *caller_userdata;
	char *called_from;
	account_subscribe_h account_subscribe;

	Evas_Object *popup;
}myaccount_appdata;

int myaccount_account_info_update(struct appdata *ad);
void myaccount_destroy_data();
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _MYACCOUNT_H_ */

