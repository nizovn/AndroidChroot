PREFIX=/media/internal/AndroidChroot/root
APP_PATH=/media/cryptofs/apps/usr/palm/applications/com.nizovn.androidchroot
FOUND=0

for ROOT in /proc/*/root; do
    LINK=$(readlink $ROOT)
    if [ "x$LINK" != "x" ]; then
        if [ "x${LINK:0:${#PREFIX}}" = "x$PREFIX" ]; then
            # this process is in the chroot...
            PID=$(basename $(dirname "$ROOT"))
            kill -9 "$PID"
            FOUND=1
        fi
    fi
done

if [ "x$FOUND" = "x1" ]; then
   $APP_PATH/kill.sh
fi

