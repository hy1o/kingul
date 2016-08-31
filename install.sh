#!/bin/sh
#
# kingul: Korean keyboard for kindle
# version: 0.5 (2016/08/31)
#
##

HACKNAME="kingul"

# Pull libOTAUtils for logging & progress handling
[ -f ./libotautils5 ] && source ./libotautils5

otautils_update_progressbar

logmsg "I" "install" "" "installing custom directory"
KINGUL_MENU=/mnt/us/extensions/kingul
KINGUL_DIR=/mnt/us/kingul
MENU_ITEMS="bin menu.json config.xml"
BIN_ITEMS="kingul libkh5"
otautils_update_progressbar
mkdir -p ${KINGUL_MENU}
mkdir -p ${KINGUL_DIR}/bin
otautils_update_progressbar

logmsg "I" "install" "" "removing/migrating old version"
rm -rf ${KINGUL_MENU}/kingul
rm -rf ${KINGUL_MENU}/bin/kingul
rm -rf ${KINGUL_DIR}/bin/kingul
if [ -f ${KINGUL_MENU}/kingul.log ] ; then
    mv ${KINGUL_MENU}/kingul.log ${KINGUL_DIR}/kingul.log
fi
if [ -f /etc/upstart/kingul_del.conf ] ; then
    rm -rf /etc/upstart/kingul_del.conf
fi
otautils_update_progressbar

logmsg "I" "install" "" "copying files"
cp -rf ${MENU_ITEMS} ${KINGUL_MENU}
cp -rf ${BIN_ITEMS} ${KINGUL_DIR}/bin
cp -rf kingul_del.conf ${KINGUL_DIR}

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
