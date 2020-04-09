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
 * Description : uni_channel_daemon.c
 * Author      : junlon2006@163.com
 * Date        : 2020.04.09
 *
 **************************************************************************/
#include "uni_channel_daemon.h"
#include "uni_log.h"
#include "uni_iot.h"

#define TAG "daemon"

int ChannDaemonRebootRequest() {
  CommAttribute attr;
  attr.reliable = true;

  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_DAEMON_REBOOT_REQUEST,
                                              NULL,
                                              0,
                                              &attr);
  if (ret != 0) {
    LOGT(TAG, "transmit failed. err=%d", ret);
    return -1;
  }

  return 0;
}

static void _do_reboot_request(char *packet, int len) {
  //TODO 616 donot need reboot now
}

int ChannDaemonReceiveCommProtocolPacket(CommPacket *packet) {
  switch (packet->cmd) {
    case CHNL_MSG_DAEMON_REBOOT_REQUEST:
      _do_reboot_request(packet->payload, packet->payload_len);
      break;
    default:
      return -1;
  }

  return 0;
}
