#!/bin/sh
#
# kingul: Korean keyboard for kindle
# version: 0.5 (2016/08/31)
# uninstaller
##

HACKNAME="kingul"

# Pull libOTAUtils for logging & progress handling
[ -f ./libotautils5 ] && source ./libotautils5
otautils_update_progressbar

# Delete upstart job
logmsg "I" "uninstall" "" "removing upstart job"
if [ -f /etc/upstart/${HACKNAME}.conf ] ; then
    rm -f /etc/upstart/${HACKNAME}.conf
fi
otautils_update_progressbar

if [ -f /mnt/us/${HACKNAME}/kingul_del.conf ] ; then
    cp -rf /mnt/us/${HACKNAME}/kingul_del.conf /etc/upstart
fi

# Remove custom directory in user storage area 
logmsg "I" "uninstall" "" "removing kingul from extension"
if [ -d /mnt/us/extensions/${HACKNAME} ] ; then
    rm -rf /mnt/us/extensions/${HACKNAME}
    logmsg "I" "uninstall" "" "kingul extension has been removed"
fi
otautils_update_progressbar

logmsg "I" "uninstall" "" "removing kingul"
if [ -d /mnt/us/${HACKNAME} ] ; then
    rm -rf /mnt/us/${HACKNAME}
    logmsg "I" "uninstall" "" "kingul has been removed"
fi
otautils_update_progressbar

logmsg "I" "uninstall" "" "done"
otautils_update_progressbar

return 0
