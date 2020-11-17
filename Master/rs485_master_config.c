
#include "rs485_master_config.h"

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
		ready_Rx();
		// put the timeout for this section
	}else{
		state=START_TRANSMITTING;
		ready_Tx();
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
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_10,0);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,0);
}

/**
  * @brief Setup transmitting pin configuration
  * @retval None
  */
void ready_Tx(void){
	transmission_state = TRANSMISSION;
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_10,1);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,1);

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
	//__HAL_UART_DISABLE_IT(huart6,);
	receiveCounter+=1;
	if(receiveCounter==1){
		state=ACK_RECEIVED;
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,1);// LED blue
	}

	if(receiveCounter==2){
		state=RESPONSE_RECEIVED;
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,1);// LED orange
		receiveCounter=0;
	}

}


/**
  * @brief transmission complete callback
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,1);
	state = TRANSMISSION_ENDED;
	ready_Rx();
}



void byteConcat(){

}

void send(sensorType sensor){
	ready_Tx();
	Tx_buffer[0]=(uint8_t)SYNC;
	Tx_buffer[1]=(uint8_t)M_TO_S;
	Tx_buffer[2]=(uint8_t)sensor;
	Tx_buffer[3]=0x00;
	Tx_buffer[4]=0x00;
	Tx_buffer[5]=0x00;
	Tx_buffer[6]=0x00;
	Tx_buffer[7]=0x00;
	Tx_buffer[8]=(uint8_t)ACK;

	uint16_t size=crcCheck(Tx_buffer,9);
	uint8_t first = size & 0xff00;//// need to re check
	uint8_t second = size & 0x00ff;
	Tx_buffer[9]=first;
	Tx_buffer[10]=second;
	Tx_buffer[11]=0x00;

	HAL_UART_Transmit_IT(&huart2,Tx_buffer,12);
	delay(100);

}



/**
  * @brief No reception or transmit
  * @retval None
  */
void stateMachine(sensorType sensor){
	//HAL_UART_Transmit_IT(&huart2,Tx_buffer,12);

    state = START_TRANSMITTING ;
	while(1){
		switch(state){

		case START_TRANSMITTING:
			send(sensor);
			// should put a timeout to this part later
			break;

		case ACK_RECEIVED:
			ackChecker();
			break;

		case TRANSMISSION_ENDED:
			state=WAITING_FOR_ACK;
			HAL_UART_Receive_IT(&huart2,Rx_buffer,12);
			break;

		case RESPONSE_RECEIVED:
			size=(uint16_t)crcCheck(Rx_buffer,9)-(uint16_t)(Rx_buffer[9]<<8|Rx_buffer[10]);
			if(size==0){
				state=ENDED_PROCESS;

			}else{
				state=START_TRANSMITTING;

			}
			ready_Tx();
			break;

		case ENDED_PROCESS:
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,1);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,1);
			return;
			break;
		}
	}
}


