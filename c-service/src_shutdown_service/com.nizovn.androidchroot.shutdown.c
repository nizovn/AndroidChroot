description "AndroidChroot Shutdown Service"

start on stopped finish

exec /var/usr/sbin/com.nizovn.androidchroot.shutdown.c

respawn
