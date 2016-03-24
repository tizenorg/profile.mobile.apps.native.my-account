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

#ifndef _MYACCOUNT_UG_ACCOUNT_LIST_H_
#define _MYACCOUNT_UG_ACCOUNT_LIST_H_

#include "myaccount_ug.h"

typedef struct __capability_data {
	char* type;
	int state;
}myaccount_capability_data;

typedef struct __account_list_priv {
	int account_id;
	char username[256];
	char display_name[256];
	char email_address[256];
	char package_name[256];
	char domain_name[60];
	char icon_path[PATH_MAX];
	char capability[128];
	int secret;
	int list_index;
	int sync_status;
	GSList *capablity_list;
	char service_sname[256];
}myaccount_list_priv;

void myaccount_list_view_create(myaccount_appdata *priv);

void myaccount_list_navi_create(myaccount_appdata *priv);

void *myaccount_init_appdata();

void *myaccount_get_appdata();

void myaccount_list_refresh_item_list(myaccount_appdata *ad);

void myaccount_ug_account_list_create(void *data);

Eina_Bool myaccount_account_list_quit_cb(void *data, Elm_Object_Item *it);


#endif /* _MYACCOUNT_UG_ACCOUNT_LIST_H_ */

