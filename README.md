# ThinkPad Dual Scan VRAM fix 
`tpdualscan.c` (also known as `tpdscan.c`) was originally written by [Michael Steiner](http://web.archive.org/web/19991114011731/http://www.zurich.ibm.com/~sti/tplinux.html) in the 90s. I've committed it to this repository so it can be found using Google.
 
As [written by](https://lists.debian.org/debian-devel-announce/2016/05/msg00001.html) Debian developer Ben Hutchings in 2015: "gcc for i386 has recently been changed to target 686-class processors and is generating code that will crash on other processors". LLVM is also [not supporting i486](https://reviews.llvm.org/D42154#977976), since it assumes at least the (original) Pentium. Although this might change in the future. Using an older gcc release is the only feasable option at the moment.

### Compilation instructions
It is needed to apply some patches to gcc 4.8.2 so it can be built using gcc 9.3.0... 🙃

```
git clone https://github.com/eloydegen/tpdualscan
cd tpdualscan
sudo apt install build-essential
https://ftp.gnu.org/gnu/gcc/gcc-4.8.2/gcc-4.8.2.tar.bz2
tar xvf gcc-4.8.2.tar.bz
cd gcc-4.8.2
./contrib/download_prerequisites
./configure
wget https://gcc.gnu.org/git/?p=gcc.git;a=commitdiff_plain;h=ec1cc0263f156f70693a62cf17b254a0029f4852 build.patch
patch -p1 < build.patch 
wget https://gcc.gnu.org/git/?p=gcc.git;a=patch;h=883312dc79806f513275b72502231c751c14ff72 build2.patch
patch -p1 < build2.patch 
make
./bin/gcc -march=i486 ./tpdualscan.c -o tpdualscan
```

Also apply these patches:
* https://github.com/google/sanitizers/issues/822#issuecomment-782124081
* https://patchwork.ozlabs.org/project/gcc/patch/6824253.3U2boEivI2@devpool21/
* https://stackoverflow.com/a/48647715

### Easier way
Open an Ubuntu 12.04 ISO and run the virtual machine. Replace the apt subdomain in sources.list with `old-releases` and install gcc and gcc-multiplib. Note that the gcc multilib version and gcc needs to be equal.

## Links
* https://groups.csail.mit.edu/mac/ftpdir/thinkpad/old-archive/HTML/tp-1995/msg01241.html
* https://www.thinkwiki.org/wiki/WD90C24
* https://www.linuxjournal.com/article/3429
