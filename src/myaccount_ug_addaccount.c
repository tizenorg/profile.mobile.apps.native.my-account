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

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif
#include "myaccount_ug_common.h"
#include "myaccount_ug_addaccount.h"
#include "myaccount_ug_account_list.h"
#include <app.h>

#define SORT_PRIOR_1 "1_"
#define SORT_PRIOR_2 "2_"
#define SORT_PRIOR_3 "3_"

static Elm_Genlist_Item_Class addacc_list_itc;

static bool _myaccount_addaccount_get_account_type_info_cb(account_type_h account_type, void *user_data);

static char *__myaccount_addaccount_gl_label_get(void *data,
											Evas_Object *obj, const char *part)
{
	addaccount_list_priv *service = (addaccount_list_priv*)data;

	if (!strcmp(part, "elm.text")) {
		char *tmp_sp_name = NULL;

		tmp_sp_name = elm_entry_utf8_to_markup(service->service_name);

		return tmp_sp_name;
	}
	return NULL;
}

bool myaccount_add_account_by_package_name_cb(account_h account,
													void *user_data)
{
	return false;
}

static void __myaccount_addaccount_gl_sel(void *data,
								Evas_Object *obj, void *event_info)
{
	myaccount_appdata *ad = myaccount_get_appdata();
	addaccount_list_priv *service = (addaccount_list_priv*)data;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	if(!ad) {
		MYACCOUNT_ERROR("__myaccount_account_list_gl_sel appdata is NULL\n");
		return;
	}

	MYACCOUNT_DBUG("ad->item_selected_state=%d\n", 	myaccount_common_get_item_selected_state());

	if( myaccount_common_get_item_selected_state() ) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	myaccount_common_set_item_selected_state(true);

	if (item != NULL) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

		if(myaccount_common_launch_application(
					MYACCOUNT_REQUEST_SIGNIN,
					strdup(service->package_name),
					NULL,
					-1,
					ad) != APP_CONTROL_ERROR_NONE ) {
			MYACCOUNT_ERROR("__myaccount_addaccount_gl_sel : Failed to launch application\n");
			myaccount_common_set_item_selected_state(false);
		}
	}
	return;
}


static Evas_Object *__myaccount_addaccount_icon_get( void *data,
											Evas_Object *obj, const char *part)
{
	char tempbuf[PATH_MAX];
	Evas_Object *icon = NULL;
	addaccount_list_priv *service_item = (addaccount_list_priv*)data;

	if (!service_item) {
	        MYACCOUNT_ERROR(" __myaccount_addaccount_icon_get: Data is NULL\n");
	        return NULL;
	}
	memset(tempbuf, 0, sizeof(char)*PATH_MAX);

	if (!strcmp(part, "elm.swallow.icon")) {
		if (strlen(service_item->icon_path) > 0) {
			MA_STRNCPY(tempbuf, service_item->icon_path, sizeof(tempbuf));
		} else {
			MA_STRNCPY(tempbuf, "A01_2_Icon_default.png", sizeof(tempbuf));
		}

		icon = elm_image_add(obj);
		if ((strstr(tempbuf, "/")) != NULL)
			elm_image_file_set(icon, tempbuf, NULL);
		else
			elm_image_file_set(icon, MA_IMAGE_EDJ_NAME, tempbuf);

		evas_object_size_hint_min_set(icon, ELM_SCALE_SIZE(30), ELM_SCALE_SIZE(30));
	}

	return icon;
}

void myaccount_addaccount_free_priv_data(myaccount_appdata *appdata)
{
	GList* it = NULL;
	myaccount_appdata *ad = (myaccount_appdata *)appdata;

	if(!ad) {
		MYACCOUNT_ERROR("No appdata!!\n");
		return;
	}

	for(it=ad->sorted_sp_list;it!=NULL;it=g_list_next(it)) {
		addaccount_list_priv* tmp = (addaccount_list_priv*)it->data;
		MA_MEMFREE(tmp);
	}

	ad->sorted_sp_list = NULL;
}

static addaccount_list_priv*
_myaccount_addaccount_create_priv_item()
{
	addaccount_list_priv* sp_info = NULL;
	sp_info = (addaccount_list_priv*)calloc(1,sizeof(addaccount_list_priv));
	if(!sp_info) {
		MYACCOUNT_ERROR("memory allocation fail\n");
		return NULL;
	}
	memset(sp_info, 0x00, sizeof(addaccount_list_priv));
	return sp_info;
}

static bool _myaccount_addaccount_get_account_type_info_cb(account_type_h account_type, void *user_data)
{
	GList **service_info_list = (GList **)user_data;
	char* type_buf = NULL;
	int type_int = -1, ret = -1;
	addaccount_list_priv* sp_info = NULL;

	if(account_type == NULL) {
		MYACCOUNT_ERROR(" account type handle is NULL \n");
		return FALSE;
	}

	sp_info = _myaccount_addaccount_create_priv_item();

	if(sp_info == NULL) {
		MYACCOUNT_ERROR(" sp_info is NULL \n");
		return FALSE;
	}

	ret = account_type_get_app_id(account_type, &type_buf);
	if(ret == ACCOUNT_ERROR_NONE ) {
		if(type_buf) {
			MA_STRNCPY(sp_info->package_name, type_buf,
								sizeof(sp_info->package_name));
		} else {
			MYACCOUNT_ERROR("No appid available\n");
		}
	} else {
		MYACCOUNT_ERROR("account_type_get_app_id return(%x)\n", ret);
	}
	MA_MEMFREE(type_buf);

	myaccount_appdata* ad = NULL;
	ad = myaccount_get_appdata();

	if(ad == NULL){
		MYACCOUNT_ERROR("app data is null\n");
	}

	ret = account_type_get_icon_path(account_type, &type_buf);
	if (ret == ACCOUNT_ERROR_NONE) {
		if(type_buf) {
			MA_STRNCPY(sp_info->icon_path, type_buf, sizeof(sp_info->icon_path));
		} else {
			MYACCOUNT_ERROR("No icon path available\n");
		}
	} else {
		MYACCOUNT_ERROR("account_type_get_app_id return(%x)\n", ret);
	}

	MA_MEMFREE(type_buf);

	char* provider_name = NULL;

	if (ad && ad->current_language) {
		if(!strcmp(ad->current_language, "en_US")) {
			ret = account_type_get_label_by_locale(account_type, "en_GB", &provider_name);
		}else{
			ret = account_type_get_label_by_locale(account_type, ad->current_language, &provider_name);
		}
	}

	if(ret != ACCOUNT_ERROR_NONE){
		/* fallback scenario */
		ret = account_type_get_label_by_locale(account_type, "default", &provider_name);
		if(ret != ACCOUNT_ERROR_NONE){
			MYACCOUNT_SLOGE("No service name(%s)\n", sp_info->package_name);
			MA_MEMFREE(sp_info);
			return TRUE;
		}
	}

	MA_STRNCPY(sp_info->service_name, provider_name,
						sizeof(sp_info->service_name));

	MA_MEMFREE(provider_name);

	/* Left below sorting value(service_sname) for future use */
	MA_SNPRINTF(sp_info->service_sname, sizeof(sp_info->service_sname), "%s%s", SORT_PRIOR_3, sp_info->service_name); 

	account_type_get_multiple_account_support(account_type, &type_int);
	sp_info->multiple_account_support = type_int;

	if(type_int == FALSE
		&& account_query_account_by_package_name(myaccount_add_account_by_package_name_cb, sp_info->package_name, NULL) == ACCOUNT_ERROR_NONE) {
		MA_MEMFREE(sp_info);
		return TRUE;
	}

	*service_info_list = g_list_append(*service_info_list, (void*)sp_info);

	return TRUE;
}

static void _myaccount_addaccount_get_account_type_info(const char* capability_filter, GList** sp_info_list)
{
	if(!sp_info_list) {
		MYACCOUNT_ERROR("sp_info_list is null\n");
		return;
	}

	if(capability_filter) {
		account_type_query_by_provider_feature(_myaccount_addaccount_get_account_type_info_cb, capability_filter, (void*)sp_info_list);
	} else {
		account_type_foreach_account_type_from_db(_myaccount_addaccount_get_account_type_info_cb, (void*)sp_info_list);
	}
}

static int _myaccount_ug_addaccount_compare(gconstpointer a, gconstpointer b)
{
	addaccount_list_priv* sp_info_a = (addaccount_list_priv*)a;
	addaccount_list_priv* sp_info_b = (addaccount_list_priv*)b;

	return g_ascii_strcasecmp(sp_info_a->service_sname, sp_info_b->service_sname);
}

static int __myaccount_addaccount_populate_service_info(myaccount_appdata *ad)
{
	int pkg_count = 0;

	_myaccount_addaccount_get_account_type_info(ad->capability_filter, &ad->sorted_sp_list);

	ad->sorted_sp_list = g_list_sort(ad->sorted_sp_list, (GCompareFunc)_myaccount_ug_addaccount_compare);
	pkg_count = g_list_length(ad->sorted_sp_list);

	return pkg_count;
}

static void _myaccount_ug_addaccount_gl_realized(void *data, Evas_Object *obj, void *ei)
{
	Elm_Object_Item *it = NULL;
	int total_count = 0;
	int index = 0;

	if(!ei) {
		MYACCOUNT_ERROR("Realized event info is NULL!!! \n");
		return;
	}

	it = (Elm_Object_Item *)ei;

	total_count = elm_genlist_items_count(obj);
	index = elm_genlist_item_index_get(it);

	if(index != 0 && index != total_count) {
		/*Add account items. index 1 and last are the seperators*/
		/* but you don't need to do something. winset support accessibility for default item */
	} else {
		/*Remove access for seperators*/
		elm_object_item_access_unregister(ei);
	}

	if(total_count <= 3) {
		//MYACCOUNT_VERBOSE("DO NOTHING only one add account item, total_count(%d) index(%d) !!! \n", total_count, index);
	} else {
		if (index == 1) {
			elm_object_item_signal_emit(ei, "elm,state,top", "");
		} else if (index == (total_count-2)) {
			elm_object_item_signal_emit(ei, "elm,state,bottom", "");
		} else  {
			elm_object_item_signal_emit(ei, "elm,state,center", "");
		}
	}
}

static void _myaccount_ug_addaccount_gl_deleted(void *data, Evas_Object *obj, void *ei)
{
	MYACCOUNT_ERROR("# _myaccount_ug_addaccount_gl_deleted\n");
	myaccount_appdata *ad = myaccount_get_appdata();
	myaccount_addaccount_free_priv_data(ad);
}

static Evas_Object * __myaccount_addaccount_create_genlist_layout(
												Evas_Object *navi_bar)
{
	Evas_Object *genlist;

	addacc_list_itc.item_style = "type1";
	addacc_list_itc.func.text_get = __myaccount_addaccount_gl_label_get;
	addacc_list_itc.func.content_get = __myaccount_addaccount_icon_get;
	addacc_list_itc.func.state_get = NULL;
	addacc_list_itc.func.del = NULL;

	genlist = elm_genlist_add(navi_bar);
	evas_object_smart_callback_add(genlist, "realized", _myaccount_ug_addaccount_gl_realized, NULL);
	evas_object_smart_callback_add(genlist, "delete", _myaccount_ug_addaccount_gl_deleted, NULL);
	return genlist;
}

static Evas_Object *__myaccount_addaccount_append_genlist_item(
											myaccount_appdata *ad, Evas_Object *genlist, int count)
{
	if (ad == NULL) {
		MYACCOUNT_ERROR("__myaccount_addaccount_create_genlist returns NULL\n");
		return NULL;
	}

	GList* iter;

	for(iter=ad->sorted_sp_list;iter!=NULL;iter=g_list_next(iter)) {
		addaccount_list_priv* tmp = (addaccount_list_priv*)iter->data;
		elm_genlist_item_append(genlist, &addacc_list_itc,
				(void *)tmp, NULL,
				ELM_GENLIST_ITEM_NONE,
				__myaccount_addaccount_gl_sel,
				(void *)tmp);
	}

	elm_genlist_block_count_set(genlist, count);

	return genlist;
}

static Evas_Object *__myaccount_addaccount_create_genlist(
											myaccount_appdata *ad, int count)
{
	Evas_Object *genlist;

	if (ad == NULL) {
		MYACCOUNT_ERROR("__myaccount_addaccount_create_genlist returns NULL\n");
		return NULL;
	}

	genlist = __myaccount_addaccount_create_genlist_layout(ad->navi_bar);

	return __myaccount_addaccount_append_genlist_item(ad, genlist, count);
}

static Evas_Object *__myaccount_addaccount_no_account(myaccount_appdata *ad, int count)
{
	Evas_Object *no_account;
	char no_content_text[100] = {0,};

	if (ad == NULL) {
		MYACCOUNT_ERROR("__myaccount_addaccount_no_account returns NULL\n");
		return NULL;
	}
	no_account = elm_layout_add(ad->navi_bar);
	elm_layout_file_set(no_account, MA_NO_ACCOUNT_EDJ_NAME, "nocontents_layout");

	elm_layout_theme_set(no_account, "layout", "nocontents", "default");
	evas_object_size_hint_weight_set(no_account, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(no_account, EVAS_HINT_FILL, EVAS_HINT_FILL);

	MA_SNPRINTF(no_content_text, sizeof(no_content_text), "<font=Sans:style=Regular color=#000000>%s</font>", dgettext(MA_UG_NAME, "IDS_MA_NPBODY_NO_ACCOUNT_PROVIDER_APPS_INSTALLED"));
	elm_object_part_text_set(no_account, "elm.text", no_content_text);

	elm_layout_signal_emit(no_account, "text,disabled", "");
	elm_layout_signal_emit(no_account, "align.center", "elm");

	return no_account;
}

Eina_Bool __myaccount_addaccount_quit_cb( void *data,
							Elm_Object_Item *it )
{
	int error_code = 0;
	myaccount_appdata *priv = (myaccount_appdata*)data;
	int count = -1;

	MYACCOUNT_DBUG("Add account list quit\n");

	if (!priv) {
		MYACCOUNT_ERROR("__myaccount_addaccount_quit_cb callback user data is null!!!\n");
		return EINA_TRUE;
	}

	if(priv->capability_filter && strlen(priv->capability_filter)){

		error_code = myaccount_common_get_account_cnt_by_capability(priv->capability_filter, &count);

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

	/*destroy add genlist before pop it*/
	if ((count > 0) && (priv->eMode == eMYACCOUNT_ACCOUNT_LIST)) {
		MYACCOUNT_DBUG("# count > 0, elm_naviframe_item_pop return EINA_TRUE\n");
		if (priv->add_genlist) {
			evas_object_del(priv->add_genlist);
			priv->add_genlist = NULL;
		}
		priv->prev_app_cnt = 0;

		return EINA_TRUE;
	}

	if (priv->ug) {
		ui_gadget_h ug = priv->ug;
		ug_destroy_me(ug);
		return EINA_FALSE;
	}

	if (priv->add_genlist) {
		evas_object_del(priv->add_genlist);
		priv->add_genlist = NULL;
	}
	priv->prev_app_cnt = 0;
	MYACCOUNT_DBUG("Add account list quit end\n");

	return EINA_TRUE;
}

void myaccount_addaccount_back_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	myaccount_appdata *ad = (myaccount_appdata*)data;
	elm_naviframe_item_pop(ad->navi_bar);
}

void myaccount_addaccount_genlist_create(void *data)
{
	myaccount_appdata *ad = data;
	Evas_Object *genlist;
	Evas_Object *l_button;
	int service_cnt=0;

	if (ad == NULL) {
		MYACCOUNT_ERROR("myaccount_addaccount_genlist_create myaccount_appdata is null\n");
		return;
	}

	service_cnt = __myaccount_addaccount_populate_service_info(ad);
	ad->prev_app_cnt = service_cnt;
	if(service_cnt > 0){
		ad->add_genlist = genlist = __myaccount_addaccount_create_genlist(ad, service_cnt);
	}else{
		/* If there's no account provider, display no account view */
		ad->add_genlist = genlist = __myaccount_addaccount_no_account(ad, service_cnt);
	}

	l_button = elm_button_add(ad->navi_bar);
	elm_object_style_set(l_button , "naviframe/back_btn/default");
	evas_object_smart_callback_add(l_button, "clicked", myaccount_addaccount_back_key_cb, ad);

	ad->add_nf_it = elm_naviframe_item_push(ad->navi_bar,
					dgettext(MA_UG_NAME, "IDS_MA_HEADER_ADD_ACCOUNT"),
					l_button, NULL, genlist, NULL);
	MYACCOUNT_DBUG("#elm_naviframe_item_pushed..");
	elm_naviframe_item_pop_cb_set(ad->add_nf_it, __myaccount_addaccount_quit_cb, (void*)ad);
}

void myaccount_addaccount_create_view(myaccount_appdata *ad)
{
	MYACCOUNT_DBUG("myaccount_addaccount_create_view start\n");
	if (ad->add_genlist == NULL) {
		MYACCOUNT_DBUG("Add account list start\n");
		myaccount_addaccount_free_priv_data(ad);
		myaccount_addaccount_genlist_create(ad);
	}
}

void myaccount_addaccount_refresh_item_list(myaccount_appdata *ad)
{
	int count = 0;

	if(!ad) {
		MYACCOUNT_ERROR("no appdata!\n");
		return;
	}

	if(!ad->add_genlist) {
		MYACCOUNT_ERROR("no genlist!\n");
		return;
	}

	/*check if app count changed or not, if not changed, then do not refresh current view*/
	int curr_cnt = 0;
	GList *sp_list = NULL;
	_myaccount_addaccount_get_account_type_info(ad->capability_filter, &sp_list);
	curr_cnt = g_list_length(sp_list);
	int valid_cnt = 0;
	if (ad->prev_app_cnt == curr_cnt) {
		int i = 0;
		for (i = 0; i < curr_cnt; i++) {
			addaccount_list_priv *info = g_list_nth_data(sp_list, i);
			if (!info)
				continue;
			int j = 0;
			for (j = 0; j < curr_cnt; j++) {
				addaccount_list_priv *prev_info = g_list_nth_data(ad->sorted_sp_list, j);
				if (!prev_info)
					continue;
				if (!g_strcmp0(prev_info->service_name, info->service_name)) {
					valid_cnt++;
				}
			}
		}
	}
	/*free temp data*/
	GList* it = NULL;
	for(it=sp_list;it!=NULL;it=g_list_next(it)) {
		addaccount_list_priv* tmp = (addaccount_list_priv*)it->data;
		MA_MEMFREE(tmp);
	}
	MYACCOUNT_DBUG("ad->prev_app_cnt[%d], valid_cnt[%d]", ad->prev_app_cnt, valid_cnt);
	if (ad->prev_app_cnt == valid_cnt) {
		MYACCOUNT_DBUG("no change, no need to refresh genlist items!\n");
		return;
	}

	elm_genlist_clear(ad->add_genlist);
	myaccount_addaccount_free_priv_data(ad);
	count = __myaccount_addaccount_populate_service_info(ad);
	ad->prev_app_cnt = count;
	__myaccount_addaccount_append_genlist_item(ad, ad->add_genlist, count);

	return;
}

