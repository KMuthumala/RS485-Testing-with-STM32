

#ifndef __RS485_MASTER_CONFIG_H
#define __RS485_MASTER_CONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include "stdio.h"
#include "string.h"


 UART_HandleTypeDef huart2;
 /**
   * @brief Transmitter states
   *
   */
 typedef enum state{

 	IDLE,
 	RECEIVING_READY,
 	RECEIVING,
 	SENDING_RESPONSE,
  	START_TRANSMITTING,
  	WAITING_FOR_ACK,
 	WAITING_FOR_RESPONSE,
 	TRANSMISSION_ENDED,
 	RECEIVE_ENDED,
 	PROCESSING,
 	ENDED_PROCESS,
 	INVALID,
 	VALID,
	TRANSMISSION,
	ACK_RECEIVED,
	RESPONSE_RECEIVED,


  }stateType;
 /**
   * @brief sensor numbers
   *
   */
 typedef enum sensor{
 	TOF1 = 0b00000001,
 	TOF2 = 0b00000010,
 	TOF3 = 0b00000011,
 	SONAR1 = 0b10000001,
 	SONAR2 = 0b10000010,
 	SONAR3 = 0b10000011,
 	SONAR4 = 0b10000100,
 }sensorType;


 typedef enum{
 	ACK  = 0b00001111,
 	NACK = 0b11110000,
 	SYNC = 0b00111100,
 	M_TO_S= 0b00001111,
 	S_TO_M= 0b11110000,
 }keyWordType;

 void ready_Tx(void);
 void ready_Rx(void);
 void idle(void);
 void ackChecker(void);
 void stateMachine(sensorType sensor);
 uint16_t crcCheck(char message[], unsigned char length);
 void send(sensorType sensor);
void byteConcat(void);
void delay(int t);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
char Tx_buffer[12];
char Rx_buffer[12];
char msg[50];
stateType state;
uint8_t temp_buffer[1];
uint8_t receiveCounter;
stateType transmission_state;
uint32_t startTime;
uint32_t endTime;
uint16_t size;



#ifdef __cplusplus
}
#endif
#endif /* __RS485_MASTER_CONFIG_H */
