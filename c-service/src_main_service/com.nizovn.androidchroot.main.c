description "AndroidChroot Main Service"

start on stopped finish

exec /var/usr/sbin/com.nizovn.androidchroot.main.c

respawn
