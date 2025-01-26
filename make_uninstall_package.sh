VERSION=`cat VERSION`
PACKAGE_NAME=Update_kingul_${VERSION}_uninstall.bin
CONTENTS="uninstall.sh libotautils5"
DEVICE=kindle5
$HOME/Workspace/KindleTool/KindleTool/Release/kindletool create ota2 -d ${DEVICE} ${CONTENTS} ${PACKAGE_NAME}
