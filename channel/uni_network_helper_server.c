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
 * Description : uni_network_helper_server.c
 * Author      : junlon2006@163.com
 * Date        : 2020.03.20
 *
 **************************************************************************/
#include "uni_network_helper.h"
#include "uni_network_helper_server.h"
#include "uni_log.h"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define TAG            "net_helper"
#define uni_malloc     malloc
#define uni_free       free
#define uni_max(a, b)  (a > b ? a : b)
#define true           1

static void _server_do_socket_init(char *packet, int len) {
  LOGT(TAG, "recv sock init event");
  SocketInitParam *request = (SocketInitParam *)packet;
  SocketInitResponse response;
  response.session_id = request->session_id;
  response.sock_fd    = socket(request->domain, request->type, request->protocol);

  LOGT(TAG, "socket inited, fd=%d, session_id=%d", response.sock_fd, request->session_id);

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_SERVER_INIT,
                                              (char *)&response,
                                              sizeof(response),
                                              &attr);
  if (ret != 0) {
    LOGT(TAG, "server do socket init, transmit failed [%d]", ret);
  }
}

static void _server_do_socket_close(char *packet, int len) {
  SocketCloseParam *request = (SocketCloseParam *)packet;
  close(request->sock_fd);
  LOGT(TAG, "server do socket close[%d]", request->sock_fd);
}

static void _server_do_socket_send(char *packet, int len) {
  SocketSendParam *request = (SocketSendParam *)packet;
  send(request->sock_fd, (char *)request + sizeof(*request), request->size, request->flags);
  LOGT(TAG, "server do socket send[%d]", request->size);
}

static void _server_do_socket_recv(char *packet, int len) {
  SocketRecvParam *request = (SocketRecvParam *)packet;
  char *recv_buf = (char *)uni_malloc(request->len);

  int recv_len = recv(request->sock_fd, recv_buf, request->len, request->flags);
  LOGT(TAG, "fd=%d, request len=%d, recv len=%d, flags=%d, recv=%s", request->sock_fd, request->len,
       recv_len, request->flags, recv_buf);

  recv_len = uni_max(recv_len, 0);

  SocketRecvResponse *response = uni_malloc(sizeof(SocketRecvResponse) + recv_len);
  response->sock_fd = request->sock_fd;
  response->len     = recv_len;
  if (recv_len > 0) {
    memcpy(response->data, recv_buf, recv_len);
  }

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_SERVER_RECV,
                                              (char *)response,
                                              sizeof(SocketRecvResponse) + recv_len,
                                              &attr);
  uni_free(recv_buf);
  uni_free(response);

  if (ret != 0) {
    LOGT(TAG, "server do socket recv[%d], transmit failed, err=%d", recv_len, ret);
  }
}

static void _server_do_socket_setoption(char *packet, int len) {
  SocketOptionParam *request = (SocketOptionParam *)packet;
  setsockopt(request->sock_fd, request->level, request->optname,
             request->optlen == 0 ? NULL : packet + sizeof(SocketOptionParam),
             request->optlen);
  LOGT(TAG, "server do socket set option");
}

static void _server_do_socket_fcntl(char *packet, int len) {
  SocketFcntlParam *request = (SocketFcntlParam *)packet;
  fcntl(request->sock_fd, request->cmd, request->val);
  LOGT(TAG, "server do socket fcntl");
}

static void _server_select(char *packet, int len, int msg_type) {
  SocketSelectParam *request = (SocketSelectParam *)packet;

  struct timeval tv;
  if (request->timeout > 0) {
    tv.tv_sec = request->timeout / 1000;
    tv.tv_usec = (request->timeout % 1000) * 1000;
  }

  /* write fd or read fd, error fd not integrate */
  fd_set fd;
  FD_ZERO(&fd);
  FD_SET(request->sock_fd, &fd);

  int able = 0;
  int ret;
  if (msg_type == CHNL_MSG_NET_SOCKET_SERVER_SELECT_WRITE) {
    ret = select(request->sock_fd + 1, NULL, &fd, NULL,
                 request->timeout > 0 ? &tv : NULL);
  } else {
    ret = select(request->sock_fd + 1, &fd, NULL, NULL,
                 request->timeout > 0 ? &tv : NULL);
  }

  if (ret > 0 && FD_ISSET(request->sock_fd, &fd)) {
    able = 1;
  }

  SocketSelectResponse response;
  response.sock_fd = request->sock_fd;
  if (msg_type == CHNL_MSG_NET_SOCKET_SERVER_SELECT_WRITE) {
    response.readable = 0;
    response.writeable = able;
  } else {
    response.readable = able;
    response.writeable = 0;
  }

  CommAttribute attr;
  attr.reliable = true;
  ret = CommProtocolPacketAssembleAndSend(msg_type,
                                          (char *)&response,
                                          sizeof(response),
                                          &attr);
  if (0 != ret) {
    LOGW(TAG, "transmit failed");
  }
}

static void _server_do_socket_select_read(char *packet, int len) {
  LOGT(TAG, "server do socket select read");
  _server_select(packet, len, CHNL_MSG_NET_SOCKET_SERVER_SELECT_READ);
}

static void _server_do_socket_select_write(char *packet, int len) {
  LOGT(TAG, "server do socket select write");
  _server_select(packet, len, CHNL_MSG_NET_SOCKET_SERVER_SELECT_WRITE);
}

static int _websock_connect(const char *host, int port) {
  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *node;
  int ret;
  int sock_fd = -1;
  char str_port[16] = {0};

  memset(&hints, 0, sizeof(hints));
  hints.ai_addrlen   = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr      = NULL;
  hints.ai_next      = NULL;
  hints.ai_flags     = 0x01; //AI_PASSIVE
  hints.ai_family    = AF_INET;
  hints.ai_socktype  = SOCK_STREAM;
  hints.ai_protocol  = IPPROTO_IP;

  LOGT(TAG, "host=%s, port=%d", host, port);

  do {
    snprintf(str_port, sizeof(str_port), "%d", port);
    if ((ret = getaddrinfo(host, str_port, &hints, &result)) != 0) {
      LOGW(TAG, "getaddrinfo failed, err=%d", ret);
      break;
    }

    for (node = result; node != NULL; node = node->ai_next) {
      sock_fd = socket(node->ai_family, node->ai_socktype, node->ai_protocol);
      if (sock_fd == -1) {
        continue;
      }

      if (connect(sock_fd, node->ai_addr, node->ai_addrlen) != -1) {
        LOGT(TAG, "web socket connect success, sock_fd=%d", sock_fd);
        break;
      }

      LOGW(TAG, "web socket connect failed, sock_fd=%d", sock_fd);
      close(sock_fd);
      sock_fd = -1;
    }

    freeaddrinfo(result);
  } while (0);

  return sock_fd;
}

static void _server_do_socket_websock_connect(char *packet, int len) {
  WebSocketInitParam *request = (WebSocketInitParam *)packet;

  WebSocketInitResponse response;
  response.session_id = request->session_id;
  response.sock_fd = _websock_connect(packet + sizeof(WebSocketInitParam), request->port);

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_SERVER_WEBSOCK_CONN,
                                              (char *)&response,
                                              sizeof(response),
                                              &attr);
  if (ret != 0) {
    LOGT(TAG, "transmit failed");
  }
}

static void _server_do_socket_socket_connect(char *packet, int len) {
  SocketConnParam *request = (SocketConnParam *)packet;

  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *node;
  char str_port[16] = {0};
  const char* host = packet + sizeof(SocketConnParam);
  snprintf(str_port, sizeof(str_port), "%d", request->port);

  memset(&hints, 0, sizeof(hints));
  hints.ai_addrlen   = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr      = NULL;
  hints.ai_next      = NULL;
  hints.ai_flags     = AI_PASSIVE;
  hints.ai_family    = AF_INET;
  hints.ai_socktype  = SOCK_STREAM;
  hints.ai_protocol  = IPPROTO_IP;

  if (0 != getaddrinfo(host, str_port, &hints, &result)) {
    LOGW(TAG, "getaddr info failed, host=%s, port=%d", host, request->port);
    return;
  }

  for (node = result; node != NULL; node = node->ai_next) {
    if (-1 != connect(request->sock_fd, node->ai_addr, node->ai_addrlen)) {
      LOGT(TAG, "connect success. host=%s, port=%d, fd=%d", host, request->port, request->sock_fd);
      break;
    } else {
      LOGW(TAG, "connect failed. host=%s, port=%d, fd=%d", host, request->port, request->sock_fd);
    }
  }

  freeaddrinfo(result);
}

int NetHelperSerRpcReceiveCommProtocolPacket(CommPacket *packet) {
  switch (packet->cmd) {
    case CHNL_MSG_NET_SOCKET_CLIENT_INIT:
      _server_do_socket_init(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_CLOSE:
      _server_do_socket_close(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_SEND:
      _server_do_socket_send(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_RECV:
      _server_do_socket_recv(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_SETOPTION:
      _server_do_socket_setoption(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_FCNTL:
      _server_do_socket_fcntl(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_SELECT_READ:
      _server_do_socket_select_read(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_SELECT_WRITE:
      _server_do_socket_select_write(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_WEBSOCK_CONN:
      _server_do_socket_websock_connect(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_CLIENT_CONN:
      _server_do_socket_socket_connect(packet->payload, packet->payload_len);
      break;

    default:
      LOGW(TAG, "not net helper event");
      return -1;
  }

  return 0;
}

