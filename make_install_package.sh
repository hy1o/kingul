VERSION=`cat VERSION`
PACKAGE_NAME=Update_kingul_${VERSION}_install_pw3_pw2_kv.bin
CONTENTS="install.sh kingul kingul.conf kingul_del.conf menu.json config.xml bin libotautils5 libkh5"
DEVICE=kindle5
$HOME/Workspace/KindleTool/KindleTool/Release/kindletool create ota2 -d ${DEVICE} ${CONTENTS} ${PACKAGE_NAME}
