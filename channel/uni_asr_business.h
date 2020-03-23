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
 * Description : uni_asr_business.h
 * Author      : junlon2006@163.com
 * Date        : 2020.03.20
 *
 **************************************************************************/
#ifndef CHANNEL_UNI_ASR_BUSINESS_H_
#define CHANNEL_UNI_ASR_BUSINESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_channel.h"

typedef enum {
  CHNL_MSG_ASR_CHALLENGE_PACK = LASR_BUSINESS_MESSAGE_BASE,
  CHNL_MSG_ASR_CHALLENGE_PACK_ACK,
  CHNL_MSG_ASR_NOISE_REDUCTION_RAW_DATA,
  CHNL_MSG_ASR_RASR_RESULT,
  CHNL_MSG_ASR_LASR_RESULT_REQUEST,
  CHNL_MSG_ASR_RECOGNIZE_REQUEST,
  CHNL_MSG_ASR_PULL_NOISE_REDUCTION_DATA_REQUEST,
} AsrBusMessageType;

typedef struct {
  uint32_t sequence; /* request sequence, sync ack flag */
} PACKED AsrBusChallengePackReq;

typedef struct {
  uint32_t sequence; /* challenge ack sequence, equal request sequence */
} PACKED AsrBusChallengePackAck;

typedef struct {
  char data[PAYLOAD_LEN]; /* 64ms data default, cannot change it */
} PACKED AsrBusNoiseReductionPcmData;

typedef struct {
  uint8_t id;                  /* 0 ~ 255, max support 2048 * 256 = 512KB */
  uint8_t max_id;              /* the last id means transmit finish point */
  char    result[PAYLOAD_LEN]; /* 2048 default, when not enough, subpackage */
} PACKED AsrBusOnlineAsrResult;

typedef struct {
  uint32_t session_id;  /* vui session id */
  char     content[64]; /* awaken content */
} PACKED AsrBusLasrResultReq;

typedef struct {
  uint32_t mode; /* 0 唤醒模式，1 识别模式, 2 关闭所有 */
} PACKED AsrBusRecognizeReq;

typedef struct {
  uint32_t mode; /* 1开始接收数据，0停止接收数据 */
} PACKED AsrBusPullNoiseReductionDataReq;

/**
 * @brief comm protocol packet hook
 * @param packet
 * @return 0 success, -1 failed
 */
int AsrBusReceiveCommProtocolPacket(CommPacket *packet);

/**
 * @brief challenge pack
 * @param request
 * @return 0 success, -1 failed
 */
int AsrBusChallengePackRequest(AsrBusChallengePackReq *request);

/**
 * @brief challenge pack response
 * @param response
 * @return 0 success, -1 failed
 */
int AsrBusChallengePackResponse(AsrBusChallengePackAck *response);

/**
 * @brief lasr result request
 * @param request
 * @return 0 success, -1 failed
 */
int AsrBusLasrResultRequest(AsrBusLasrResultReq *request);

/**
 * @brief lasr recognize mode request
 * @param request
 * @return 0 success, -1 failed
 */
int AsrBusRecognizeRequest(AsrBusRecognizeReq *request);

/**
 * @brief noise reduction data request
 * @param request
 * @return 0 success, -1 failed
 */
int AsrBusPullNoiseReductionDataRequest(AsrBusPullNoiseReductionDataReq *request);

/**
 * @brief noise reduction data
 * @param request
 * @return 0 success, -1 failed
 */
int AsrBusNoiseReductionPcmDataPush(AsrBusNoiseReductionPcmData *request);

/**
 * @brief rasr result response
 * @param response
 * @return 0 success, -1 failed
 */
int AsrBusOnlineAsrResultResponse(AsrBusOnlineAsrResult *response);

#ifdef __cplusplus
}
#endif
#endif //  CHANNEL_UNI_ASR_BUSINESS_H_
