/*=============================================================================
 Copyright (C) 2010 WebOS Internals <support@webos-internals.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 =============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "luna_service.h"
#include "luna_methods.h"
#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <sys/wait.h>

#define API_VERSION "1"
#define ROOT_PATH "/media/internal/AndroidChroot"
#define APP_PATH "/media/cryptofs/apps/usr/palm/applications/com.nizovn.androidchroot" 
#define UINPUT_LOCATION ROOT_PATH "/root/dev/uinput"
int client=0;
int mount=0;
int uinput_fd;
void open_uinput();
int send_uevent(int fd, __u16 type, __u16 code, __s32 value);

//
// A dummy method, useful for unimplemented functions or as a status function.
// Called directly from webOS, and returns directly to webOS.
//
bool dummy_method(LSHandle* lshandle, LSMessage *message, void *ctx) {
  LSError lserror;
  LSErrorInit(&lserror);

  if (!LSMessageReply(lshandle, message, "{\"returnValue\": true}", &lserror)) goto error;

  return true;
 error:
  LSErrorPrint(&lserror, stderr);
  LSErrorFree(&lserror);
 end:
  return false;
}

//
// Return the current API version of the service.
// Called directly from webOS, and returns directly to webOS.
//
bool version_method(LSHandle* lshandle, LSMessage *message, void *ctx) {
  LSError lserror;
  LSErrorInit(&lserror);

  if (!LSMessageReply(lshandle, message, "{\"returnValue\": true, \"version\": \"" VERSION "\", \"apiVersion\": \"" API_VERSION "\"}", &lserror)) goto error;

  return true;
 error:
  LSErrorPrint(&lserror, stderr);
  LSErrorFree(&lserror);
 end:
  return false;
}

//
// Return a polite response.
// Called directly from webOS, and returns directly to webOS.
//
bool get_method(LSHandle* lshandle, LSMessage *message, void *ctx) {
	LSError lserror;
	LSErrorInit(&lserror);

// Local buffer to store the reply
	char reply[MAXLINLEN];

// Extract the id argument from the message
	json_t *object = json_parse_document(LSMessageGetPayload(message));
	json_t *name = json_find_first_label(object, "target");               
	if (!name || (name->child->type != JSON_STRING)) {
		if (!LSMessageReply(lshandle, message,
				"{\"returnValue\": false, \"errorCode\": -1, \"errorText\": \"Invalid or missing target\"}",
			&lserror)) goto error;
		return true;
	}
	if(strcmp(name->child->text,"files")==0){
		if(access(ROOT_PATH "/ramdisk.ext3", F_OK) == -1){
			sprintf(reply, "{\"returnValue\": false, \"file\": \"ramdisk.ext3\"}");
			goto message;}
		if(access(ROOT_PATH "/system.ext3", F_OK) == -1){
			sprintf(reply, "{\"returnValue\": false, \"file\": \"system.ext3\"}");
			goto message;}
		if(access(ROOT_PATH "/data.ext3", F_OK) == -1){
			sprintf(reply, "{\"returnValue\": false, \"file\": \"data.ext3\"}");
			goto message;}
		if(access(ROOT_PATH "/cache.ext3", F_OK) == -1){
			sprintf(reply, "{\"returnValue\": false, \"file\": \"cache.ext3\"}");
			goto message;}
		sprintf(reply, "{\"returnValue\": true}");
		goto message;
	}
	if(strcmp(name->child->text,"swap")==0){
		if(access(ROOT_PATH "/swap.ext3", F_OK) == -1)
			sprintf(reply, "{\"returnValue\": false}");
		else
			sprintf(reply, "{\"returnValue\": true}");
		goto message;
	}
	if(strcmp(name->child->text,"mount")==0){
		if(access(ROOT_PATH "/root/init", F_OK) == -1)
			sprintf(reply, "{\"returnValue\": false}");
		else
			sprintf(reply, "{\"returnValue\": true}");
		goto message;
	}
	if(strcmp(name->child->text,"client")==0){
		if(WEXITSTATUS(system("killall -s 0 -e " APP_PATH "/c-service/androidchroot_client"))==0)
			sprintf(reply, "{\"returnValue\": true}");
		else
			sprintf(reply, "{\"returnValue\": false}");
		goto message;
	}
	sprintf(reply, "{\"returnValue\": false, \"errorCode\": -1, \"errorText\": \"Invalid target !%s!\"}",name->child->text);
	if (!LSMessageReply(lshandle, message,reply, &lserror)) goto error;
	return true;
message:
	if (!LSMessageReply(lshandle, message, reply, &lserror)) goto error;
	return true;
error:
	LSErrorPrint(&lserror, stderr);
	LSErrorFree(&lserror);
	return false;
}

//
// Called directly from webOS, and returns directly to webOS.
//
bool set_method(LSHandle* lshandle, LSMessage *message, void *ctx) {
	LSError lserror;
	LSErrorInit(&lserror);

// Local buffer to store the reply
	char reply[MAXLINLEN];

// Extract the id argument from the message
	json_t *object = json_parse_document(LSMessageGetPayload(message));
	json_t *name = json_find_first_label(object, "state");               
	if (!name || (name->child->type != JSON_STRING)) {
		if (!LSMessageReply(lshandle, message,
			"{\"returnValue\": false, \"errorCode\": -1, \"errorText\": \"Invalid or missing state\"}",
			&lserror)) goto error;
		return true;
	}
	if(strcmp(name->child->text,"on")==0){
		system(APP_PATH "/scripts/start.sh");
		mount=true;
		sprintf(reply, "{\"returnValue\": true}");
		goto message;
	}
	if(strcmp(name->child->text,"off")==0){
		system("killall -s USR2 -e " APP_PATH "/c-service/androidchroot_client");
		sprintf(reply, "{\"returnValue\": true}");
		goto message;
	}
	sprintf(reply, "{\"returnValue\": false, \"errorCode\": -1, \"errorText\": \"Invalid target !%s!\"}",name->child->text);
	if (!LSMessageReply(lshandle, message,reply, &lserror)) goto error;
	return true;
message:
	if (!LSMessageReply(lshandle, message, reply, &lserror)) goto error;
	return true;
error:
	LSErrorPrint(&lserror, stderr);
	LSErrorFree(&lserror);
	return false;
}
LSMethod luna_methods[] = {
  { "status",		dummy_method },
  { "version",		version_method },

  { "getstate",		get_method },
  { "setstate",		set_method },

  { 0, 0 }
};

bool register_methods(LSPalmService *serviceHandle, LSError lserror) {
  return LSPalmServiceRegisterCategory(serviceHandle, "/", luna_methods,
				       NULL, NULL, NULL, &lserror);
}
