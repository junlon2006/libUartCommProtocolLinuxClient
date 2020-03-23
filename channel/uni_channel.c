/**************************************************************************
 * Copyright (C) 2020-2020  Junlon2006
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **************************************************************************
 *
 * Description : uni_channel.c
 * Author      : junlon2006@163.com
 * Date        : 2020.03.10
 *
 **************************************************************************/
#include "uni_channel.h"
#include "uni_log.h"
#include "uni_communication.h"
#include "uni_asr_business.h"
#include "uni_network_helper_server.h"
#include "uni_event_list.h"

#include <stdlib.h>
#include <string.h>

#define TAG     "channel"

typedef struct {
  EventListHandle event_handle;
} Channel;

static Channel g_channel = {0};

/* interrupt callback, cannot block */
void ChnlReceiveCommProtocolPacket(CommPacket *packet) {
  CommPacket *event = (CommPacket *)malloc(packet->payload_len + sizeof(CommPacket));
  event->cmd = packet->cmd;
  event->payload_len = packet->payload_len;
  memcpy(event->payload, packet->payload, packet->payload_len);
  EventListAdd(g_channel.event_handle, event, EVENT_LIST_PRIORITY_HIGHEST);
}

static void _event_list_event_handler(void *event) {
  CommPacket *packet = (CommPacket *)event;

  if (0 == NetHelperSerRpcReceiveCommProtocolPacket(packet)) {
    return;
  }

  if (0 == AsrBusReceiveCommProtocolPacket(packet)) {
    return;
  }

  LOGW(TAG, "recv unsupport cmd=%d", packet->cmd);
}

static void _event_list_event_free_handler(void *event) {
  free(event);
}

static void _event_list_create() {
  g_channel.event_handle = EventListCreate(_event_list_event_handler,
                                           _event_list_event_free_handler);
}

int ChnlInit(void) {
  _event_list_create();
  LOGT(TAG, "channel init success");
  return 0;
}

