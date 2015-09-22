# Development Environment

Development of this project was primarily undertaken in a Linux virtual machine running Debian 8 (Jessie) using the Eclipse (Mars) IDE and an ARM Hard Float cross compiler tool chain. Binaries were built on the development machine and transferred via ssh to the BeagleBone Black.  Remote debugging with Eclipse is also possible (though a little difficult for a multi-threaded application).

Contents
--------
* [Debian Jessie](#debian-jessie)
* [Cross Compilation Tools](#cross-Compilation-Cools)
* [SSH to/from your BeagleBone Black](#ssh-tofrom-your-beaglebone-black)
* [Remote Debugging](#remote-debugging)
* [ARM Emulation](#arm-emulation)
	* [Chrooting into an SD Card](chrooting-into-an-sd-card)
* [References](#references)

- - - -

##Debian Jessie
To install Debian Jessie follow the [Debian GNU/Linux Installation Guide](https://www.debian.org/releases/stable/amd64/).

- - - -

##Cross Compilation Tools
A cross compilation toolchain is required to build binary executable files for the ARM based BeagleBone Black using a more powerful desktop computer.  Typically the desktop computer has an Intel x86 processor.  The default C/C++ compiler and toolchain installed with Debian amd64 will only build executables to run on an Intel x86 processor.  To work around this a cross-compiler is required, one that runs on x86 but builds binaries for ARM.

Linux, particularly Debian, is used to achieve this as it has a realtively simple method of installing a cross compilation toolchain.

####1. Change to a root shell in your Debian machine:

	user@debian:~$ sudo su -

####2. Create an apt source.list file for the embedian toolchain:

	root@debian:~# nano -w /etc/apt/apt.sources.d/embedian.list
	
Add the following content:

	# emdebian
	deb http://www.emdebian.org/debian unstable main
	deb http://ftp.au.debian.org/debian unstable main contrib non-free
	
Exit and save the file with `Ctrl-x`

####3. Install the embedian signature key

	root@debian:~# curl http://embedian.org/embedian-toolchain-archive.key | apt-key add -

####4. Install the cross compiler toolchain:

	root@debian:~# apt-get install build-essential \
						   {libc6,libc6-dev,linux-libc-dev,libstdc++6}-armhf-cross \
						   {binutils,gcc-4.7,g++-4.7}-arm-linux-gnueabihf
	
####5. Create symlinks to installed compiler versions
When using Eclipse it is easier to simply specify the compiler as `arm-linux-gnueabihf-gcc`, so we'll make a couple of symlinks to the versions of gcc and g++ that we just installed:

	root@debian:~# cd /usr/bin
	root@debian:/usr/bin# ln -sf arm-linux-gnueabihf-gcc-4.7 arm-linux-gnueabihf-gcc
	root@debian:/usr/bin# ln -sf arm-linux-gnueabihf-g++-4.7 arm-linux-gnueabihf-g++
	root@debian:/usr/bin# exit
	
####6. Install a Java Runtime Environment (JRE) for Eclipse
Eclipse is written in Java and requires a suitable Java Runtime Environment (JRE).  The one that may be installed by default in Debian probably won't play nicely with Eclipse.  The easiest thing to do is install the latest JRE direct from Oracle (formerly Sun).

Head over to [www.java.com](http://www.java.com) and find the latest installer for Linux
> eg: http://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html.  You want the Linux x64 .tar.gz file.

	user@debian:~$ wget http://download.oracle.com/otn-pub/java/jdk/8u60-b27/jre-8u60-linux-x64.tar.gz
	user@debian:~$ sudo mkdir /usr/java
	user@debian:~$ sudo tar zxvf jre-8u60-linux-x64.tar.gz
	user@debian:~$ sudo mv jre1.8.0_60/ /usr/java
	user@debian:~$ sudo update-alternatives --install "/usr/bin/java" "java" "/usr/java/jre1.8.0_60/bin/java" 1
	user@debian:~$ sudo update-alternatives --set java /usr/bin/java/jre1.8.0_60/bin/java
	
####7. Installing & Configuring Eclipse
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
	- `/usr/arm-linux-gnueabuihf/lib`
	
####8. Useful Eclipse Plugins
Eclipse has a myriad of plugins that can be used to alter the functions of the IDE (even the look and feel of the user interface).  Here's a couple that are useful during development
* Eclipse GitHub integration with task focused interface
* Eclox plugin for Eclipse (add `http://download.gna.org/eclox/update/` as a new site in **Help > Install New Software**)

To fully utilise the Doxygen plugin make sure Doxygen itself is installed on your system:

	user@debian:~$ sudo apt-get install doxygen

- - - -
	
##SSH to/from your BeagleBone Black
SSH is the easiest and most common method for accessing your BeagleBone Black, either to get shell access (a command line prompt) or to copy files easily.  To make the process easier you can configure ssh keys for authentication on both your BeagleBone Black and your development machine.

###Development Machine
Execute the following commands on your development machine to generate a new key (replace debian@beaglebone.local with the appropriate user and host for your BeagleBone Black).  Leave the password blank when asked.

	user@debian:~$ ssh-keygen -t rsa -f ~/.ssh/beaglebone_rsa
	user@debian:~$ ssh-copy-id ~/.ssh/beaglebone_rsa.pub debian@beaglebone.local
 	user@debian:~$ ssh-add
 	
###BeagleBone Black
The process is the same, but in this instance we name the key for to be the devlopment machine.  This will allow you to ssh/scp from the BeagleBone Black to your dev machine. Again, alter usernames and hosts as appropriate for your system and leave the password blank when asked.

	root@beaglebone:~$ ssh-keygen -t rsa -f ~/.ssh/development_rsa
	root@beaglebone:~$ ssh-copy-id ~/.ssh/development_rsa.pub user@development.local
 	root@beaglebone:~$ ssh-add
 
###Automatically copy compiled executable from Eclipse to BBB
It is possible to have Eclipse automatically copy the compiled executable from your development environment directly to the BBB on completion of a successfull compilation.  To achieve this go to **Project > Properties > C/C++ Build > Settings > Build Steps > Post-build steps** and set the command to be:

	scp ${ProjName} root@beaglebone.local:/home/debian/

- - - -
	
##Remote Debugging
Using Eclipse it is possible to execute your application on the BeagleBone Black and debug it within Eclipse on you development machine.  To do this you need to install packages on both the BBB and your development machine:

###BeagleBone Black

	user@debian:~$ sudo apt-get install gdbserver
	
###Development Machine

	user@debian:~$ sudo apt-get install gdb-multiarch
	
You will also need to create the following file in your Eclipse Project directory:

	user@debian:~$ nano -w .gdbinit

Add the following content to it:

	set architecture arm
	
####Configure Eclipse for Remote Debugging
To configure Eclipse for remote debugging make sure you are in the C/C++ Perspective View and go to **Run > Debug Configurations > C/C++ Remote Application** and add a new configuration. Most settings should be self evident, however there are a couple that will need customisation:
* Remote Absolute File Path for C/C++ Application
* Debugger > GDB debugger, set to: `/usr/bin/gdb-multiarch
* Debugger > GDB command file, set to: `.gdbinit`
You might also need to configure a new connection to your BeagleBone Black, choose **SSH** as the connection type and follow the prompts.

Once done you should be able to launch your remote debugging target from Eclipse and step through your program as it executes on your BBB.

- - - -

##ARM Emulation
It is possible to run ARM binaries on an x86 system using something called QEMU.  QEMU is an emulation system that can translate the ARM instruction calls on the fly and execute them on an x86 processor.  This can be handy if you want to quickly test a small aspect of a program that doesn't rely on any specific hardware features (like GPIOs).

####1. Install QEMU
To get started install the required qemu packages on your development machine:

	user@debian:~$ sudo apt-get install qemu-user-static

####2. Install an ARMHF Change Root
Next you will need to install an armhf Change Root (chroot).  This is essentially a basic installation of Debian for the ARM architecture:

	user@debian:~$ sudo mkdir /BBBchroot
	user@debian:~$ sudo apt-get install debootstrap
	user@debian:~$ sudo debootstrap --foreign --verbose --arch=armhf --include=\
					aptitude,iputils-ping,module-init-tools,ifupdown,\
					iproute,nano,wget,udev jessie /BBBchroot \
					http://ftp.us.debian.org/debian

This will install a chroot to `/BBBchroot`

####3. Finish Change Root installation
If you try to execute any commands in this directory (eg: `/BBBchroot/bin/ls`) they will fail at the moment because they are all ARM executables and we haven't configured the emulation yet.

To configure QEMU in the chroot do the following:

	user@debian:~$ sudo cp /usr/bin/qemu-arm-static /BBBchroot/usr/bin

Now to enter the emulated ARM environment run the following command

	user@debian:~$ sudo chroot /BBBchroot/

Now we need to finish the Debian installation that we started earlier and set a password for the root account in the chroot environment:

	name!@debian:~$ cd /debootsrap
	name!@debian:~$ ./debootstrap --second-stage

That will install a suite of packages, when it is done set a password and exit.

	name!@debian:~$ passwd
	name!@debian:~$ exit

Now to execute an ARM binary you simply copy it inside `/BBBchroot`, say at `/BBBchroot/root` and then enter the chroot environment with:

	user@debian:~$ sudo chroot /BBBchroot/

From there you can execute your program and it will run as if it was on an ARM processor.
> **Note:** If your program requires access to hardware specific devices such as GPIOs or UARTs these will not be available.  The same is true for files located in `/sys`, `/proc` and `/dev`

###Chrooting into an SD Card
QEMU can also be used to chroot into an SD card image of your BeagleBone Black. There are a variety of handy scripts on your BeagleBone Black at `/opt/scripts/tools`. In the `eMMC` directory you will find a script that can copy the eMMC (flash) of your BeagleBone Black to a uSD card.  Simply insert an empty uSD card of at least 8GB and then run:

	debian@beaglebone:/opt/scripts/tools/eMMC$ sudo beaglebone-black-make-microSD-flasher-from-eMMC.sh

The LEDs on the BeagleBone Black should begin cycling back and forth, when this is done your SD card is ready.

To chroot into the SD card insert it into your development machine (if it's a virtual machine you might need to pass the device through from the host) and mount it somewhere.  You'll need the rootfs partition mounted.

Once mounted we copy `qemu-arm-static` to `/usr/bin` like before and then chroot in:

	user@debian:~$ sudo cp /usr/bin/qemu-arm-static /media/user/rootfs/usr/bin
	user@debian:~$ sudo chroot /media/user/rootfs

Viola! You are now in a chroot environment that is identical to your BeagleBone Black (with the exception of hardware specific devices as stated previously).

- - - -

##References
Most of the material here was gleaned from hours of time spent with the Google search engine.

Two other invaluable references were:
* [Exploring BeagleBone](http://exploringbeaglebone.com) - I highly recommend the book, it is worth the price.
* [Derek Molloy's BeagleBone Articles](http://derekmolloy.ie/beaglebone/)
