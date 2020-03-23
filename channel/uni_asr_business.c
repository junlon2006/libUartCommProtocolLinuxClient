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
 * Description : uni_asr_business.c
 * Author      : junlon2006@163.com
 * Date        : 2020.03.20
 *
 **************************************************************************/
#include "uni_asr_business.h"
#include "uni_log.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define TAG  "asr_business"
#define true 1

int AsrBusChallengePackRequest(AsrBusChallengePackReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(CHNL_MSG_ASR_CHALLENGE_PACK,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int AsrBusChallengePackResponse(AsrBusChallengePackAck *response) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(CHNL_MSG_ASR_CHALLENGE_PACK_ACK,
                                           (char *)response,
                                           sizeof(*response),
                                           &attr);
}

int AsrBusLasrResultRequest(AsrBusLasrResultReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(CHNL_MSG_ASR_LASR_RESULT_REQUEST,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int AsrBusRecognizeRequest(AsrBusRecognizeReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(CHNL_MSG_ASR_RECOGNIZE_REQUEST,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int AsrBusPullNoiseReductionDataRequest(AsrBusPullNoiseReductionDataReq *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(CHNL_MSG_ASR_PULL_NOISE_REDUCTION_DATA_REQUEST,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int AsrBusNoiseReductionPcmDataPush(AsrBusNoiseReductionPcmData *request) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(CHNL_MSG_ASR_NOISE_REDUCTION_RAW_DATA,
                                           (char *)request,
                                           sizeof(*request),
                                           &attr);
}

int AsrBusOnlineAsrResultResponse(AsrBusOnlineAsrResult *response) {
  CommAttribute attr;
  attr.reliable = true;
  return CommProtocolPacketAssembleAndSend(CHNL_MSG_ASR_RASR_RESULT,
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

static int _noise_reduction_raw_data(char *packet, int len) {
  LOGD(TAG, "receive noise reduction raw data");

  AsrBusNoiseReductionPcmData *data = (AsrBusNoiseReductionPcmData *)packet;
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
  AsrBusLasrResultReq *lasr_result = (AsrBusLasrResultReq *)packet;
  LOGT(TAG, "receive lasr request, session_id=%d, content=%s",
       lasr_result->session_id, lasr_result->content);
  return 0;
}

static int _recognize_request(char *packet, int len) {
  AsrBusRecognizeReq *recogn = (AsrBusRecognizeReq *)packet;
  LOGT(TAG, "receive recogn request, mode=%d", recogn->mode);
  return 0;
}

static int _pull_noise_reduction_data_request(char *packet, int len) {
  AsrBusPullNoiseReductionDataReq *pull = (AsrBusPullNoiseReductionDataReq *)packet;
  LOGT(TAG, "receive pull data request, mode=%d", pull->mode);
  return 0;
}

int AsrBusReceiveCommProtocolPacket(CommPacket *packet) {
  switch (packet->cmd) {
    case CHNL_MSG_ASR_CHALLENGE_PACK:
      _challenge_pack(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_ASR_CHALLENGE_PACK_ACK:
      _challenge_pack_ack(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_ASR_NOISE_REDUCTION_RAW_DATA:
      _noise_reduction_raw_data(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_ASR_RASR_RESULT:
      _rasr_result(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_ASR_LASR_RESULT_REQUEST:
      _lasr_result_request(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_ASR_RECOGNIZE_REQUEST:
      _recognize_request(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_ASR_PULL_NOISE_REDUCTION_DATA_REQUEST:
      _pull_noise_reduction_data_request(packet->payload, packet->payload_len);
      break;
    default:
      return -1;
  }

  return 0;
}
