
#include "rs485_slave_config.h"



/**
  * @brief CRC calculations 16 bit
  * @retval crc val
  */
uint16_t crcCheck( char message[], unsigned char length){
	const unsigned char CRC7_POLY = 0x91;
	uint16_t i, j, crc = 0;
  for (i = 0; i < length; i++){
    crc ^= message[i];
    for (j = 0; j < 16; j++){
      if (crc & 1)
        crc ^= CRC7_POLY;
      crc >>= 1;
    }
  }
  return crc;
}

/**
  * @brief check ACK
  * @retval None
  */
void ackChecker(){
	if(Rx_buffer[8]==(uint8_t)ACK){
		state=WAITING_FOR_RESPONSE;
		// put the timeout for this section
	}else{
		state=START_TRANSMITTING;
	}
}


void delay(int t){
	startTime=HAL_GetTick();
	while(endTime-startTime<t){

	}
}


/**
  * @brief Setup receiving pin configuration
  * @retval None
  */
void ready_Rx(void){
	transmission_state = RECEIVING;
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,0);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,0);
}

/**
  * @brief Setup transmitting pin configuration
  * @retval None
  */
void ready_Tx(void){
	transmission_state = TRANSMISSION;
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,1);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,1);

}


/**
  * @brief No reception or transmit
  * @retval None
  */
void idle(void){
	//HAL_GPIO_WritePin(GPIOD,GPIO_PIN_10,GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,GPIO_PIN_SET);
}

/**
  * @brief reception complete callback
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	state=RECEIVE_ENDED;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,1);

	//receive();
}


/**
  * @brief transmission complete callback
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(state==WAITING_FOR_ACK){

	}
	state = TRANSMISSION_ENDED;
	state=ENDED_PROCESS;
//	sprintf(msg,"Tx--CpltCallback\r\n");
//	HAL_UART_Transmit(&huart2,(uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}



void send(sensorType sensor,keyWordType ack,uint8_t x){
	ready_Tx();

	Tx_buffer[0]=(uint8_t)SYNC;
	Tx_buffer[1]=(uint8_t)S_TO_M;
	Tx_buffer[2]=(uint8_t)sensor;
	Tx_buffer[3]=x;
	Tx_buffer[4]=x;
	Tx_buffer[5]=x;
	Tx_buffer[6]=x;
	Tx_buffer[7]=x;
	Tx_buffer[8]=(uint8_t)ack;

	uint16_t size=crcCheck(Tx_buffer,9);
	uint8_t first = size & 0xff00;//// need to re check
	uint8_t second = size & 0x00ff;
	Tx_buffer[9]=first;
	Tx_buffer[10]=second;
	Tx_buffer[11]=0x00;
	HAL_UART_Transmit_IT(&huart6,Tx_buffer,12);
	delay(100);
	//HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,1);
	//HAL_UART_Transmit(&huart6,Tx_buffer,12, HAL_MAX_DELAY);

//	if(state==){
//
//	}
}

//void receive(void){
	//HAL_UART_Receive(&huart6,Rx_buffer,12, HAL_MAX_DELAY);
//}

/**
  * @brief No reception or transmit
  * @retval None
  */
void stateMachine(sensorType sensor){
	address=1;
	receiveCounter=0;
    state = RECEIVING_READY;
    ready_Rx();
	while(1){
		switch(state){

			case RECEIVING_READY:
				ready_Rx();
				HAL_UART_Receive_IT(&huart6,Rx_buffer,12);
				//HAL_UART_Receive(&huart6,Rx_buffer,12, HAL_MAX_DELAY);
				// should put a timeout to this part later
				break;

			case RECEIVE_ENDED:
				if(Rx_buffer[1]==(uint8_t)M_TO_S){
					if(Rx_buffer[2]==(uint8_t)address){
						uint16_t size=(uint16_t)crcCheck(Rx_buffer,9)-(uint16_t)(Rx_buffer[9]<<8|Rx_buffer[10]);//////need to re check//////////////////
						 if(size==0){

							 send(sensor,ACK,0);
							 state=SENT_ACK;
							 delay(500);
						 }else{
							 send(sensor,NACK,0);
							 state=SENT_NACK;


						}
						 delay(100);
					}else{
						state = NOT_TO_ME ;
					}
				}else{
					state = NOT_TO_ME ;

				}
				break;

			case NOT_TO_ME:
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,0);
				state = RECEIVING_READY;
				receiveCounter=0;
				//__HAL_UART_FLUSH_DRREGISTER(&huart6);
				break;

			case SENT_ACK:
				state = SENDING_RESPONSE;
				delay(100);
				send(sensor,ACK,25);///send the data requested
				delay(100);
				receiveCounter=0;
				//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,0);
				break;

			case SENT_NACK:
				//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,0);
				state = RECEIVING_READY;
				break;

			case ENDED_PROCESS:
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,0);
				return;
				break;
		}
	}
}


