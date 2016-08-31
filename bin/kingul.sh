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
	lipc-set-prop com.lab126.keyboard languages "$LIPC_PROP":ko
    else
	lipc-set-prop com.lab126.keyboard languages "$LIPC_PROP2"
    fi
}


## Main
case "${1}" in
	"restart" )
		${1}
	;;
	"toggle_selection" )
		${1}
	;;
	* )
		kh_msg "invalid action (${1})" W v "invalid action"
	;;
esac
