description "AndroidChroot Client Service"

start on stopped finish

exec /var/usr/sbin/com.nizovn.androidchroot.client.c

respawn
