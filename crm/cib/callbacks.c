/* 
 * Copyright (C) 2004 Andrew Beekhof <andrew@beekhof.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include <sys/param.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <hb_api.h>
#include <clplumbing/uids.h>
#include <clplumbing/cl_uuid.h>
#include <clplumbing/cl_malloc.h>
#include <clplumbing/Gmain_timeout.h>

#include <crm/crm.h>
#include <crm/cib.h>
#include <crm/msg_xml.h>
#include <crm/common/ipc.h>
#include <crm/common/ctrl.h>
#include <crm/common/xml.h>
#include <crm/common/msg.h>

#include <cibio.h>
#include <callbacks.h>
#include <cibmessages.h>
#include <cibprimatives.h>
#include <notify.h>


extern GMainLoop*  mainloop;
extern gboolean cib_shutdown_flag;
extern gboolean stand_alone;
extern const char* cib_root;

extern enum cib_errors cib_update_counter(
	crm_data_t *xml_obj, const char *field, gboolean reset);

extern void GHFunc_count_peers(
	gpointer key, gpointer value, gpointer user_data);
extern enum cib_errors revision_check(
	crm_data_t *cib_update, crm_data_t *cib_copy, int flags);

void initiate_exit(void);
void terminate_ha_connection(const char *caller);
gint cib_GCompareFunc(gconstpointer a, gconstpointer b);
gboolean cib_msg_timeout(gpointer data);
void cib_GHFunc(gpointer key, gpointer value, gpointer user_data);
gboolean can_write(int flags);
HA_Message *cib_msg_copy(HA_Message *msg, gboolean with_data);
gboolean ccm_manual_check(gpointer data);
void send_cib_replace(const HA_Message *sync_request, const char *host);
void cib_process_request(
	HA_Message *request, gboolean privileged, gboolean force_synchronous,
	gboolean from_peer, cib_client_t *cib_client);
HA_Message *cib_construct_reply(HA_Message *request, HA_Message *output, int rc);

gboolean syncd_once = FALSE;

extern GHashTable *client_list;
extern GHashTable *ccm_membership;
extern GHashTable *peer_hash;

int        next_client_id  = 0;
gboolean   cib_is_master   = FALSE;
gboolean   cib_have_quorum = FALSE;
char *     ccm_transition_id = NULL;
extern const char *cib_our_uname;
extern ll_cluster_t *hb_conn;
extern unsigned long cib_num_ops, cib_num_local, cib_num_updates, cib_num_fail;
extern unsigned long cib_bad_connects, cib_num_timeouts;
extern longclock_t cib_call_time;
extern enum cib_errors cib_status;

static HA_Message *
cib_prepare_common(HA_Message *root, const char *section)
{
	HA_Message *data = NULL;
	
	/* extract the CIB from the fragment */
	if(root == NULL) {
		return NULL;

	} else if(safe_str_eq(crm_element_name(root), XML_TAG_FRAGMENT)
		|| safe_str_eq(crm_element_name(root), F_CIB_CALLDATA)) {
		data = find_xml_node(root, XML_TAG_CIB, TRUE);
		if(data != NULL) {
			crm_debug_3("Extracted CIB from %s", TYPE(root));
		} else {
			crm_log_xml_debug_4(root, "No CIB");
		}
		
	} else {
		data = root;
	}

	/* grab the section specified for the command */
	if(section != NULL
	   && data != NULL
	   && safe_str_eq(crm_element_name(data), XML_TAG_CIB)){
		int rc = revision_check(data, the_cib, 0/* call_options */);
		if(rc == cib_ok) {
			data = get_object_root(section, data);
			if(data != NULL) {
				crm_debug_3("Extracted %s from CIB", section);
			} else {
				crm_log_xml_debug_4(root, "No Section");
			}
		} else {
			crm_debug_2("Revision check failed");
		}
		
	}

	crm_log_xml_debug_4(root, "cib:input");
	return copy_xml(data);
}

static gboolean
verify_section(const char *section)
{
	if(section == NULL) {
		return TRUE;
	} else if(safe_str_eq(section, XML_TAG_CIB)) {
		return TRUE;
	} else if(safe_str_eq(section, XML_CIB_TAG_STATUS)) {
		return TRUE;
	} else if(safe_str_eq(section, XML_CIB_TAG_CRMCONFIG)) {
		return TRUE;
	} else if(safe_str_eq(section, XML_CIB_TAG_NODES)) {
		return TRUE;
	} else if(safe_str_eq(section, XML_CIB_TAG_RESOURCES)) {
		return TRUE;
	} else if(safe_str_eq(section, XML_CIB_TAG_CONSTRAINTS)) {
		return TRUE;
	}
	return FALSE;
}


static enum cib_errors
cib_prepare_none(HA_Message *request, HA_Message **data, const char **section)
{
	*data = NULL;
	*section = cl_get_string(request, F_CIB_SECTION);
	if(verify_section(*section) == FALSE) {
		return cib_bad_section;
	}
	return cib_ok;
}

static enum cib_errors
cib_prepare_data(HA_Message *request, HA_Message **data, const char **section)
{
	HA_Message *input_fragment = get_message_xml(request, F_CIB_CALLDATA);
	*section = cl_get_string(request, F_CIB_SECTION);
	*data = cib_prepare_common(input_fragment, *section);
	free_xml(input_fragment);
	
	if(verify_section(*section) == FALSE) {
		return cib_bad_section;
	}
	return cib_ok;
}

static enum cib_errors
cib_prepare_sync(HA_Message *request, HA_Message **data, const char **section)
{
	*section = cl_get_string(request, F_CIB_SECTION);
	*data = request;
	if(verify_section(*section) == FALSE) {
		return cib_bad_section;
	}
	return cib_ok;
}

static enum cib_errors
cib_prepare_diff(HA_Message *request, HA_Message **data, const char **section)
{
	HA_Message *input_fragment = NULL;
	const char *update     = cl_get_string(request, F_CIB_GLOBAL_UPDATE);

	*data = NULL;
	*section = NULL;

	if(crm_is_true(update)) {
	    input_fragment = get_message_xml(request, F_CIB_UPDATE_DIFF);  
		
	} else {
	    input_fragment = get_message_xml(request, F_CIB_CALLDATA); 
	}

	CRM_CHECK(input_fragment != NULL,crm_log_message(LOG_WARNING, request));
	*data = cib_prepare_common(input_fragment, NULL);
	free_xml(input_fragment);
	return cib_ok;
}

static enum cib_errors
cib_cleanup_query(const char *op, HA_Message **data, HA_Message **output) 
{
	CRM_DEV_ASSERT(*data == NULL);
	return cib_ok;
}

static enum cib_errors
cib_cleanup_data(const char *op, HA_Message **data, HA_Message **output) 
{
	free_xml(*output);
	free_xml(*data);
	return cib_ok;
}

static enum cib_errors
cib_cleanup_output(const char *op, HA_Message **data, HA_Message **output) 
{
	free_xml(*output);
	return cib_ok;
}

static enum cib_errors
cib_cleanup_none(const char *op, HA_Message **data, HA_Message **output) 
{
	CRM_DEV_ASSERT(*data == NULL);
	CRM_DEV_ASSERT(*output == NULL);
	return cib_ok;
}

static enum cib_errors
cib_cleanup_sync(const char *op, HA_Message **data, HA_Message **output) 
{
	/* data is non-NULL but doesnt need to be free'd */
 	CRM_DEV_ASSERT(*output == NULL);
	return cib_ok;
}

/*
typedef struct cib_operation_s
{
	const char* 	operation;
	gboolean	modifies_cib;
	gboolean	needs_privileges;
	gboolean	needs_quorum;
	enum cib_errors (*prepare)(HA_Message *, crm_data_t**, const char **);
	enum cib_errors (*cleanup)(crm_data_t**, crm_data_t**);
	enum cib_errors (*fn)(
		const char *, int, const char *,
		crm_data_t*, crm_data_t*, crm_data_t**, crm_data_t**);
} cib_operation_t;
*/
/* technically bump does modify the cib...
 * but we want to split the "bump" from the "sync"
 */
cib_operation_t cib_server_ops[] = {
	{NULL,		   FALSE, FALSE, FALSE, cib_prepare_none, cib_cleanup_none,   cib_process_default},
	{CIB_OP_QUERY,     FALSE, FALSE, FALSE, cib_prepare_none, cib_cleanup_query,  cib_process_query},
	{CIB_OP_MODIFY,    TRUE,  TRUE,  TRUE,  cib_prepare_data, cib_cleanup_data,   cib_process_modify},
	{CIB_OP_UPDATE,    TRUE,  TRUE,  TRUE,  cib_prepare_data, cib_cleanup_data,   cib_process_change},
	{CIB_OP_APPLY_DIFF,TRUE,  TRUE,  TRUE,  cib_prepare_diff, cib_cleanup_data,   cib_process_diff},
	{CIB_OP_SLAVE,     FALSE, TRUE,  FALSE, cib_prepare_none, cib_cleanup_none,   cib_process_readwrite},
	{CIB_OP_SLAVEALL,  FALSE, TRUE,  FALSE, cib_prepare_none, cib_cleanup_none,   cib_process_readwrite},
	{CIB_OP_SYNC_ONE,  FALSE, TRUE,  FALSE, cib_prepare_sync, cib_cleanup_sync,   cib_process_sync_one},
	{CIB_OP_MASTER,    FALSE, TRUE,  FALSE, cib_prepare_none, cib_cleanup_none,   cib_process_readwrite},
	{CIB_OP_ISMASTER,  FALSE, TRUE,  FALSE, cib_prepare_none, cib_cleanup_none,   cib_process_readwrite},
	{CIB_OP_BUMP,      TRUE,  TRUE,  TRUE,  cib_prepare_none, cib_cleanup_output, cib_process_bump},
	{CIB_OP_REPLACE,   TRUE,  TRUE,  TRUE,  cib_prepare_data, cib_cleanup_data,   cib_process_replace},
	{CIB_OP_CREATE,    TRUE,  TRUE,  TRUE,  cib_prepare_data, cib_cleanup_data,   cib_process_change},
	{CIB_OP_DELETE,    TRUE,  TRUE,  TRUE,  cib_prepare_data, cib_cleanup_data,   cib_process_delete},
	{CIB_OP_DELETE_ALT,TRUE,  TRUE,  TRUE,  cib_prepare_data, cib_cleanup_data,   cib_process_change},
	{CIB_OP_SYNC,      FALSE, TRUE,  FALSE, cib_prepare_sync, cib_cleanup_sync,   cib_process_sync},
	{CRM_OP_QUIT,	   FALSE, TRUE,  FALSE, cib_prepare_none, cib_cleanup_none,   cib_process_quit},
	{CRM_OP_PING,	   FALSE, FALSE, FALSE, cib_prepare_none, cib_cleanup_output, cib_process_ping},
	{CIB_OP_ERASE,     TRUE,  TRUE,  TRUE,  cib_prepare_none, cib_cleanup_output, cib_process_erase},
	{CRM_OP_NOOP,	   FALSE, FALSE, FALSE, cib_prepare_none, cib_cleanup_none,   cib_process_default},
	{"cib_shutdown_req",FALSE, TRUE, FALSE, cib_prepare_sync, cib_cleanup_sync,   cib_process_shutdown_req},
};

int send_via_callback_channel(HA_Message *msg, const char *token);

enum cib_errors cib_process_command(
	HA_Message *request, HA_Message **reply,
	crm_data_t **cib_diff, gboolean privileged);

gboolean cib_common_callback(IPC_Channel *channel, cib_client_t *cib_client,
			     gboolean force_synchronous, gboolean privileged);

enum cib_errors cib_get_operation_id(const HA_Message * msg, int *operation);

gboolean cib_process_disconnect(IPC_Channel *channel, cib_client_t *cib_client);
int num_clients = 0;

static void
cib_ipc_connection_destroy(gpointer user_data)
{
	cib_client_t *cib_client = user_data;
	
	/* cib_process_disconnect */

	if(cib_client == NULL) {
		crm_debug_4("Destroying %p", user_data);
		return;
	}

	if(cib_client->source != NULL) {
		crm_debug_4("Deleting %s (%p) from mainloop",
			    cib_client->name, cib_client->source);
		G_main_del_IPC_Channel(cib_client->source); 
		cib_client->source = NULL;
	}
	
	crm_debug_3("Destroying %s (%p)", cib_client->name, user_data);
	num_clients--;
	crm_debug_2("Num unfree'd clients: %d", num_clients);
	crm_free(cib_client->name);
	crm_free(cib_client->callback_id);
	crm_free(cib_client->id);
	crm_free(cib_client);
	crm_debug_4("Freed the cib client");

	return;
}

static cib_client_t *
cib_client_connect_common(
	IPC_Channel *channel, const char *channel_name,
	gboolean (*callback)(IPC_Channel *channel, gpointer user_data))
{
	gboolean can_connect = TRUE;
	cib_client_t *new_client = NULL;
	crm_debug_3("Connecting channel");

	if (channel == NULL) {
		crm_err("Channel was NULL");
		can_connect = FALSE;
		cib_bad_connects++;

	} else if (channel->ch_status != IPC_CONNECT) {
		crm_err("Channel was disconnected");
		can_connect = FALSE;
		cib_bad_connects++;
		
	} else if(channel_name == NULL) {
		crm_err("user_data must contain channel name");
		can_connect = FALSE;
		cib_bad_connects++;
		
	} else if(cib_shutdown_flag) {
		crm_info("Ignoring new client [%d] during shutdown",
			channel->farside_pid);
		return NULL;
		
	} else {
		crm_malloc0(new_client, sizeof(cib_client_t));
		num_clients++;
		new_client->channel = channel;
		new_client->channel_name = channel_name;

		crm_debug_3("Created channel %p for channel %s",
			  new_client, new_client->channel_name);

		channel->ops->set_recv_qlen(channel, 1024);
		channel->ops->set_send_qlen(channel, 1024);

		if(callback != NULL) {
			new_client->source = G_main_add_IPC_Channel(
				G_PRIORITY_DEFAULT, channel, FALSE, callback,
				new_client, cib_ipc_connection_destroy);
		}

		crm_debug_3("Channel %s connected for client %s",
			    new_client->channel_name, new_client->id);
	}

	return new_client;
}

gboolean
cib_client_connect_rw_synch(IPC_Channel *channel, gpointer user_data)
{
	cib_client_t *new_client = NULL;
	new_client = cib_client_connect_common(
		channel, cib_channel_rw_synchronous, cib_rw_synchronous_callback);

	if(new_client == NULL) {
		return FALSE;
	}
	return TRUE;
}

gboolean
cib_client_connect_ro_synch(IPC_Channel *channel, gpointer user_data)
{
	cib_client_t *new_client = NULL;
	new_client = cib_client_connect_common(
		channel, cib_channel_ro_synchronous, cib_ro_synchronous_callback);

	if(new_client == NULL) {
		return FALSE;
	}
	return TRUE;
}

gboolean
cib_client_connect_rw_ro(IPC_Channel *channel, gpointer user_data)
{
	cl_uuid_t client_id;
	HA_Message *reg_msg = NULL;
	cib_client_t *new_client = NULL;
	char uuid_str[UU_UNPARSE_SIZEOF];
	gboolean (*callback)(IPC_Channel *channel, gpointer user_data);
	
	callback = cib_ro_callback;
	if(safe_str_eq(user_data, cib_channel_rw)) {
		callback = cib_rw_callback;
	}

	new_client = cib_client_connect_common(
		channel,
		callback==cib_ro_callback?cib_channel_ro:cib_channel_rw,
		callback);

	if(new_client == NULL) {
		return FALSE;
	}
	
	cl_uuid_generate(&client_id);
	cl_uuid_unparse(&client_id, uuid_str);

	CRM_CHECK(new_client->id == NULL, crm_free(new_client->id));
	new_client->id = crm_strdup(uuid_str);
	
	cl_uuid_generate(&client_id);
	cl_uuid_unparse(&client_id, uuid_str);

	CRM_CHECK(new_client->callback_id == NULL, crm_free(new_client->callback_id));
	new_client->callback_id = crm_strdup(uuid_str);
	
	/* make sure we can find ourselves later for sync calls
	 * redirected to the master instance
	 */
	g_hash_table_insert(client_list, new_client->id, new_client);
	
	reg_msg = ha_msg_new(3);
	ha_msg_add(reg_msg, F_CIB_OPERATION, CRM_OP_REGISTER);
	ha_msg_add(reg_msg, F_CIB_CLIENTID,  new_client->id);
	ha_msg_add(
		reg_msg, F_CIB_CALLBACK_TOKEN, new_client->callback_id);
	
	send_ipc_message(channel, reg_msg);		
	crm_msg_del(reg_msg);
	
	return TRUE;
}

gboolean
cib_client_connect_null(IPC_Channel *channel, gpointer user_data)
{
	cib_client_t *new_client = NULL;
	new_client = cib_client_connect_common(
		channel, cib_channel_callback, cib_null_callback);

	if(new_client == NULL) {
		return FALSE;
	}
	return TRUE;
}

gboolean
cib_rw_callback(IPC_Channel *channel, gpointer user_data)
{
	gboolean result = FALSE;
	result = cib_common_callback(channel, user_data, FALSE, TRUE);
	return result;
}


gboolean
cib_ro_synchronous_callback(IPC_Channel *channel, gpointer user_data)
{
	gboolean result = FALSE;
	result = cib_common_callback(channel, user_data, TRUE, FALSE);
	return result;
}

gboolean
cib_rw_synchronous_callback(IPC_Channel *channel, gpointer user_data)
{
	gboolean result = FALSE;
	result = cib_common_callback(channel, user_data, TRUE, TRUE);
	return result;
}

gboolean
cib_ro_callback(IPC_Channel *channel, gpointer user_data)
{
	gboolean result = FALSE;
	result = cib_common_callback(channel, user_data, FALSE, FALSE);
	return result;
}

gboolean
cib_null_callback(IPC_Channel *channel, gpointer user_data)
{
	gboolean keep_connection = TRUE;
	HA_Message *op_request = NULL;
	HA_Message *registered = NULL;
	cib_client_t *cib_client = user_data;
	cib_client_t *hash_client = NULL;
	const char *type = NULL;
	const char *uuid_ticket = NULL;
	const char *client_name = NULL;
	gboolean register_failed = FALSE;

	if(cib_client == NULL) {
		crm_err("Discarding IPC message from unknown source"
			" on callback channel.");
		return FALSE;
	}
	
	while(IPC_ISRCONN(channel)) {
		crm_msg_del(op_request);

		if(channel->ops->is_message_pending(channel) == 0) {
			break;
		}

		op_request = msgfromIPC_noauth(channel);
		if(op_request == NULL) {
			break;
		}
		
		type = cl_get_string(op_request, F_CIB_OPERATION);
		if(safe_str_eq(type, T_CIB_NOTIFY) ) {
			/* Update the notify filters for this client */
			int on_off = 0;
			ha_msg_value_int(
				op_request, F_CIB_NOTIFY_ACTIVATE, &on_off);
			type = cl_get_string(op_request, F_CIB_NOTIFY_TYPE);

			crm_info("Setting %s callbacks for %s: %s",
				 type, cib_client->name, on_off?"on":"off");
			
			if(safe_str_eq(type, T_CIB_POST_NOTIFY)) {
				cib_client->post_notify = on_off;
				
			} else if(safe_str_eq(type, T_CIB_PRE_NOTIFY)) {
				cib_client->pre_notify = on_off;

			} else if(safe_str_eq(type, T_CIB_UPDATE_CONFIRM)) {
				cib_client->confirmations = on_off;

			} else if(safe_str_eq(type, T_CIB_DIFF_NOTIFY)) {
				cib_client->diffs = on_off;

			} else if(safe_str_eq(type, T_CIB_REPLACE_NOTIFY)) {
				cib_client->replace = on_off;

			}
			continue;
			
		} else if(safe_str_neq(type, CRM_OP_REGISTER) ) {
			crm_warn("Discarding IPC message from %s on callback channel",
				 cib_client->id);
			continue;
		}

		uuid_ticket = cl_get_string(op_request, F_CIB_CALLBACK_TOKEN);
		client_name = cl_get_string(op_request, F_CIB_CLIENTNAME);

		CRM_DEV_ASSERT(uuid_ticket != NULL);
		if(crm_assert_failed) {
			register_failed = crm_assert_failed;
		}
		
		CRM_DEV_ASSERT(client_name != NULL);
		if(crm_assert_failed) {
			register_failed = crm_assert_failed;
		}

		if(register_failed == FALSE) {
			hash_client = g_hash_table_lookup(client_list, uuid_ticket);
			if(hash_client != NULL) {
				crm_err("Duplicate registration request..."
					" disconnecting");
				register_failed = TRUE;
			}
		}
		
		if(register_failed) {
			crm_err("Registration request failed... disconnecting");
			crm_msg_del(op_request);
			return FALSE;
		}

		CRM_CHECK(cib_client->id == NULL, crm_free(cib_client->id));
		CRM_CHECK(cib_client->name == NULL, crm_free(cib_client->name));
		cib_client->id   = crm_strdup(uuid_ticket);
		cib_client->name = crm_strdup(client_name);
		g_hash_table_insert(client_list, cib_client->id, cib_client);

		crm_debug_2("Registered %s on %s channel",
			    cib_client->id, cib_client->channel_name);

		if(safe_str_eq(cib_client->name, CRM_SYSTEM_TENGINE)) {
			/* The TE is _always_ interested in these
			 * Enable now to avoid timing issues
			 */
			cib_client->diffs = TRUE;
		}		

		registered = ha_msg_new(2);
		ha_msg_add(registered, F_CIB_OPERATION, CRM_OP_REGISTER);
		ha_msg_add(registered, F_CIB_CLIENTID,  cib_client->id);
		
		send_ipc_message(channel, registered);
		crm_msg_del(registered);

		if(channel->ch_status == IPC_CONNECT) {
			break;
		}
	}
	crm_msg_del(op_request);

	if(channel->ch_status != IPC_CONNECT) {
		crm_debug_2("Client disconnected");
		keep_connection = cib_process_disconnect(channel, cib_client);	
	}
	
	return keep_connection;
}

void
cib_common_callback_worker(HA_Message *op_request, cib_client_t *cib_client,
			   gboolean force_synchronous, gboolean privileged);

void
cib_common_callback_worker(HA_Message *op_request, cib_client_t *cib_client,
			   gboolean force_synchronous, gboolean privileged)
{
	int rc = cib_ok;
	int call_type = 0;
	const char *op = NULL;

	longclock_t call_stop = 0;
	longclock_t call_start = 0;
	
	call_start = time_longclock();
	cib_client->num_calls++;
	op = cl_get_string(op_request, F_CIB_OPERATION);

	rc = cib_get_operation_id(op_request, &call_type);
	if(rc != cib_ok) {
		crm_debug("Invalid operation %s from %s/%s",
			  op, cib_client->name, cib_client->channel_name);
		
	} else {
		crm_debug_2("Processing %s operation from %s/%s",
			    op, cib_client->name, cib_client->channel_name);
	}
	
	if(rc == cib_ok) {
		cib_process_request(
			op_request, force_synchronous, privileged, FALSE,
			cib_client);
	}
		
	call_stop = time_longclock();
	cib_call_time += (call_stop - call_start);
}

gboolean
cib_common_callback(IPC_Channel *channel, cib_client_t *cib_client,
		    gboolean force_synchronous, gboolean privileged)
{
	int lpc = 0;
	HA_Message *op_request = NULL;
	gboolean keep_channel = TRUE;

	if(cib_client == NULL) {
		crm_err("Receieved call from unknown source. Discarding.");
		return FALSE;
	}

	if(cib_client->name == NULL) {
		cib_client->name = crm_itoa(channel->farside_pid);
	}
	if(cib_client->id == NULL) {
		cib_client->id = crm_strdup(cib_client->name);
		g_hash_table_insert(client_list, cib_client->id, cib_client);
	}
	
	crm_debug_2("Callback for %s on %s channel",
		    cib_client->id, cib_client->channel_name);

	while(IPC_ISRCONN(channel)) {
		if(channel->ops->is_message_pending(channel) == 0) {
			break;
		}

		op_request = msgfromIPC_noauth(channel);
		if (op_request == NULL) {
			perror("Receive failure:");
			break;
		}

		lpc++;
		crm_assert_failed = FALSE;

		crm_log_message_adv(LOG_MSG, "Client[inbound]", op_request);
		ha_msg_add(op_request, F_CIB_CLIENTID, cib_client->id);
		ha_msg_add(op_request, F_CIB_CLIENTNAME, cib_client->name);
		
		cib_common_callback_worker(
			op_request, cib_client, force_synchronous, privileged);

		crm_msg_del(op_request);

		if(channel->ch_status == IPC_CONNECT) {
			break;
		}
	}

	crm_debug_2("Processed %d messages", lpc);

	if(channel->ch_status != IPC_CONNECT) {
		crm_debug_2("Client disconnected");
		keep_channel = cib_process_disconnect(channel, cib_client);	
	}

	return keep_channel;
}

extern void cib_send_remote_msg(void *session, HA_Message *msg);

static void
do_local_notify(HA_Message *notify_src, const char *client_id, gboolean sync_reply, gboolean from_peer) 
{
	/* send callback to originating child */
	cib_client_t *client_obj = NULL;
	HA_Message *client_reply = NULL;
	enum cib_errors local_rc = cib_ok;

	crm_debug_2("Performing notification");
	
	client_reply = cib_msg_copy(notify_src, TRUE);
	
	if(client_id != NULL) {
		client_obj = g_hash_table_lookup(
			client_list, client_id);
	} else {
		crm_debug_2("No client to sent the response to."
			    "  F_CIB_CLIENTID not set.");
	}
	
	crm_debug_3("Sending callback to request originator");
	if(client_obj == NULL) {
		local_rc = cib_reply_failed;
		
	} else if (crm_str_eq(client_obj->channel_name, "remote", FALSE)) {
		crm_debug("Send message over TLS connection");
		cib_send_remote_msg(client_obj->channel, client_reply);
		
	} else {
		const char *client_id = client_obj->callback_id;
		crm_debug_2("Sending %ssync response to %s %s",
			    sync_reply?"":"an a-",
			    client_obj->name,
			    from_peer?"(originator of delegated request)":"");
		
		if(sync_reply) {
			client_id = client_obj->id;
		}
		local_rc = send_via_callback_channel(client_reply, client_id);
	} 
	
	if(local_rc != cib_ok && client_obj != NULL) {
		crm_warn("%sSync reply to %s failed: %s",
			 sync_reply?"":"A-",
			 client_obj?client_obj->name:"<unknown>", cib_error2string(local_rc));
	}

	ha_msg_del(client_reply);
}

static void
parse_local_options(
	cib_client_t *cib_client, int call_type, int call_options, const char *host, const char *op, 
	gboolean *local_notify, gboolean *needs_reply, gboolean *process, gboolean *needs_forward) 
{
	if(cib_server_ops[call_type].modifies_cib
	   && !(call_options & cib_inhibit_bcast)) {
		/* we need to send an update anyway */
		*needs_reply = TRUE;
	} else {
		*needs_reply = FALSE;
	}
	
	if(host == NULL && (call_options & cib_scope_local)) {
		crm_debug_2("Processing locally scoped %s op from %s",
			    op, cib_client->name);
		*local_notify = TRUE;
		
	} else if(host == NULL && cib_is_master) {
		crm_debug_2("Processing master %s op locally from %s",
			    op, cib_client->name);
		*local_notify = TRUE;
		
	} else if(safe_str_eq(host, cib_our_uname)) {
		crm_debug_2("Processing locally addressed %s op from %s",
			    op, cib_client->name);
		*local_notify = TRUE;

	} else if(stand_alone) {
		*needs_forward = FALSE;
		*local_notify = TRUE;
		*process = TRUE;
		
	} else {
		crm_debug_2("%s op from %s needs to be forwarded to %s",
			    op, cib_client->name,
			    host?host:"the master instance");
		*needs_forward = TRUE;
		*process = FALSE;
	}		
}

static gboolean
parse_peer_options(
	int call_type, HA_Message *request, 
	gboolean *local_notify, gboolean *needs_reply, gboolean *process, gboolean *needs_forward) 
{
	const char *op         = cl_get_string(request, F_CIB_OPERATION);
	const char *originator = cl_get_string(request, F_ORIG);
	const char *host       = cl_get_string(request, F_CIB_HOST);
	const char *reply_to   = cl_get_string(request, F_CIB_ISREPLY);
	const char *update     = cl_get_string(request, F_CIB_GLOBAL_UPDATE);
	const char *delegated  = cl_get_string(request, F_CIB_DELEGATED);

	if(safe_str_eq(op, "cib_shutdown_req")) {
		if(reply_to != NULL) {
			crm_debug("Processing %s from %s", op, host);
			*needs_reply = FALSE;
			
		} else {
			crm_debug("Processing %s reply from %s", op, host);
		}
		return TRUE;
		
	} else if(crm_is_true(update) && safe_str_eq(reply_to, cib_our_uname)) {
		crm_debug_2("Processing global/peer update from %s"
			    " that originated from us", originator);
		*needs_reply = FALSE;
		if(cl_get_string(request, F_CIB_CLIENTID) != NULL) {
			*local_notify = TRUE;
		}
		return TRUE;
		
	} else if(crm_is_true(update)) {
		crm_debug_2("Processing global/peer update from %s", originator);
		*needs_reply = FALSE;
		return TRUE;

	} else if(host != NULL && safe_str_eq(host, cib_our_uname)) {
		crm_debug_2("Processing request sent to us from %s", originator);
		return TRUE;

	} else if(delegated != NULL && cib_is_master == TRUE) {
		crm_debug_2("Processing request sent to master instance from %s",
			originator);
		return TRUE;

	} else if(reply_to != NULL && safe_str_eq(reply_to, cib_our_uname)) {
		crm_debug_2("Forward reply sent from %s to local clients",
			  originator);
		*process = FALSE;
		*needs_reply = FALSE;
		*local_notify = TRUE;
		return TRUE;

	} else if(delegated != NULL) {
		crm_debug_2("Ignoring msg for master instance");

	} else if(host != NULL) {
		/* this is for a specific instance and we're not it */
		crm_debug_2("Ignoring msg for instance on %s", crm_str(host));
		
	} else if(reply_to == NULL && cib_is_master == FALSE) {
		/* this is for the master instance and we're not it */
		crm_debug_2("Ignoring reply to %s", crm_str(reply_to));
		
	} else {
		crm_err("Nothing for us to do?");
		crm_log_message_adv(LOG_ERR, "Peer[inbound]", request);
	}

	return FALSE;
}

		
static void
forward_request(HA_Message *request, cib_client_t *cib_client, int call_options)
{
	HA_Message *forward_msg = NULL;
	const char *op         = cl_get_string(request, F_CIB_OPERATION);
	const char *host       = cl_get_string(request, F_CIB_HOST);

	forward_msg = cib_msg_copy(request, TRUE);
	ha_msg_add(forward_msg, F_CIB_DELEGATED, cib_our_uname);
	
	if(host != NULL) {
		crm_debug_2("Forwarding %s op to %s", op, host);
		send_ha_message(hb_conn, forward_msg, host, FALSE);
		
	} else {
		crm_debug_2("Forwarding %s op to master instance", op);
		send_ha_message(hb_conn, forward_msg, NULL, FALSE);
	}
	
	if(call_options & cib_discard_reply) {
		crm_debug_2("Client not interested in reply");
		
	} else if(call_options & cib_sync_call) {
		/* keep track of the request so we can time it
		 * out if required
		 */
		crm_debug_2("Registering delegated call from %s",
			    cib_client->id);
		cib_client->delegated_calls = g_list_append(
			cib_client->delegated_calls, forward_msg);
		forward_msg = NULL;
		
	} 
	crm_msg_del(forward_msg);
}

static void
send_peer_reply(
	HA_Message *msg, crm_data_t *result_diff, const char *originator, gboolean broadcast)
{
	HA_Message *reply_copy = NULL;

	CRM_ASSERT(msg != NULL);

 	reply_copy = cib_msg_copy(msg, TRUE);
	
	if(broadcast) {
		/* this (successful) call modified the CIB _and_ the
		 * change needs to be broadcast...
		 *   send via HA to other nodes
		 */
		int diff_add_updates = 0;
		int diff_add_epoch   = 0;
		int diff_add_admin_epoch = 0;
		
		int diff_del_updates = 0;
		int diff_del_epoch   = 0;
		int diff_del_admin_epoch = 0;

		char *digest = NULL;
		
		cib_diff_version_details(
			result_diff,
			&diff_add_admin_epoch, &diff_add_epoch, &diff_add_updates, 
			&diff_del_admin_epoch, &diff_del_epoch, &diff_del_updates);

		crm_debug("Sending update diff %d.%d.%d -> %d.%d.%d",
			    diff_del_admin_epoch,diff_del_epoch,diff_del_updates,
			    diff_add_admin_epoch,diff_add_epoch,diff_add_updates);

		ha_msg_add(reply_copy, F_CIB_ISREPLY, originator);
		ha_msg_add(reply_copy, F_CIB_GLOBAL_UPDATE, XML_BOOLEAN_TRUE);
		ha_msg_mod(reply_copy, F_CIB_OPERATION, CIB_OP_APPLY_DIFF);

		digest = calculate_xml_digest(the_cib, FALSE, TRUE);
		ha_msg_mod(result_diff, XML_ATTR_DIGEST, digest);
/* 		crm_log_xml_debug(the_cib, digest); */
		crm_free(digest);
		
 		add_message_xml(reply_copy, F_CIB_UPDATE_DIFF, result_diff);
		crm_log_message(LOG_DEBUG_3, reply_copy);
  		send_ha_message(hb_conn, reply_copy, NULL, TRUE);
		
	} else if(originator != NULL) {
		/* send reply via HA to originating node */
		crm_debug_2("Sending request result to originator only");
		ha_msg_add(reply_copy, F_CIB_ISREPLY, originator);
  		send_ha_message(hb_conn, reply_copy, originator, FALSE);
	}
	
	crm_msg_del(reply_copy);
}
	
void
cib_process_request(
	HA_Message *request, gboolean force_synchronous, gboolean privileged,
	gboolean from_peer, cib_client_t *cib_client) 
{
	int call_type    = 0;
	int call_options = 0;

	gboolean process = TRUE;		
	gboolean needs_reply = TRUE;
	gboolean local_notify = FALSE;
	gboolean needs_forward = FALSE;
	crm_data_t *result_diff = NULL;
	
	enum cib_errors rc = cib_ok;
	HA_Message *op_reply = NULL;
	
	const char *op         = cl_get_string(request, F_CIB_OPERATION);
	const char *originator = cl_get_string(request, F_ORIG);
	const char *host       = cl_get_string(request, F_CIB_HOST);
	const char *update     = cl_get_string(request, F_CIB_GLOBAL_UPDATE);

	crm_debug_4("%s Processing msg %s",
		  cib_our_uname, cl_get_string(request, F_SEQ));

	cib_num_ops++;
	if(cib_num_ops == 0) {
		cib_num_fail = 0;
		cib_num_local = 0;
		cib_num_updates = 0;
		crm_info("Stats wrapped around");
	}
	
	if(host != NULL && strlen(host) == 0) {
		host = NULL;
	}	

	ha_msg_value_int(request, F_CIB_CALLOPTS, &call_options);
	crm_debug_4("Retrieved call options: %d", call_options);
	if(force_synchronous) {
		call_options |= cib_sync_call;
	}
	
	crm_debug_2("Processing %s message (%s) for %s...",
		    from_peer?"peer":"local",
		    from_peer?originator:cib_our_uname, host?host:"master");

	rc = cib_get_operation_id(request, &call_type);

	if(cib_server_ops[call_type].modifies_cib) {
		cib_num_updates++;
	}
	
	if(rc != cib_ok) {
		/* TODO: construct error reply */
		crm_err("Pre-processing of command failed: %s",
			cib_error2string(rc));
		
	} else if(from_peer == FALSE) {
		parse_local_options(cib_client, call_type, call_options, host, op,
				    &local_notify, &needs_reply, &process, &needs_forward);
		
	} else if(parse_peer_options(call_type, request, &local_notify,
				     &needs_reply, &process, &needs_forward) == FALSE) {
		return;
	}
	crm_debug_3("Finished determining processing actions");

	if(call_options & cib_discard_reply) {
		needs_reply = cib_server_ops[call_type].modifies_cib;
		local_notify = FALSE;
	}
	
	if(needs_forward) {
		forward_request(request, cib_client, call_options);
		return;
	}

	if(cib_status != cib_ok) {
	    rc = cib_status;
	    crm_err("Operation ignored, cluster configuration is invalid."
		    " Please repair and restart: %s",
		    cib_error2string(cib_status));
	    op_reply = cib_construct_reply(request, the_cib, cib_status);

	} else if(process) {
		cib_num_local++;
		crm_debug_2("Performing local processing:"
			    " op=%s origin=%s/%s,%s (update=%s)",
			    cl_get_string(request, F_CIB_OPERATION), originator,
			    cl_get_string(request, F_CIB_CLIENTID),
			    cl_get_string(request, F_CIB_CALLID), update);
		
		rc = cib_process_command(
			request, &op_reply, &result_diff, privileged);

		crm_debug_2("Processing complete");

		if(rc == cib_diff_resync || rc == cib_diff_failed
		   || rc == cib_old_data) {
			crm_warn("%s operation failed: %s",
				crm_str(op), cib_error2string(rc));
			
		} else if(rc != cib_ok) {
			cib_num_fail++;
			crm_err("%s operation failed: %s",
				crm_str(op), cib_error2string(rc));
			crm_log_message_adv(LOG_DEBUG, "CIB[output]", op_reply);
			crm_log_message_adv(LOG_INFO, "Input message", request);
		}

		if(op_reply == NULL && (needs_reply || local_notify)) {
			crm_err("Unexpected NULL reply to message");
			crm_log_message(LOG_ERR, request);
			needs_reply = FALSE;
			local_notify = FALSE;
		}		
	}
	crm_debug_3("processing response cases");

	if(local_notify) {
		const char *client_id = cl_get_string(request, F_CIB_CLIENTID);
		if(process == FALSE) {
			do_local_notify(request, client_id, call_options & cib_sync_call, from_peer);
		} else {
			do_local_notify(op_reply, client_id, call_options & cib_sync_call, from_peer);
		}
	}

	/* from now on we are the server */ 
	if(needs_reply == FALSE || stand_alone) {
		/* nothing more to do...
		 * this was a non-originating slave update
		 */
		crm_debug_2("Completed slave update");

	} else if(rc == cib_ok
		  && result_diff != NULL
		  && !(call_options & cib_inhibit_bcast)) {
		send_peer_reply(request, result_diff, originator, TRUE);
		
	} else if(call_options & cib_discard_reply) {
		crm_debug_4("Caller isn't interested in reply");
		
	} else if (from_peer) {
		crm_debug_2("Directing reply to %s", originator);
		
		if(call_options & cib_inhibit_bcast) {
			crm_debug_3("Request not broadcast: inhibited");
		}
		if(cib_server_ops[call_type].modifies_cib == FALSE || result_diff == NULL) {
			crm_debug_3("Request not broadcast: R/O call");
		}
		if(rc != cib_ok) {
			crm_debug_3("Request not broadcast: call failed: %s",
				    cib_error2string(rc));
		}

		send_peer_reply(op_reply, result_diff, originator, FALSE);
	}
	
	crm_msg_del(op_reply);
	free_xml(result_diff);

	return;	
}

HA_Message *
cib_construct_reply(HA_Message *request, HA_Message *output, int rc) 
{
	int lpc = 0;
	HA_Message *reply = NULL;
	
	const char *name = NULL;
	const char *value = NULL;
	const char *names[] = {
		F_CIB_OPERATION,
		F_CIB_CALLID,
		F_CIB_CLIENTID,
		F_CIB_CALLOPTS
	};

	crm_debug_4("Creating a basic reply");
	reply = ha_msg_new(8);
	ha_msg_add(reply, F_TYPE, T_CIB);

	for(lpc = 0; lpc < DIMOF(names); lpc++) {
		name = names[lpc];
		value = cl_get_string(request, name);
		ha_msg_add(reply, name, value);
	}

	ha_msg_add_int(reply, F_CIB_RC, rc);

	if(output != NULL) {
		crm_debug_4("Attaching reply output");
		add_message_xml(reply, F_CIB_CALLDATA, output);
	}
	return reply;
}

enum cib_errors
cib_process_command(HA_Message *request, HA_Message **reply,
		    crm_data_t **cib_diff, gboolean privileged)
{
	crm_data_t *output   = NULL;
	crm_data_t *input    = NULL;

	crm_data_t *current_cib = NULL;
	crm_data_t *result_cib  = NULL;
	
	int call_type      = 0;
	int call_options   = 0;
	enum cib_errors rc = cib_ok;
	enum cib_errors rc2 = cib_ok;

	int log_level = LOG_DEBUG_3;
	crm_data_t *filtered = NULL;
	
	const char *op = NULL;
	const char *section = NULL;
	gboolean config_changed = FALSE;
	gboolean global_update = crm_is_true(
		cl_get_string(request, F_CIB_GLOBAL_UPDATE));
	
	CRM_ASSERT(cib_status == cib_ok);

	*reply = NULL;
	*cib_diff = NULL;
	if(per_action_cib) {
		CRM_CHECK(the_cib == NULL, free_xml(the_cib));
		the_cib = readCibXmlFile(cib_root, "cib.xml", FALSE);
		CRM_CHECK(the_cib != NULL, return cib_NOOBJECT);
	}
	current_cib = the_cib;
	
	/* Start processing the request... */
	op = cl_get_string(request, F_CIB_OPERATION);
	ha_msg_value_int(request, F_CIB_CALLOPTS, &call_options);
	rc = cib_get_operation_id(request, &call_type);
	
	if(rc == cib_ok &&
	   cib_server_ops[call_type].needs_privileges
	   && privileged == FALSE) {
		/* abort */
		rc = cib_not_authorized;
	}
	
	if(rc == cib_ok
	   && stand_alone == FALSE
	   && global_update == FALSE
	   && cib_server_ops[call_type].needs_quorum
	   && can_write(call_options) == FALSE) {
		rc = cib_no_quorum;
	}

	/* prevent NUMUPDATES from being incrimented - apply the change as-is */
	if(global_update) {
		call_options |= cib_inhibit_bcast;
		call_options |= cib_force_diff;		
	}

	rc2 = cib_server_ops[call_type].prepare(request, &input, &section);
	if(rc == cib_ok) {
		rc = rc2;
	}
	
	if(rc != cib_ok) {
		crm_debug_2("Call setup failed: %s", cib_error2string(rc));
		goto done;
		
	} else if(cib_server_ops[call_type].modifies_cib == FALSE) {
		rc = cib_server_ops[call_type].fn(
			op, call_options, section, input,
			current_cib, &result_cib, &output);

		CRM_CHECK(result_cib == NULL, free_xml(result_cib));
		goto done;
	}	

	/* Handle a valid write action */

	if((call_options & cib_inhibit_notify) == 0) {
	    cib_pre_notify(call_options, op,
			   get_object_root(section, current_cib), input);
	}

	if(rc == cib_ok) {
	    result_cib = copy_xml(current_cib);
	    
	    rc = cib_server_ops[call_type].fn(
		op, call_options, section, input,
		current_cib, &result_cib, &output);
	}
		
	if(rc == cib_ok) {
	    
	    CRM_DEV_ASSERT(result_cib != NULL);
	    CRM_DEV_ASSERT(current_cib != result_cib);
	    
	    update_counters(__FILE__, __FUNCTION__, result_cib);
	    config_changed = cib_config_changed(current_cib, result_cib, &filtered);
	    
	    if(global_update) {
		/* skip */
		CRM_CHECK(call_type == 4 || call_type == 11,
			  crm_err("Call type: %d", call_type);
			  crm_log_message(LOG_ERR, request));
		crm_debug_2("Skipping update: global replace");
		
	    } else if(cib_server_ops[call_type].fn == cib_process_change
		      && (call_options & cib_inhibit_bcast)) {
		/* skip */
		crm_debug_2("Skipping update: inhibit broadcast");
		
	    } else {
		cib_update_counter(result_cib, XML_ATTR_NUMUPDATES, FALSE);

		if(config_changed) {
		    cib_update_counter(result_cib, XML_ATTR_NUMUPDATES, TRUE);
		    cib_update_counter(result_cib, XML_ATTR_GENERATION, FALSE);
		}
	    }
			
	    if(do_id_check(result_cib, NULL, TRUE, FALSE)) {
		rc = cib_id_check;
		if(call_options & cib_force_diff) {
		    crm_err("Global update introduces id collision!");
		}
		
	    } else {
		*cib_diff = diff_cib_object(current_cib, result_cib, FALSE);
	    }
	}		
	
	if(rc != cib_ok) {
	    free_xml(result_cib);
	    
	} else {
	    rc = activateCibXml(result_cib, config_changed);
	    if(rc != cib_ok) {
		crm_warn("Activation failed");
	    }
	}
	
	if((call_options & cib_inhibit_notify) == 0) {
	    const char *call_id = cl_get_string(request, F_CIB_CALLID);
	    const char *client = cl_get_string(request, F_CIB_CLIENTNAME);
	    cib_post_notify(call_options, op, input, rc, the_cib);
	    cib_diff_notify(call_options, client, call_id, op,
			    input, rc, *cib_diff);
	}

	if(rc == cib_dtd_validation && global_update) {
	    log_level = LOG_WARNING;
	    crm_log_xml_info(input, "cib:global_update");
	} else if(rc != cib_ok) {
	    log_level = LOG_DEBUG_4;
	} else if(cib_is_master && config_changed) {
	    log_level = LOG_INFO;
	} else if(cib_is_master) {
	    log_level = LOG_DEBUG;
	    log_xml_diff(LOG_DEBUG_2, filtered, "cib:diff:filtered");
	    
	} else if(config_changed) {
	    log_level = LOG_DEBUG_2;
	} else {
	    log_level = LOG_DEBUG_3;
	}
	
	log_xml_diff(log_level, *cib_diff, "cib:diff");
	free_xml(filtered);		

  done:
	if((call_options & cib_discard_reply) == 0) {
		*reply = cib_construct_reply(request, output, rc);
	}

	if(call_type >= 0) {
		cib_server_ops[call_type].cleanup(op, &input, &output);
	}
	if(per_action_cib) {
		uninitializeCib();
	}
	return rc;
}

int
send_via_callback_channel(HA_Message *msg, const char *token) 
{
	cib_client_t *hash_client = NULL;
	GList *list_item = NULL;
	enum cib_errors rc = cib_ok;
	
	crm_debug_3("Delivering msg %p to client %s", msg, token);

	if(token == NULL) {
		crm_err("No client id token, cant send message");
		if(rc == cib_ok) {
			rc = cib_missing;
		}
		
	} else {
		/* A client that left before we could reply is not really
		 * _our_ error.  Warn instead.
		 */
		hash_client = g_hash_table_lookup(client_list, token);
		if(hash_client == NULL) {
			crm_warn("Cannot find client for token %s", token);
			rc = cib_client_gone;
			
		} else if(hash_client->channel == NULL) {
			crm_err("Cannot find channel for client %s", token);
			rc = cib_client_corrupt;

		} else if(hash_client->channel->ops->get_chan_status(
				  hash_client->channel) == IPC_DISCONNECT) {
			crm_warn("Client %s has disconnected", token);
			rc = cib_client_gone;
			cib_num_timeouts++;
		}
	}

	/* this is a more important error so overwriting rc is warrented */
	if(msg == NULL) {
		crm_err("No message to send");
		rc = cib_reply_failed;
	}

	if(rc == cib_ok) {
		list_item = g_list_find_custom(
			hash_client->delegated_calls, msg, cib_GCompareFunc);
	}
	
	if(list_item != NULL) {
		/* remove it - no need to time it out */
		HA_Message *orig_msg = list_item->data;
		crm_debug_3("Removing msg from delegated list");
		hash_client->delegated_calls = g_list_remove(
			hash_client->delegated_calls, orig_msg);
		CRM_DEV_ASSERT(orig_msg != msg);
		crm_msg_del(orig_msg);
	}
	
	if(rc == cib_ok) {
		crm_debug_3("Delivering reply to client %s", token);
		if(send_ipc_message(hash_client->channel, msg) == FALSE) {
			crm_warn("Delivery of reply to client %s/%s failed",
				hash_client->name, token);
			rc = cib_reply_failed;
		}
	}
	
	return rc;
}

gint cib_GCompareFunc(gconstpointer a, gconstpointer b)
{
	const HA_Message *a_msg = a;
	const HA_Message *b_msg = b;

	int msg_a_id = 0;
	int msg_b_id = 0;
	
	ha_msg_value_int(a_msg, F_CIB_CALLID, &msg_a_id);
	ha_msg_value_int(b_msg, F_CIB_CALLID, &msg_b_id);
	
	if(msg_a_id == msg_b_id) {
		return 0;
	} else if(msg_a_id < msg_b_id) {
		return -1;
	}
	return 1;
}


gboolean
cib_msg_timeout(gpointer data)
{
	crm_debug_4("Checking if any clients have timed out messages");
/* 	g_hash_table_foreach(client_list, cib_GHFunc, NULL); */
	return TRUE;
}


void
cib_GHFunc(gpointer key, gpointer value, gpointer user_data)
{
	int timeout = 0; /* 1 iteration == 10 seconds */
	HA_Message *msg = NULL;
	HA_Message *reply = NULL;
	const char *host_to = NULL;
	cib_client_t *client = value;
	GListPtr list = client->delegated_calls;

	while(list != NULL) {
		
		msg = list->data;
		ha_msg_value_int(msg, F_CIB_TIMEOUT, &timeout);
		
		if(timeout <= 0) {
			list = list->next;
			continue;

		} else {
			int seen = 0;
			ha_msg_value_int(msg, F_CIB_SEENCOUNT, &seen);
			crm_debug_4("Timeout %d, seen %d", timeout, seen);
			if(seen < timeout) {
				crm_debug_4("Updating seen count for msg from client %s",
					    client->id);
				seen += 10;
				ha_msg_mod_int(msg, F_CIB_SEENCOUNT, seen);
				list = list->next;
				continue;
			}
		}
		
		cib_num_timeouts++;
		host_to = cl_get_string(msg, F_CIB_HOST);
		crm_warn("Sending operation timeout msg to client %s",
			 client->id);
		
		reply = ha_msg_new(4);
		ha_msg_add(reply, F_TYPE, T_CIB);
		ha_msg_add(reply, F_CIB_OPERATION,
			   cl_get_string(msg, F_CIB_OPERATION));
		ha_msg_add(reply, F_CIB_CALLID,
			   cl_get_string(msg, F_CIB_CALLID));
		if(host_to == NULL) {
			ha_msg_add_int(reply, F_CIB_RC, cib_master_timeout);
		} else {
			ha_msg_add_int(reply, F_CIB_RC, cib_remote_timeout);
		}
		
		send_ipc_message(client->channel, reply);

		list = list->next;
		client->delegated_calls = g_list_remove(
			client->delegated_calls, msg);

		crm_msg_del(msg);
		crm_msg_del(reply);
	}
}

gboolean
cib_process_disconnect(IPC_Channel *channel, cib_client_t *cib_client)
{
	if (channel == NULL) {
		CRM_DEV_ASSERT(cib_client == NULL);
		
	} else if (cib_client == NULL) {
		crm_err("No client");
		
	} else {
		CRM_DEV_ASSERT(channel->ch_status != IPC_CONNECT);
		crm_debug_2("Cleaning up after client disconnect: %s/%s/%s",
			    crm_str(cib_client->name),
			    cib_client->channel_name,
			    cib_client->id);
		
		if(cib_client->id != NULL) {
			if(!g_hash_table_remove(client_list, cib_client->id)) {
				crm_err("Client %s not found in the hashtable",
					cib_client->name);
			}
		}		
	}

	if(cib_shutdown_flag && g_hash_table_size(client_list) == 0) {
		crm_info("All clients disconnected...");
		initiate_exit();
	}
	
	return FALSE;
}

gboolean
cib_ha_dispatch(IPC_Channel *channel, gpointer user_data)
{
	ll_cluster_t *hb_cluster = (ll_cluster_t*)user_data;

	crm_debug_3("Invoked");
	if(IPC_ISRCONN(channel)) {
		if(hb_cluster->llc_ops->msgready(hb_cluster) == 0) {
			crm_debug_2("no message ready yet");
		}
		/* invoke the callbacks but dont block */
		hb_cluster->llc_ops->rcvmsg(hb_cluster, 0);
	}
	
	return (channel->ch_status == IPC_CONNECT);
}

void
cib_peer_callback(HA_Message * msg, void* private_data)
{
	int call_type = 0;
	int call_options = 0;
	const char *originator = cl_get_string(msg, F_ORIG);
	const char *seq        = cl_get_string(msg, F_SEQ);
	const char *op         = cl_get_string(msg, F_CIB_OPERATION);

	crm_log_message_adv(LOG_MSG, "Peer[inbound]", msg);
	crm_debug_2("Peer %s message (%s) from %s", op, seq, originator);
	
	if(originator == NULL || safe_str_eq(originator, cib_our_uname)) {
 		crm_debug_2("Discarding %s message %s from ourselves", op, seq);
		return;

	} else if(ccm_membership == NULL) {
 		crm_info("Discarding %s message (%s) from %s:"
			 " membership not established", op, seq, originator);
		return;
		
	} else if(g_hash_table_lookup(ccm_membership, originator) == NULL) {
 		crm_warn("Discarding %s message (%s) from %s:"
			 " not in our membership", op, seq, originator);
		return;

	} else if(cib_get_operation_id(msg, &call_type) != cib_ok) {
 		crm_debug("Discarding %s message (%s) from %s:"
			  " Invalid operation", op, seq, originator);
		return;
	}

	crm_debug_2("Processing %s msg (%s) from %s",op, seq, originator);

	ha_msg_value_int(msg, F_CIB_CALLOPTS, &call_options);
	crm_debug_4("Retrieved call options: %d", call_options);

	if(cl_get_string(msg, F_CIB_CLIENTNAME) == NULL) {
 		ha_msg_add(msg, F_CIB_CLIENTNAME, originator);
	}
	
	cib_process_request(msg, FALSE, TRUE, TRUE, NULL);

	return;
}

HA_Message *
cib_msg_copy(HA_Message *msg, gboolean with_data) 
{
	int lpc = 0;
	const char *field = NULL;
	const char *value = NULL;
	HA_Message *value_struct = NULL;

	static const char *field_list[] = {
		F_TYPE		,
		F_CIB_CLIENTID  ,
		F_CIB_CALLOPTS  ,
		F_CIB_CALLID    ,
		F_CIB_OPERATION ,
		F_CIB_ISREPLY   ,
		F_CIB_SECTION   ,
		F_CIB_HOST	,
		F_CIB_RC	,
		F_CIB_DELEGATED	,
		F_CIB_OBJID	,
		F_CIB_OBJTYPE	,
		F_CIB_EXISTING	,
		F_CIB_SEENCOUNT	,
		F_CIB_TIMEOUT	,
		F_CIB_CALLBACK_TOKEN	,
		F_CIB_GLOBAL_UPDATE	,
		F_CIB_CLIENTNAME	,
		F_CIB_NOTIFY_TYPE	,
		F_CIB_NOTIFY_ACTIVATE
	};
	
	static const char *data_list[] = {
		F_CIB_CALLDATA  ,
		F_CIB_UPDATE	,
		F_CIB_UPDATE_RESULT
	};


	HA_Message *copy = NULL;

	copy = ha_msg_new(10);

	if(copy == NULL) {
		return copy;
	}
	
	for(lpc = 0; lpc < DIMOF(field_list); lpc++) {
		field = field_list[lpc];
		value = cl_get_string(msg, field);
		if(value != NULL) {
			ha_msg_add(copy, field, value);
		}
	}
	for(lpc = 0; with_data && lpc < DIMOF(data_list); lpc++) {
		field = data_list[lpc];
		value_struct = get_message_xml(msg, field);
		if(value_struct != NULL) {
			add_message_xml(copy, field, value_struct);
		}
		free_xml(value_struct);
	}
	return copy;
}


enum cib_errors
cib_get_operation_id(const HA_Message * msg, int *operation) 
{
	int lpc = 0;
	int max_msg_types = DIMOF(cib_server_ops);
	const char *op = cl_get_string(msg, F_CIB_OPERATION);

	for (lpc = 0; lpc < max_msg_types; lpc++) {
		if (safe_str_eq(op, cib_server_ops[lpc].operation)) {
			*operation = lpc;
			return cib_ok;
		}
	}
	crm_err("Operation %s is not valid", op);
	*operation = -1;
	return cib_operation;
}

void
cib_client_status_callback(const char * node, const char * client,
			   const char * status, void * private)
{
	if(safe_str_eq(client, CRM_SYSTEM_CIB)) {
		crm_info("Status update: Client %s/%s now has status [%s]",
			 node, client, status);
		g_hash_table_replace(peer_hash, crm_strdup(node), crm_strdup(status));
		set_connected_peers(the_cib);
	}
	return;
}

extern oc_ev_t *cib_ev_token;

gboolean
ccm_manual_check(gpointer data)
{
	int rc = 0;
	oc_ev_t *ccm_token = cib_ev_token;
	
	crm_debug("manual check");	
	rc = oc_ev_handle_event(ccm_token);
	if(0 == rc) {
		return TRUE;

	} else {
		crm_err("CCM connection appears to have failed: rc=%d.", rc);
		return FALSE;
	}
}

gboolean cib_ccm_dispatch(int fd, gpointer user_data)
{
	int rc = 0;
	oc_ev_t *ccm_token = (oc_ev_t*)user_data;
	crm_debug_2("received callback");	
	rc = oc_ev_handle_event(ccm_token);
	if(0 == rc) {
		return TRUE;

	}

	crm_err("CCM connection appears to have failed: rc=%d.", rc);

	/* eventually it might be nice to recover and reconnect... but until then... */
	crm_err("Exiting to recover from CCM connection failure");
	exit(2);
	
	return FALSE;
}

int current_instance = 0;
void 
cib_ccm_msg_callback(
	oc_ed_t event, void *cookie, size_t size, const void *data)
{
	gboolean update_id = FALSE;
	gboolean update_quorum = FALSE;
	const oc_ev_membership_t *membership = data;

	CRM_ASSERT(membership != NULL);

	crm_debug("Process CCM event=%s (id=%d)",
		  ccm_event_name(event), membership->m_instance);

	if(current_instance > membership->m_instance) {
		crm_err("Membership instance ID went backwards! %d->%d",
			current_instance, membership->m_instance);
		CRM_ASSERT(current_instance <= membership->m_instance);
	}
	
	switch(event) {
		case OC_EV_MS_NEW_MEMBERSHIP:
		case OC_EV_MS_INVALID:
			update_id = TRUE;
			update_quorum = TRUE;
			break;
		case OC_EV_MS_PRIMARY_RESTORED:
			update_id = TRUE;
			break;
		case OC_EV_MS_NOT_PRIMARY:
			crm_debug_2("Ignoring transitional CCM event: %s",
				  ccm_event_name(event));
			break;
		case OC_EV_MS_EVICTED:
			crm_err("Evicted from CCM: %s", ccm_event_name(event));
			update_quorum = TRUE;
			break;
		default:
			crm_err("Unknown CCM event: %d", event);
	}
	
	if(update_id) {
		CRM_CHECK(membership != NULL, return);
	
		if(ccm_transition_id != NULL) {
			crm_free(ccm_transition_id);
			ccm_transition_id = NULL;
		}
		current_instance = membership->m_instance;
		ccm_transition_id = crm_itoa(membership->m_instance);
		set_transition(the_cib);
	}
	
	if(update_quorum) {
		unsigned int members = 0;
		int offset = 0;
		unsigned int lpc = 0;

		cib_have_quorum = ccm_have_quorum(event);

		if(cib_have_quorum) {
 			crm_xml_add(
				the_cib,XML_ATTR_HAVE_QUORUM,XML_BOOLEAN_TRUE);
		} else {
 			crm_xml_add(
				the_cib,XML_ATTR_HAVE_QUORUM,XML_BOOLEAN_FALSE);
		}
		
		crm_debug("Quorum %s after event=%s (id=%d)", 
			  cib_have_quorum?"(re)attained":"lost",
			  ccm_event_name(event), membership->m_instance);
		
		if(membership != NULL && membership->m_n_out != 0) {
			members = membership->m_n_out;
			offset = membership->m_out_idx;
			for(lpc = 0; lpc < members; lpc++) {
				oc_node_t a_node = membership->m_array[lpc+offset];
				crm_info("LOST: %s", a_node.node_uname);
				g_hash_table_remove(
					ccm_membership, a_node.node_uname);	
			}
		}
		
		if(membership != NULL && membership->m_n_member != 0) {
			members = membership->m_n_member;
			offset = membership->m_memb_idx;
			for(lpc = 0; lpc < members; lpc++) {
				oc_node_t a_node = membership->m_array[lpc+offset];
				char *uname = crm_strdup(a_node.node_uname);
				crm_info("PEER: %s", uname);
				g_hash_table_replace(
					ccm_membership, uname, uname);	
			}
		}
	}
	
	oc_ev_callback_done(cookie);
	set_connected_peers(the_cib);
	
	return;
}


gboolean
can_write(int flags)
{
	if(cib_have_quorum) {
		return TRUE;

	} else if((flags & cib_quorum_override) != 0) {
		return TRUE;
	}

	return FALSE;
}

static gboolean
cib_force_exit(gpointer data)
{
	crm_notice("Forcing exit!");
	terminate_ha_connection(__FUNCTION__);
	return FALSE;
}

void
initiate_exit(void)
{
	int active = 0;
	HA_Message *leaving = NULL;

	g_hash_table_foreach(peer_hash, GHFunc_count_peers, &active);
	if(active < 2) {
		terminate_ha_connection(__FUNCTION__);
		return;
	} 

	crm_info("Sending disconnect notification to %d peers...", active);

	leaving = ha_msg_new(3);	
	ha_msg_add(leaving, F_TYPE, "cib");
	ha_msg_add(leaving, F_CIB_OPERATION, "cib_shutdown_req");
	
	send_ha_message(hb_conn, leaving, NULL, TRUE);
	crm_msg_del(leaving);
	
	Gmain_timeout_add(crm_get_msec("5s"), cib_force_exit, NULL);
}

void
terminate_ha_connection(const char *caller) 
{
	if(hb_conn != NULL) {
		crm_info("%s: Disconnecting heartbeat", caller);
		hb_conn->llc_ops->signoff(hb_conn, FALSE);
	} else {
		crm_err("%s: No heartbeat connection", caller);
		exit(LSB_EXIT_OK);
	}
}
