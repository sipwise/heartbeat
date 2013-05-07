/* include/hb_config.h.  Generated from hb_config.h.in by configure.  */
/*
 * hb_config.h: Definitions from the Linux-HA heartbeat program
 *             for out-of-tree projects
 *
 * Copyright (C) 2007 Andrew Beekhof <abeekhof@suse.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef _HB_CONFIG_H
#define _HB_CONFIG_H

/* have new heartbeat api */
#define HAVE_NEW_HB_API 1

/* Define to 1 if the mgmt-daemon and friends are enabled. */
#define MGMT_ENABLED 1

/* $malloc_desc */
/* #undef CL_USE_LIBC_MALLOC */

/* Set if CLOCK_T is adequate by itself for the "indefinite future" (>= 100
   years) */
/* #undef CLOCK_T_IS_LONG_ENOUGH */

/* Group to own API fifos */
#define HA_APIGROUP "haclient"

/* GID to own API fifos */
#define HA_APIGID 496

/* User to own CCM fifos */
#define HA_CCMUSER "hacluster"

/* UID to own CCM fifos */
#define HA_CCMUID 498

/* Web site base URL */
#define HA_URLBASE "http://linux-ha.org/"

/* Logging Daemon IPC socket name */
#define HA_LOGDAEMON_IPC "/var/lib/log_daemon"

/* ocf ra dir */
#define OCF_RA_DIR "/usr/lib/ocf/resource.d/"

/* ocf root dir */
#define OCF_ROOT_DIR "/usr/lib/ocf"

/* heartbeat rc script directory */
#define HA_RC_DIR "/etc/ha.d/rc.d"

/* heartbeat logging socket */
#define HA_LOGDAEMON_IPC "/var/lib/log_daemon"

/* lib directory */
#define HA_LIBDIR "/usr/lib"

/* lib heartbeat directory */
#define HA_LIBHBDIR "/usr/lib/heartbeat"

/* top directory of area to drop core files in */
#define HA_COREDIR "/var/lib/heartbeat/cores"

/* var lib directory */
#define HA_VARLIBDIR "/var/lib"

/* var lib heartbeat directory */
#define HA_VARLIBHBDIR "/var/lib/heartbeat"

/* var lock directory */
#define HA_VARLOCKDIR "/var/lock"

/* var log directory */
#define HA_VARLOGDIR "/var/log"

/* var run directory */
#define HA_VARRUNDIR "/var/run"

/* var run heartbeat directory */
#define HA_VARRUNHBDIR "/var/run/heartbeat"

/* Heartbeat configuration directory */
#define HA_HBCONF_DIR "/etc/ha.d"

/* heartbeat ra dir */
#define HB_RA_DIR "/etc/ha.d/resource.d/"

/* lsb ra dir */
#define LSB_RA_DIR "/etc/init.d"

/* stonith ra dir */
#define STONITH_RA_DIR "/usr/lib/heartbeat/stonith.d/"

/* pils plugin dir */
#define HA_PLUGIN_DIR "/usr/lib/heartbeat/plugins"

/* ra plugin dir */
#define LRM_PLUGIN_DIR "/usr/lib/heartbeat/plugins/RAExec"

/* directory for the external stonith plugins */
#define STONITH_EXT_PLUGINDIR "/usr/lib/stonith/plugins/external"

#endif /* _HB_CONFIG_H */
