### RF reader for M1 Card based on STM32 and MFRC500

**1. ** RF reader module Schematic and PCB design.

**2. ** STM32 code from drivers to apps follow hierarchical principle.
```cpp
├── main.c
├── Apps.c
|   └── App.h
├── ISO14443A.c
|   └── ISO14443A.h
├── MFRC500.c
|   └── MFRC500.h
├── ParallelPort.c
|   └── ParallelPort.h
├── USART.c
|   └── USART.h
├── PcComm.c
└── └── PcComm.h
```
**3. ** DLL API used SerialPort

**4. ** MFC Application using DLL APIs for module & software system testing demo. 

```cpp
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
```