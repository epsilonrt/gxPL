# gxPL
*Library, Tools and Framework for xPL*

---
Copyright 2015-2016 (c), epsilonRT                  

Full documentation and installation is in the doc/ directory or on 
Project Homepage: [http://www.epsilonrt.com/gxPL](http://www.epsilonrt.com/gxPL)

gxPL is a fork of xPL4Linux whose development seems to have stopped since 2006. 
The development was resumed starting from version V1.3a released October 18, 2006.
The objectives of the fork are:

*    Make the code modular describing xPL items as presented in the core documentation
*    Separating the hardware layer and the application layer because it is the main advantage of xPL!
*    Eliminate static variables to make the relocatable code and to be able to instantiate the "objects"

gxPL aims to provide a cross-platform framework xPL in C, C ++ and Python using 
only open source software. The letter 'g' in gxPL means "generic".
xPL is an application protocol that is often above the Ethernet protocol, 
however, it was decided to make a real separation between the application layer 
and the network layer to evolve easily. 

This project is under development and has, for now, only C API (Linux/Unix, ATMEL AVR8).
 
gxPL is a xPL framework that hides most of the details of 
dealing with xPL.  It will handle filtering messages, sending 
heartbeats, formatting and parsing messages, directing messages to 
handlers based on where they were bound from, etc.  
You can use it as the simplest level to format/send/receive/parse xPL 
messages or add various layers of additional management to simplify more 
complex xPL idioms.  
It's designed to be easy to design a new program around or to 
integrate into an existing program.  
It includes some example applications showing how to use various 
features of the framework, including the source to the gxpl-hub.

You can get the latest development version using git (public, read-only) :

    $ git clone http://github.com/epsilonrt/gxPL.git

You can browse the source code on 
[https://github.com/epsilonrt/gxPL](https://github.com/epsilonrt/gxPL)

##Basic Installation

These are installation instructions.

* Build and install the dependencies:

        git clone http://github.com/epsilonrt/sysio.git
        cd sysio
        make
        sudo make install


* Clone sources from github.com git repos (or unzip archive file):

        git clone http://github.com/epsilonrt/gxPL.git


* Build and install the library:

        cd gxPL
        make
        sudo make install

    The shared and static libraries are installed into /usr/local/lib and 
    header files is installed in /usr/local/include.  
    You can changed the prefix in the Makefile if you'd like.

* Build and install the tools (hub, logger ...):

        cd tools
        make
        sudo make install

* Run hub tool as daemon:

        gxpl-hub
    
    **Be sure to open the UDP Port 3865 on your firewall, or your xPL 
    network may not work !**

* Build and run clock example:

        cd ../examples
        make
        cd clock
        ./gxpl-clock -d

* If you plan to compile examples for the target AVR8 you must also install avrio:

        git clone http://github.com/epsilonrt/avrio.git
        cd avrio
        sudo make install
        
        make set-profile
        # Remove AVRIO_ROOT=... in /home/pascal/.profile
        # AVRIO_ROOT=/home/pascal/src/avrio was added
        #  in /home/pascal/.profile
        # You must log out for this to take effect.

---
gxPL is licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License.  
You may obtain a copy of the License at:

http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

> The xPL4Linux project that is no longer updated is Copyright (c) 2006, Gerald R Duprey Jr  
> xPL4Linux original homepage: [http://www.xpl4java.org/gxPL/](http://www.xpl4java.org/gxPL/)
