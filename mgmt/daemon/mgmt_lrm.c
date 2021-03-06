/*
 * Linux HA management library
 *
 * Author: Huang Zhen <zhenhltc@cn.ibm.com>
 * Copyright (c) 2005 International Business Machines
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <lha_internal.h>

#include <unistd.h>
#include <glib.h>

#include <heartbeat.h>
#include <clplumbing/cl_log.h>
#include <clplumbing/cl_syslog.h>

#include "mgmt_internal.h"
#include <lrm/lrm_api.h>



static char* on_rsc_class(char* argv[], int argc);
static char* on_rsc_type(char* argv[], int argc);
static char* on_rsc_provider(char* argv[], int argc);
static char* on_rsc_metadata(char* argv[], int argc);


ll_lrm_t* lrm = NULL;
int init_lrm(void);
void final_lrm(void);

int
init_lrm(void)
{
	int ret;
	int i, max_try = 5;

	lrm = ll_lrm_new("lrm");
	for (i = 0; i < max_try ; i++) {
		ret = lrm->lrm_ops->signon(lrm,"mgmtd");
		if (ret == HA_OK) {
			break;
		}
		mgmt_log(LOG_INFO,"login to lrm: %d, ret:%d",i,ret);
		sleep(1);
	}
	if (ret != HA_OK) {
		mgmt_log(LOG_INFO,"login to lrm failed");
		lrm->lrm_ops->delete(lrm);
		lrm = NULL;
		return -1;
	}

	reg_msg(MSG_RSC_CLASSES, on_rsc_class);
	reg_msg(MSG_RSC_TYPES, on_rsc_type);
	reg_msg(MSG_RSC_PROVIDERS, on_rsc_provider);
	reg_msg(MSG_RSC_METADATA, on_rsc_metadata);
	return 0;
}	


void
final_lrm(void)
{
	if (lrm != NULL) {
		lrm->lrm_ops->signoff(lrm);
		lrm->lrm_ops->delete(lrm);
		lrm = NULL;
	}
}

char* 
on_rsc_class(char* argv[], int argc)
{
	GList* classes;
	GList* cur;
	char* ret = cl_strdup(MSG_OK);
	classes = lrm->lrm_ops->get_rsc_class_supported(lrm);
	cur = classes;
	while (cur != NULL) {
		ret = mgmt_msg_append(ret, (char*)cur->data);
		cur = g_list_next(cur);
	}
	lrm_free_str_list(classes);
	return ret;
}

char* 
on_rsc_type(char* argv[], int argc)
{
	
	GList* types;
	GList* cur;
	char* ret;

	ARGC_CHECK(2)
	
	ret = cl_strdup(MSG_OK);
	types = lrm->lrm_ops->get_rsc_type_supported(lrm, argv[1]);
	cur = types;
	while (cur != NULL) {
		ret = mgmt_msg_append(ret, (char*)cur->data);
		cur = g_list_next(cur);
	}
	lrm_free_str_list(types);
	return ret;
}

char* 
on_rsc_provider(char* argv[], int argc)
{
	GList* providers;
	GList* cur;
	char* ret = cl_strdup(MSG_OK);
	providers = lrm->lrm_ops->get_rsc_provider_supported(lrm, argv[1], argv[2]);
	cur = providers;
	while (cur != NULL) {
		ret = mgmt_msg_append(ret, (char*)cur->data);
		cur = g_list_next(cur);
	}
	lrm_free_str_list(providers);
	return ret;
}
char*
on_rsc_metadata(char* argv[], int argc)
{
	char* ret;
	char* metadata = lrm->lrm_ops->get_rsc_type_metadata(lrm, argv[1], argv[2], argv[3]);
	if (metadata != NULL) {
		ret = cl_strdup(MSG_OK);
		ret = mgmt_msg_append(ret, metadata);
		g_free(metadata);
		return ret;
	}
	return cl_strdup(MSG_FAIL);
}
