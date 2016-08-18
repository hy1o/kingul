#!/bin/sh
#
# kingul: Korean keyboard for kindle
# version: 0.1 (2016/08/16)
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
