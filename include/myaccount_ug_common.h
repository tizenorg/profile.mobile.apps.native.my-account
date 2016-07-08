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

#ifndef _MYACCOUNT_UG_COMMON_H_
#define _MYACCOUNT_UG_COMMON_H_

#include <Evas.h>
#include <account.h>
#include <app.h>
#include <vconf.h>
#include <package-manager.h>
#include <glib.h>

#define ENABLE_NOTI

typedef enum {
	MYACCOUNT_TYPE_EXCHANGE = 0,
	MYACCOUNT_TYPE_GOOGLE,
	MYACCOUNT_TYPE_PICASA,
	MYACCOUNT_TYPE_YOUTUBE,
	MYACCOUNT_TYPE_OTHER,
	MYACCOUNT_TYPE_SAMSUNG,
	MYACCOUNT_TYPE_CSC,
	MYACCOUNT_TYPE_TIZEN,
	MYACCOUNT_TYPE_UNKNOWN
}MyAccountProviderType;

typedef enum {
	MYACCOUNT_REQUEST_SIGNIN = 0,
	MYACCOUNT_REQUEST_VIEW
}MyAccountRequestViewType;

void *myaccount_init_appdata();

void myaccount_destroy_appdata();

void *myaccount_get_appdata();

void myaccount_common_set_item_selected_state(bool val);

bool myaccount_common_get_item_selected_state();

void myaccount_common_get_icon_by_name(char *account_name,
											char *icon_path);

MyAccountProviderType myaccount_common_get_provider_type(
												char *package_name);

void myaccount_common_lowercase(char* src, char* dest);

void myaccount_common_delete_modal_popup(Evas_Object *popup);

Eina_Bool myaccount_common_modal_popup_timer_cb(gpointer data);

int myaccount_common_launch_application(MyAccountRequestViewType
													request_type,
													char *package_name,
													char *username,
													int account_id,
													void *data );

int myaccount_common_get_all_account_cnt(int *count, bool include_secret);

int myaccount_common_get_account_cnt_by_capability(const char* capability_type, int *count);

int myaccount_common_get_account_cnt_by_appid(const char* appid, int *count);

void myaccount_common_handle_notification(const char* event_type);

void myaccount_common_listen_pkgmgr_for_pkg_uninstall();

int myaccount_common_get_account_type_count(const char* capability_filter);

#define MA_UG_NAME "setting-myaccount-efl"

#define MA_IMAGE_EDJ_NAME "/usr/apps/setting-myaccount-efl/res/edje/setting-myaccount-efl/myaccount_edc_images.edj"

#define MA_NO_ACCOUNT_EDJ_NAME "/usr/apps/setting-myaccount-efl/res/edje/setting-myaccount-efl/myaccount_nocontent.edj"

#define BUFSIZE 1024

#define MA_SNPRINTF(dest,size,format,arg...)	\
	do { \
			snprintf(dest,size-1,format,##arg); \
	}while(0)
	/*	If the same pointer is passed to free twice, 	known as a double free. To avoid this, set pointers to
NULL after passing 	them to free: free(NULL) is safe (it does nothing).
	 */

#define MA_STRNCPY(dest,src,size)	\
		do { \
			if(src != NULL && dest != NULL && size > 0) \
			{	\
				strncpy(dest,src,size-1); \
			}	\
		}while(0);

#define MA_MEMFREE(ptr)	\
		if (ptr != NULL) {	\
			free(ptr);	\
			ptr = NULL; \
		}	\

#endif /* _MYACCOUNT_UG_COMMON_H_ */

