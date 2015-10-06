Linux based xPL hub with supervisory functions  
> Copyright (c) 2005 -- Gerry Duprey  
> Copyright (c) 2015, Pascal JEAN aka epsilonRT

## Overview

app-hub is an implementation of the hub necessary for using xPL based
applications on a linux computer.  It is compatible with any xPL application
needing a hub, regardless of the underlying framework the application uses.

Each computer that runs xPL based applications needs one xPL hub running.  In
cases where the computer has more than one network interface, you may need to
run one hub per interface, but that is a moderately unusual setup.

app-hub comes with an automatic supervisory function.  When you start app-hub,
a very small, simple and well tested supervisor is what really runs.  It then
starts the hub and monitors it.  If the hub dies for any reason, the
supervisor will clean it up and start a new hub.  Since hubs are so esscential
to the operation of xPL applications, this sort of function is crucial.

The Hub portion of app-hub has been proven in operation on a number of
machines for long periods of times.  It's exceedingly unlikely it would crash
or cease to function in the first place.  Given that reliability and the
supervisory abilities to restart it in such a case, you should be able to
expect nearly 100% uptime. 

If the hub ever dies and needs to be restarted, app-hub will make an entry in
your system log file (using the syslog facility).  In addition, in a worst
case scenario where the hub cannot run and the supervisor keeps trying to
restart it (which could consume a lot of system recources), there is a
"circuit breaker" that will kick in after the supervisor attempts to start he
hub more than 10,000 times.  If that happens, the supervisor will log a
message to the system explaining what happened (again, via syslog) and then
terminate.  In such a massive failure, 10,000 iterations can be completed in a
few minutes, so the machine will not be crippled for long.  This happening is
*very*, very unlikely.

In all the above just means you can expect 100% uptime from app-hub once it is
started.

## Installation 

Two executables are supplied -- app-hub and gxPLHub_static.  On most systems,
the app-hub executable is all you need.  However, if you get errors running
the app-hub (errors about linkage, libraries, etc), it may be the that app-hub
is not compatible with your system.  In that case, use the gxPLHub_static
executable (and just rename it to app-hub).  This a larger executable and will
consume a little bit more of RAM, but has no dependencies on your system.

Put the app-hub executable somewhere on your system.  /usr/local/bin is a good
place on most systems, though it could go in /usr/bin too.  Insure it has
execute and read permission (i.e. chmod a+rx xpl_Hub).

Next, arrange to haave the app-hub executable launched at system boot time.
On many system, this can be accomplished by adding a line to the /etc/rc.local
script.  Simply edit the script and add a line like this to the end of the
script

/usr/local/bin/app-hub

That's it -- it'll bind to your primary network interface and run.


## Configuration

In most cases, you do not need to do any configuration.

## Stopping the hub

In general, stopping the hub isn't needed when shutting down the computer --
whatever normal way the computer stops processes will be fine (even a rude
KILL/9).

However, if you should want the hub to stop, there are two processes to consider.  

When you look at the processes (use a command like "ps -ef | grep app-hub" to
list them), one will have a parent process ID (ppid) of 1 and one will have a
parent process id of the first one.

The one with the ppid of 1 is the supervisor.  If you wish to totally stop all
hub activity for good (or at least until you manually launch the app-hub
program and/or reboot), send a "TERM" signal to it (i.e. if it's pid (not
ppid) is 17223, then "kill -TERM 17223" would do it).  The supervisor will
stop the hub it is supervising and then stop itself.

On the otherhand, if you kill the other app-hub process, the one with the ppid
of the supervisory process, you will be killing an instance of the hub.  Once
it's dead, the supervisor will notice this and immediatly spawn a new one to
take it's place.  This may be what you want if you suspect the hub has locked
up but hasn't died (though this would be very unlikely).  Using a TERM kill
would be best, but really almost any ignal (TERM, INT, KILL) will work.

## Source Code

If you would like the source code for app-hub, go to the same web page you
downloaded this from and download the gxPLApplication framework.  It has
installation instructions for building the framework (should be a quick few
second build on most any system).  There is then an examples/ subdirectory
that contains the app-hub.c code which uses that framework.

Building the hub from source is easy as lonk as you first installed the
framework using the directions contained in the INSTALL file.
