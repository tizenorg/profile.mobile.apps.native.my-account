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

#ifndef _MYACCOUNT_UG_ADD_ACCOUNT_H_
#define _MYACCOUNT_UG_ADD_ACCOUNT_H_

#include "myaccount_ug.h"
#include "myaccount_ug_common.h"

typedef struct __addaccount_list_priv {
	char service_name[256];
	char service_sname[256];
	char package_name[256];
	char icon_path[1024];
	MyAccountProviderType sp_type;
	bool multiple_account_support;
}addaccount_list_priv;

void *myaccount_addaccount_create(ui_gadget_h ug,
						enum ug_mode mode, app_control_h data, void *priv);

void myaccount_addaccount_create_view(myaccount_appdata *ad);

void myaccount_addaccount_refresh_item_list(myaccount_appdata *ad);

void myaccount_addaccount_free_priv_data(myaccount_appdata *appdata);

void *myaccount_init_appdata();

void *myaccount_get_appdata();

#endif /* _MYACCOUNT_UG_ADD_ACCOUNT_H_ */

