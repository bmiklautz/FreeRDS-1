/**
 * FreeRDS: FreeRDP Remote Desktop Services (RDS)
 *
 * Copyright 2013 Marc-Andre Moreau <marcandre.moreau@gmail.com>
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

#include "channels.h"

#include <freerds/icp_client_stubs.h>

int freerds_channels_post_connect(rdsConnection* session)
{
	BOOL allowed;

	if (WTSVirtualChannelManagerIsChannelJoined(session->vcm, "cliprdr"))
	{
		allowed = FALSE;
		freerds_icp_IsChannelAllowed(session->id, "cliprdr", &allowed);
		printf("channel %s is %s\n", "cliprdr", allowed ? "allowed" : "not allowed");

		printf("Channel %s registered\n", "cliprdr");
		session->cliprdr = cliprdr_server_context_new(session->vcm);
		session->cliprdr->Start(session->cliprdr);
	}

	if (WTSVirtualChannelManagerIsChannelJoined(session->vcm, "rdpdr"))
	{
		allowed = FALSE;
		freerds_icp_IsChannelAllowed(session->id, "rdpdr", &allowed);
		printf("channel %s is %s\n", "rdpdr", allowed ? "allowed" : "not allowed");

		printf("Channel %s registered\n", "rdpdr");
		session->rdpdr = rdpdr_server_context_new(session->vcm);
		session->rdpdr->Start(session->rdpdr);
	}

	if (WTSVirtualChannelManagerIsChannelJoined(session->vcm, "rdpsnd"))
	{
		allowed = FALSE;
		freerds_icp_IsChannelAllowed(session->id, "rdpsnd", &allowed);
		printf("channel %s is %s\n", "rdpsnd", allowed ? "allowed" : "not allowed");

		printf("Channel %s registered\n", "rdpsnd");
		session->rdpsnd = rdpsnd_server_context_new(session->vcm);
		session->rdpsnd->Start(session->rdpsnd);
	}

	return 0;
}
