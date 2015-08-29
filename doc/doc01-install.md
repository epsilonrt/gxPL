Basic Installation

These are installation instructions.

Clone sources from epsilonrt.com git repos (or untar archive file):

<pre class="fragment">
git clone http://git.epsilonrt.com/gxPL
</pre>

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

Run hub tool as daemon:

<pre class="fragment">
gxpl-hub
</pre>

Build and run clock example:

<pre class="fragment">
cd ../examples
make
cd clock
./gxpl-clock -xlpdebug
</pre>

The shared and static libraries are installed into /usr/local/lib and 
header files is installed in /usr/local/include.  
You can changed the destinations in the Makefile if you'd like.
