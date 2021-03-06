/**
 * Property levels
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

#ifndef __PROPERTYLEVEL_H_
#define __PROPERTYLEVEL_H_

#include <string>

typedef enum _PROPERTY_LEVEL
{
	Global = 1,
	UserGroup = 2,
	User = 3
}
PROPERTY_LEVEL, *PPROPERTY_LEVEL;

typedef enum _PROPERTY_STORE_TYPE
{
	BoolType = 1,
	NumberType = 2,
	StringType = 3
} PROPERTY_STORE_TYPE, *PPROPERTY_STORE_TYPE;


typedef struct _PROPERTY_STORE_HELPER {
	PROPERTY_STORE_TYPE type;
	bool boolValue;
	long numberValue;
	std::string stringValue;
}PROPERTY_STORE_HELPER, *PPROPERTY_STORE_HELPER;

#endif /* __PROPERTYLEVEL_H_ */
