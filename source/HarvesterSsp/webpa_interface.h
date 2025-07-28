/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
/**
 * @file webpa_interface.h
 *
 * @description This header defines parodus log levels
 *
 */

#ifndef _WEBPA_INTERFACE_H_
#define _WEBPA_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "wifi_hal_generic.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define DEVICE_PROPS_FILE  "/etc/device.properties"

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/**
 *  Returns parodus URL.
 *
 *  @note The caller should ensure that size of url is URL_SIZE or more.
 *
 *  @param url   [out] URL where parodus is listening.
 */
void get_parodus_url(char **url);
BOOL Cosa_FindDestComp(char* pObjName,char** ppDestComponentName, char** ppDestPath);
void initparodusTask();
void sendWebpaMsg(char *serviceName, char *dest, char *trans_id, char *contentType, char *payload, unsigned int payload_len);
const char *rdk_logger_module_fetch(void);
char * getDeviceMac();
#ifdef __cplusplus
}
#endif

#endif