#!/bin/bash

set -xe

mkdir keys
# We don't really need a usable PK, so just generate a public key for it and discard the private key
openssl req -new -x509 -newkey rsa:2048 -subj "/CN=Rockchip Platform Key/" -keyout /dev/null -outform DER -out keys/pk.cer -days 7300 -nodes -sha256
curl -L https://go.microsoft.com/fwlink/?LinkId=321185 -o keys/ms_kek.cer
curl -L https://go.microsoft.com/fwlink/?linkid=321192 -o keys/ms_db1.cer
curl -L https://go.microsoft.com/fwlink/?linkid=321194 -o keys/ms_db2.cer
curl -L https://uefi.org/sites/default/files/resources/dbxupdate_arm64.bin -o keys/arm64_dbx.bin

export EDK2_SECUREBOOT_FLAGS=" \
  -D DEFAULT_KEYS=TRUE \
  -D PK_DEFAULT_FILE=keys/pk.cer \
  -D KEK_DEFAULT_FILE1=keys/ms_kek.cer \
  -D DB_DEFAULT_FILE1=keys/ms_db1.cer \
  -D DB_DEFAULT_FILE2=keys/ms_db2.cer \
  -D DBX_DEFAULT_FILE1=keys/arm64_dbx.bin \
  -D SECURE_BOOT_ENABLE=TRUE"

export EDK2_BUILD_FLAGS=" \
  ${EDK2_SECUREBOOT_FLAGS}"

cp -a bdy-g98-dts/rk3588-bdy-g98.dtb devicetree/vendor/

./build.sh --clean
./build.sh --device bdy-g98 --release Debug --edk2-flags "${EDK2_BUILD_FLAGS}"
cp -a RK3588_NOR_FLASH.img BDY_G98_UEFI.img

TIMESTAMP=$(date +%Y%m%d)
mkdir -p release
sudo cp -a BDY_G98_UEFI.img release/BDY_G98_UEFI.img
sudo cp -a BDY_G98_UEFI.img release/BDY_G98_UEFI-${TIMESTAMP}.img
sudo cp -a BDY_G98_UEFI.img RKDevTool_Release_v3.37/
sudo rar a release/BDY_G98_UEFI_With_RKDevTool.rar RKDevTool_Release_v3.37
sudo rar a release/BDY_G98_UEFI_With_RKDevTool-${TIMESTAMP}.rar RKDevTool_Release_v3.37

ls -alh release

