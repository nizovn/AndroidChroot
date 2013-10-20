APP_PATH=/media/cryptofs/apps/usr/palm/applications/com.nizovn.androidchroot
cat < /media/internal/AndroidChroot/root/AndroidChroot/power_fifo
$APP_PATH/kill.sh
$APP_PATH/umount.sh
