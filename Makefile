APPID = com.nizovn.androidchroot

emulator:
	cd c-service && ${MAKE}

device:
	cd c-service && ${MAKE} DEVICE=1

.PHONY : package
package: 
	rm -rf package
	mkdir package
	rsync -av --progress ./ ./package --exclude-from=.gitignore --exclude=c-service --exclude=Makefile --exclude=.git --exclude=.gitignore
	mkdir package/c-service
	cp c-service/main_service package/c-service/androidchroot_main_service
	cp c-service/shutdown_service package/c-service/androidchroot_shutdown_service
	cp c-service/client_service package/c-service/androidchroot_client_service
	cp c-service/client package/c-service/androidchroot_client
	cp c-service/src_main_service/com* package/c-service/
	cp c-service/src_shutdown_service/com* package/c-service/
	cp c-service/src_client_service/com* package/c-service/
	palm-package package
	ar q ${APPID}_*.ipk pmPostInstall.script
	ar q ${APPID}_*.ipk pmPreRemove.script

test: package
	- palm-install -r ${APPID}
	palm-install ${APPID}_*.ipk
	palm-launch ${APPID}
clean:
	rm -rf package

clobber: clean
	cd c-service && ${MAKE} clobber
