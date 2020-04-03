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
 * Description : uni_audio_control.h
 * Author      : junlon2006@163.com
 * Date        : 2020.04.01
 *
 **************************************************************************/
#ifndef SDK_CHANNEL_INC_UNI_AUDIO_CONTROL_H_
#define SDK_CHANNEL_INC_UNI_AUDIO_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_channel.h"

typedef enum {
  CHNL_MSG_AUDIO_CTRL_START = AUDIO_CONTROL_MESSAGE_BASE,
  CHNL_MSG_AUDIO_CTRL_START_ACK,

  CHNL_MSG_AUDIO_CTRL_STOP,
  CHNL_MSG_AUDIO_CTRL_STOP_ACK,

  CHNL_MSG_AUDIO_CTRL_PAUSE,
  CHNL_MSG_AUDIO_CTRL_PAUSE_ACK,

  CHNL_MSG_AUDIO_CTRL_RESUME,
  CHNL_MSG_AUDIO_CTRL_RESUME_ACK,

  CHNL_MSG_AUDIO_CTRL_VOLUME,
  CHNL_MSG_AUDIO_CTRL_VOLUME_ACK,

  CHNL_MSG_AUDIO_CTRL_AUDIO_END,
  CHNL_MSG_AUDIO_CTRL_AUDIO_END_ACK,

  CHNL_MSG_AUDIO_CTRL_CNT,
} AudioControlMessageType;

typedef enum {
  NETWORK_URL_MP3 = 0,
  LOCAL_FILE_MP3,
} AudioSourceType;

typedef enum {
  AUDIO_VOLUME_MIN = 0,
  AUDIO_VOLUME_MID,
  AUDIO_VOLUME_MAX
} AudioVolumeLimitType;

typedef enum {
  VOLUME_SET_VALUE = 0,
  VOLUME_SET_LIMITED,
  VOLUME_GET,
} AudioVolumeProcessType;

typedef struct {
  AudioSourceType type;
  char            file_name[0];
} PACKED AudioCtrlStartRequest;

typedef struct {
  int ret;
} PACKED AudioCtrlStartResponse;

typedef struct {
  AudioSourceType type;
  char            file_name[0];
} PACKED AudioCtrlStopRequest;

typedef struct {
  int ret;
} PACKED AudioCtrlStopResponse;

typedef struct {
  AudioSourceType type;
  char            file_name[0];
} PACKED AudioCtrlPauseRequest;

typedef struct {
  int ret;
} PACKED AudioCtrlPauseResponse;

typedef struct {
  AudioSourceType type;
  int             offset;
  char            file_name[0];
} PACKED AudioCtrlResumeRequest;

typedef struct {
  int ret;
} PACKED AudioCtrlResumeResponse;

typedef struct {
  AudioVolumeProcessType type;
  void*                  args;
} PACKED AudioCtrlVolumeRequest;

typedef struct {
  int ret;
} PACKED AudioCtrlVolumeResponse;

typedef struct {
  AudioSourceType type;
  char            file_name[0];
} PACKED AudioCtrlAudioEndRequest;

typedef void (*AudioEndCallBack)(int type, const char *file_name);

#ifdef __cplusplus
}
#endif
#endif // SDK_CHANNEL_INC_UNI_AUDIO_CONTROL_H_
