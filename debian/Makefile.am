#
# linux-ha: Linux-HA heartbeat code
#
# Copyright (C) 2001 Michael Moerz
#                    This instance created by Horms
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
MAINTAINERCLEANFILES    = Makefile.in

EXTRA_DIST		= apply changelog compat control copyright \
			  rules \
			  heartbeat.dirs heartbeat.install \
			  heartbeat.postinst heartbeat.postrm \
			  heartbeat.preinst \
			  heartbeat-dev.dirs heartbeat-dev.install \
			  heartbeat-2.prerm \
			  ldirectord.install ldirectord.postinst \
			  ldirectord.postrm \
			  patches/ipv6-check.patch \
			  patches/init-remote_fs.patch \
			  patches/init-openhpid.patch \
			  patches/init-ntp.patch \
			  patches/series/2.1.3-1 \
			  patches/series/2.1.3-2

clean-local:
	rm -f *.substvars *.debhelper files
	rm -r -f heartbeat heartbeat-dev ldirectord \
		libpils0 libpils-dev libstonith0 libstonith-dev stonith tmp
