/**
 * FreeRDS: FreeRDP Remote Desktop Services (RDS)
 * Weston Module
 *
 * Copyright 2013 Thincast Technologies GmbH
 * Copyright 2013 Bernhard Miklautz <bernhard.miklautz@thincast.com>
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

#include <winpr/crt.h>
#include <winpr/wlog.h>
#include <winpr/pipe.h>
#include <winpr/synch.h>
#include <winpr/thread.h>
#include <winpr/environment.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <freerds/module.h>
#include <freerds/backend.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "weston_module.h"

RDS_MODULE_CONFIG_CALLBACKS gConfig;
RDS_MODULE_STATUS_CALLBACKS gStatus;

struct rds_module_weston
{
	RDS_MODULE_COMMON commonModule;

	wLog* log;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE monitorThread;
	HANDLE monitorStopEvent;
};
typedef struct rds_module_weston rdsModuleWeston;

void monitoring_thread(void *arg)
{
	DWORD ret = 0;
	int status;
	rdsModuleWeston *weston = (rdsModuleWeston*)arg;

	while (1)
	{
		ret = waitpid(weston->pi.dwProcessId, &status, WNOHANG);
		if (ret != 0)
		{
			break;
		}
		if (WaitForSingleObject(weston->monitorStopEvent, 200) == WAIT_OBJECT_0)
		{
			// monitorStopEvent triggered
			WLog_Print(weston->log, WLOG_DEBUG, "s %d: monitor stop event", weston->commonModule.sessionId);
			return;
		}
	}
	GetExitCodeProcess(weston->pi.hProcess, &ret);
	WLog_Print(weston->log, WLOG_DEBUG, "s %d: Weston process exited with %d (monitoring thread)", weston->commonModule.sessionId, ret);
	gStatus.shutdown(weston->commonModule.sessionId);
	return;
}


RDS_MODULE_COMMON* weston_rds_module_new(void)
{
	rdsModuleWeston* weston = (rdsModuleWeston*) malloc(sizeof(rdsModuleWeston));

	WLog_Init();

	weston->log = WLog_Get("com.freerds.module.weston");
	WLog_OpenAppender(weston->log);

	WLog_SetLogLevel(weston->log, WLOG_DEBUG);

	WLog_Print(weston->log, WLOG_DEBUG, "RdsModuleNew");

	return (RDS_MODULE_COMMON*) weston;
}

void weston_rds_module_free(RDS_MODULE_COMMON* module)
{
	rdsModuleWeston* weston = (rdsModuleWeston*) module;
	WLog_Print(weston->log, WLOG_DEBUG, "RdsModuleFree");
	WLog_Uninit();
	free(module);
}

void initResolutions(rdsModuleWeston * weston,  long * xres, long * yres, long * colordepth) {
	char tempstr[256];

	long maxXRes = 0, maxYRes = 0, minXRes = 0, minYRes = 0;
	long connectionXRes = 0, connectionYRes = 0, connectionColorDepth = 0;

	if (!gConfig.getPropertyNumber(weston->commonModule.sessionId, "module.weston.maxXRes", &maxXRes)) {
		WLog_Print(weston->log, WLOG_ERROR, "Setting: module.weston.maxXRes not defined, NOT setting FREERDS_SMAX or FREERDS_SMIN\n");
	}
	if (!gConfig.getPropertyNumber(weston->commonModule.sessionId, "module.weston.maxYRes", &maxYRes)) {
		WLog_Print(weston->log, WLOG_ERROR, "Setting: module.weston.maxYRes not defined, NOT setting FREERDS_SMAX or FREERDS_SMIN\n");
	}
	if (!gConfig.getPropertyNumber(weston->commonModule.sessionId, "module.weston.minXRes", &minXRes)) {
		WLog_Print(weston->log, WLOG_ERROR, "Setting: module.weston.minXRes not defined, NOT setting FREERDS_SMAX or FREERDS_SMIN\n");
	}
	if (!gConfig.getPropertyNumber(weston->commonModule.sessionId, "module.weston.minYRes", &minYRes)){
		WLog_Print(weston->log, WLOG_ERROR, "Setting: module.weston.minYRes not defined, NOT setting FREERDS_SMAX or FREERDS_SMIN\n");
	}

	if ((maxXRes != 0) && (maxYRes != 0)){
		sprintf_s(tempstr, sizeof(tempstr), "%dx%d", (unsigned int) maxXRes,(unsigned int) maxYRes );
		SetEnvironmentVariableEBA(&weston->commonModule.envBlock, "FREERDS_SMAX", tempstr);
	}
	if ((minXRes != 0) && (minYRes != 0)) {
		sprintf_s(tempstr, sizeof(tempstr), "%dx%d", (unsigned int) minXRes,(unsigned int) minYRes );
		SetEnvironmentVariableEBA(&weston->commonModule.envBlock, "FREERDS_SMIN", tempstr);
	}

	gConfig.getPropertyNumber(weston->commonModule.sessionId, "connection.xres", &connectionXRes);
	gConfig.getPropertyNumber(weston->commonModule.sessionId, "connection.yres", &connectionYRes);
	gConfig.getPropertyNumber(weston->commonModule.sessionId, "connection.colordepth", &connectionColorDepth);

	if ((connectionXRes == 0) || (connectionYRes == 0)) {
		WLog_Print(weston->log, WLOG_ERROR, "got no XRes or YRes from client, using config values");

		if (!gConfig.getPropertyNumber(weston->commonModule.sessionId, "module.weston.xres", xres))
			*xres = 1024;

		if (!gConfig.getPropertyNumber(weston->commonModule.sessionId, "module.weston.yres", yres))
			*yres = 768;

		if (!gConfig.getPropertyNumber(weston->commonModule.sessionId, "module.weston.colordepth", colordepth))
			*colordepth = 24;
		return;
	}

	if ((maxXRes > 0 ) && (connectionXRes > maxXRes)) {
		*xres = maxXRes;
	} else if ((minXRes > 0 ) && (connectionXRes < minXRes)) {
		*xres = minXRes;
	} else {
		*xres = connectionXRes;
	}

	if ((maxYRes > 0 ) && (connectionYRes > maxYRes)) {
		*yres = maxYRes;
	} else if ((minYRes > 0 ) && (connectionYRes < minYRes)) {
		*yres = minYRes;
	} else {
		*yres = connectionYRes;
	}

	if (connectionColorDepth == 0) {
		connectionColorDepth = 16;
	}
	*colordepth = connectionColorDepth;

}

char* weston_rds_module_start(RDS_MODULE_COMMON* module)
{
	BOOL status;
	char* pipeName;
	char* qPipeName;
	long xres = 1024, yres = 768,colordepth = 16;
	char lpCommandLine[256];
	const char* endpoint = "weston";
	char envstr[256];

	rdsModuleWeston* weston = (rdsModuleWeston*) module;
	DWORD SessionId = weston->commonModule.sessionId;
	char *appName = "weston";
	weston->monitorStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	WLog_Print(weston->log, WLOG_DEBUG, "RdsModuleStart: SessionId: %d Endpoint: %s",
			(int) SessionId, endpoint);

	pipeName = (char*) malloc(256);
	freerds_named_pipe_get_endpoint_name(SessionId, endpoint, pipeName, 256);
	freerds_named_pipe_clean_endpoint(SessionId, endpoint);

	ZeroMemory(&(weston->si), sizeof(STARTUPINFO));
	weston->si.cb = sizeof(STARTUPINFO);
	ZeroMemory(&(weston->pi), sizeof(PROCESS_INFORMATION));

	sprintf_s(envstr, sizeof(envstr), "%d", (int) (weston->commonModule.sessionId));
	SetEnvironmentVariableEBA(&weston->commonModule.envBlock, "FREERDS_SID", envstr);

	initResolutions(weston,&xres,&yres,&colordepth);

	qPipeName = (char*) malloc(256);
	sprintf_s(qPipeName, 256, "/tmp/.pipe/FreeRDS_%d_%s", (int) SessionId, endpoint);
	SetEnvironmentVariableEBA(&weston->commonModule.envBlock, "FREERDS_PIPE_PATH", qPipeName);

	sprintf_s(lpCommandLine, sizeof(lpCommandLine), "%s --backend=freerds-backend.so --width=%d --height=%d --freerds-pipe=%s", appName, xres, yres, appName, qPipeName);
	free(qPipeName);

	WLog_Print(weston->log, WLOG_DEBUG, "Starting process with command line: %s", lpCommandLine);

	status = CreateProcessA(NULL, lpCommandLine,
			NULL, NULL, FALSE, 0, weston->commonModule.envBlock, NULL,
			&(weston->si), &(weston->pi));

	if (0 == status)
	{
		WLog_Print(weston->log, WLOG_ERROR, "Could not start weston application %s", appName);
		return NULL;
	}

	WLog_Print(weston->log, WLOG_DEBUG, "Process %d/%d created with status: %d", weston->pi.dwProcessId,weston->pi.dwThreadId, status);

	if (!WaitNamedPipeA(pipeName, 5 * 1000))
	{
		fprintf(stderr, "WaitNamedPipe failure: %s\n", pipeName);
		return NULL;
	}

	weston->monitorThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) monitoring_thread, weston, 0, NULL);
	return pipeName;
}

int weston_rds_module_stop(RDS_MODULE_COMMON* module)
{
	rdsModuleWeston* weston = (rdsModuleWeston*) module;

	WLog_Print(weston->log, WLOG_DEBUG, "RdsModuleStop");

	SetEvent(weston->monitorStopEvent);
	WaitForSingleObject(weston->monitorThread, INFINITE);

	TerminateProcess(weston->pi.hProcess,0);
	WaitForSingleObject(weston->pi.hProcess, INFINITE);
	return 0;
}

int RdsModuleEntry(RDS_MODULE_ENTRY_POINTS* pEntryPoints)
{
	pEntryPoints->Version = 1;
	pEntryPoints->Name = "weston";

	pEntryPoints->New = weston_rds_module_new;
	pEntryPoints->Free = weston_rds_module_free;

	pEntryPoints->Start = weston_rds_module_start;
	pEntryPoints->Stop = weston_rds_module_stop;

	gStatus = pEntryPoints->status;
	gConfig = pEntryPoints->config;

	return 0;
}

