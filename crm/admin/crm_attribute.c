
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

#include <crm/crm.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

#include <heartbeat.h>
#include <hb_api.h>
#include <clplumbing/uids.h>
#include <clplumbing/Gmain_timeout.h>

#include <crm/msg_xml.h>
#include <crm/common/xml.h>
#include <crm/common/ctrl.h>
#include <crm/common/ipc.h>

#include <crm/cib.h>
#include <sys/utsname.h>

#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif
void usage(const char *cmd, int exit_status);

gboolean BE_QUIET = FALSE;
gboolean DO_WRITE = TRUE;
gboolean DO_DELETE = FALSE;

char *dest_node = NULL;
char *set_name  = NULL;
char *attr_id   = NULL;
char *attr_name = NULL;
const char *type       = NULL;
const char *rsc_id     = NULL;
const char *dest_uname = NULL;
const char *attr_value = NULL;
const char *crm_system_name = "crm_master";

#define OPTARGS	"V?GDQU:u:s:n:v:l:t:i:!r:"

int
main(int argc, char **argv)
{
	gboolean is_done = FALSE;
	cib_t *	the_cib = NULL;
	enum cib_errors rc = cib_ok;
	
	int cib_opts = cib_sync_call;
	int argerr = 0;
	int flag;

#ifdef HAVE_GETOPT_H
	int option_index = 0;
	static struct option long_options[] = {
		/* Top-level Options */
		{"verbose",     0, 0, 'V'},
		{"help",        0, 0, '?'},
		{"quiet",       0, 0, 'Q'},
		{"get-value",   0, 0, 'G'},
		{"delete-attr", 0, 0, 'D'},
		{"node-uname",  1, 0, 'U'},
		{"node-uuid",   1, 0, 'u'},
		{"set-name",    1, 0, 's'},
		{"attr-id",     1, 0, 'i'},
		{"attr-name",   1, 0, 'n'},
		{"attr-value",  1, 0, 'v'},
		{"resource-id", 1, 0, 'r'},
		{"lifetime",    1, 0, 'l'},
		{"type",        1, 0, 't'},
		{"inhibit-policy-engine", 0, 0, '!'},

		{0, 0, 0, 0}
	};
#endif

	crm_system_name = basename(argv[0]);
	crm_log_init(crm_system_name, LOG_ERR, FALSE, FALSE, argc, argv);
	
	if(argc < 2) {
		usage(crm_system_name, LSB_EXIT_EINVAL);
	}
	
	while (1) {
#ifdef HAVE_GETOPT_H
		flag = getopt_long(argc, argv, OPTARGS,
				   long_options, &option_index);
#else
		flag = getopt(argc, argv, OPTARGS);
#endif
		if (flag == -1)
			break;

		switch(flag) {
			case 'V':
				cl_log_enable_stderr(TRUE);
				alter_debug(DEBUG_INC);
				break;
			case '?':
				usage(crm_system_name, LSB_EXIT_OK);
				break;
			case 'G':
				DO_WRITE = FALSE;
				break;
			case 'Q':
				BE_QUIET = TRUE;
				break;
			case 'D':
				DO_DELETE = TRUE;
				break;
			case 'U':
				crm_debug_2("Option %c => %s", flag, optarg);
				dest_uname = optarg;
				break;
			case 'u':
				crm_debug_2("Option %c => %s", flag, optarg);
				dest_node = crm_strdup(optarg);
				break;
			case 's':
				crm_debug_2("Option %c => %s", flag, optarg);
				set_name = crm_strdup(optarg);
				break;
			case 'l':
				crm_debug_2("Option %c => %s", flag, optarg);
				type = optarg;
				break;
			case 't':
				crm_debug_2("Option %c => %s", flag, optarg);
				type = optarg;
				break;
			case 'n':
				crm_debug_2("Option %c => %s", flag, optarg);
				attr_name = crm_strdup(optarg);
				break;
			case 'i':
				crm_debug_2("Option %c => %s", flag, optarg);
				attr_id = crm_strdup(optarg);
				break;
			case 'v':
				crm_debug_2("Option %c => %s", flag, optarg);
				attr_value = optarg;
				break;
			case 'r':
				crm_debug_2("Option %c => %s", flag, optarg);
				rsc_id = optarg;
				break;
			case '!':
				crm_warn("Inhibiting notifications for this update");
				cib_opts |= cib_inhibit_notify;
				break;
			default:
				printf("Argument code 0%o (%c) is not (?yet?) supported\n", flag, flag);
				++argerr;
				break;
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}

	if (optind > argc) {
		++argerr;
	}

	if (argerr) {
		usage(crm_system_name, LSB_EXIT_GENERIC);
	}

	the_cib = cib_new();
	rc = the_cib->cmds->signon(
		the_cib, crm_system_name, cib_command_synchronous);

	if(rc != cib_ok) {
		fprintf(stderr, "Error signing on to the CIB service: %s\n",
			cib_error2string(rc));
		return rc;
	}	

	if(safe_str_eq(crm_system_name, "crm_attribute")
	   && type == NULL && dest_uname == NULL) {
		/* we're updating cluster options - dont populate dest_node */
		type = XML_CIB_TAG_CRMCONFIG;

	} else if(dest_uname == NULL) {
		struct utsname name;
		if(uname(&name) < 0) {
			cl_perror("uname(2) call failed");
			return 1;
		}
		dest_uname = name.nodename;
		crm_info("Detected uname: %s", dest_uname);
	}

	if(dest_node == NULL && dest_uname != NULL) {
		rc = query_node_uuid(the_cib, dest_uname, &dest_node);
		if(rc != cib_ok) {
			fprintf(stderr,"Could not map uname=%s to a UUID: %s\n",
				dest_uname, cib_error2string(rc));
			return rc;
		} else {
			crm_info("Mapped %s to %s",
				 dest_uname, crm_str(dest_node));
		}
	}

	if(safe_str_eq(crm_system_name, "crm_master")) {
		int len = 0;
		if(safe_str_eq(type, "reboot")) {
			type = XML_CIB_TAG_STATUS;
		} else {
			type = XML_CIB_TAG_NODES;
		}
		rsc_id = getenv("OCF_RESOURCE_INSTANCE");

		if(rsc_id == NULL && dest_node == NULL) {
			fprintf(stderr, "This program should only ever be "
				"invoked from inside an OCF resource agent.\n");
			fprintf(stderr, "DO NOT INVOKE MANUALLY FROM THE COMMAND LINE.\n");
			return 1;

		} else if(dest_node == NULL) {
			fprintf(stderr, "Could not determine node UUID.\n");
			return 1;

		} else if(rsc_id == NULL) {
			fprintf(stderr, "Could not determine resource name.\n");
			return 1;
		}
		
		len = 8 + strlen(rsc_id);
		crm_malloc0(attr_name, len);
		snprintf(attr_name, len, "master-%s", rsc_id);

		len = 3 + strlen(type) + strlen(attr_name) + strlen(dest_node);
		crm_malloc0(attr_id, len);
		snprintf(attr_id, len, "%s-%s-%s", type, attr_name, dest_node);

		len = 8 + strlen(dest_node);
		crm_malloc0(set_name, len);
		snprintf(set_name, len, "master-%s", dest_node);
		
	} else if(safe_str_eq(crm_system_name, "crm_failcount")) {
		type = XML_CIB_TAG_STATUS;
		if(rsc_id == NULL) {
			fprintf(stderr,"Please specify a resource with -r\n");
			return 1;
		}
		if(dest_node == NULL) {
			fprintf(stderr,"Please specify a node with -U or -u\n");
			return 1;	
		}
	
		set_name = NULL;
		attr_name = crm_concat("fail-count", rsc_id, '-');
		
	} else if(safe_str_eq(crm_system_name, "crm_standby")) {
		if(dest_node == NULL) {
			fprintf(stderr,"Please specify a node with -U or -u\n");
			return 1;

		} else if(DO_DELETE) {
			rc = delete_standby(
				the_cib, dest_node, type, attr_value);
			
		} else if(DO_WRITE) {
			if(attr_value == NULL) {
				fprintf(stderr, "Please specify 'true' or 'false' with -v\n");
				return 1;
			}
			rc = set_standby(the_cib, dest_node, type, attr_value);

		} else {
			char *read_value = NULL;
			char *scope = NULL;
			if(type) {
				scope = crm_strdup(type);
			}
			rc = query_standby(
				the_cib, dest_node, &scope, &read_value);

			if(BE_QUIET == FALSE) {
				if(attr_id) {
					fprintf(stdout, "id=%s ", attr_id);
				}
				if(attr_name) {
					fprintf(stdout, "name=%s ", attr_name);
				}
				fprintf(stdout, "scope=%s value=%s\n",
					scope, read_value?read_value:"(null)");
				
			} else if(read_value != NULL) {
				fprintf(stdout, "%s\n", read_value);
			} else if(rc == cib_NOTEXISTS) {
				fprintf(stdout, "off\n");
				rc = cib_ok;
			}
			crm_free(scope);
		}
		is_done = TRUE;

	} else if (type == NULL) {
		type = XML_CIB_TAG_NODES;
		return 1;
	}

	if(safe_str_eq(type, XML_CIB_TAG_CRMCONFIG)) {
		dest_node = NULL;
	}	

	if(is_done) {
			
	} else if(DO_DELETE) {
		rc = delete_attr(the_cib, cib_opts, type, dest_node, set_name,
				 attr_id, attr_name, attr_value, TRUE);

		if(rc == cib_NOTEXISTS) {
		    /* Nothing to delete...
		     * which means its not there...
		     * which is what the admin wanted
		     */
		    rc = cib_ok;
		} else if(rc != cib_missing_data
			  && safe_str_eq(crm_system_name, "crm_failcount")) {
			char *now_s = NULL;
			time_t now = time(NULL);
			now_s = crm_itoa(now);
			update_attr(the_cib, cib_sync_call,
				    XML_CIB_TAG_CRMCONFIG, NULL, NULL, NULL,
				    "last-lrm-refresh", now_s, TRUE);
			crm_free(now_s);
		}
			
	} else if(DO_WRITE) {
		CRM_DEV_ASSERT(type != NULL);
		CRM_DEV_ASSERT(attr_name != NULL);
		CRM_DEV_ASSERT(attr_value != NULL);

		rc = update_attr(the_cib, cib_opts, type, dest_node, set_name,
				 attr_id, attr_name, attr_value, TRUE);

	} else {
		char *read_value = NULL;
		rc = read_attr(the_cib, type, dest_node, set_name,
			       attr_id, attr_name, &read_value, TRUE);

		if(rc == cib_NOTEXISTS 
		   && safe_str_eq(crm_system_name, "crm_failcount")) {
			rc = cib_ok;
			read_value = crm_strdup("0");
		}
		
		crm_info("Read %s=%s %s%s",
			 attr_name, crm_str(read_value),
			 set_name?"in ":"", set_name?set_name:"");

		if(rc == cib_missing_data) {
		    rc = cib_ok;
		    
		} else if(BE_QUIET == FALSE) {
			fprintf(stdout, "%s%s %s%s value=%s\n",
				attr_id?"id=":"", attr_id?attr_id:"",
				attr_name?"name=":"", attr_name?attr_name:"",
				read_value?read_value:"(null)");

		} else if(read_value != NULL) {
			fprintf(stdout, "%s\n", read_value);
		}
	}
	the_cib->cmds->signoff(the_cib);
	if(rc == cib_missing_data) {
		    printf("Please choose from one of the matches above and suppy the 'id' with --attr-id\n");
	} else if(rc != cib_ok) {
		fprintf(stderr, "Error performing operation: %s\n",
			cib_error2string(rc));
	}
	return rc;
}


void
usage(const char *cmd, int exit_status)
{
	FILE *stream;

	stream = exit_status ? stderr : stdout;
	if(safe_str_eq(cmd, "crm_master")) {
		fprintf(stream, "usage: %s [-?VQ] -(D|G|v) [-l]\n", cmd);

	} else if(safe_str_eq(cmd, "crm_standby")) {
		fprintf(stream, "usage: %s [-?V] -(u|U) -(D|G|v) [-l]\n", cmd);

	} else if(safe_str_eq(cmd, "crm_failcount")) {
		fprintf(stream, "usage: %s [-?V] -(u|U) -(D|G|v) -r\n", cmd);

	} else {
		fprintf(stream, "usage: %s [-?V] -(D|G|v) [options]\n", cmd);
	}
	
	fprintf(stream, "Options\n");
	fprintf(stream, "\t--%s (-%c)\t: this help message\n", "help", '?');
	fprintf(stream, "\t--%s (-%c)\t: "
		"turn on debug info. additional instances increase verbosity\n",
		"verbose", 'V');
	fprintf(stream, "\t--%s (-%c)\t: Print only the value on stdout"
		" (use with -G)\n", "quiet", 'Q');
	fprintf(stream, "\t--%s (-%c)\t: "
		"Retrieve rather than set the %s\n", "get-value", 'G',
		safe_str_eq(cmd, "crm_master")?"named attribute":"preference to be promoted");
	fprintf(stream, "\t--%s (-%c)\t: "
		"Delete rather than set the attribute\n", "delete-attr", 'D');
	fprintf(stream, "\t--%s (-%c) <string>\t: "
		"Value to use (ignored with -G)\n", "attr-value", 'v');
	fprintf(stream, "\t--%s (-%c) <string>\t: "
		"The 'id' of the attribute. Advanced use only.\n", "attr-id", 'i');

	if(safe_str_eq(cmd, "crm_master")) {
		fprintf(stream, "\t--%s (-%c) <string>\t: "
			"How long the preference lasts (reboot|forever)\n",
			"lifetime", 'l');
		exit(exit_status);
	} else if(safe_str_eq(cmd, "crm_standby")) {
		fprintf(stream, "\t--%s (-%c) <node_uuid>\t: "
			"UUID of the node to change\n", "node-uuid", 'u');
		fprintf(stream, "\t--%s (-%c) <node_uname>\t: "
			"uname of the node to change\n", "node-uname", 'U');
		fprintf(stream, "\t--%s (-%c) <string>\t: "
			"How long the preference lasts (reboot|forever)\n"
			"\t    If a forever value exists, it is ALWAYS used by the CRM\n"
			"\t    instead of any reboot value\n", "lifetime", 'l');
		exit(exit_status);
	}
	
	fprintf(stream, "\t--%s (-%c) <node_uuid>\t: "
		"UUID of the node to change\n", "node-uuid", 'u');
	fprintf(stream, "\t--%s (-%c) <node_uname>\t: "
		"uname of the node to change\n", "node-uname", 'U');

	if(safe_str_eq(cmd, "crm_failcount")) {
		fprintf(stream, "\t--%s (-%c) <resource name>\t: "
			"name of the resource to operate on\n", "resource-id", 'r');
	} else {
		fprintf(stream, "\t--%s (-%c) <string>\t: "
			"Set of attributes in which to read/write the attribute\n",
			"set-name", 's');
		fprintf(stream, "\t--%s (-%c) <string>\t: "
			"Attribute to set\n", "attr-name", 'n');
		fprintf(stream, "\t--%s (-%c) <string>\t: "
			"Which section of the CIB to set the attribute: (%s|%s|%s)\n",
			"type", 't',
			XML_CIB_TAG_NODES, XML_CIB_TAG_STATUS, XML_CIB_TAG_CRMCONFIG);
		fprintf(stream, "\t    -t=%s options: -(U|u) -n [-s]\n", XML_CIB_TAG_NODES);
		fprintf(stream, "\t    -t=%s options: -(U|u) -n [-s]\n", XML_CIB_TAG_STATUS);
		fprintf(stream, "\t    -t=%s options: -n [-s]\n", XML_CIB_TAG_CRMCONFIG);
	}

	if(safe_str_neq(crm_system_name, "crm_standby")) {
		fprintf(stream, "\t--%s (-%c)\t: "
			"Make a change and prevent the TE/PE from seeing it straight away.\n"
			"\t    You may think you want this option but you don't."
			" Advanced use only - you have been warned!\n", "inhibit-policy-engine", '!');
	}	
	
	fflush(stream);

	exit(exit_status);
}
