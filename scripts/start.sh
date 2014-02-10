ROOT_PATH=/media/internal/AndroidChroot

if [ -f $ROOT_PATH/root/init ]; then
echo "already running"
exit 1
fi
mount $ROOT_PATH/root.ext3 $ROOT_PATH/root
mount $ROOT_PATH/system.ext3 $ROOT_PATH/root/system
mount $ROOT_PATH/data.ext3 $ROOT_PATH/root/data
mount $ROOT_PATH/cache.ext3 $ROOT_PATH/root/cache
mount -o bind $ROOT_PATH/sdcard $ROOT_PATH/root/mnt/sdcard
swapon $ROOT_PATH/swap.ext3 -p 200
chroot $ROOT_PATH/root /init &
cat < $ROOT_PATH/root/AndroidChroot/uinput_fifo
exit 0
