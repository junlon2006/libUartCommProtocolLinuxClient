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
 * Description : uni_network_helper_client.h
 * Author      : junlon2006@163.com
 * Date        : 2020.03.20
 *
 **************************************************************************/
#ifndef SDK_CHANNEL_INC_UNI_NETWORK_HELPER_CLIENT_H_
#define SDK_CHANNEL_INC_UNI_NETWORK_HELPER_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_channel.h"

/**
 * @brief net helper client init
 * @param void
 * @return 0 success, -1 failed
 */
int NetHelperCliInit(void);

/**
 * @brief comm protocol packet hook
 * @param packet
 * @return 0 success, -1 failed
 */
int NetHelperCliRpcReceiveCommProtocolPacket(CommPacket *packet);

/**
 * @brief rpc client socket request
 * @param domain
 * @param type
 * @param protocol
 * @return socket fd, -1 failed
 */
int NetHelperCliRpcSocketInit(int domain, int type, int protocol);

/**
 * @brief rpc client socket close request
 * @param socket
 * @return -1 failed, 0 success
 */
int NetHelperCliRpcSocketClose(int socket);

/**
 * @brief rpc client socket connect
 * @param socket
 * @param host
 * @param port
 */
int NetHelperCliRpcSocketConnect(int socket, const char* host, int port);

/**
 * @brief rpc client send
 * @param socket
 * @param data
 * @param size
 * @param flags
 * @return actual send size, -1 failed
 */
int NetHelperCliRpcSend(int socket, const void *data, uint32_t size, int flags);

/**
 * @brief rpc client recv
 * @param socket
 * @param mem
 * @param len
 * @param flags
 * @return actual recv size, -1 failed
 */
int NetHelperCliRpcRecv(int socket, void *mem, uint32_t len, int flags);

/**
 * @brief rpc client set socket option
 * @param socket
 * @param level
 * @param optname
 * @param optval
 * @param optlen
 * @return 0 success, -1 failed
 */
int NetHelperCliRpcSetSockOption(int socket, int level, int optname,
                                 const void *optval, uint32_t optlen);

/**
 * @brief rpc client fcntl
 * @param socket
 * @param cmd
 * @param val
 * @return 0 success, -1 failed
 */
int NetHelperCliRpcFcntl(int socket, int cmd, int val);

/**
 * @brief rpc client select write
 * @param sock_fd
 * @param timeout ms
 * @return 0 success, -1 failed
 */
int NetHelperCliRpcSelectWrite(int sock_fd, int timeout);

/**
 * @brief rpc client select read
 * @param timeout
 * @return 0 success, -1 failed
 */
int NetHelperCliRpcSelectRead(int sock_fd, int timeout);

/**
 * @brief rpc client websocket connect
 * @param host
 * @param port
 * @return 0 success, -1 failed
 */
int NetHelperCliRpcConnectWebSocket(const char* host, int port);

#ifdef __cplusplus
}
#endif
#endif // SDK_CHANNEL_INC_UNI_NETWORK_HELPER_CLIENT_H_

