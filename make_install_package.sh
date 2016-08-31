PACKAGE_NAME=Update_kingul_0.5_install_pw3_pw2_kv.bin
CONTENTS="install.sh kingul kingul.conf kingul_del.conf menu.json config.xml bin libotautils5 libkh5"
DEVICE=kindle5
kindletool create ota2 -d ${DEVICE} ${CONTENTS} ${PACKAGE_NAME}
