# Development Environment

Development of this project was primarily undertaken in a Linux virtual machine running Debian 8 (Jessie) using the Eclipse (Mars) IDE and an ARM Hard Float cross compiler tool chain.

## Installation

### Debian Jessie
To install Debian Jessie follow the [Debian GNU/Linux Installation Guide](https://www.debian.org/releases/stable/amd64/).

### Cross Compilation Tools
A cross compilation toolchain is required to build binary executable files for the ARM based BeagleBone Black using a more powerful desktop computer.  Typically the desktop computer has an Intel x86 processor.  The default C/C++ compiler and toolchain installed with Debian amd64 will only build executables to run on an Intel x86 processor.  To work around this a cross-compiler is required, one that runs on x86 but builds binaries for ARM.

Linux, particularly Debian, is used to achieve this as it has a realtively simple method of installing a cross compilation toolchain.

#####1. Change to a root shell in your Debian machine:

	user@debian:~$ sudo su -`

#####2. Create an apt source.list file for the embedian toolchain:

	root@debian:~# nano -w /etc/apt/apt.sources.d/embedian.list
	
Add the following content:

	# emdebian
	deb http://www.emdebian.org/debian unstable main
	deb http://ftp.au.debian.org/debian unstable main contrib non-free
	
 Exit and save the file with `Ctrl-x`

#####3. Install the embedian signature key

	curl http://embedian.org/embedian-toolchain-archive.key | sudo apt-key add -

#####4. Install the cross compiler toolchain:

	apt-get install build-essential {libc6,libc6-dev,linux-libc-dev,libstdc++6}-armhf-cross {binutils,gcc-4.7,g++-4.7}-arm-linux-gnueabihf
	
#####5. Create symlinks to installed compiler versions
When using Eclipse it is easier to simply specify the compiler as `arm-linux-gnueabihf-gcc`, so we'll make a couple of symlinks to the versions of gcc and g++ that we just installed:

	cd /usr/bin
	ln -sf arm-linux-gnueabihf-gcc-4.7 arm-linux-gnueabihf-gcc
	ln -sf arm-linux-gnueabihf-g++-4.7 arm-linux-gnueabihf-g++
	exit
	
#####6. Install a Java Runtime Environment (JRE) for Eclipse
Eclipse is written in Java and requires a suitable Java Runtime Environment (JRE).  The one that may be installed by default in Debian probably won't play nicely with Eclipse.  The easiest thing to do is install the latest JRE direct from Oracle (formerly Sun).

Head over to [www.java.com](http://www.java.com) and find the latest installer for Linux, eg: http://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html.  You want the Linux x64 .tar.gz file.

	wget http://download.oracle.com/otn-pub/java/jdk/8u60-b27/jre-8u60-linux-x64.tar.gz
	sudo mkdir /usr/java
	sudo tar zxvf jre-8u60-linux-x64.tar.gz
	sudo mv jre1.8.0_60/ /usr/java
	sudo update-alternatives --install "/usr/bin/java" "java" "/usr/java/jre1.8.0_60/bin/java" 1
	sudo update-alternatives --set java /usr/bin/java/jre1.8.0_60/bin/java
	
######7. Installing & Configuring Eclipse
Download the latest version of the Eclipse CDT (C/C++ Development Tooling) from [www.eclipse.org](http://www.eclipse.org).  The latest version is currently **Mars**.
Once the file has downloaded move it to the home directory and unpack it.  Eclipse will be installed in the user account only (ie: not system wide)

	user@debian:~/Downloads$ mv eclipse* ~/
	user@debian:~/Downloads$ cd ..
	user@debian:~$ tar xvf eclipse-cpp-mars-R-linux-gtk-x86_64.tar.gz
	user@debian:~$ cd eclipse
	user@debian:~/eclipse$ ./eclipse &
	
* When you create a new project select **Cross GCC** as the toolchain to be used.
* At the "Cross GCC Command" prompt enter `arm-linux-gnueabihf-` as the cross-compiler prefix.
* Set the path to `/usr/bin`

If compilation fails due to unfound headers or libraries go into the **Project Properties > C/C++ > General > Paths and Symbols** and set the following:
* Includes > GNU C
	- Include Directory: `/usr/arm-linux-gnueabuihf/include`
* Includes > GNU C++
	- Include Directory: `/usr/arm-linux-gnueabuihf/include/c++/4.7.2` (or the version you have installed)
* Library Paths (*not* Libraries)
	- ``/usr/arm-linux-gnueabuihf/lib`

 