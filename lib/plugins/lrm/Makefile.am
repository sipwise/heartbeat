#
# Author: Sun Jiang Dong <sunjd@cn.ibm.com>
# Copyright (c) 2004 International Business Machines
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
MAINTAINERCLEANFILES = Makefile.in
LRM_DIR = lrm
INCLUDES 			=	-I$(top_builddir)/include -I$(top_srcdir)/include \
					-I$(top_builddir)/linux-ha -I$(top_srcdir)/linux-ha \
					-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl

halibdir			=	$(libdir)/@HB_PKG@
havarlibdir			=	$(localstatedir)/lib/@HB_PKG@
COMMONLIBS			=	$(top_builddir)/lib/clplumbing/libplumb.la   \
                        		$(GLIBLIB)

lrmdir				=	$(HA_VARLIBDIR)/$(HB_PKG)/$(LRM_DIR)
plugindir               	= 	$(halibdir)/plugins/RAExec
apigid                          =       @HA_APIGID@

plugin_LTLIBRARIES = lsb.la ocf.la heartbeat.la stonith.la
lsb_la_SOURCES = raexeclsb.c racommon.c
lsb_la_LDFLAGS = -L$(top_builddir)/lib/pils -lpils -export-dynamic -module -avoid-version

ocf_la_SOURCES = raexecocf.c racommon.c
ocf_la_LDFLAGS = -L$(top_builddir)/lib/pils -lpils -export-dynamic -module -avoid-version

stonith_la_SOURCES = raexecstonith.c racommon.c
stonith_la_LDFLAGS = -L$(top_builddir)/lib/pils -lpils -export-dynamic -module -avoid-version \
		     -L$(top_builddir)/lib/fencing -lstonithd \
		     -L$(top_builddir)/lib/stonith -lstonith

heartbeat_la_SOURCES = raexechb.c racommon.c
heartbeat_la_LDFLAGS = -L$(top_builddir)/lib/pils -lpils -export-dynamic -module -avoid-version

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(lrmdir)
	-chgrp $(apigid) $(DESTDIR)/$(lrmdir)
	chmod 770 $(DESTDIR)/$(lrmdir)
