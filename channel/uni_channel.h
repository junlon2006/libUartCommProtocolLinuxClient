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
 * Description : uni_channel.h
 * Author      : junlon2006@163.com
 * Date        : 2020.03.10
 *
 **************************************************************************/
#ifndef CHANNEL_UNI_CHANNEL_H_
#define CHANNEL_UNI_CHANNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_communication.h"
#include <inttypes.h>

#define PACKED                       __attribute__ ((packed))
#define PAYLOAD_LEN                 (2048)

#define LASR_BUSINESS_MESSAGE_BASE  (1000)
#define NETWORK_HELPER_MESSAGE_BASE (2000)
#define AUDIO_CONTROL_MESSAGE_BASE  (3000)

/**
 * @brief chnl init
 * @param void
 * @return 0 success, 1 failed
 */
int ChnlInit(void);

/**
 * @brief comm protocol packet hook
 * @param packet
 * @return void
 */
void ChnlReceiveCommProtocolPacket(CommPacket *packet);

#ifdef __cplusplus
}
#endif
#endif  // CHANNEL_UNI_CHANNEL_H_
