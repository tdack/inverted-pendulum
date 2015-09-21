# 3rd Party Libraries

Some 3rd party libraries were used at various times during development.

1. BlackLib
===========
BlackLib source code is included within the repository and the required portions are included in main application.  No extra steps are required to utilise this library.

2. POCO 1.6.0
=============
POCO is a large 3rd party library that offers a number of additional features.  It is used to implement a web socket server to supply parameter updates to a remote client.

To utilise POCO the library needs to be compiled on both the BeagleBone Black and on the development machine.

On the development machine a cross compiler is required to produce ARM binaries that can be linked to produce an executable for the BeagleBone.

2.1 Building POCO for the development environment (cross compiler)
==================================================================

Download and unzip the POCO-1.6.0 source code from poco.org.  Add the following as `poco-1.6.0/build/config/BeagleBoneBlack`

```
#
# $Id: //poco/1.4/build/config/BeagleBoneBlack#1 $
#
# BeagleBoneBlack
#
# Make settings for Debian BeagleBone Black
#

#
# General Settings
#
LINKMODE          ?= SHARED
POCO_PREFIX        = /usr/arm-linux-gnueabihf
POCO_TARGET_OSNAME = Linux
POCO_TARGET_OSARCH = armv7-a
TOOL               = arm-linux-gnueabihf

#
# Define Tools
#
CC      = $(TOOL)-gcc
CXX     = $(TOOL)-g++
LINK    = $(CXX)
STRIP   = $(TOOL)-strip
LIB     = $(TOOL)-ar -cr
RANLIB  = $(TOOL)-ranlib
SHLIB   = $(CXX) -shared -Wl,-soname,$(notdir $@) -o $@
SHLIBLN = $(POCO_BASE)/build/script/shlibln
DEP     = $(POCO_BASE)/build/script/makedepend.gcc
SHELL   = sh
RM      = rm -rf
CP      = cp
MKDIR   = mkdir -p

#
# Extension for Shared Libraries
#
SHAREDLIBEXT     = .so.$(target_version)
SHAREDLIBLINKEXT = .so

#
# Compiler and Linker Flags
#
CFLAGS          = -mfpu=neon -mfloat-abi=hard -mtune=cortex-a8 -march=armv7-a -ffast-math
CFLAGS32        =
CFLAGS64        =
CXXFLAGS        = -mfpu=neon -mfloat-abi=hard -mtune=cortex-a8 -march=armv7-a -ffast-math
CXXFLAGS32      =
CXXFLAGS64      =
LINKFLAGS       =
LINKFLAGS32     =
LINKFLAGS64     =
STATICOPT_CC    =
STATICOPT_CXX   =
STATICOPT_LINK  = -static
SHAREDOPT_CC    = -fPIC
SHAREDOPT_CXX   = -fPIC
SHAREDOPT_LINK  = -Wl,-rpath,$(LIBPATH)
DEBUGOPT_CC     = -g -D_DEBUG
DEBUGOPT_CXX    = -g -D_DEBUG
DEBUGOPT_LINK   = -g
RELEASEOPT_CC   = -O3 -DNDEBUG
RELEASEOPT_CXX  = -O2 -DNDEBUG
RELEASEOPT_LINK = -O2

#
# System Specific Flags
#
SYSFLAGS = -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_REENTRANT -D_THREAD_SAFE -DPOCO_NO_FPENVIRONMENT

#
# System Specific Libraries
#
SYSLIBS  = -lpthread -ldl -lrt
```

Configure the source with:
    ./configure --config=BeagleBoneBlack --no-tests --no-samples --prefix=/usr/arm-linux-gnueabihf/

Compile the library:
    make

Install the library:
    sudo make install

The object files and headers will be installed in:
    /usr/arm-linux-gnueabihf/lib
    /usr/arm-linux-gnueabihf/include/Poco

