Basic Installation

These are installation instructions.

* Build and install the dependencies:

<pre class="fragment">
        git clone http://github.com/epsilonrt/sysio.git
        cd sysio
        make
        sudo make install
</pre>

* Clone sources from github.com git repos (or unzip archive file):

<pre class="fragment">
        git clone http://github.com/epsilonrt/gxPL.git
</pre>


* Build and install the library:

<pre class="fragment">
        cd gxPL
        make
        sudo make install
</pre>

The shared and static libraries are installed into /usr/local/lib and 
header files is installed in /usr/local/include.  
You can changed the destinations in the Makefile if you'd like.

* Build and install the tools (hub, logger ...):

<pre class="fragment">
        cd tools
        make
        sudo make install
</pre>

* Run hub tool as daemon:

<pre class="fragment">
        gxpl-hub
</pre>

**Be sure to open the UDP Port 3865 on your firewall, or your xPL 
network may not work !**

* Build and run clock example:

<pre class="fragment">
        cd ../examples
        make
        cd clock
        ./gxpl-clock -d
</pre>

* If you plan to compile examples for the target AVR8 you must also install avrio:

<pre class="fragment">
        git clone http://github.com/epsilonrt/avrio.git
        cd avrio
        sudo make install
        
        make set-profile
        # Remove AVRIO_ROOT=... in /home/pascal/.profile
        # AVRIO_ROOT=/home/pascal/src/avrio was added
        #  in /home/pascal/.profile
        # You must log out for this to take effect.
</pre>
