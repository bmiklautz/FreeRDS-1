/**
 * Callback Handlers for Modules
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "CallBacks.h"
#include "TaskModuleShutdown.h"
#include <winpr/wlog.h>
#include <appcontext/ApplicationContext.h>

namespace freerds
{
	namespace sessionmanager
	{
		namespace module
		{
			static wLog* logger_CallBacks = WLog_Get("freerds.SessionManager.module.callbacks");

			int CallBacks::shutdown(long sessionId) {
				moduleNS::TaskModuleShutdownPtr task = moduleNS::TaskModuleShutdownPtr(new moduleNS::TaskModuleShutdown());
				task->setSessionId(sessionId);
				APP_CONTEXT.addTask(task);
				return 0;
			}


		}
	}
}

