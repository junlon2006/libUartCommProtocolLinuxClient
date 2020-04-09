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
 * Description : uni_iot_debug_client.c
 * Author      : junlon2006@163.com
 * Date        : 2020.04.08
 *
 **************************************************************************/
#include "uni_iot_debug_client.h"
#include "uni_log.h"
#include "uni_iot.h"

#define TAG "iot-debug"

int IotDebugSet(int log_level, int enable_log_uart_out) {
  int ret;

  IotDebugRequest request;
  request.log_level             = log_level;
  request.enable_uart_debug_out = enable_log_uart_out;

  CommAttribute attr;
  attr.reliable = true;

  ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_IOT_DEBUG_REQUEST,
                                          (char *)&request,
                                          sizeof(request),
                                          &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed. err=%d", ret);
  }

  return 0;
}

static void _do_iot_debug(char *packet, int len) {
  static uni_mutex_t mutex = NULL;
  if (mutex == NULL) {
    mutex = LogGetSyncLock();
  }

  uni_pthread_mutex_lock(mutex);
  printf("%s%s", "\033[0m\033[47;33m[IOT]\033[0m ",
         packet + sizeof(IotDebugLogOutRequest));
  uni_pthread_mutex_unlock(mutex);
}

int IotDebugCliRpcReceiveCommProtocolPacket(CommPacket *packet) {
  switch (packet->cmd) {
    case CHNL_MSG_IOT_DEBUG_LOG_REQUEST:
      _do_iot_debug(packet->payload, packet->payload_len);
      break;
    default:
      return -1;
  }

  return 0;
}
