Installing xPL4Linux

* Untar the xPL4Linux.tar file into an empty directory or clone from 
  git repos.  
* Go into the xPL4Linux directory and type make. This will compile the 
  components of xPL4Linux library (xPLLib).  
* If there are no errors (*), you can install it into your systems by 
  becoming root and typing "make install".  

<div class="fragment"><pre class="fragment">
git clone http://git.epsilonrt.com/xPL4Linux
cd xPL4Linux
make
sudo make install_utils
sudo make install
cd tools
make
xpl-hub
sudo make install
cd ../examples
make
cd clock
./xpl-clock -xlpdebug
</pre></div>

The shared and static libraries are installed into /usr/local/lib and 
header files is installed in /usr/local/include.  
You can changed the destinations in the Makefile if you'd like.
