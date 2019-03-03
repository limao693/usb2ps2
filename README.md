## 用途
公司或者主机设备没有usb接口，只能使用PS/2接口键盘。此程序完成USB HID keyboard协议向PS/2的scan code set 2转换。
## 需要的硬件设备
1. arduino uno R3
2. USB Host Shield
## 使用说明
1. 默认PS/2接口的的四条数据线中，数据段和时钟段连接在Arduino的（4，2）。
2. 使用IDE刷入usb2ps2.ino即可

## 参考文件
1. [USB HID to PS/2 Scan Code Translation Table](file:///Users/charles/Downloads/translate.pdf)
2. [PS2键盘接口说明](http://www.burtonsys.com/ps2_chapweske.htm)
3. [模拟PS/2键盘](https://www.arduino.cn/thread-77766-1-1.html)



## Application scenario
The company or host device does not have a usb interface and can only use the PS/2 interface keyboard. This program completes the conversion of the USB HID keyboard protocol to the PS/2 scan code set 2.
## Required hardware devices
1. Arduino uno R3
2. USB Host Shield
## Instructions for use
1. Among the four data lines of the default PS/2 interface, the data segment and the clock segment are connected to the Arduino (4, 2).
2. Use the IDE to brush usb2ps2.ino

## reference document
1. [USB HID to PS/2 Scan Code Translation Table] (file:///Users/charles/Downloads/translate.pdf)
2. [PS2 Keyboard Interface Description] (http://www.burtonsys.com/ps2_chapweske.htm)
3. [Analog PS/2 Keyboard] (https://www.arduino.cn/thread-77766-1-1.html)
