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
 * Description : uni_channel.h
 * Author      : junlon2006@163.com
 * Date        : 2020.03.10
 *
 **************************************************************************/

#ifndef SDK_CHANNEL_INC_UNI_CHANNEL_H_
#define SDK_CHANNEL_INC_UNI_CHANNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_communication.h"

#define PACKED   __attribute__ ((packed))

typedef enum {
  CHNL_MESSAGE_CHALLENGE_PACK = 1,
  CHNL_MESSAGE_CHALLENGE_PACK_ACK,
  CHNL_MESSAGE_NETWORK_REQUEST,
  CHNL_MESSAGE_NETWORK_RESPONSE,
  CHNL_MESSAGE_NOISE_REDUCTION_RAW_DATA,
  CHNL_MESSAGE_RASR_RESULT,
  CHNL_MESSAGE_LASR_RESULT_REQUEST,
  CHNL_MESSAGE_RECOGNIZE_REQUEST,
  CHNL_MESSAGE_PULL_NOISE_REDUCTION_DATA_REQUEST,
} ChnlMessageType;

typedef struct {
  unsigned int type;     /* CHNL_MESSAGE_CHALLENGE_PACK */
  unsigned int sequence; /* request sequence, sync ack flag */
} PACKED ChnlChallengePackReq;

typedef struct {
  unsigned int type;     /* CHNL_MESSAGE_CHALLENGE_PACK_ACK */
  unsigned int sequence; /* challenge ack sequence, equal request sequence */
} PACKED ChnlChallengePackAck;

typedef struct {
  unsigned int type;     /* CHNL_MESSAGE_NETWORK_REQUEST */
  unsigned int sequence; /* request sequence, sync ack flag */
} PACKED ChnlNetworkStatusReq;

typedef struct {
  unsigned int type;     /* CHNL_MESSAGE_NETWORK_RESPONSE */
  unsigned int sequence; /* ack sequence, equal request sequence */
  unsigned int online;   /* 0 offline, 1 online */
} PACKED ChnlNetworkStatusResp;

typedef struct {
  unsigned int type;      /* CHNL_MESSAGE_NOISE_REDUCTION_RAW_DATA */
  char         data[2048];/* 64ms data default, cannot change it */
} PACKED ChnlNoiseReductionPcmData;

typedef struct {
  unsigned int  type;        /* CHNL_MESSAGE_RASR_RESULT */
  unsigned char id;          /* 0 ~ 255, max support 512 * 256 = 128KB, enough */
  unsigned char max_id;      /* the last id means transmit finish point */
  char          result[2048];/* 2048 default, when not enough, subpackage */
} PACKED ChnlOnlineAsrResult;

typedef struct {
  unsigned int type;         /* CHNL_MESSAGE_LASR_RESULT_REQUEST */
  unsigned int session_id;   /* vui session id */
  char         content[64];  /* awaken content */
} PACKED ChnlLasrResultReq;

typedef struct {
  unsigned int type;         /* CHNL_MESSAGE_RECOGNIZE_REQUEST */
  unsigned int mode;         /* 0 唤醒模式，1 识别模式, 2 关闭所有 */
} PACKED ChnlRecognizeReq;

typedef struct {
  unsigned int type;         /* CHNL_MESSAGE_PULL_NOISE_REDUCTION_DATA_REQUEST */
  unsigned int mode;         /* 1开始接收数据，0停止接收数据 */
} PACKED ChnlPullNoiseReductionDataReq;

int ChnlChallengePackRequest(ChnlChallengePackReq *request);
int ChnlChallengePackResponse(ChnlChallengePackAck *response);

int ChnlNetworkStatusRequest(ChnlNetworkStatusReq *request);
int ChnlNetworkStatusResponse(ChnlNetworkStatusResp *response);

int ChnlLasrResultRequest(ChnlLasrResultReq *request);
int ChnlRecognizeRequest(ChnlRecognizeReq *request);

int ChnlPullNoiseReductionDataRequest(ChnlPullNoiseReductionDataReq *request);

int ChnlNoiseReductionPcmDataPush(ChnlNoiseReductionPcmData *request);
int ChnlOnlineAsrResultResponse(ChnlOnlineAsrResult *response);

void ChnlReceiveCommProtocolPacket(CommPacket *packet);

int ChnlInit();

#ifdef __cplusplus
}
#endif
#endif  // SDK_CHANNEL_INC_UNI_CHANNEL_H_
