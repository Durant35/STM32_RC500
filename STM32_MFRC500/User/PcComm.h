#ifndef _PC_COMM_H_
#define _PC_COMM_H_

#define RX_BUFFER_SIZE 64
#define TRUE 1
#define FALSE 0

#define BYTE_START 0xBC
#define BYTE_END 0xFE

struct UsartData{
	unsigned char UartCmd;
	unsigned char UartDataLen;
	unsigned char UartData[32];
	signed char UartErrCode;
};

void Mfrc_Loop(void);
void usart_process(struct UsartData *pi);
void send_feedback(struct UsartData *pi);
void send_error(struct UsartData *pi);

#endif
