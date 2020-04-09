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
 * Description : uni_iot_debug.h
 * Author      : junlon2006@163.com
 * Date        : 2020.04.08
 *
 **************************************************************************/
#ifndef SDK_CHANNEL_INC_UNI_IOT_DEBUG_H_
#define SDK_CHANNEL_INC_UNI_IOT_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_channel.h"

typedef enum {
  CHNL_MSG_IOT_DEBUG_REQUEST = IOT_DEBUG_MESSAGE_BASE,
  CHNL_MSG_IOT_DEBUG_RESPONSE,

  CHNL_MSG_IOT_DEBUG_LOG_REQUEST,
  CHNL_MSG_IOT_DEBUG_LOG_RESPONSE,

  CHNL_MSG_IOT_DEBUG_CTRL_CNT,
} IotDebugMessageType;

typedef struct {
  int log_level;
  int enable_uart_debug_out;
} PACKED IotDebugRequest;

typedef struct {
  int ret;
} PACKED IotDebugResponse;

typedef struct {
  int  log_length;
  char log[0];
} PACKED IotDebugLogOutRequest;

#ifdef __cplusplus
}
#endif
#endif // SDK_CHANNEL_INC_UNI_IOT_DEBUG_H_
