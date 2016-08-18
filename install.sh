#!/bin/sh
#
# kingul: Korean keyboard for kindle
# version: 0.1 (2016/08/16)
#
##

HACKNAME="kingul"

# Pull libOTAUtils for logging & progress handling
[ -f ./libotautils5 ] && source ./libotautils5

otautils_update_progressbar

logmsg "I" "install" "" "installing custom directory"

mkdir -p /mnt/us/extensions/kingul
cp kingul /mnt/us/extensions/kingul/

_RET=$?
if [ ${_RET} -ne 0 ] ; then
    logmsg "C" "install" "code=${_RET}" "fail to copy binary"
    return 1
fi
otautils_update_progressbar

# Setup startup script
logmsg "I" "install" "" "installing upstart job"
cp -f ${HACKNAME}.conf /etc/upstart/${HACKNAME}.conf

otautils_update_progressbar


logmsg "I" "install" "" "cleaning up"
rm -f ${HACKNAME}.conf kingul

otautils_update_progressbar

logmsg "I" "install" "" "done"

otautils_update_progressbar

return 0
