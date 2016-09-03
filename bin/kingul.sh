#!/bin/sh
#
# KUAL Kingul actions helper script
#
# v0.5 (08/31/2016)
# modified from NiLuJe's script
#
##

# Get hackname from the script's path (NOTE: Will only work for scripts called from /mnt/us/extensions/${KH_HACKNAME})
KH_HACKNAME="${PWD##/mnt/us/extensions/}"

# Try to pull our custom helper lib
libkh_fail="false"
# Handle both the K5 & legacy helper, so I don't have to maintain the exact same thing in two different places :P
for my_libkh in libkh5 libkh ; do
	_KH_FUNCS="/mnt/us/${KH_HACKNAME}/bin/${my_libkh}"
	if [ -f ${_KH_FUNCS} ] ; then
		. ${_KH_FUNCS}
		# Got it, go away!
		libkh_fail="false"
		break
	else
		libkh_fail="true"
	fi
done

if [ "${libkh_fail}" == "true" ] ; then
	# Pull default helper functions for logging
	_FUNCTIONS=/etc/rc.d/functions
	[ -f ${_FUNCTIONS} ] && . ${_FUNCTIONS}
	# We couldn't get our custom lib, abort
	msg "couldn't source libkh5 nor libkh from '${KH_HACKNAME}'" W
	exit 0
fi

# We need the proper privileges...
if [ "$(id -u)" -ne 0 ] ; then
	kh_msg "unprivileged user, aborting" E v
	exit 1
fi

## Enable a specific trigger file in the hack's basedir
# Arg 1 is exact config trigger file name
##
enable_hack_trigger_file()
{
	if [ $# -lt 1 ] ; then
		kh_msg "not enough arguments passed to enable_hack_trigger_file ($# while we need at least 1)" W v "missing trigger file name"
	fi

	kh_trigger_file="${KH_HACK_BASEDIR}/${1}"

	touch "${kh_trigger_file}"
}

## Remove a specific trigger file in the hack's basedir
# Arg 1 is exact config trigger file name
##
disable_hack_trigger_file()
{
	if [ $# -lt 1 ] ; then
		kh_msg "not enough arguments passed to disable_hack_trigger_file ($# while we need at least 1)" W v "missing trigger file name"
		return 1
	fi

	kh_trigger_file="${KH_HACK_BASEDIR}/${1}"

	rm -f "${kh_trigger_file}"
}

version()
{
    VERSION=`/mnt/us/kingul/bin/kingul -v`
    kh_msg "                       ${VERSION}" I v
}


## Try to restart kingul
restart()
{
	kh_msg "Restart kingul" I a
	stop kingul > /tmp/root/asdf
	start kingul >> /tmp/root/asdf
	kh_msg "Restart kingul done" I a
}

toggle_selection()
{
    LIPC_PROP=`lipc-get-prop com.lab126.keyboard languages`

    LIPC_PROP2=${LIPC_PROP/:ko/}
    LIPC_PROP2=${LIPC_PROP2/ko:/}

    LEN=$(( ${#LIPC_PROP} - ${#LIPC_PROP2} ))

    if [ $LEN -eq 0 ]; then
	kh_msg "                  Select Korean keyboard" I v
	lipc-set-prop com.lab126.keyboard languages "$LIPC_PROP":ko
    else
	kh_msg "                  Deselect Korean keyboard" I v
	lipc-set-prop com.lab126.keyboard languages "$LIPC_PROP2"
    fi
}

enable_debug()
{
    enable_hack_trigger_file "DEBUG"
    stop kingul
    start kingul
    kh_msg "Debug enabled" I a
}

disable_debug()
{
    disable_hack_trigger_file "DEBUG"
    stop kingul
    start kingul
    kh_msg "Debug disabled" I a
}

## Main
case "${1}" in
	"version" )
		${1}
	;;
	"restart" )
		${1}
	;;
	"toggle_selection" )
		${1}
	;;
	"enable_debug" )
		${1}
	;;
	"disable_debug" )
		${1}
	;;
	* )
		kh_msg "invalid action (${1})" W v "invalid action"
	;;
esac
