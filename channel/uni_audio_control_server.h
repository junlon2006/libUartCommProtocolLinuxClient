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
 * Description : uni_audio_control_server.h
 * Author      : junlon2006@163.com
 * Date        : 2020.04.01
 *
 **************************************************************************/
#ifndef SDK_CHANNEL_INC_UNI_AUDIO_CONTROL_SERVER_H_
#define SDK_CHANNEL_INC_UNI_AUDIO_CONTROL_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_channel.h"

/**
 * @brief comm protocol packet hook
 * @param packet
 * @return 0 success, -1 failed
 */
int AudioCtrlSerRpcReceiveCommProtocolPacket(CommPacket *packet);

/**
 * @brief mp3 audio end hook
 * @param void
 * @return void
 */
void AudioCtrlMp3PlayerAudioEndCallBack(void);

#ifdef __cplusplus
}
#endif
#endif  //  SDK_CHANNEL_INC_UNI_AUDIO_CONTROL_SERVER_H_
