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
 * Description : uni_network_helper_client.c
 * Author      : junlon2006@163.com
 * Date        : 2020.03.20
 *
 **************************************************************************/
#include "uni_network_helper_client.h"
#include "uni_network_helper.h"
#include "uni_log.h"
#include "uni_iot.h"

#include <unistd.h>

#define TAG           "net_helper_cli"
#define FD_SIZE_MAX   (8)

typedef enum {
  SOCK_INIT_IDLE = 0,
  SOCK_INIT_WAIT,
  SOCK_INIT_DONE,
} SockStatus;

typedef struct {
  /* multi-thread support */
  uni_sem_t sem[(CHNL_MSG_NET_CNT - NETWORK_HELPER_MESSAGE_BASE) >> 1];
  int       session_id;
  int       status;
  int       sock_fd;
  int       sock_init_errno;
  char      *recv_buf;
  int       recv_len;
  int       recv_errno;
  int       send_len;
  int       send_errno;
  int       readable;
  int       writeable;
  int       conn_ret;
  int       conn_errno;
  int       socksetopt_ret;
  int       socksetopt_errno;
  int       fnctl_ret;
  int       fnctl_errno;
  int       close_ret;
  int       close_errno;
} SocketResource;

static SocketResource g_socks[FD_SIZE_MAX] = {0};
static uni_mutex_t    g_mutex = NULL;
static uni_sem_t      g_net_status_sem;
static NetWorkStatus  g_net_status = NET_DISCONNECTED;
static void           (*g_net_status_change) (NetWorkStatus status) = NULL;

static int _get_sem_index_by_client_msg(int type) {
  return (type - NETWORK_HELPER_MESSAGE_BASE) >> 1;
}

static void _sock_fds_init() {
  int i, j;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    g_socks[i].session_id = -1;
    g_socks[i].status     = SOCK_INIT_IDLE;
    g_socks[i].sock_fd    = -1;

    for (j = 0; j < (CHNL_MSG_NET_CNT - NETWORK_HELPER_MESSAGE_BASE) >> 1; j++) {
      uni_sem_init(&g_socks[i].sem[j], 0);
    }
  }

  uni_sem_init(&g_net_status_sem, 0);
  uni_pthread_mutex_init(&g_mutex);
}

static int _get_current_session_id() {
  static uint32_t session_id = 0;
  return ++session_id;
}

static int _get_fds_idx() {
  int i;
  int idx = -1;

  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_IDLE) {
      idx = i;
      break;
    }
  }

  LOGT(TAG, "find idle sock idx=%d", idx);
  return idx;
}

int NetHelperCliRpcNetStatusChangeHookRegister(void (*hook) (NetWorkStatus status)) {
  g_net_status_change = hook;
  return 0;
}

int NetHelperCliRpcNetStatusGet(void) {
  CommAttribute attr;
  attr.reliable = true;

  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_STATUS_CLIENT_REQUEST,
                                              NULL,
                                              0,
                                              &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed. err=%d", ret);
    return -1;
  }

  uni_sem_wait(g_net_status_sem);
  LOGT(TAG, "net status=%d", g_net_status);
  return g_net_status;
}

int NetHelperCliRpcSocketInit(int domain, int type, int protocol) {
  SocketInitParam request;
  int idx;

  uni_pthread_mutex_lock(g_mutex);
  {
    idx = _get_fds_idx();
    if (-1 == idx) {
      uni_pthread_mutex_unlock(g_mutex);
      LOGW(TAG, "fds max size=%d, please close some first", FD_SIZE_MAX);
      return -1;
    }

    int session_id = _get_current_session_id();
    g_socks[idx].session_id = session_id;
    g_socks[idx].status     = SOCK_INIT_WAIT;
    g_socks[idx].sock_fd    = -1;

    request.domain     = domain;
    request.type       = type;
    request.protocol   = protocol;
    request.session_id = session_id;
  }
  uni_pthread_mutex_unlock(g_mutex);

  CommAttribute attr;
  attr.reliable = true;
  if (0 != CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_CLIENT_INIT,
                                             (char *)&request,
                                             sizeof(request),
                                             &attr)) {
    /* release g_socks[idx] */
    uni_pthread_mutex_lock(g_mutex);
    g_socks[idx].status = SOCK_INIT_IDLE;
    uni_pthread_mutex_unlock(g_mutex);

    LOGW(TAG, "transmit failed");
    return -1;
  }

  int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_INIT);
  uni_sem_wait(g_socks[idx].sem[sem_idx]);
  /* return idx insteads of sock_fd */
  return idx;
}

int NetHelperCliRpcSocketClose(int socket) {
  SocketCloseParam request;
  CommAttribute attr;

  request.sock_fd = g_socks[socket].sock_fd;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_CLIENT_CLOSE,
                                              (char *)&request,
                                              sizeof(request),
                                              &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed");
    return -1;
  }

  int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_CLOSE);
  uni_sem_wait(g_socks[socket].sem[sem_idx]);

  /* socket is idx */
  uni_pthread_mutex_lock(g_mutex);
  g_socks[socket].status  = SOCK_INIT_IDLE;
  g_socks[socket].sock_fd = -1;
  uni_pthread_mutex_unlock(g_mutex);

  return g_socks[socket].close_ret;
}

int NetHelperCliRpcSocketConnect(int socket, const char* host, int port) {
  int len = sizeof(SocketConnParam) + strlen(host) + 1;
  SocketConnParam *request = (SocketConnParam *)uni_malloc(len);
  request->port            = port;
  request->sock_fd         = g_socks[socket].sock_fd;
  strcpy((char *)request->host, host);

  LOGT(TAG, "connect fd=%d, host=%s, port=%d", request->sock_fd,
       request->host, request->port);

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_CLIENT_CONN,
                                              (char *)request,
                                              len,
                                              &attr);
  uni_free(request);

  if (ret != 0) {
    LOGW(TAG, "transmit failed");
    return -1;
  }

  /* block mode, issue when peer death */
  int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_CONN);
  uni_sem_wait(g_socks[socket].sem[sem_idx]);
  return g_socks[socket].conn_ret;
}

int NetHelperCliRpcSend(int socket, const void *data, uint32_t size, int flags) {
  /* donnot check status now */
  SocketSendParam *request;
  request          = (SocketSendParam *)uni_malloc(sizeof(SocketSendParam) + size);
  request->sock_fd = g_socks[socket].sock_fd;
  request->flags   = flags;
  request->size    = size;

  memcpy(request->data, data, size);

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_CLIENT_SEND,
                                              (char *)request,
                                              sizeof(*request) + request->size,
                                              &attr);
  uni_free(request);

  if (ret != 0) {
    LOGW(TAG, "transmit failed");
    return -1;
  }

  int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_SEND);
  uni_sem_wait(g_socks[socket].sem[sem_idx]);
  return g_socks[socket].send_len;
}

int NetHelperCliRpcRecv(int socket, void *mem, uint32_t len, int flags) {
  /* donnot check status now */
  SocketRecvParam request;
  request.sock_fd = g_socks[socket].sock_fd;
  request.len     = len;
  request.flags   = flags;

  g_socks[socket].recv_buf = mem;

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_CLIENT_RECV,
                                              (char *)&request,
                                              sizeof(request),
                                              &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed");
    return -1;
  }

  /* TODO block mode, potential risk when peer death */
  int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_RECV);
  uni_sem_wait(g_socks[socket].sem[sem_idx]);

  return g_socks[socket].recv_len;
}

int NetHelperCliRpcSetSockOption(int socket, int level, int optname,
                                 const void *optval, uint32_t optlen) {
  SocketOptionParam *request = uni_malloc(sizeof(SocketOptionParam) + optlen);
  request->sock_fd = g_socks[socket].sock_fd;
  request->level   = level;
  request->optname = optname;
  request->optlen  = optlen;

  if (optlen > 0) {
    memcpy(request->optval, optval, optlen);
  }

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_CLIENT_SETOPTION,
                                              (char *)request,
                                              sizeof(*request) + optlen,
                                              &attr);
  uni_free(request);

  if (ret != 0) {
    LOGW(TAG, "transmit failed");
    return -1;
  }

  int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_SETOPTION);
  uni_sem_wait(g_socks[socket].sem[sem_idx]);

  return g_socks[socket].socksetopt_ret;
}

int NetHelperCliRpcFcntl(int socket, int cmd, int val) {
  SocketFcntlParam request;
  request.sock_fd = g_socks[socket].sock_fd;
  request.cmd     = cmd;
  request.val     = val;

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_CLIENT_FCNTL,
                                              (char *)&request,
                                              sizeof(request),
                                              &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed");
    return -1;
  }

  int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_FCNTL);
  uni_sem_wait(g_socks[socket].sem[sem_idx]);

  return g_socks[socket].fnctl_ret;
}

/* read & write support, error not integrate now */
static int _client_select(int sock_fd, int timeout, int msg_type) {
  SocketSelectParam request;
  request.sock_fd = g_socks[sock_fd].sock_fd;
  request.timeout = timeout;

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(msg_type,
                                              (char *)&request,
                                              sizeof(request),
                                              &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed");
    return -1;
  }

  int sem_idx = _get_sem_index_by_client_msg(msg_type);
  uni_sem_wait(g_socks[sock_fd].sem[sem_idx]);

  return msg_type == CHNL_MSG_NET_SOCKET_CLIENT_SELECT_WRITE ?
         g_socks[sock_fd].writeable : g_socks[sock_fd].readable;
}

int NetHelperCliRpcSelectWrite(int sock_fd, int timeout) {
  return _client_select(sock_fd, timeout, CHNL_MSG_NET_SOCKET_CLIENT_SELECT_WRITE);
}

int NetHelperCliRpcSelectRead(int sock_fd, int timeout) {
  return _client_select(sock_fd, timeout, CHNL_MSG_NET_SOCKET_CLIENT_SELECT_READ);
}

int NetHelperCliRpcConnectWebSocket(const char* host, int port) {
  int idx;
  int session_id;
  uni_pthread_mutex_lock(g_mutex);
  {
    idx = _get_fds_idx();
    if (idx == -1) {
      uni_pthread_mutex_unlock(g_mutex);
      LOGW(TAG, "fds max size=%d, please close some first", FD_SIZE_MAX);
      return -1;
    }

    session_id = _get_current_session_id();
    g_socks[idx].session_id = session_id;
    g_socks[idx].status = SOCK_INIT_WAIT;
    g_socks[idx].sock_fd = -1;
  }
  uni_pthread_mutex_unlock(g_mutex);

  int len = sizeof(WebSocketInitParam) + strlen(host) + 1;
  WebSocketInitParam *request = uni_malloc(len);
  request->session_id = session_id;
  request->port = port;
  strcpy(request->host, host);

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_NET_SOCKET_CLIENT_WEBSOCK_CONN,
                                              (char *)request,
                                              len,
                                              &attr);
  uni_free(request);
  if (ret != 0) {
    /* release g_socks[idx] */
    uni_pthread_mutex_lock(g_mutex);
    g_socks[idx].status = SOCK_INIT_IDLE;
    uni_pthread_mutex_unlock(g_mutex);

    LOGW(TAG, "transmit failed");
    return -1;
  }

  /* block mode */
  int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_WEBSOCK_CONN);
  uni_sem_wait(g_socks[idx].sem[sem_idx]);
  return idx;
}

static void _client_do_socket_init_response(char *packet, int len) {
  SocketInitResponse *response = (SocketInitResponse *)packet;
  int i;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    /* find idx by session_id */
    if (g_socks[i].session_id == response->session_id) {
      uni_pthread_mutex_lock(g_mutex);
      {
        g_socks[i].sock_fd         = response->sock_fd;
        g_socks[i].sock_init_errno = response->err_code;
        g_socks[i].status          = SOCK_INIT_DONE;

        LOGT(TAG, "fd=%d, errno=%d, session_id=%d", g_socks[i].sock_fd,
             g_socks[i].sock_init_errno, g_socks[i].session_id);
      }
      uni_pthread_mutex_unlock(g_mutex);

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_INIT);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      break;
    }
  }

  LOGT(TAG, "client do socket int response");
}

static void _client_do_socket_send_response(char *packet, int len) {
  int i;
  SocketSendResponse *response = (SocketSendResponse *)packet;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_DONE &&
        g_socks[i].sock_fd == response->sock_fd) {
      LOGD(TAG, "send data, fd=%d, len=%d, errno=%d", response->sock_fd,
           response->ret, response->err_code);

      g_socks[i].send_len   = response->ret;
      g_socks[i].send_errno = response->err_code;

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_SEND);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      return;
    }
  }
}

static void _client_do_socket_recv_response(char *packet, int len) {
  int i;
  SocketRecvResponse *response = (SocketRecvResponse *)packet;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_DONE &&
        g_socks[i].sock_fd == response->sock_fd) {
      LOGD(TAG, "recv data. len=%d, fd=%d", response->len, response->sock_fd);

      g_socks[i].recv_len   = response->len;
      g_socks[i].recv_errno = response->err_code;
      if (response->len > 0) {
        memcpy(g_socks[i].recv_buf,
               packet + sizeof(SocketRecvResponse),
               response->len);
      }

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_RECV);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      return;
    }
  }
}

static void _client_do_socket_writeable_response(char *packet, int len) {
  int i;
  SocketSelectResponse *response = (SocketSelectResponse *)packet;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_DONE &&
        g_socks[i].sock_fd == response->sock_fd) {
      g_socks[i].writeable = response->writeable;

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_SELECT_WRITE);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      return;
    }
  }
}

static void _client_do_socket_readable_response(char *packet, int len) {
  int i;
  SocketSelectResponse *response = (SocketSelectResponse *)packet;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_DONE &&
        g_socks[i].sock_fd == response->sock_fd) {
      g_socks[i].readable = response->readable;

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_SELECT_READ);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      return;
    }
  }
}

static void _client_do_socket_websock_connect_response(char *packet, int len) {
  WebSocketInitResponse *response = (WebSocketInitResponse *)packet;
  int i;
  //TODO handle error
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].session_id == response->session_id) {
      uni_pthread_mutex_lock(g_mutex);
      g_socks[i].sock_fd = response->sock_fd;
      g_socks[i].status = SOCK_INIT_DONE;
      uni_pthread_mutex_unlock(g_mutex);

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_WEBSOCK_CONN);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      return;
    }
  }
}

static void _client_do_socket_connect_response(char *packet, int len) {
  SocketConnResponse *response = (SocketConnResponse *)packet;
  int i;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_DONE &&
        g_socks[i].sock_fd == response->sock_fd) {
      g_socks[i].conn_ret   = response->ret;
      g_socks[i].conn_errno = response->err_code;

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_CONN);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      return;
    }
  }
}

static void _client_do_socket_setsockoption_response(char *packet, int len) {
  SocketOptionResponse *response = (SocketOptionResponse *)packet;
  int i;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_DONE &&
        g_socks[i].sock_fd == response->sock_fd) {
      g_socks[i].socksetopt_ret  = response->ret;
      g_socks[i].sock_init_errno = response->err_code;

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_SETOPTION);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      return;
    }
  }
}

static void _client_do_socket_fcntl_response(char *packet, int len) {
  SocketFcntlResponse *response = (SocketFcntlResponse *)packet;
  int i;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_DONE &&
        g_socks[i].sock_fd == response->sock_fd) {
      g_socks[i].fnctl_ret   = response->ret;
      g_socks[i].fnctl_errno = response->err_code;

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_FCNTL);
      uni_sem_post(g_socks[i].sem[sem_idx]);
      return;
    }
  }
}

static void _client_do_socket_close_response(char *packet, int len) {
  SocketCloseResponse *response = (SocketCloseResponse *)packet;
  int i;
  for (i = 0; i < FD_SIZE_MAX; i++) {
    if (g_socks[i].status == SOCK_INIT_DONE &&
        g_socks[i].sock_fd == response->sock_fd) {
      g_socks[i].close_ret   = response->ret;
      g_socks[i].close_errno = response->err_code;

      int sem_idx = _get_sem_index_by_client_msg(CHNL_MSG_NET_SOCKET_CLIENT_CLOSE);
      uni_sem_post(g_socks[i].sem[sem_idx]);
    }
  }
}

static void _client_do_net_status_response(char *packet, int len) {
  NetWorkStatusInfo *info = (NetWorkStatusInfo *)packet;
  g_net_status = info->status;
  uni_sem_post(g_net_status_sem);
}

static void _client_do_net_status_broadcast(char *packet, int len) {
  NetWorkStatusInfo *info = (NetWorkStatusInfo *)packet;
  g_net_status = info->status;
  LOGT(TAG, "get server broadcast net status=%d", g_net_status);

  if (g_net_status_change) {
    g_net_status_change(g_net_status);
  }
}

int NetHelperCliRpcReceiveCommProtocolPacket(CommPacket *packet) {
  switch (packet->cmd) {
    case CHNL_MSG_NET_SOCKET_SERVER_INIT:
      _client_do_socket_init_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_SEND:
      _client_do_socket_send_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_RECV:
      _client_do_socket_recv_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_SELECT_WRITE:
      _client_do_socket_writeable_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_SELECT_READ:
      _client_do_socket_readable_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_WEBSOCK_CONN:
      _client_do_socket_websock_connect_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_CONN:
      _client_do_socket_connect_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_SETOPTION:
      _client_do_socket_setsockoption_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_FCNTL:
      _client_do_socket_fcntl_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_SOCKET_SERVER_CLOSE:
      _client_do_socket_close_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_STATUS_SERVER_RESPONSE:
      _client_do_net_status_response(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_NET_STATUS_SERVER_BROADCAST_REQUEST:
      _client_do_net_status_broadcast(packet->payload, packet->payload_len);
      break;
    default:
      return -1;
  }

  return 0;
}

int NetHelperCliInit() {
  _sock_fds_init();
  return 0;
}

