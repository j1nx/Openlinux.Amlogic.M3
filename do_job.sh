#!/bin/bash

script_name=${0##*/}
param_count=$#
builder=
builder_kernel=
builder_modules=
defconfig=
makemenuconfig=
doclean=
device=
wifi=
is_error=

# This is hardcoded
make_threads=4
configs_folder=arch/arm/configs
cross_path=/home/stane/Work/Linaro/2013.04/bin/arm-linux-gnueabihf-

# Text color variables
txtund=$(tput sgr 0 1)          # Underline
txtbld=$(tput bold)             # Bold
bldred=${txtbld}$(tput setaf 1) #  red
bldblu=${txtbld}$(tput setaf 4) #  blue
bldwht=${txtbld}$(tput setaf 7) #  white
txtrst=$(tput sgr0)             # Reset
info=${bldwht}*${txtrst}        # Feedback
pass=${bldblu}*${txtrst}
warn=${bldred}*${txtrst}
ques=${bldblu}?${txtrst}

getred()
{
    return "${bldred}$1$(tput sgr0)"
}

getblue()
{
    return "($pass $1)"
}

show_header()
{
	echo " "
	echo "############################################################"
	echo "#####     $(tput sgr 0 1)Set of Amlogic kernel tools by Stane1983$(tput sgr0)     #####"
	echo "############################################################"
	echo " "
}

show_help()
{
	echo "Usage:"
	echo " "
	echo "   $script_name [make|menuconfig|clean|mrproper|copy] [defconfig|kernel|modules]"
	echo "             [wifi_module]"
	echo " "
}

show_error()
{
    from_function=$1
    error_message=$2
    
    echo " "
    echo "############################################################"
    echo "#####                      $(tput bold)$(tput setaf 1)ERROR$(tput sgr0)                       #####"
    echo "############################################################"
    echo " "
    echo "Function:      $(tput bold)$(tput setaf 4)$from_function$(tput sgr0)"
    echo "Error message:"
    echo "$error_message"
    echo " "
    is_error=1
}

show_message()
{
    message=$1
    echo "$message"
}

do_work()
{
    # We want to make
    if [ "$1" == "make" ] ; then
	if [ ! "$2" == "kernel" -a ! "$2" == "modules" ] ; then
	    # Seems that we provided defconfig file to our script
	    # Check if it exists
	    if [ -f $configs_folder/$2 ] ; then
		defconfig=$2
	    elif [ -f $configs_folder/$2_defconfig ] ; then
		defconfig=$2_defconfig
	    else
		# We did not find defconfig, show error
		show_error "do_work" "Defconfig ${txtbld}$(tput setaf 1)$2$(tput sgr0) not found. please check your config folder."
		is_error=1
	    fi
	    if [ ! is_error ] ; then
		show_message "Writting defconfig to .config..."
		show_message "Defconfig is: ${txtbld}$(tput setaf 1)$defconfig$(tput sgr0)"
		make_defconfig
	    fi
	elif [ "$2" == "kernel" ]; then
	    if [ -f ./.config ] ; then
		show_message "Building uImage..."
		make_uimage
	    else
		show_error "do_work" "You wanted to create uImage without selecting configuration."
	    fi
	elif [ "$2" == "modules" ]; then
	    if [ -f ./.config ] ; then
		show_message "Building modules..."
		make_modules
	    else
		show_error "do_work" "You wanted to build modules without selecting configuration."
	    fi
	fi
    # We want to open menuconfig
    elif [ "$1" == "menuconfig" ] ; then
	if [ -f ./.config ] ; then
	    show_message "Opening menuconfig..."
	    make_menuconfig
	else
	    show_error "do_work" "You wanted to open menuconfig without selecting configuration."
	fi
    fi
}

make_defconfig()
{
    make $defconfig CROSS_COMPILE=$cross_path -j$make_threads
}

make_uimage()
{
    make uImage CROSS_COMPILE=$cross_path -j$make_threads
}

make_modules()
{
    make modules CROSS_COMPILE=$cross_path -j$make_threads
}

make_menuconfig()
{
    make menuconfig CROSS_COMPILE=$cross_path -j$make_threads
}

show_header
if [ param_count == 0 ] ; then
    show_help
else
    do_work $@
fi