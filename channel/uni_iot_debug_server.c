/**************************************************************************
 * Copyright (C) 2020-2020  Unisound
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
 * Description : uni_iot_debug_server.c
 * Author      : junlon2006@163.com
 * Date        : 2020.04.08
 *
 **************************************************************************/
#include "uni_iot_debug_server.h"
#include "uni_log.h"

#define TAG "iot-debug"

static void _do_iot_debug_request(char *packet, int len) {
  IotDebugRequest *request = (IotDebugRequest *)packet;
  LOGT(TAG, "do iot-debug configure, level=%d, enable=%d",
       request->log_level, request->enable_uart_debug_out);

  LogLevelSet(request->log_level);
  LogIotDebug(request->enable_uart_debug_out, IotDebugOutRequest);
}

int IotDebugOutRequest(char *out) {
  IotDebugLogOutRequest *request;
  int len = strlen(out) + 1;
  request = (IotDebugLogOutRequest *)uni_malloc(sizeof(IotDebugLogOutRequest) + len);
  request->log_length = len;
  strcpy(request->log, out);

  CommAttribute attr;
  attr.reliable = false;

  CommProtocolPacketAssembleAndSend(CHNL_MSG_IOT_DEBUG_LOG_REQUEST,
                                    (char *)request,
                                    sizeof(IotDebugLogOutRequest) + len,
                                    &attr);

  uni_free(request);

  return 0;
}

int IotDebugReceiveCommProtocolPacket(CommPacket *packet) {
  switch (packet->cmd) {
    case CHNL_MSG_IOT_DEBUG_REQUEST:
      _do_iot_debug_request(packet->payload, packet->payload_len);
      break;
    default:
      return -1;
  }

  return 0;
}
