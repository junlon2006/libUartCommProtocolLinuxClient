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
 * Description : uni_network_helper.h
 * Author      : junlon2006@163.com
 * Date        : 2020.03.20
 *
 **************************************************************************/
#ifndef CHANNEL_UNI_NETWORK_HELPER_H_
#define CHANNEL_UNI_NETWORK_HELPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_channel.h"

typedef enum {
  CHNL_MSG_NET_SOCKET_CLIENT_INIT = NETWORK_HELPER_MESSAGE_BASE,
  CHNL_MSG_NET_SOCKET_SERVER_INIT,

  CHNL_MSG_NET_SOCKET_CLIENT_CLOSE,
  CHNL_MSG_NET_SOCKET_SERVER_CLOSE,

  CHNL_MSG_NET_SOCKET_CLIENT_SEND,
  CHNL_MSG_NET_SOCKET_SERVER_SEND,

  CHNL_MSG_NET_SOCKET_CLIENT_RECV,
  CHNL_MSG_NET_SOCKET_SERVER_RECV,

  CHNL_MSG_NET_SOCKET_CLIENT_SETOPTION,
  CHNL_MSG_NET_SOCKET_SERVER_SETOPTION,

  CHNL_MSG_NET_SOCKET_CLIENT_FCNTL,
  CHNL_MSG_NET_SOCKET_SERVER_FCNTL,

  CHNL_MSG_NET_SOCKET_CLIENT_SELECT_READ,
  CHNL_MSG_NET_SOCKET_SERVER_SELECT_READ,

  CHNL_MSG_NET_SOCKET_CLIENT_SELECT_WRITE,
  CHNL_MSG_NET_SOCKET_SERVER_SELECT_WRITE,

  CHNL_MSG_NET_SOCKET_CLIENT_WEBSOCK_CONN,
  CHNL_MSG_NET_SOCKET_SERVER_WEBSOCK_CONN,

  CHNL_MSG_NET_SOCKET_CLIENT_CONN,
  CHNL_MSG_NET_SOCKET_SERVER_CONN,

  CHNL_MSG_NET_STATUS_CLIENT_REQUEST,
  CHNL_MSG_NET_STATUS_SERVER_RESPONSE,

  CHNL_MSG_NET_STATUS_SERVER_BROADCAST_REQUEST,
  CHNL_MSG_NET_STATUS_CLIENT_RESPONSE,

  CHNL_MSG_NET_SOFTAP_ENABLE_REQUEST,
  CHNL_MSG_NET_SOFTAP_ENABLE_RESPONSE,

  CHNL_MSG_NET_SOFTAP_DISABLE_REQUEST,
  CHNL_MSG_NET_SOFTAP_DISABLE_RESPONSE,

  CHNL_MSG_NET_CNT,
} NetworkMessageType;

typedef enum {
  NET_CONNECTED = 0,
  NET_CONNECTING,
  NET_CONNECTED_FAILED,
  NET_DISCONNECTED,
} NetWorkStatus;

typedef struct {
  NetWorkStatus status;
} PACKED NetWorkStatusInfo;

typedef struct {
  int session_id;
  int domain;
  int type;
  int protocol;
} PACKED SocketInitParam;

typedef struct {
  int session_id;
  int sock_fd;
  int err_code;
} PACKED SocketInitResponse;

typedef struct {
  int sock_fd;
} PACKED SocketCloseParam;

typedef struct {
  int sock_fd;
  int ret;
  int err_code;
} PACKED SocketCloseResponse;

typedef struct {
  int      sock_fd;
  int      flags;
  uint32_t size;
  char     data[0];
} PACKED SocketSendParam;

typedef struct {
  int sock_fd;
  int ret;
  int err_code;
} PACKED SocketSendResponse;

typedef struct {
  int sock_fd;
  int len;
  int flags;
} PACKED SocketRecvParam;

typedef struct {
  int  sock_fd;
  int  len;
  int  err_code;
  char data[0];
} PACKED SocketRecvResponse;

typedef struct {
  int  sock_fd;
  int  level;
  int  optname;
  int  optlen;
  char optval[0];
} PACKED SocketOptionParam;

typedef struct {
  int sock_fd;
  int ret;
  int err_code;
} PACKED SocketOptionResponse;

typedef struct {
  int sock_fd;
  int cmd;
  int val;
} PACKED SocketFcntlParam;

typedef struct {
  int sock_fd;
  int ret;
  int err_code;
} PACKED SocketFcntlResponse;

typedef struct {
  int sock_fd;
  int timeout;
} PACKED SocketSelectParam;

typedef struct {
  int sock_fd;
  int readable;
  int writeable;
} PACKED SocketSelectResponse;

typedef struct {
  int  session_id;
  int  port;
  char host[0];
} PACKED WebSocketInitParam;

typedef struct {
  int session_id;
  int sock_fd;
} PACKED WebSocketInitResponse;

typedef struct {
  int sock_fd;
  int port;
  int host[0];
} PACKED SocketConnParam;

typedef struct {
  int sock_fd;
  int ret;
  int err_code;
} PACKED SocketConnResponse;

typedef struct {
  int ret;
  int err_code;
} PACKED SoftApEnableResponse;

typedef struct {
  int ret;
  int err_code;
} PACKED SoftApDisableResponse;

#ifdef __cplusplus
}
#endif
#endif // CHANNEL_UNI_NETWORK_HELPER_H_

