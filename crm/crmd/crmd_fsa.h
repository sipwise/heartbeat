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
#ifndef CRMD_FSA__H
#define CRMD_FSA__H

#include <fsa_defines.h>
#include <ocf/oc_event.h>
#include <clplumbing/ipc.h>
#include <hb_api.h>
#include <lrm/lrm_api.h>
#include <crm/crm.h>
#include <crm/cib.h>
#include <crm/common/xml.h>

struct crmd_ccm_data_s 
{
		oc_ev_membership_t *oc;
		oc_ed_t event;
};


struct oc_node_list_s
{
		guint id; /* membership id */
		oc_ed_t last_event;

		guint members_size;
		GHashTable *members; /* contents: oc_node_t * */

		guint new_members_size;
		GHashTable *new_members; /* contents: oc_node_t * */

		guint dead_members_size;
		GHashTable *dead_members; /* contents: oc_node_t * */
};
typedef struct oc_node_list_s oc_node_list_t;

/* copy from struct client_child in heartbeat.h
 *
 * Plus a couple of other things
 */
struct crm_subsystem_s {
		pid_t	pid;		  /* Process id of child process */
		const char*	name;     /* executable name */
		const char*	path;	  /* Command location */
 		const char*	command;  /* Command with path */
		const char*	args;     /* Command arguments */
		crmd_client_t*  client;   /* Client connection object */
		
		gboolean	sent_kill;
		IPC_Channel	*ipc;	  /* How can we communicate with it */
		long long	flag_connected;	  /*  */
		long long	flag_required;	  /*  */
};

typedef struct fsa_timer_s fsa_timer_t;
struct fsa_timer_s 
{
		guint	source_id;	/* timer source id */
		int	period_ms;	/* timer period */
		enum crmd_fsa_input fsa_input;
		gboolean (*callback)(gpointer data);
		gboolean repeat;
};

enum fsa_data_type {
	fsa_dt_none,
	fsa_dt_ha_msg,
	fsa_dt_xml,
	fsa_dt_lrm,
	fsa_dt_ccm
};


typedef struct fsa_data_s fsa_data_t;
struct fsa_data_s 
{
		int id;
		enum crmd_fsa_input fsa_input;
		enum crmd_fsa_cause fsa_cause;
		long long	    actions;
		const char	   *origin;
		void		   *data;
		enum fsa_data_type  data_type;
};



extern enum crmd_fsa_state s_crmd_fsa(enum crmd_fsa_cause cause);

/* Global FSA stuff */
extern volatile gboolean do_fsa_stall;
extern volatile enum crmd_fsa_state fsa_state;
extern volatile long long fsa_input_register;
extern volatile long long fsa_actions;

extern oc_node_list_t *fsa_membership_copy;
extern ll_cluster_t   *fsa_cluster_conn;
extern ll_lrm_t       *fsa_lrm_conn;
extern cib_t	      *fsa_cib_conn;

extern const char *fsa_our_uname;
extern char	  *fsa_our_uuid;
extern char	  *fsa_pe_ref; /* the last invocation of the PE */
extern char       *fsa_our_dc;
extern char	  *fsa_our_dc_version;
extern GListPtr   fsa_message_queue;

extern fsa_timer_t *election_trigger;		/*  */
extern fsa_timer_t *election_timeout;		/*  */
extern fsa_timer_t *shutdown_escalation_timer;	/*  */
extern fsa_timer_t *integration_timer;
extern fsa_timer_t *finalization_timer;
extern fsa_timer_t *wait_timer;
extern fsa_timer_t *recheck_timer;

extern GTRIGSource *fsa_source;

extern struct crm_subsystem_s *cib_subsystem;
extern struct crm_subsystem_s *te_subsystem;
extern struct crm_subsystem_s *pe_subsystem;

extern GHashTable *welcomed_nodes;
extern GHashTable *integrated_nodes;
extern GHashTable *finalized_nodes;
extern GHashTable *confirmed_nodes;
extern GHashTable *crmd_peer_state;

/* these two should be moved elsewhere... */
extern void do_update_cib_nodes(gboolean overwrite, const char *caller);
extern gboolean do_dc_heartbeat(gpointer data);

gboolean add_cib_op_callback(
	int call_id, gboolean only_success, void *user_data,
	void (*callback)(const HA_Message*, int, int, crm_data_t*,void*));

#define AM_I_DC is_set(fsa_input_register, R_THE_DC)
#define AM_I_OPERATIONAL (is_set(fsa_input_register, R_STARTING)==FALSE)
extern int current_ccm_membership_id;
extern int saved_ccm_membership_id;

#include <fsa_proto.h>
#include <crmd_utils.h>

#endif
