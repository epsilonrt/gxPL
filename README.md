# gxPLApplication  
*Tools, Application and Framework for xPL*

---
Copyright 2015 (c), Pascal JEAN aka epsilonRT  

Full documentation and installation is in the doc/ directory or on 
Project Homepage: [http://www.epsilonrt.com/gxPLApplication](http://www.epsilonrt.com/gxPLApplication)

**This project is under development and has, for now, only C API**

gxPLApplication is a fork of xPL4Linux whose development seems to have stopped 
since 2006. The development was resumed starting from version V1.3a 
released October 18, 2006.  
gxPLApplication aims to provide a cross-platform framework xPL in C, C ++ and 
Python using only open source software. The letter 'g' in gxPLApplication means 
"generic".  
xPL is an application protocol that is often above the Ethernet 
protocol, however, it was decided to make a real separation between 
the application layer and the network layer to evolve easily.  
 
gxPLApplication is a xPL framework that hides most of the details of 
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

You can get the latest development version using git :

    $ git clone http://git.epsilonrt.com/gxPLApplication

You can browse the source code on 
[http://gitweb.epsilonrt.com/gxPLApplication.git](http://gitweb.epsilonrt.com/gxPLApplication.git)

> xPL4Linux is Copyright (c) 2006, Gerald R Duprey Jr  
> xPL4Linux original homepage: [http://www.xpl4java.org/gxPL/](http://www.xpl4java.org/gxPL/)

---
gxPLApplication is licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License.  
You may obtain a copy of the License at:

http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

