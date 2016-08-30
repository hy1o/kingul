#!/bin/sh
#
# kingul: Korean keyboard for kindle
# version: 0.4 (2016/08/30)
# uninstaller
##

HACKNAME="kingul"

# Pull libOTAUtils for logging & progress handling
[ -f ./libotautils5 ] && source ./libotautils5

otautils_update_progressbar

LIPC_PROP=`lipc-get-prop com.lab126.keyboard languages`
LIPC_PROP=${LIPC_PROP/:ko/}
LIPC_PROP=${LIPC_PROP/ko:/}
lipc-set-prop com.lab126.keyboard languages "$LIPC_PROP"

otautils_update_progressbar

# Delete upstart job
logmsg "I" "uninstall" "" "removing upstart job"
if [ -f /etc/upstart/${HACKNAME}.conf ] ; then
    rm -f /etc/upstart/${HACKNAME}.conf
fi

otautils_update_progressbar

# Remove custom directory in userstore?
logmsg "I" "uninstall" "" "removing kingul from extension"
if [ -d /mnt/us/extensions/${HACKNAME} ] ; then
    rm -rf /mnt/us/extensions/${HACKNAME}
    logmsg "I" "uninstall" "" "kingul has been removed"
fi

otautils_update_progressbar

logmsg "I" "uninstall" "" "done"

otautils_update_progressbar

return 0
