#!/bin/sh
#
# Linux-HA: CIM Provider register tool 
#
# Author: Jia Ming Pan <jmltc@cn.ibm.com>
# Copyright (c) 2005 International Business Machines
# Licensed under the GNU GPL.
#
#

sh /usr/share/heartbeat/heartbeat/cim/do_register.sh -t  \
-r /usr/share/heartbeat/heartbeat/cim/LinuxHA.reg \
-m /usr/share/heartbeat/heartbeat/cim/LinuxHA.mof

