### RF reader for M1 Card based on STM32 and MFRC500
**1. ** RF reader module Schematic and PCB design.

**2. ** STM32 code from drivers to apps follow hierarchical principle.
```cpp
.
├── .d
├── node_modules
├── public
├── scaffolds
├── source
|   ├── _posts
|   └── img
├── themes
├── _config.yml
├── db.json
└── package.json
```
3. DLL API used SerialPort
4. MFC Application using DLL APIs for module & software system testing demo. 


MFRC500_API
	应用程序DLL API，VC6.0 工程源代码

MFRC500_DEMO
	系统测试上位机软件，VC6.0 工程源代码

STM32_MFRC500
	下位机软件，KEIL5 工程源代码

RF_Reader.PcbDoc
	射频读卡模块PCB图

RF_Reader.pdf
	射频读卡模块原理图

基于MFRC500和STM32的RFID读写器设计与实现.pdf
	毕业论文
