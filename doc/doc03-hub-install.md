xPL Hub
> Copyright (c) 2005 -- Gerry Duprey  
> Copyright (c) 2015, epsilonRT                

## Overview

gxpl-hub is an implementation of the hub necessary for using xPL based
applications on a linux computer. It is compatible with any xPL application
needing a hub, regardless of the underlying framework the application uses.

Each computer that runs xPL based applications needs one xPL hub running.  In
cases where the computer has more than one network interface, you may need to
run one hub per interface, but that is a moderately unusual setup.

gxpl-hub comes with an automatic supervisory function.  When you start gxpl-hub,
a very small, simple and well tested supervisor is what really runs.  It then
starts the hub and monitors it.  If the hub dies for any reason, the
supervisor will clean it up and start a new hub.  Since hubs are so esscential
to the operation of xPL applications, this sort of function is crucial.

The Hub portion of gxpl-hub has been proven in operation on a number of
machines for long periods of times.  It's exceedingly unlikely it would crash
or cease to function in the first place.  Given that reliability and the
supervisory abilities to restart it in such a case, you should be able to
expect nearly 100% uptime. 

If the hub ever dies and needs to be restarted, gxpl-hub will make an entry in
your system log file (using the syslog facility).  In addition, in a worst
case scenario where the hub cannot run and the supervisor keeps trying to
restart it (which could consume a lot of system recources), there is a
"circuit breaker" that will kick in after the supervisor attempts to start he
hub more than 10,000 times.  If that happens, the supervisor will log a
message to the system explaining what happened (again, via syslog) and then
terminate.  In such a massive failure, 10,000 iterations can be completed in a
few minutes, so the machine will not be crippled for long.  This happening is
*very*, very unlikely.

In all the above just means you can expect 100% uptime from gxpl-hub once it is
started.

## Installation 

Build and install the library:

<pre class="fragment">
cd gxPL
make
sudo make install
</pre>

Build and install the tools (hub, logger ...):

<pre class="fragment">
cd tools
make
sudo make install
</pre>

## Testing the hub

Run the hub:

<pre class="fragment">
gxpl-hub -D
</pre>

## Hub service
The installation process installs a service in /etc/init.d, then it is 
easy to make automatic startup hub.

### Configuration

In most cases, you do not need to do any configuration but the service
may be configured in the /etc/gxpl-hub.conf file.

### Managing the hub service

for starting the service:

<pre class="fragment">
sudo /etc/init.d/gxpl-hub start
</pre>

The following operations are available:

- start
- stop
- restart
- status

On Debian for example, you can start hub service at boot time with:

<pre class="fragment">
sudo update-rc.d gxpl-hub enable
</pre>

