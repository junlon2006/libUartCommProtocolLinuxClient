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
 * Description : uni_uart.c
 * Author      : junlon2006@163.com
 * Date        : 2020.03.18
 *
 **************************************************************************/
#include "uni_uart.h"
#include "uni_communication.h"
#include "uni_log.h"
#include "uni_ringbuf.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#define TAG                       "uart"
#define PROTOCOL_BUFFER_MAX_SIZE  (4096)

static int              uart_fd = -1;
static int              is_running = 0;
static RingBufferHandle ringbuf = NULL;

static int _set_speed(speed_t *speed) {
  int status;
  struct termios options;

  tcgetattr(uart_fd, &options);
  tcflush(uart_fd, TCIOFLUSH);
  cfsetispeed(&options, *speed);
  cfsetospeed(&options, *speed);

  status = tcsetattr(uart_fd, TCSANOW, &options);
  if (0 != status) {
    return -1;
  }

  tcflush(uart_fd, TCIOFLUSH);

  return 0;
}

static void _set_option(struct termios *options) {
  cfmakeraw(options);            /* 配置为原始模式 */
  options->c_cflag &= ~CSIZE;
  options->c_cflag |= CS8;       /* 8位数据位 */
  options->c_iflag |= INPCK;     /* disable parity checking */
  options->c_cflag &= ~CSTOPB;   /* 一个停止位 */
  options->c_cc[VTIME] = 0;      /*设置等待时间*/
  options->c_cc[VMIN] = 0;       /*最小接收字符*/
}

static int _set_parity() {
  struct termios options;
  if (tcgetattr(uart_fd, &options) != 0) {
    return -1;
  }

  _set_option(&options);
  if (tcsetattr(uart_fd, TCSANOW, &options) != 0) {
    return -1;
  }

  tcflush(uart_fd, TCIOFLUSH);
  return 0;
}

static void _free_all() {
  if (uart_fd) {
    close(uart_fd);
    uart_fd = 0;
  }
}

static void *_recv_task(void *arg) {
  unsigned char buffer[PROTOCOL_BUFFER_MAX_SIZE] = {0};
  fd_set rfds;
  struct timeval tv;
  int ret;
  int read_len;

  while (is_running) {
    FD_ZERO(&rfds);
    FD_SET(uart_fd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 100 * 1000;

    ret = select(uart_fd + 1, &rfds, NULL, NULL, &tv);
    if (0 < ret && FD_ISSET(uart_fd, &rfds)) {
      read_len = read(uart_fd, buffer, sizeof(buffer));
      if (read_len > 0) {
        if (RingBufferGetFreeSize(ringbuf) > read_len) {
          RingBufferWrite(ringbuf, buffer, read_len);
        } else {
          LOGW(TAG, "uart ringbuf full");
        }
      }
    }
  }

  _free_all();
  return NULL;
}

static void *_protocol_stack_parse_task(void *args) {
  int len;
  char buf[2048];
  while (is_running) {
    len = RingBufferGetDataSize(ringbuf);
    if (len == 0) {
      usleep(1000 * 2);
    }

    len = len > sizeof(buf) ? sizeof(buf) : len;
    RingBufferRead(buf, len, ringbuf);
    CommProtocolReceiveUartData((unsigned char *)buf, len);
  }
}

static int _create_worker_thread() {
  pthread_t pid;
  pthread_create(&pid, NULL, _recv_task, NULL);
  pthread_detach(pid);

  pthread_create(&pid, NULL, _protocol_stack_parse_task, NULL);
  pthread_detach(pid);
  return 0;
}

int UartInitialize(UartConfig *config) {
  uart_fd = open(config->device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (-1 == uart_fd) {
    LOGE(TAG, "open %s failed[%s]", config->device, strerror(errno));
    return -1;
  }

  if (0 != _set_speed(&config->speed)) {
    LOGE(TAG, "set speed failed");
    return -1;
  }

  if (0 != _set_parity()) {
    LOGE(TAG, "set parity failed");
    return -1;
  }

  ringbuf = RingBufferCreate(8192);
  is_running = 1;
  _create_worker_thread();

  LOGT(TAG, "uart init success");
  return 0;
}

int UartFinalize() {
  is_running = 0;
  return 0;
}

int UartWrite(char *buf, int len) {
  return write(uart_fd, buf, len);
}

