#include "uni_log.h"
#include "uni_uart.h"
#include "uni_communication.h"
#include "uni_channel.h"
#include "uni_asr_business.h"
#include "uni_event_list.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define TAG "main"

static EventListHandle g_event_list = NULL;

static void _comm_recv_packet_handler(CommPacket *packet) {
  CommPacket *comm_packet  = (CommPacket *)malloc(sizeof(CommPacket) + packet->payload_len);
  comm_packet->cmd         = packet->cmd;
  comm_packet->payload_len = packet->payload_len;
  memcpy(comm_packet->payload, packet->payload, packet->payload_len);

  EventListAdd(g_event_list, comm_packet, EVENT_LIST_PRIORITY_MEDIUM);
  LOGD(TAG, "recv packet, cmd=%d, len=%d", packet->cmd, packet->payload_len);
}

/* 唤醒模式 */
static void _do_wakeup_mode() {
  AsrBusRecognizeReq request;
  request.mode = 0;
  if (0 == AsrBusRecognizeRequest(&request)) {
    LOGT(TAG, "switch to wakeup mode success");
  } else {
    LOGW(TAG, "switch to command mode failed, "
         "please use --conn to check connection or try --wakeup again");
  }
}

/* 识别模式 */
static void _do_cmd_mode() {
  AsrBusRecognizeReq request;
  request.mode = 1;
  if (0 == AsrBusRecognizeRequest(&request)) {
    LOGT(TAG, "switch to command mode success");
  } else {
    LOGW(TAG, "switch to command mode failed, "
         "please use --conn to check connection or try --command again");
  }
}

/* 开始接收降噪后音频数据 [16000 sample, 16bit, 1channel] */
static void _do_pull_raw_data() {
  AsrBusPullNoiseReductionDataReq request;
  request.mode = 1;
  if (0 == AsrBusPullNoiseReductionDataRequest(&request)) {
    char workspace[2048] = {0};
    getcwd(workspace, sizeof(workspace));
    LOGT(TAG, "start record success. audio save in file %s/asr.pcm", workspace);
  } else {
    LOGW(TAG, "start record failed, "
         "please use --conn to check connection or try --start again");
  }
}

/* 停止接收降噪后音频数据 */
static void _stop_raw_data_recv() {
  AsrBusPullNoiseReductionDataReq request;
  request.mode = 0;
  if (0 == AsrBusPullNoiseReductionDataRequest(&request)) {
    LOGT(TAG, "stop record success");
  } else {
    LOGW(TAG, "stop record failed, "
         "please use --conn to check connection or try --stop again");
  }
}

/* 挑战包，检测串口连接是否正常 */
static void _challenge() {
  AsrBusChallengePackReq request;
  request.sequence = 0;
  int ret = AsrBusChallengePackRequest(&request);
  if (0 == ret) {
    LOGT(TAG, "uart connect hummingbird board success");
  } else {
    LOGW(TAG, "uart connect hummingbird board failed");
  }
}

static void _usage(void) {
  const char * usage = ""
  "Usage: Linux uart protocol communication\n"
  "  --help      Get all support command line.\n"
  "  --conn      Try connect hummingbird, check uart connect status.\n"
  "  --wakeup    Set lasr to Wakeup mode.\n"
  "  --command   Set lasr to command mode.\n"
  "  --start     Start record asr data.\n"
  "  --stop      Stop record asr data.\n"
  "  --quit      Quit uart-cli.\n";
  printf("%s", usage);
}

static void _parse_command_line(char *cmd, int len) {
  cmd[len - 1] = '\0';
  LOGD(TAG, "cmd=%s", cmd);

  if (0 == strcmp(cmd, "--wakeup")) {
    _do_wakeup_mode();
    return;
  }

  if (0 == strcmp(cmd, "--command")) {
    _do_cmd_mode();
    return;
  }

  if (0 == strcmp(cmd, "--start")) {
    _do_pull_raw_data();
    return;
  }

  if (0 == strcmp(cmd, "--stop")) {
    _stop_raw_data_recv();
    return;
  }

  if (0 == strcmp(cmd, "--conn")) {
    _challenge();
    return;
  }

  if (0 == strcmp(cmd, "--help")) {
    _usage();
    return;
  }

  if (0 == strcmp(cmd, "--quit")) {
    exit(0);
  }

  _usage();
}

static void __event_list_event_handler(void *event) {
  CommPacket *packet = (CommPacket *)event;
  ChnlReceiveCommProtocolPacket(packet);
}

static void __event_list_event_free_handler(void *event) {
  free(event);
}

int main(int argc, char *argv[]) {
  LogLevelSet(N_LOG_TRACK);

  if (argc != 2) {
    LOGE(TAG, "usage: sudo ./uart-cli <Linux Uart Device Name>.\n"
         "example: sudo ./uart_cli /dev/ttyUSB0\n"
         "how to find Device: please read reademe.txt");
    return -1;
  }

  signal(SIGPIPE, SIG_IGN);

  UartConfig uart_config;
  snprintf(uart_config.device, sizeof(uart_config.device), "%s", argv[1]);
  uart_config.speed = B921600;
  if (0 != UartInitialize(&uart_config)) {
    return -1;
  }

  g_event_list = EventListCreate(__event_list_event_handler, __event_list_event_free_handler);
  if (NULL != g_event_list) {
    LOGE(TAG, "create eventlist failed");
    return -1;
  }

  CommProtocolInit(UartWrite, _comm_recv_packet_handler);

  ChnlInit();

  LOGT(TAG, "uart-cli start, you can input --help to get support command list");

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
  }

  return 0;
}

