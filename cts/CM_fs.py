#!/usr/bin/python

'''CTS: Cluster Testing System: Failsafe dependent modules...

Classes related to testing high-availability clusters...

Lots of things are implemented.

Lots of things are not implemented.

We have many more ideas of what to do than we've implemented.
 '''

__copyright__='''
Copyright (C) 2000, 2001 Alan Robertson <alanr@unix.sh>
Licensed under the GNU GPL.
'''

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

#import types, string, select, sys, time, re, os, random, struct
#from os import system
#from UserDict import UserDict
#from syslog import *
#from popen2 import Popen3

class FailSafeCM(ClusterManager):
    '''
    The FailSafe cluster manager class.
    Not implemented yet.
    '''
    def __init__(self, randseed=None):

        ClusterManager.__init__(self, randseed=randseed)
        self.update({
            "Name"	       : "FailSafe",
            "StartCmd"	       : None, # Fix me!
            "StopCmd"	       : None, # Fix me!
            "StatusCmd"	       : None, # Fix me!
            "RereadCmd"	       : None, # Fix me!
            "TestConfigDir"    : None, # Fix me!
            "LogFileName"      : None, # Fix me!

            "Pat:We_started"   : None, # Fix me!
            "Pat:They_started" : None, # Fix me!
            "Pat:We_stopped"   : None, # Fix me!
            "Pat:They_stopped" : None, # Fix me!

            "BadRegexes"     : None, # Fix me!
        })
        self._finalConditions()

    def SyncTestConfigs(self):
        pass

    def SetClusterConfig(self):
        pass

    def ResourceGroups(self):
          raise ValueError("Forgot to write ResourceGroups()")
