# nexfscli
nexfscli.c Nexustorage nexfs commandline interface

This file is part of Nexustorage Nexfs Storage stack

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Nexfs Public Preview Basic management of Nexfs with nexfscli

The recommended way to manage Nexfs is through the utility nexfscli which can be downloaded with nexfs.server.

nexfscli is also released as opensource allowing the nexfs community to tailor it to their needs while providing a working example of how to manage nexfs through code using the nexfs direct management interface.

To run nexfs who must either be the root user, or belong to the nexfs admin cmd group, and the cmd group must have the appropriate rights assigned.

nexfscli is a command-line utility that must be run on the same environment as the nexfs server.

It is also recommended that the nexfscli is used to start and stop the nexfs server.

Using nexfscli

To use nexfscli simply run the command followed by the required commandline arguments, for example to get the nexfs server status you will run:

nexfscli server status

Or to get to current loglevel of the running nexfs server, run:

nexfscli liveconfig get loglevel

Getting help

To get a list of the most common nexfscli options run:

	nexfscli help

To get a full list including less commonly run options:

	nexfscli -allhelp

Starting, stopping nexfs

To start nexfs run:

	nexfscli server start
To nicely stop nexfs, run:

	nexfscli server stop


While it is always advisable to perform a nice nexfs server stop, which pauses client traffic and attempts to flush any open buffers, if a  fast and hard stop of nexfs is required, run:

	nexfscli server forcestop

Note: running a force stop is not recommended other than as a last resort, a force stop will not flush open data buffer to permanent storage, which could result in data corruption. 


Getting the nexfs server status

To get the current nexfs server status, run

	nexfscli server status

Which for a running nexfs server will result in output similar to:

	Nexfs Mountpoint: /mnt/nexfs
	Server Status: OK
	Release: p1.00rc1
	Software Expires: Fri 2021-07-30 12:18:45
	Process ID: 2874971
	Uptime: 2 days 2 hours 14 minutes 49 seconds
	Open Files: 3/113
	Tier1 Struct Status: OK
	Tier1 Status: OK
	Tier1 Theashold Level: OK
	Tier1 Floor Level: FloorWaterMark
	Tier2 Status: OK
	Tier2 Theashold Level: OK
	Tier2 Floor Level: OK
	Tier3 Status: OK
	Job Scheduler: Enabled
	BG Migration Scheduler: Enabled
	Deletion Scheduler: Enabled
	Manual Commands: Enabled

Getting nexfs release information

To retrieve the version of a running nexfs server, run

	nexfscli release get nexfs 

and to get the version of nexfscli, run

	nexfscli release get nexfscli

Getting and changing a running nexfs servers log level

To retrieve the current (live) log level of a running nexfs server, run

	nexfscli liveconfig get loglevel

To change the current (live) log level of a running nexfs server, run

	nexfscli liveconfig set loglevel {NEWLOGLEVEL}

Replace {NEXLOGLEVEL} with one of:

CRIT, ERR, WARNING, NOTICE, INFO or DEBUG

For example to set nexfs server to log at debug level, run

	nexfscli liveconfig set loglevel DEBUG

Note: this will only change the loglevel for the running nexfs server, when the server is restarted the loglevel will revert back to that set in the configuration file. See Nexfs Configuration below for more information.

Nexfs Configuration using nexfscli

Many of the nexfs server settings can be made “live”, this means that there is no need to restart the nexfs server for changes to become active. 

But, just changing the live setting will not make that change permanent, when the nexfs server starts it reads permanent settings from its configurations files. As such if you want to change the live setting and for that new setting to become permanent, you need to change it in both the “liveconfig” and also the configuration file “configfile”. Both of these can be updated using nexfscli


Note: configuration tag names are case sensitive 

Getting a list of all live nexfs settings

To get a list of the current live configuration settings, which may be different from the permanent setting that will become active when the nexfs server restarts, run:

nexfscli liveconfig dump all


Which will result in a listing of all configurations settings along with their current “live” values. 

 
Getting a list of all permanent nexfs settings

To get a list of the permanent configuration settings, which may be different to the current live process settings, run:

nexfscli configile dump all


Which will result in a listing of all configurations settings along with their current “permanent” values. 


Getting the value of a single live nexfs settings

To get a list of a single live configuration settings, which may be different from the permanent setting that will become active when the nexfs server restarts, run:

	nexfscli liveconfig get value {CONFIGTAGNAME}

Replace  {CONFIGTAGNAME} with the configuration name you would like to retrieve the value of, a list of configuration names and their usage can be found here


Getting the value of a single permanent nexfs settings

To get a list of a single permanent configuration settings, which may be different from the live setting, run:

	nexfscli configfile get value {CONFIGTAGNAME}

Replace  {CONFIGTAGNAME} with the configuration name you would like to retrieve the value of, a list of configuration names and their usage can be found here


Getting more details on a single configuration setting

You can get more details of a configuration setting, including a basic description, allowed values and if a restart is needed to activate changes, run:

	nexfscli configfile get all {CONFIGTAGNAME}

Replace  {CONFIGTAGNAME} with the configuration name you would like to retrieve the value of, a list of configuration names and their usage can be found here

For example to get more details about the MOUNTPOINT configuration setting (tag), run:

nexfscli configfile get all MOUNTPOINT

While will result in output similar to:

	Tag Label = 'MOUNTPOINT'
Value = '/mnt/nexfs'
Help Text = 'NEXFS Mountpoint, needed for nexfscli to work, must match the nexfs mountpoint although this setting is not used here by the nexfs storgae server itself𖅓�'
Validation:String = '1'
Validation:Min(Value/StringLength) = '2'
Validation:Max(Value/StringLength) = '2048'


Change the value of a single live nexfs settings

To set the value of a single live configuration settings, which may be different from the permanent setting that will become active when the nexfs server restarts, run:

	nexfscli liveconfig set {CONFIGTAGNAME} {NEWVALUE}

Replace  {CONFIGTAGNAME} with the configuration name you would like to retrieve the value of, a list of configuration names and their usage can be found here

Replace {NEWVALUE} will the new required setting

For example, to pause the nexfs server, run

nexfscli liveconfig set NEXFSPAUSED 1


Note: live configuration changes are not permanent and their values will revert back to the permanent configuration value when nexfs is restarted, to make changes permanent you must change the permanent value, as detailed below.

Change the value of a single permanent nexfs settings

To set permanent configuration settings, which may be different from the live setting unless the same change is made in the live config or the nexfs server restarts, run:

	nexfscli configfile set {CONFIGTAGNAME} {NEWVALUE}

Replace  {CONFIGTAGNAME} with the configuration name you would like to retrieve the value of, a list of configuration names and their usage can be found here

Replace {NEWVALUE} will the new required setting

For example, to change the nexfs mountpoint, run

nexfscli configfile set MOUNTPOINT /mnt/nexfs

Note: permanent configuration file changes do not become live unless the nexfs server is restarted or the live value is updated, as detailed above.


Also, see advanced nexfscli usage and managing nexfs using the nexfs direct management interface.
