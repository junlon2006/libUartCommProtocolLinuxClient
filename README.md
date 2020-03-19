# Linux uart客户端，基于uart communication protocol的可靠通信业务
使用方法：  
step 1. 查找Linux当前用于通信的Uart设备号  
1.1、将uart连接到Linux
1.2、Linux Terminal 输入以下命令
```
dmesg | grep tty
```
1.3、查看Terminal输出
```
[331337.028662] cp210x ttyUSB0: usb_serial_generic_read_bulk_callback - urb stopped: -32
[331337.029838] cp210x ttyUSB0: usb_serial_generic_read_bulk_callback - urb stopped: -32
[331337.525789] cp210x ttyUSB0: failed set request 0x7 status: -19
[331337.525792] cp210x ttyUSB0: failed set request 0x12 status: -19
[331337.525794] cp210x ttyUSB0: failed set request 0x0 status: -19
[331337.525946] cp210x ttyUSB0: cp210x converter now disconnected from ttyUSB0
[331409.905706] usb 2-1: cp210x converter now attached to ttyUSB1
[331436.476345] cp210x ttyUSB1: cp210x converter now disconnected from ttyUSB1
[331444.196485] usb 2-1: cp210x converter now attached to ttyUSB1
```
1.4、通过最后一行找到串口名为：ttyUSB1，则UART设备全路径为/dev/ttyUSB1
1.5、./build.sh  (如无法执行，先执行chmod +x build.sh)
1.6、sudo ./uart-cli /dev/ttyUSB1

step 2. 命令行
```
Usage: Linux uart protocol communication
  --help      Get all support command line.
  --conn      Try connect hummingbird, check uart connect status.
  --wakeup    Set lasr to Wakeup mode.
  --command   Set lasr to command mode.
  --start     Start record asr data.
  --stop      Stop record asr data.
  --quit      Quit uart-cli.
```
即：
输入 --wakeup  切换到唤醒模式
输入 --command 切换到命令识别模式
输入 --start   切换到录音模式
输入 --stop    切换到停止录音模式
输入 --conn    检测当前串口连接状态是否完好
输入 --quit    退出uart-cli
输入 --help    显示所有支持的命令行

Test：
Linux client, ubuntu 16.04 test pass

enjoy it!
FAQ please send message to junlon2006@163.com
