/**
 * FreeRDS: FreeRDP Remote Desktop Services (RDS)
 * Module helper
 *
 * Copyright 2013 Thincast Technologies GmbH
 * Copyright 2013 DI (FH) Martin Haimberger <martin.haimberger@thincast.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FREERDS_MODULE_HELPER_H
#define FREERDS_MODULE_HELPER_H

#include <freerds/module.h>

void initResolutions(char * basePath,RDS_MODULE_CONFIG_CALLBACKS * config, long sessionId ,char ** envBlock ,  long * xres, long * yres, long * colordepth);

bool getPropertyBoolWrapper(char * basePath, RDS_MODULE_CONFIG_CALLBACKS * config,long sessionID, char* path, bool* value);
bool getPropertyNumberWrapper(char * basePath, RDS_MODULE_CONFIG_CALLBACKS * config,long sessionID, char* path, long* value);
bool getPropertyStringWrapper(char * basePath, RDS_MODULE_CONFIG_CALLBACKS * config,long sessionID, char* path, char* value, unsigned int valueLength);


#endif /* FREERDS_MODULE_HELPER_H */
