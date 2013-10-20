PREFIX=/media/internal/AndroidChroot/root
COUNT=0

while grep -q "$PREFIX" /proc/mounts; do
    COUNT=$(($COUNT+1))
    if [ $COUNT -ge 20 ]; then
        echo "failed to umount $PREFIX"
        exit 1
    fi
    grep "$PREFIX" /proc/mounts | \
        cut -d\  -f2 | LANG=C sort -r | xargs -r -n 1 umount -l || sleep 1
done

