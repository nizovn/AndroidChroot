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

#define API_VERSION "1"
#define ROOT_PATH "/media/internal/AndroidChroot"
#define APP_PATH "/media/cryptofs/apps/usr/palm/applications/com.nizovn.androidchroot" 
#define UINPUT_LOCATION ROOT_PATH "/root/dev/uinput"

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
// Called directly from webOS, and returns directly to webOS.
//
bool shutdown_method(LSHandle* lshandle, LSMessage *message, void *ctx) {
	LSError lserror;
	LSErrorInit(&lserror);

// Local buffer to store the reply
	char reply[MAXLINLEN];

// Extract the id argument from the message
	json_t *object = json_parse_document(LSMessageGetPayload(message));
	json_t *name = json_find_first_label(object, "state");               
	if (!name || (name->child->type != JSON_STRING)) {
		if (!LSMessageReply(lshandle, message,
			"{\"returnValue\": false, \"errorCode\": -1, \"errorText\": \"Invalid or missing autoshutdown state\"}",
			&lserror)) goto error;
		return true;
	}
	system(APP_PATH "/scripts/shutdown.sh");
	system("killall -s USR1 -e " APP_PATH "/c-service/androidchroot_client");
	sprintf(reply, "{\"returnValue\": true}");
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

  { "autoshutdown",	shutdown_method },

  { 0, 0 }
};

bool register_methods(LSPalmService *serviceHandle, LSError lserror) {
  return LSPalmServiceRegisterCategory(serviceHandle, "/", luna_methods,
				       NULL, NULL, NULL, &lserror);
}
