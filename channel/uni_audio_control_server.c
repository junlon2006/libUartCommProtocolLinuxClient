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
 * Description : uni_audio_control_server.c
 * Author      : junlon2006@163.com
 * Date        : 2020.04.01
 *
 **************************************************************************/
#include "uni_audio_control_server.h"
#include "uni_audio_control.h"
#include "uni_log.h"

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef true
#define true 1
#endif

#define TAG "audio-service"

static char            g_file_name[1024]    = {0};
static AudioSourceType g_audio_source_type  = NETWORK_URL_MP3;
static pthread_mutex_t g_lock               = PTHREAD_MUTEX_INITIALIZER;

static void _do_start(char *packet, int len) {
  AudioCtrlStartRequest *request = (AudioCtrlStartRequest *)packet;
  int ret = 0;

  pthread_mutex_lock(&g_lock);
  strcpy(g_file_name, request->file_name);
  g_audio_source_type = request->type;
  pthread_mutex_unlock(&g_lock);

  AudioCtrlStartResponse response;
  response.ret = ret;

  CommAttribute attr;
  attr.reliable = true;
  ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_AUDIO_CTRL_START_ACK,
                                          (char *)&response,
                                          sizeof(response),
                                          &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed. err=%d", ret);
  }

  AudioCtrlMp3PlayerAudioEndCallBack();
}

static void _do_stop(char *packet, int len) {
  AudioCtrlStopRequest *request = (AudioCtrlStopRequest *)packet;
  int ret = 0;

  AudioCtrlStopResponse response;
  response.ret = ret;

  CommAttribute attr;
  attr.reliable = true;
  ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_AUDIO_CTRL_STOP_ACK,
                                          (char *)&response,
                                          sizeof(response),
                                          &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed. err=%d", ret);
  }
}

static void _do_pause(char *packet, int len) {
  AudioCtrlPauseRequest *request = (AudioCtrlPauseRequest *)packet;
  int ret = 0;

  AudioCtrlPauseResponse response;
  response.ret = ret;

  CommAttribute attr;
  attr.reliable = true;
  ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_AUDIO_CTRL_PAUSE_ACK,
                                          (char *)&response,
                                          sizeof(response),
                                          &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed. err=%d", ret);
  }
}

static void _do_resume(char *packet, int len) {
  AudioCtrlResumeRequest *request = (AudioCtrlResumeRequest *)packet;
  int ret = 0;

  AudioCtrlResumeResponse response;
  response.ret = ret;

  CommAttribute attr;
  attr.reliable = true;
  ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_AUDIO_CTRL_RESUME_ACK,
                                          (char *)&response,
                                          sizeof(response),
                                          &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed. err=%d", ret);
  }
}

static void _do_volume(char *packet, int len) {
  AudioCtrlVolumeRequest *request = (AudioCtrlVolumeRequest *)packet;
  int ret = 0;

  AudioCtrlVolumeResponse response;
  response.ret = ret;

  CommAttribute attr;
  attr.reliable = true;
  ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_AUDIO_CTRL_VOLUME_ACK,
                                          (char *)&response,
                                          sizeof(response),
                                          &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed. err=%d", ret);
  }
}

int AudioCtrlSerRpcReceiveCommProtocolPacket(CommPacket *packet) {
  switch (packet->cmd) {
    case CHNL_MSG_AUDIO_CTRL_START:
      _do_start(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_AUDIO_CTRL_STOP:
      _do_stop(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_AUDIO_CTRL_PAUSE:
      _do_pause(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_AUDIO_CTRL_RESUME:
      _do_resume(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_AUDIO_CTRL_VOLUME:
      _do_volume(packet->payload, packet->payload_len);
      break;
    default:
      break;
  }

  return 0;
}

void AudioCtrlMp3PlayerAudioEndCallBack(void) {
  int len = sizeof(AudioCtrlAudioEndRequest) + strlen(g_file_name) + 1;
  AudioCtrlAudioEndRequest *request = (AudioCtrlAudioEndRequest *)malloc(len);

  pthread_mutex_lock(&g_lock);
  request->type = g_audio_source_type;
  strcpy(request->file_name, g_file_name);
  pthread_mutex_unlock(&g_lock);

  LOGT(TAG, "audio end. file_name=%s, type=%d", request->file_name, request->type);
  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_AUDIO_CTRL_AUDIO_END,
                                              (char *)request,
                                              len,
                                              &attr);
  free(request);
  if (ret != 0) {
    LOGW(TAG, "transimit failed");
  }
}
