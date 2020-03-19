/**************************************************************************
 * Copyright (C) 2020-2020 Junlon2006
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
#include "uni_event_list.h"

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define true    1
#define TAG     "channel"

typedef struct {
  EventListHandle event_handle;
} Channel;

static Channel g_channel = {0};

int ChnlChallengePackRequest(ChnlChallengePackReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(request->type,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int ChnlChallengePackResponse(ChnlChallengePackAck *response) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(response->type,
                                           (char *)response,
                                           sizeof(*response),
                                           &attr);
}

int ChnlNetworkStatusRequest(ChnlNetworkStatusReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(request->type,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int ChnlNetworkStatusResponse(ChnlNetworkStatusResp *response) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(response->type,
                                           (char *)response,
                                           sizeof(*response),
                                           &attr);
}

int ChnlLasrResultRequest(ChnlLasrResultReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(request->type,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int ChnlRecognizeRequest(ChnlRecognizeReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(request->type,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int ChnlPullNoiseReductionDataRequest(ChnlPullNoiseReductionDataReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(request->type,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int ChnlNoiseReductionPcmDataPush(ChnlNoiseReductionPcmData *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(request->type,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int ChnlOnlineAsrResultResponse(ChnlOnlineAsrResult *response) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(response->type,
                                           (char *)response,
                                           sizeof(*response),
                                           &attr);
}

static int _challenge_pack(char *packet, int len) {
  LOGT(TAG, "receive challenge pack");
  return 0;
}

static int _challenge_pack_ack(char *packet, int len) {
  LOGT(TAG, "receive challenge ack pack");
  return 0;
}

static int _network_status_request(char *packet, int len) {
  LOGT(TAG, "receive network request");
  return 0;
}

static int _network_status_response(char *packet, int len) {
  LOGT(TAG, "receive network response");
  return 0;
}

static int _noise_reduction_raw_data(char *packet, int len) {
  LOGD(TAG, "receive noise reduction raw data");
  ChnlNoiseReductionPcmData *data = (ChnlNoiseReductionPcmData *)packet;
  static int fd = -1;
  if (-1 == fd) {
    fd = open("asr.pcm", O_RDWR | O_CREAT | O_APPEND | O_SYNC, 0664);
  }

  if (fd > 0) {
    write(fd, data->data, sizeof(data->data));
  }

  return 0;
}

static int _rasr_result(char *packet, int len) {
  LOGT(TAG, "receive rasr result");
  return 0;
}

static int _lasr_result_request(char *packet, int len) {
  ChnlLasrResultReq *lasr_result = (ChnlLasrResultReq *)packet;
  LOGT(TAG, "receive lasr result request, session_id=%d, content=%s",
       lasr_result->session_id, lasr_result->content);
  return 0;
}

static int _recognize_request(char *packet, int len) {
  ChnlRecognizeReq *recogn = (ChnlRecognizeReq *)packet;
  LOGT(TAG, "receive recogn request, mode=%d", recogn->mode);
  return 0;
}

static int _pull_noise_reduction_data_request(char *packet, int len) {
  ChnlPullNoiseReductionDataReq *pull = (ChnlPullNoiseReductionDataReq *)packet;
  LOGT(TAG, "receive pull data request, mode=%d", pull->mode);
  return 0;
}

/* async mode, cannot block protocol stack */
void ChnlReceiveCommProtocolPacket(CommPacket *packet) {
  CommPacket *event = (CommPacket *)malloc(packet->payload_len + sizeof(CommPacket));
  event->cmd = packet->cmd;
  event->payload_len = packet->payload_len;
  memcpy(event->payload, packet->payload, packet->payload_len);
  EventListAdd(g_channel.event_handle, event, EVENT_LIST_PRIORITY_HIGHEST);
}

static void _event_list_event_handler(void *event) {
  CommPacket *packet = (CommPacket *)event;
  switch (packet->cmd) {
    case CHNL_MESSAGE_CHALLENGE_PACK:
      _challenge_pack(packet->payload, packet->payload_len);
      break;
    case CHNL_MESSAGE_CHALLENGE_PACK_ACK:
      _challenge_pack_ack(packet->payload, packet->payload_len);
      break;
    case CHNL_MESSAGE_NETWORK_REQUEST:
      _network_status_request(packet->payload, packet->payload_len);
      break;
    case CHNL_MESSAGE_NETWORK_RESPONSE:
      _network_status_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MESSAGE_NOISE_REDUCTION_RAW_DATA:
      _noise_reduction_raw_data(packet->payload, packet->payload_len);
      break;
    case CHNL_MESSAGE_RASR_RESULT:
      _rasr_result(packet->payload, packet->payload_len);
      break;
    case CHNL_MESSAGE_LASR_RESULT_REQUEST:
      _lasr_result_request(packet->payload, packet->payload_len);
      break;
    case CHNL_MESSAGE_RECOGNIZE_REQUEST:
      _recognize_request(packet->payload, packet->payload_len);
      break;
    case CHNL_MESSAGE_PULL_NOISE_REDUCTION_DATA_REQUEST:
      _pull_noise_reduction_data_request(packet->payload, packet->payload_len);
    break;
    default:
      LOGD(TAG, "unsupport message, type=%d", packet->cmd);
      break;
  }
}

static void _event_list_event_free_handler(void *event) {
  free(event);
}

static void _event_list_create() {
  g_channel.event_handle = EventListCreate(_event_list_event_handler, _event_list_event_free_handler);
}

int ChnlInit() {
  _event_list_create();
}
