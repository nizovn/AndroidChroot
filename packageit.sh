STAGING_DIR=STAGING/com.nizovn.androidchroot
BUILDDIR="Build_Device"
rm -rf $STAGING_DIR
rm *.ipk
mkdir -p $STAGING_DIR
./buildit_for_device.sh pre
cp $BUILDDIR/client $STAGING_DIR
cp appinfo.json $STAGING_DIR
cp icon.png $STAGING_DIR
cp scripts/* $STAGING_DIR
palm-package $STAGING_DIR
