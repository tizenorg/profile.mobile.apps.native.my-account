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

#include <vconf.h>
#include "myaccount_ug_account_list.h"
#include "myaccount_ug_addaccount.h"
#include "myaccount_ug_common.h"

/* left for future use */
#define SORT_PRIOR_1 "1_"
#define SORT_PRIOR_2 "2_"
#define SORT_PRIOR_3 "3_"

static Elm_Genlist_Item_Class account_list_itc;

static int account_index = 0;

#define TOOLBAR_HEIGHT 		0

static void __myaccount_account_list_addaccount_cb( void *data, Evas_Object *obj, void* event_info );
static void _myaccount_ug_account_list_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv);
//static void _myaccount_ug_account_list_result_cb(ui_gadget_h ug, service_h app_control, void *priv);
static void _myaccount_ug_account_list_result_cb(ui_gadget_h ug, app_control_h app_control, void *priv);
static void _myaccount_ug_account_list_destroy_cb(ui_gadget_h ug, void *priv);
static void __myaccount_account_list_append_genlist(myaccount_appdata *ad, int count);
static void __rotate_more_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info);

static char *__myaccount_account_list_get_capablity_string_value(const char* capability_type)
{
	if (!strcmp(capability_type, "http://tizen.org/account/capability/contact"))
		return dgettext(MA_UG_NAME, "IDS_PB_TAB_CONTACTS");
	else if (!strcmp(capability_type, "http://tizen.org/account/capability/calendar"))
		return dgettext("sys_string", "IDS_COM_BODY_S_PLANNER");
	else if (!strcmp(capability_type ,"http://tizen.org/account/capability/photo"))
		return dgettext(MA_UG_NAME, "IDS_COM_BODY_GALLERY");
	else if (!strcmp(capability_type ,"http://tizen.org/account/capability/video"))
		return dgettext(MA_UG_NAME, "IDS_ST_BODY_VIDEOS");
	else if (!strcmp(capability_type ,"http://tizen.org/account/capability/email"))
		return dgettext(MA_UG_NAME, "IDS_COM_BODY_EMAIL");
 	else if (!strcmp(capability_type ,"http://tizen.org/account/capability/music"))
		return dgettext("sys_string", "IDS_COM_BODY_MUSIC");
 	else if (!strcmp(capability_type ,"http://tizen.org/account/capability/message"))
		return dgettext(MA_UG_NAME, "IDS_COM_BODY_MESSAGES");
 	else
		return NULL;//return dgettext(MA_UG_NAME, "IDS_COM_POP_UNKNOWN");
}

bool _myaccount_account_list_get_capablity_text(const char* capability_type,
										account_capability_state_e capability_state,
										void *user_data)
{
	/* get capabilities of the account*/
	char *capability_string = NULL;
	myaccount_list_priv *account_info = (myaccount_list_priv*)user_data;
	char *capability_textbuf = account_info->capability;

	if(!account_info) {
		return false;
	}

	myaccount_capability_data *cap_data = (myaccount_capability_data*)malloc(sizeof(myaccount_capability_data));
	if (!cap_data) {
		MYACCOUNT_ERROR("malloc failed\n");
		return true;
	}
	memset(cap_data, 0, sizeof(myaccount_capability_data));

	cap_data->type = strdup(capability_type);
	cap_data->state = capability_state;

	account_info->capablity_list = g_slist_append(account_info->capablity_list, (gpointer)cap_data);

	if (capability_state == ACCOUNT_CAPABILITY_ENABLED) {
		capability_string = __myaccount_account_list_get_capablity_string_value(
															capability_type);

		if (capability_string != NULL) {
			if(strlen(capability_textbuf) > 0) {
				char *temp2 = strdup(capability_textbuf);
				if (!temp2) {
					MYACCOUNT_ERROR("strdup returns NULL\n");
					return false;
				}
				MA_SNPRINTF(capability_textbuf, 127, "%s, %s", temp2, capability_string);
				MA_MEMFREE(temp2);
			} else {
				MA_SNPRINTF(capability_textbuf, 127, "%s%s", capability_textbuf,
															capability_string);
			}
		}

	}

	return true;
}

void myaccount_account_list_free_priv_data(myaccount_appdata *appdata)
{
	GList* it = NULL;
	myaccount_appdata *ad = (myaccount_appdata *)appdata;

	if(!ad) {
		MYACCOUNT_ERROR("No appdata!!\n");
		return;
	}

	for(it=ad->sorted_account_list;it!=NULL;it=g_list_next(it)) {
		myaccount_list_priv* tmp = (myaccount_list_priv*)it->data;
		MA_MEMFREE(tmp);
	}

	if(ad->account_info_list) {
		g_list_free(ad->account_info_list);
		ad->account_info_list = NULL;
	}

	ad->sorted_account_list = NULL;

}

static myaccount_list_priv*
_myaccount_account_list_create_priv_item()
{
	myaccount_list_priv* account_info = NULL;
	account_info = (myaccount_list_priv*)calloc(1,sizeof(myaccount_list_priv));
	if(!account_info) {
		MYACCOUNT_ERROR("memory allocation fail\n");
	}
	return account_info;
}


bool _myaccount_account_list_account_info_cb(account_h account,
														void *user_data)
{
	GList **account_info_list = (GList **)user_data;
	int error_code = -1;
	myaccount_list_priv* account_info = NULL;

	if (!account) {
		MYACCOUNT_ERROR("_myaccount_account_list_account_info_cb:account handle is NULL\n");
		return false;
	} else {

		account_info = _myaccount_account_list_create_priv_item();
		if ( account_info == NULL ) {
			MYACCOUNT_ERROR("_myaccount_account_list_create_priv_item fail\n");
			return false;
		}

		char *temptxt = NULL;
		int id = -1;
		account_secrecy_state_e secret;

		error_code = account_get_user_name(account, &temptxt);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("account_get_user_name: Failed \n");
		} else if (temptxt && strlen(temptxt)) {
			MA_STRNCPY(account_info->username, temptxt,
									sizeof(account_info->username));
		}
		MA_MEMFREE(temptxt);
		error_code = account_get_display_name(account, &temptxt);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("account_get_display_name: Failed \n");
		} else if (temptxt && strlen(temptxt)) {
			MA_STRNCPY(account_info->display_name, temptxt,
									sizeof(account_info->display_name));
		}
		MA_MEMFREE(temptxt);
		error_code = account_get_email_address(account, &temptxt);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("account_get_email_address: Failed \n");
		} else if (temptxt && strlen(temptxt)) {
			MA_STRNCPY(account_info->email_address, temptxt,
									sizeof(account_info->email_address));
		}

		MA_MEMFREE(temptxt);
		error_code = account_get_package_name(account, &temptxt);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("account_get_package_name: Failed \n");
		} else if (temptxt && strlen(temptxt)) {
			MA_STRNCPY(account_info->package_name, temptxt,
									sizeof(account_info->package_name));
		}
		MA_MEMFREE(temptxt);
		error_code = account_get_account_id(account, &id);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("account_get_account_id: Failed \n");
		} else if (id > 0) {
			account_info->account_id = id;
		}
		error_code = account_get_secret(account, &secret);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("account_get_secret: Failed \n");
		} else {
			account_info->secret = secret;
		}
		error_code = account_get_domain_name(account, &temptxt);
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("account_get_domain_name: Failed \n");
		} else if (temptxt && strlen(temptxt)) {
			MA_STRNCPY(account_info->domain_name, temptxt,
									sizeof(account_info->domain_name));
		}

		MA_MEMFREE(temptxt);
		error_code = account_get_icon_path(account, &temptxt);
		if (error_code != ACCOUNT_ERROR_NONE) {
				MYACCOUNT_ERROR("account_get_icon_path: Failed \n");
		} else if (temptxt && strlen(temptxt)) {
			MA_STRNCPY(account_info->icon_path, temptxt, sizeof(account_info->icon_path));
		} else {

			account_type_h account_type = NULL;

			error_code = account_type_create(&account_type);

			if(error_code == ACCOUNT_ERROR_NONE){

				error_code = account_type_query_by_app_id((const char*)account_info->package_name, &account_type);

				if(error_code == ACCOUNT_ERROR_NONE){
					char* provider_icon = NULL;
					error_code = account_type_get_icon_path(account_type, &provider_icon);

					if(error_code != ACCOUNT_ERROR_NONE){
						MYACCOUNT_ERROR("account provider icon get Failed \n");
					} else if (provider_icon && strlen(provider_icon)) {
						MA_STRNCPY(account_info->icon_path, provider_icon, sizeof(account_info->icon_path));
					} else {
						MYACCOUNT_DBUG("account provider icon is not set \n");
					}
					MA_MEMFREE(provider_icon);
				}

				error_code = account_type_destroy(account_type);
				if(error_code != ACCOUNT_ERROR_NONE){
					MYACCOUNT_ERROR("type_destroy destroy Failed \n");
				}
			}
		}

		MA_MEMFREE(temptxt);
		if(!strlen(account_info->icon_path))
			myaccount_common_get_icon_by_name(account_info->domain_name ,
													account_info->icon_path);

		error_code = account_get_sync_support(account,
						(account_sync_state_e *)&(account_info->sync_status));
		if (error_code != ACCOUNT_ERROR_NONE) {
			MYACCOUNT_ERROR("account_get_sync_support: Failed \n");
		}

		error_code = account_get_capability_all(account,
									_myaccount_account_list_get_capablity_text,
									(void *)account_info);
		if (error_code != ACCOUNT_ERROR_NONE)
			MYACCOUNT_ERROR("account_get_capability: Failed \n");


		/* No need to sort, sort as the order user register */
		/* Left below sorting value(service_sname) for future use */
		MA_SNPRINTF(account_info->service_sname, sizeof(account_info->service_sname), "%s", SORT_PRIOR_3);

		*account_info_list = g_list_append(*account_info_list, (void*)account_info);
		account_index++;
	}
	return true;
}

static int _myaccount_ug_account_list_compare(gconstpointer a, gconstpointer b)
{
	myaccount_list_priv* account_info_a = (myaccount_list_priv*)a;
	myaccount_list_priv* account_info_b = (myaccount_list_priv*)b;

	return g_ascii_strcasecmp(account_info_a->service_sname, account_info_b->service_sname);
}

static int __myaccount_account_list_populate_account_info(myaccount_appdata *ad)
{
	int count=0;

	account_foreach_account_from_db(_myaccount_account_list_account_info_cb,
											&ad->account_info_list);

	ad->sorted_account_list = g_list_sort(ad->account_info_list, (GCompareFunc)_myaccount_ug_account_list_compare);
	count = g_list_length(ad->sorted_account_list);

	return count;
}

Eina_Bool myaccount_account_list_quit_cb(void *data, Elm_Object_Item *it)
{
	int error_code=0;
	myaccount_appdata *priv = (myaccount_appdata*)data;

	MYACCOUNT_DBUG("account list view quit\n");

	if (!priv) {
		MYACCOUNT_ERROR("myaccount_account_list_quit_cb callback user data is null!!!\n");
		return EINA_TRUE;
	}

	if(priv->popup != NULL)
	{
		evas_object_del(priv->popup);
		priv->popup = NULL;
	}

	myaccount_account_list_free_priv_data(priv);

	account_index = 0;

	if (priv->ug) {
		ui_gadget_h ug = priv->ug;
		error_code = ug_destroy_me(ug);
		MYACCOUNT_SLOGD("myaccount_account_list_quit_cb callback : return = %d!!!\n",
												error_code);
		return EINA_FALSE;
	}
	return EINA_TRUE;
}

void myaccount_account_list_back_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	myaccount_appdata *ad = (myaccount_appdata*)data;
	elm_naviframe_item_pop(ad->navi_bar);
}

static void __myaccount_account_list_addaccount_cb( void *data,
											Evas_Object *obj, void *event_info )
{
	myaccount_appdata *priv = (myaccount_appdata*)data;
	myaccount_addaccount_create_view(priv);
	if(priv->popup != NULL)
	{
		evas_object_del(priv->popup);
		priv->popup = NULL;
	}
}

static char *__myaccount_account_list_gl_label_get(void *data,
										Evas_Object *obj, const char *part)
{
	myaccount_list_priv *account_info_item = (myaccount_list_priv*)data;
	char domain_name_lower[128] = {0,};

	if (!account_info_item) {
		MYACCOUNT_ERROR(" __myaccount_account_list_gl_label_get: Data is NULL\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text")) {
		myaccount_common_lowercase(account_info_item->domain_name,
												domain_name_lower);

		if (strlen(account_info_item->display_name) > 0)
			return strdup(account_info_item->display_name);
		else if (strlen(account_info_item->email_address) > 0)
			return strdup(account_info_item->email_address);
		else if (strlen(account_info_item->username) > 0)
			return strdup(account_info_item->username);
		else
			return strdup("Unknown");
	} else if (!strcmp(part, "elm.text.sub")) {
		if (strlen(account_info_item->capability) <=0 )
			return strdup(dgettext(MA_UG_NAME, "IDS_MA_BODY_SIGNED_IN"));
		else
			return strdup(account_info_item->capability);
	}

	return NULL;
}


static Evas_Object *__myaccount_account_list_gl_icon_get(void *data,
											Evas_Object *obj, const char *part)
{
	char tempbuf[PATH_MAX];
	Evas_Object *icon = NULL;
	myaccount_list_priv *account_info_item = (myaccount_list_priv*)data;

	if (!account_info_item) {
                MYACCOUNT_ERROR("__myaccount_account_list_gl_icon_get: Data is NULL\n");
                return NULL;
	}
	memset(tempbuf, 0, sizeof(char)*PATH_MAX);

	if (!strcmp(part, "elm.swallow.icon")) {
		if (strlen(account_info_item->icon_path) > 0) {
			MA_SNPRINTF(tempbuf, sizeof(tempbuf), "%s", account_info_item->icon_path);
		} else {
			MA_SNPRINTF(tempbuf, sizeof(tempbuf), "%s", "A01_2_Icon_default.png");
		}

		icon = elm_image_add(obj);
		if (strstr(tempbuf, "/"))
			elm_image_file_set(icon, tempbuf, NULL);
		else
			elm_image_file_set(icon, MA_IMAGE_EDJ_NAME, tempbuf);

		evas_object_size_hint_min_set(icon, ELM_SCALE_SIZE(30), ELM_SCALE_SIZE(30));
	}

	if (!strcmp(part, "elm.swallow.end")) {
		if (account_info_item->sync_status == ACCOUNT_SYNC_NOT_SUPPORT)
			return NULL;
		MA_SNPRINTF(tempbuf, sizeof(tempbuf), "%s", "account_icon_syncing.png");

		if(strlen(tempbuf) > 0) {
			icon = elm_image_add(obj);
			if (strstr(tempbuf, "/"))
				elm_image_file_set(icon, tempbuf, NULL);
			else
				elm_image_file_set(icon, MA_IMAGE_EDJ_NAME, tempbuf);

			evas_object_color_set(icon, 67, 116, 217, 255);
			evas_object_size_hint_min_set(icon, ELM_SCALE_SIZE(30), ELM_SCALE_SIZE(30));
		}
	}

	return icon;
}

static void _myaccount_ug_account_list_layout_cb(ui_gadget_h ug,
										enum ug_mode mode, void *priv)
{
	Evas_Object *base;
	myaccount_appdata *ad;

	if (!ug||!priv) {
		MYACCOUNT_ERROR("layout cb error ug=%p, priv=%p\n", ug, priv);
		return;
	}
	ad = (myaccount_appdata*)priv;
	base = ug_get_layout(ug);
	if (!base)
		MYACCOUNT_ERROR("layout cb base is null\n");

	switch (mode) {
	case UG_MODE_FRAMEVIEW:
			elm_object_part_content_set (ad->layout_main, "content", base);
			break;
	case UG_MODE_FULLVIEW:
			evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
			evas_object_show(base);
			break;
	default:
			break;
	}
}

//static void _myaccount_ug_account_list_result_cb(ui_gadget_h ug, service_h app_control, void *priv)
static void _myaccount_ug_account_list_result_cb(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	//MYACCOUNT_VERBOSE("_myaccount_ug_account_list_result_cb\n");
}

static void _myaccount_ug_account_list_destroy_cb(ui_gadget_h ug, void *priv)
{
	myaccount_appdata *mydata = (myaccount_appdata*)priv;

	if (!ug||!mydata) {
		MYACCOUNT_ERROR("destroy cb error ug=%p, priv=%p\n", ug, mydata);
		return;
	}
	myaccount_common_set_item_selected_state(false);

	ug_destroy(ug);
	MYACCOUNT_SLOGD("_myaccount_ug_account_list_destroy_cb");
#ifndef ENABLE_NOTI
	myaccount_common_handle_notification(NULL);
#endif
}

static void __myaccount_accountlist_set_ug_cbs(struct ug_cbs *cbs,
												myaccount_appdata *priv)
{
	if (!cbs || !priv) {
		MYACCOUNT_ERROR("__myaccount_addaccount_ug_create_email: Invalid param cbs=%p, priv=%p\n",
								cbs, priv);
		return;
	}
	cbs->layout_cb = _myaccount_ug_account_list_layout_cb;
	cbs->result_cb = _myaccount_ug_account_list_result_cb;
	cbs->destroy_cb = _myaccount_ug_account_list_destroy_cb;
	cbs->priv = (void *)priv;
}

static void __myaccount_ug_list_call_ug(void *data, myaccount_list_priv *account)
{
	myaccount_appdata *ad = (myaccount_appdata*)data;
	ui_gadget_h ug = NULL;
	struct ug_cbs cbs = {0,};
	app_control_h app_control;
	__attribute__((__unused__)) int ret = APP_CONTROL_ERROR_NONE;

	if (!account) {
		MYACCOUNT_ERROR("__myaccount_ug_list_call_ug : account info NULL\n");
		return;
	}

	__myaccount_accountlist_set_ug_cbs(&cbs, ad);

	ret = app_control_create(&app_control);
	if (ret != ACCOUNT_ERROR_NONE) {
		MYACCOUNT_ERROR("account_delete_from_db_by_id error = %d", ret);
		return;
	}

	ret = myaccount_common_launch_application(MYACCOUNT_REQUEST_VIEW,
								strdup(account->package_name),
								strdup(account->username),
								account->account_id,
								ad);	
	if (ret != ACCOUNT_ERROR_NONE) {
		MYACCOUNT_ERROR("myaccount_common_launch_application error = %d", ret);
		return;
	}	

	ad->ug_called = ug;
	app_control_destroy(app_control);

}

static void __myaccount_account_list_gl_sel(void *data,
										Evas_Object *obj, void *event_info)
{
	myaccount_appdata *ad = myaccount_get_appdata();
	if( myaccount_common_get_item_selected_state() ) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}
	myaccount_common_set_item_selected_state(true);
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	MYACCOUNT_SLOGD("__myaccount_account_list_gl_sel data=%p, obj=%p, event_info=%p\n",
									data, obj, event_info);
	myaccount_list_priv *account = (myaccount_list_priv*)data;
	MYACCOUNT_SLOGD("__myaccount_account_list_gl_sel : account name : %s\n",
									account->domain_name);

	if (item != NULL)
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	__myaccount_ug_list_call_ug((myaccount_appdata*)ad, account);
	return;
}

static void __myaccount_account_list_item_selected(void *data,
										Evas_Object *obj, void *event_info)
{
	//MYACCOUNT_VERBOSE("__myaccount_account_list_item_selected \n");
}

static void __myaccount_account_list_append_genlist(myaccount_appdata *ad,
														int count)
{
	MYACCOUNT_SLOGD ("\n count received in __myaccount_account_list_append_genlist = %d \n",
										count);
	bool supported = false;

	if (count > 0) {
		GList* iter;
		for(iter=ad->sorted_account_list;iter!=NULL;iter=g_list_next(iter)) {
			myaccount_list_priv* tmp = (myaccount_list_priv*)iter->data;
			supported = false;
			if (tmp->secret == ACCOUNT_SECRECY_VISIBLE) {
				if (ad->capability_filter) {
					GSList* siter;
					myaccount_list_priv *acc_info = NULL;
					acc_info = tmp;

					for (siter = acc_info->capablity_list; siter != NULL; siter = g_slist_next(siter)) {
						myaccount_capability_data *cap_data = (myaccount_capability_data*)siter->data;

						if (!strcmp(cap_data->type, ad->capability_filter)) {
							supported = true;
							break;
						}
					}

					if (supported) {
						elm_genlist_item_append(ad->account_genlist,
										&account_list_itc,
										(void *)tmp, NULL,
										ELM_GENLIST_ITEM_NONE,
										__myaccount_account_list_gl_sel,
										(void *)tmp);

					}
				} else {
					elm_genlist_item_append(ad->account_genlist,
									&account_list_itc,
									(void *)tmp, NULL,
									ELM_GENLIST_ITEM_NONE,
									__myaccount_account_list_gl_sel,
									(void *)tmp);
				}
			}
		}

	} else {
		myaccount_addaccount_create_view(ad);
	}
	return;
}

static void _myaccount_ug_account_gl_realized(void *data, Evas_Object *obj, void *ei)
{
	/* Add logic for list item when that displayed */
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

	if(total_count <= 4)
	{
		//MYACCOUNT_INFO("DO NOTHING only one account exist, total_count(%d) index(%d) !!! \n", total_count, index);
	} else {
		if (index == 1) {
		} else if (index == 2) {
			elm_object_item_signal_emit(ei, "elm,state,top", "");
		} else if (index == (total_count-2)) {
			elm_object_item_signal_emit(ei, "elm,state,bottom", "");
		} else  {
			elm_object_item_signal_emit(ei, "elm,state,center", "");
		}
	}
}

static Evas_Object *__myaccount_account_list_create_genlist(
													myaccount_appdata *ad)
{
	Evas_Object *genlist;

	if (ad == NULL) {
		MYACCOUNT_ERROR("__myaccount_account_list_create_genlist returns NULL\n");
		return NULL;
	}

	int account_count = 0;
	account_count = __myaccount_account_list_populate_account_info(ad);

	account_list_itc.item_style = "type1";
	account_list_itc.func.text_get = __myaccount_account_list_gl_label_get;
	account_list_itc.func.content_get = __myaccount_account_list_gl_icon_get;
	account_list_itc.func.state_get = NULL;
	account_list_itc.func.del = NULL;

	ad->account_genlist = genlist = elm_genlist_add(ad->navi_bar);
	evas_object_smart_callback_add(genlist, "realized", _myaccount_ug_account_gl_realized, NULL);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_mode_set(genlist, EINA_TRUE);

	evas_object_smart_callback_add(genlist, "selected",
					__myaccount_account_list_item_selected,
					(myaccount_appdata*) ad);

	__myaccount_account_list_append_genlist(ad, account_count);

	return genlist;
}

static void __move_more_ctxpopup(myaccount_appdata *ad)
{
	MYACCOUNT_DBUG("__move_more_ctxpopup");

	Evas_Coord w = 0;
	Evas_Coord h = 0;

	elm_win_screen_size_get(ad->win_main, NULL, NULL, &w, &h);
	int pos = elm_win_rotation_get(ad->win_main);

	switch (pos) {
		case 0:
		case 180:
			 evas_object_move(ad->popup, (w / 2), h);
			break;
		case 90:
			evas_object_move(ad->popup,  (h / 2), w);
			break;
		case 270:
			evas_object_move(ad->popup, (h / 2), w);
			break;
	}


}

static void __rotate_more_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	MYACCOUNT_DBUG("__rotate_more_ctxpopup_cb");
	myaccount_appdata *ad = data;

	__move_more_ctxpopup(ad);
}
static void _delete_ctxpopup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MYACCOUNT_DBUG("_delete_toolbar_ctxpopup_cb");
	myaccount_appdata *ad = data;
	if (elm_win_wm_rotation_supported_get(ad->win_main)) {
		if (ad->popup) {
			evas_object_smart_callback_del(elm_object_top_widget_get(ad->popup), "wm,rotation,changed", __rotate_more_ctxpopup_cb);
		}
	}
	if (ad->popup) {
		eext_object_event_callback_del(ad->popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb);
		eext_object_event_callback_del(ad->popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb);		
		
		evas_object_event_callback_del(ad->popup, EVAS_CALLBACK_DEL, _delete_ctxpopup_cb);
	}
}

static void __create_more_ctxpopup(myaccount_appdata *ad)
{
	MYACCOUNT_DBUG("__create_more_ctxpopup");

	Evas_Object *popup = elm_ctxpopup_add(ad->navi_bar);
	ad->popup = popup;

	elm_object_style_set(popup, "more/default");
	elm_ctxpopup_auto_hide_disabled_set(popup, EINA_TRUE);
	if (elm_win_wm_rotation_supported_get(ad->win_main)) {
		evas_object_smart_callback_add(elm_object_top_widget_get(popup), "wm,rotation,changed", __rotate_more_ctxpopup_cb, ad);
	}
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _delete_ctxpopup_cb, ad);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, popup);

	elm_ctxpopup_item_append(popup, dgettext(MA_UG_NAME,"IDS_PB_BODY_ADD_ACCOUNT"), NULL, __myaccount_account_list_addaccount_cb, ad);

	elm_ctxpopup_direction_priority_set(popup, ELM_CTXPOPUP_DIRECTION_UP,
						ELM_CTXPOPUP_DIRECTION_LEFT,
						ELM_CTXPOPUP_DIRECTION_RIGHT,
						ELM_CTXPOPUP_DIRECTION_DOWN);

	__move_more_ctxpopup(ad);
	evas_object_show(popup);
}

static void __delete_more_cb(void *data)
{
	MYACCOUNT_DBUG("__delete_more_cb");
	myaccount_appdata *ad = data;
	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
}

static void __more_cb(void *data, Evas_Object *obj, void *event_info)
{
	MYACCOUNT_DBUG("__more_cb");
	myaccount_appdata *ad = data;
	__delete_more_cb(data);
	__create_more_ctxpopup(ad);
}

void myaccount_ug_account_list_create(void *data)
{
	myaccount_appdata *ad = data;
	Evas_Object *genlist = NULL;
	Evas_Object *back_btn = NULL;
	Evas_Object *more_button = NULL;	

	if (ad == NULL) {
		MYACCOUNT_ERROR("myaccount_ug_account_list_create myaccount_appdata is null\n");
		return;
	}

	genlist = __myaccount_account_list_create_genlist(ad);

	back_btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(back_btn, "clicked", myaccount_account_list_back_key_cb, ad);

	ad->nf_it = elm_naviframe_item_push(ad->navi_bar,
							dgettext(MA_UG_NAME,
							"IDS_ST_BODY_ACCOUNT_LIST"),
							back_btn, NULL, genlist, NULL);

	elm_naviframe_item_pop_cb_set(ad->nf_it, myaccount_account_list_quit_cb, (void*)ad);

	int count = 0;
	count = myaccount_common_get_account_type_count(ad->capability_filter);

	MYACCOUNT_DBUG("TOTAL avaiable sp count %d\n", count);

	eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	more_button = elm_button_add(ad->navi_bar);
	elm_object_style_set(more_button, "naviframe/more/default");
	evas_object_smart_callback_add(more_button, "clicked", __more_cb, ad);
	elm_object_item_part_content_set(ad->nf_it, "toolbar_more_btn", more_button);	
}

static Evas_Object *__myaccount_account_list_create_bg(Evas_Object *parent)
{
	Evas_Object *bg = elm_bg_add(parent);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(bg);
    return bg;
}

static Evas_Object *__myaccount_account_list_create_main_layout(
													Evas_Object *parent,
													Evas_Object *bg)
{
	Evas_Object *layout;

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set( layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	edje_object_signal_emit(_EDJ(layout), "elm,state,show,indicator", "elm");
	edje_object_signal_emit(_EDJ(layout), "elm,state,show,content", "elm");
	elm_object_part_content_set ( layout, "elm.swallow.bg", bg);

	evas_object_show( layout );

	return layout;
}

static Evas_Object *__myaccount_account_list_create_navi_layout(
															Evas_Object *parent)
{
	Evas_Object *navi_bar;

	navi_bar = elm_naviframe_add(parent);
	elm_object_part_content_set ( parent, "elm.swallow.content", navi_bar );
	evas_object_show(navi_bar);

	return navi_bar;
}

static void __myaccount_account_list_init_main_view(myaccount_appdata *ad)
{
	ad->bg = __myaccount_account_list_create_bg(ad->win_main);
	ad->layout_main = __myaccount_account_list_create_main_layout(ad->win_main, ad->bg);
	ad->base = ad->layout_main;
}

void myaccount_list_navi_create(myaccount_appdata *ad)
{
	ad->navi_bar = __myaccount_account_list_create_navi_layout(ad->layout_main);

	eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_BACK, myaccount_account_list_back_key_cb, (void*)ad);
}

void myaccount_list_view_create(myaccount_appdata *priv)
{
	__attribute__((__unused__)) struct ug_cbs cbs = {0,};
	cbs.layout_cb = _myaccount_ug_account_list_layout_cb;
	cbs.result_cb = _myaccount_ug_account_list_result_cb;
	cbs.destroy_cb = _myaccount_ug_account_list_destroy_cb;
	cbs.priv = (void *)priv;

	MYACCOUNT_DBUG("Account list start\n");
	__myaccount_account_list_init_main_view(priv);
}

void myaccount_list_refresh_item_list(myaccount_appdata *ad)
{
	int count = 0;

	if(!ad) {
		MYACCOUNT_ERROR("no appdata!\n");
		return;
	}

	if(!ad->account_genlist) {
		MYACCOUNT_ERROR("no genlist!\n");
		return;
	}

	MYACCOUNT_DBUG("Account list start\n");

	elm_genlist_clear(ad->account_genlist);

	myaccount_account_list_free_priv_data(ad);

	account_index = 0;
	count = __myaccount_account_list_populate_account_info(ad);

	__myaccount_account_list_append_genlist(ad, count);

	return;
}

