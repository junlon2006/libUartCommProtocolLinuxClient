#include "uni_log.h"
#include "uni_uart.h"
#include "uni_communication.h"
#include "uni_channel.h"


#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#define TAG "main"

static void _comm_recv_packet_handler(CommPacket *packet) {
  LOGD(TAG, "recv packet, cmd=%d, len=%d", packet->cmd, packet->payload_len);
  ChnlReceiveCommProtocolPacket(packet);
}

static long uni_get_clock_time_ms(void) {
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    return (ts.tv_sec * 1000L + (ts.tv_nsec / 1000000));
  }

  return 0;
}

static void _do_wakeup_mode() {
  ChnlRecognizeReq request;
  request.mode = 0;
  request.type = CHNL_MESSAGE_RECOGNIZE_REQUEST;
  ChnlRecognizeRequest(&request);
}

static void _do_cmd_mode() {
  ChnlRecognizeReq request;
  request.mode = 1;
  request.type = CHNL_MESSAGE_RECOGNIZE_REQUEST;
  ChnlRecognizeRequest(&request);
}

static void _do_pull_raw_data() {
  ChnlPullNoiseReductionDataReq request;
  request.type = CHNL_MESSAGE_PULL_NOISE_REDUCTION_DATA_REQUEST;
  request.mode = 1;
  ChnlPullNoiseReductionDataRequest(&request);
}

static void _stop_raw_data_recv() {
  ChnlPullNoiseReductionDataReq request;
  request.type = CHNL_MESSAGE_PULL_NOISE_REDUCTION_DATA_REQUEST;
  request.mode = 0;
  ChnlPullNoiseReductionDataRequest(&request);
}

static void _parse_command_line(char *cmd, int len) {
  cmd[len - 1] = '\0';
  LOGT(TAG, "cmd=%s", cmd);

  if (0 == strcmp(cmd, "wakeup")) {
    _do_wakeup_mode();
    return;
  }

  if (0 == strcmp(cmd, "cmd")) {
    _do_cmd_mode();
    return;
  }

  if (0 == strcmp(cmd, "data1")) {
    _do_pull_raw_data();
    return;
  }

  if (0 == strcmp(cmd, "data0")) {
    _stop_raw_data_recv();
    return;
  }
}

int main(int argc, char *argv[]) {
  LogLevelSet(N_LOG_TRACK);

  UartConfig uart_config;
  snprintf(uart_config.device, sizeof(uart_config.device), "%s", "/dev/ttyUSB0");
  uart_config.speed = B921600;
  if (0 != UartInitialize(&uart_config)) {
    return -1;
  }

  CommProtocolInit(UartWrite, _comm_recv_packet_handler);

  ChnlInit();

  char cmd[1024];
  while (1) {
    int nread = read(fileno(stdin), cmd, sizeof(cmd));
    if (nread == -1) {
      LOGW(TAG, "read from stdin error[%s]", strerror(errno));
      continue;
    }

    if (nread <= 1) {
      continue;
    }

    _parse_command_line(cmd, nread);
    LOGT(TAG, "nread=%d", nread);
  }

  return 0;
}

