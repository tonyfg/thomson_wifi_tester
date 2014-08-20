# Thomson wifi password tester

Small C program I created a few years ago to test the performance of hashing implementations in several encryption libraries by implementing the Thomson wifi router default password calculation algorithm.
It supports multiple CPUs as well as multicore CPUs and compiles against libopenssl, libtomcrypt, and libgcrypt.

Since there are almost no vulnerable routers out there anymore I'm releasing this into the wild.


PS: In case anyone cares, libtomcrypt seems to be the faster of the 3 libraries (at least in my computer).



The source code is licenced under the WTFPL, but when compiled with each of the SSL libraries it is also licenced under that library's licence to avoid licencing conflicts.


[![WTFPL][2]][1]

© 2014 WTFPL – Do What the Fuck You Want to Public License.

  [1]: http://www.wtfpl.net/
  [2]: http://www.wtfpl.net/wp-content/uploads/2012/12/wtfpl-badge-1.png
