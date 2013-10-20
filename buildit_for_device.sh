#!/bin/bash

#######################################################################
##### Enter the name of the file or directory to be sent to device  ###
##### APP="<name of app file> or <appdir>"                          ###
#######################################################################
export APP="client"

#######################################################################
#### Enter the relative path to the executable here.                ###
#### EXEC="<relative path to exec>"                                 ###
#######################################################################
export EXEC="client"

#######################################################################
### List your source files here                                     ###
### SRC="<source1> <source2>"                                       ###
#######################################################################
export SRC="src/main.cpp"


#######################################################################
### List the libraries needed.                                      ###
### LIBS="-l<libname>"                                              ###
#######################################################################
export LIBS="-lSDL -lGLES_CM -lpdl -lSDL_image"

#######################################################################
### Name your output executable                                     ###
### OUTFILE="<executable-name>"                                     ###
#######################################################################
export OUTFILE="client"


###################################
######## Do not edit below ########
###################################

###################################
######## Checking the setup #######
###################################

if [ ! "$PalmPDK" ];then
        export PalmPDK=/opt/PalmPDK
fi

# Set the device specific compiler options. By default, a binary that
# will run on both Pre and Pixi will be built. These option only need to be
# set for a particular device if more performance is necessary.
if [ "$1" == "pre" ]; then
	DEVICEOPTS="-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp"
else
	DEVICEOPTS="-mcpu=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp"
fi

export BUILDDIR="Build_Device"

PATH=$PATH:${PalmPDK}/arm-gcc/bin

CC="arm-none-linux-gnueabi-gcc"

ARCH=""
SYSROOT="${PalmPDK}/arm-gcc/sysroot"

INCLUDEDIR="${PalmPDK}/include"
LIBDIR="${PalmPDK}/device/lib"

CPPFLAGS="-I${INCLUDEDIR} -I${INCLUDEDIR}/SDL --sysroot=$SYSROOT -I/home/nin/AndroidChroot/linux-2.6.32/include"
LDFLAGS="-L${LIBDIR} -Wl,--allow-shlib-undefined"
SRCDIR="."
###################################

if [ -e "$BUILDDIR" ]; then
	rm -rf "$BUILDDIR"
fi
mkdir -p $BUILDDIR

if [ "$SRC" == "" ];then
	echo "Source files not specified. Please edit the SRC variable inside this script."
	exit 1
fi

if [ "$OUTFILE" == "" ];then
	echo "Output file name not specified. Please edit the OUTFILE variable inside this script."
	exit 1
fi
echo "Building for Device"
echo "$CC $DEVICEOPTS $CPPFLAGS $LDFLAGS $LIBS -o $BUILDDIR/$OUTFILE $SRCDIR/$SRC"
$CC $DEVICEOPTS $CPPFLAGS $LDFLAGS $LIBS -o $BUILDDIR/$OUTFILE $SRCDIR/$SRC

echo -e "\nPutting binary into $BUILDDIR.\n"
