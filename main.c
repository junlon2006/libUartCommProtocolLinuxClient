#include "uni_log.h"
#include "uni_uart.h"
#include "uni_communication.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define TAG "main"

static void _comm_recv_packet_handler(CommPacket *packet) {
  LOGD(TAG, "recv packet, cmd=%d, len=%d", packet->cmd, packet->payload_len);
}

static long uni_get_clock_time_ms(void) {
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    return (ts.tv_sec * 1000L + (ts.tv_nsec / 1000000));
  }

  return 0;
}

static void *_serial_send_process(void *args) {
  char buf[2048];

  int cost;
  int avg_speed;
  int total_len = 0;
  int start_time = -1;
  int start;
  int now;

  start_time = uni_get_clock_time_ms();
  start = start_time;

  CommAttribute attr;
  attr.reliable = 1;

  while (1) {
    now = uni_get_clock_time_ms();
    if (0 == CommProtocolPacketAssembleAndSend(100, buf, sizeof(buf), &attr)) {
      total_len += sizeof(buf);
      if (now - start >= 1000) {
        cost = (now - start_time) / 1000;
        avg_speed = (int)((float)total_len / (float)(now - start_time) * 1000.0 / 1024.0);
        LOGW(TAG, "total=%dKB, cost=%d-%02d:%02d:%02d, speed=%dKB/s",
            total_len >> 10,
            cost / (3600 * 24),
            cost % (3600 * 24) / 3600,
            cost % (3600 * 24) % 3600 / 60,
            cost % (3600 * 24) % 3600 % 60,
            avg_speed);
        start = now;
      }
    }
  }
}

static void _send_task() {
  pthread_t uart_send_pid;
  pthread_create(&uart_send_pid, NULL, _serial_send_process, NULL);
  pthread_detach(uart_send_pid);
}


int main() {
  LogLevelSet(N_LOG_TRACK);

  UartConfig uart_config;
  snprintf(uart_config.device, sizeof(uart_config.device), "%s", "/dev/ttyUSB0");
  uart_config.speed = B921600;
  UartInitialize(&uart_config);

  CommProtocolInit(UartWrite, _comm_recv_packet_handler);

  _send_task();
  while (1) {
    usleep(1000 * 1000 * 30);
  }

  return 0;
}
