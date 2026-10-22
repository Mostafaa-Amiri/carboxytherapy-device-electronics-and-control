/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "string.h" // FOR UART_DMA Function
#include "EEPROM.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C2_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define RxBuf_SIZE   14
#define MainBuf_SIZE 14  

uint8_t RxBuf[RxBuf_SIZE];
uint8_t MainBuf[MainBuf_SIZE];

uint16_t oldPos = 0;
uint16_t newPos = 0;

uint8_t var_write[8]  						= {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00}; 
uint8_t pressure_error_write[8]  	= {0x5A, 0xA5, 0x05, 0x82, 0x17, 0x00, 0x00, 0x01}; // 0x17 = needle - 0x18 = high pressure
uint8_t ready_stop_write[8]				= {0x5A, 0xA5, 0x05, 0x82, 0x16, 0x00, 0x00, 0x00}; // 1 : start / 0:stop
//uint8_t calibration_write[8]  	= {0x5A, 0xA5, 0x05, 0x82, 0x16, 0x50, 0x00, 0x01}; //2 : Succes / 0:reset
uint8_t bar_write[8]  						= {0x5A, 0xA5, 0x05, 0x82, 0x15, 0x00, 0x00, 0x00};
uint8_t cal_p_r_s_write[8]  			= {0x5A, 0xA5, 0x05, 0x82, 0x55, 0x00, 0x00, 0x00};
//uint8_t barcode_write[14]	 			= {0x5A, 0xA5, 0x0B, 0x82, 0x28, 0x00, 0x01, 0xB6, 0x9B, 0x4B, 0xAC, 0xD0, 0x5F, 0x15}; //BARCODE
uint8_t let_start[10]							= {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x01}; // when micro is stablished starts the LCD procces from the page 01 - [9] : pade ID - [7] : change page
uint8_t mute_write[10]  					= {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x80, 0x5A, 0x00, 0x00, 0x30};

int inject_1cc_check = 0;

int long_time_error_flag = 0;
int long_time_error_flag_counter = 0;
int timer_high_pressure_error = 0;

int x = 0; // for stop after ready : helper 1
int y = 1; // for stop after ready : helper 2 - must be 1 for first time to run
int rdy = 0; // for stop after ready : realising when is ready
int send_stop_one_time = 0;
int send_run_one_time = 0;
int ready_icon_one_time = 0;

int relay_ntc_one_time = 1;
int timer_relay_ntc = 0;

int one_one = 0; //cal_page
int ready_1cc_30 = 0;
int error_cancel = 0;
int one_send = 0;

int pressure = 0;
int time = 0;
int time_check = 0;

uint16_t cycle_w = 0;
int ready_stop;
int ready_stop_check = 0;
int error_check = 0;
int page_control_check = 0;

int pressure_user = 0;
int pressure_user_check = 0;
int volume = 0;
int send_volume = 0;
int loading = 0;

volatile int timer_check = 0; // counters in timer interrupt should declare as VOLATILE
int timer_200ms = 0;
uint8_t timer_min = 0;
uint16_t timer_hour = 0;
volatile int timer_p_t_check = 0 ; // counters in timer interrupt should declare as VOLATILE
volatile int timer_calibration = 0;  // counters in timer interrupt should declare as VOLATILE
volatile int timer_injection = 0;  // counters in timer interrupt should declare as VOLATILE
int beep = 0;
int beep_check = 1;
int beep_timer = 0;
int long_time_error = 0;
volatile int timer_long_time_error = 0; // counters in timer interrupt should declare as VOLATILE

float pr1 = 0;
float pr2 = 0;
int check = 0;
int check2 = 0;

uint8_t  eeprom[4] = {0, 0, 0, 0};

int calibration_page_ready_stop = 0;
int calibration_page_control = 0;
int calibration_set = 0;
int ready_stop_calibration = 0;
int pressure_calibration = 0;
int zarib_t_c = 1;	
int mode = 0;
int mode_a = 0;
int one_time = 0;

int buzzer = 0;
int buzzer_last_state = 0;

int timer_error_buzzer = 0; 
int error_buzzer_flag = 0;
int high_pressure_error_flag = 0;
int nail_error_flag = 0;					
int timer_nail_error = 0;

volatile int timer_heater = 0; // counters in timer interrupt should declare as VOLATILE
int inject_1cc = 0;
volatile int timer_1cc = 0; // counters in timer interrupt should declare as VOLATILE
volatile int check_1cc = 0; // counters in timer interrupt should declare as VOLATILE

int pedal = 0;
int pedal_check = 0; // to send only one UART cmd at a time
int pedal_sequence = 0; // to avoid conflict between pedal and touch start when user uses both of them
int page_control = 0; // if user exits the control page , process stops
int ready_stop_cp = 0; // ready_stop for motor_pwm in calibration_page

int pressure_selection[47] = {0, 2, 2, 2, 2 , 2, 2, 2, 3 , 3, 2,  3,  2,  2,  3, 2, 2, 2, 2,  3,  3,  3,  3, 2, 2, 2, 2, 2 , 2, 2, 2, 3 , 3, 2,  3,  2,  2,  3, 2, 2, 2, 2,  3,  3,  3,  3, 2};
int volume_selection[47]   = {0, 1, 2, 4, 8 , 4, 2, 1, 6 , 2, 1,  6,  6, 10,  6, 2, 2, 4, 4, 10,  4,  4,  4, 4, 1, 2, 4, 8 , 4, 2, 1, 6 , 2, 1,  6,  6, 10,  6, 2, 2, 4, 4, 10,  4,  4,  4, 4};
int time_selection[47]     = {0, 2, 4, 8, 16, 8, 4, 2, 18, 6, 2, 18, 12, 20, 18, 4, 4, 8, 8, 30, 12, 12, 12, 8, 2, 4, 8, 16, 8, 4, 2, 18, 6, 2, 18, 12, 20, 18, 4, 4, 8, 8, 30, 12, 12, 12, 8};
	
	//times is in senconds : p * v 	 = s (if s < 2 ? s = 2) 
	
int motor_pwm (int pressure_1)
{
		
		int calibration_set_1;
	
		if   ((ready_stop == 1) || (pedal + x + y >= 2)) // for stoping after beeing ready
		{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //VALVE on
				
				if (error_check == 1)
				{										
 					  pressure_error_write[4] = 0x18; //high pressure error 
						pressure_error_write[7] = 0x00; 		//icon animation blinking stops
						HAL_Delay(30);	
						HAL_UART_Transmit_DMA(&huart1, pressure_error_write, 8);
					
						pressure_error_write[4] = 0x17; //needle pressure error 
						pressure_error_write[7] = 0x00; 		//icon animation blinking stops
						HAL_Delay(30);	
						HAL_UART_Transmit_DMA(&huart1, pressure_error_write, 8);
					
						error_check = 0;
				}
								
				if ( pressure_1 > pressure_user)
				{					
						if ((pressure_1 - pressure_user) > 50) // WHILE ( (pressure - (float)pressure_user) >= 50.0)
						{	
						TIM3->PSC = 1125-1;
						TIM3->ARR  = 8000 -1;	// f = 8 hz
						TIM3->CCR1 = 8000;		// dt = 100%
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
						calibration_set_1 = 0;
						return calibration_set_1;
						}
				
						if (((pressure_1 - pressure_user) <= 50) && ((pressure_1 - pressure_user) > 40) )
						{
						TIM3->PSC = 1125-1;
						TIM3->ARR  = 8000 -1; // f = 8 hz
						TIM3->CCR1 = 6000;		// dt = 75%
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);		
						calibration_set_1 = 0;
						return calibration_set_1;
						}
						
						if (((pressure_1 - pressure_user) <= 40) && ((pressure_1 - pressure_user) > 30) )
						{
						TIM3->PSC = 1125-1;
						TIM3->ARR  = 16000 -1; // f = 4 hz
						TIM3->CCR1 = 12000;		 // dt = 75 %
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);	
						calibration_set_1 = 0;
						return calibration_set_1;
						}
						
						if (((pressure_1 - pressure_user) <= 30) && ((pressure_1 - pressure_user) > 20) )
						{
						TIM3->PSC = 1125-1;
						TIM3->ARR  = 64000 -1; // f = 1 hz
						TIM3->CCR1 = 36000;		 // dt = 56 %
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);	
						calibration_set_1 = 0;
						return calibration_set_1;
						}
						
						if (((pressure_1 - pressure_user) <= 20) && ((pressure_1 - pressure_user) > 10) )
						{
						TIM3->PSC = 1125-1;
						TIM3->ARR  = 64000 -1; // f = 1 hz
						TIM3->CCR1 = 24000;		 // dt = 36 %
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);		
						calibration_set_1 = 0;
						return calibration_set_1;
						}
						
						if (((pressure_1 - pressure_user) <= 10) && ((pressure_1 - pressure_user) > 2) )
						{
						TIM3->PSC = 2250-1;
						TIM3->ARR  = 64000 -1; // f = 0.25 hz
						TIM3->CCR1 = 6400;		 // dt = 40 %
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);	
						calibration_set_1 = 0;
						return calibration_set_1;
						}
						
						if ( (pressure_1 - pressure_user) <= 2)
						{								
								TIM3->CCR1 = 0;
								calibration_set_1 = 1;
								return calibration_set_1;
						}
					}
////////////////////////////////////////////////////////
				else
				{						
						if ( (pressure_user - pressure_1) > 25)
						{
						TIM3->PSC = 1125-1;
						TIM3->ARR  = 16000 -1; // f = 4hz
						TIM3->CCR1 = 12000;		 // dt = 75 %
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);	
						calibration_set_1 = 0;
						return calibration_set_1;
						}

						if ( ((pressure_user - pressure_1) <= 25) && ((pressure_user - pressure_1) > 10) )
						{
								TIM3->PSC = 1125-1;
								TIM3->ARR  = 32000 -1; // f = 2hz
								TIM3->CCR1 = 20000;		 // dt = 62.5 %
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);	
								calibration_set_1 = 0;
								return calibration_set_1;
						}
						
						if ( ((pressure_user - pressure_1) <= 10) && ((pressure_user - pressure_1) > 5) )
						{
								TIM3->PSC = 1125-1;
								TIM3->ARR  = 64000 -1; // f = 1 hz
								TIM3->CCR1 = 20000;		 // dt = 32 %
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);	
								calibration_set_1 = 0;
								return calibration_set_1;
						}
						
						if ( ((pressure_user - pressure_1) <= 5) && ((pressure_user - pressure_1) > 2) )
						{
								TIM3->PSC = 1125-1;
								TIM3->ARR  = 64000 -1; // f = 1 hz
								TIM3->CCR1 = 6400;		 // dt = 10 %
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);	
								calibration_set_1 = 0;
								return calibration_set_1;
						}
						
						if ( (pressure_user - pressure_1) <= 2)
						{
								
								TIM3->CCR1 = 0;
								calibration_set_1 = 1;
								return calibration_set_1;
								
						}						
		  	}
			}
			return calibration_set_1;
}
	
	
int ads1115_pressure_adc()
{
				int pressure0 = 0;
				int pressure1 = 0;
				unsigned char ADSwrite[3];
				uint16_t v = 0;
				float voltage[2] = {0, 0};
				
				ADSwrite[0] = 0x01; //0000,0001: config register
				ADSwrite[1] = 0xC0; //1100,0000: 1= begin conv/  100= A0 TO GND : 110=A2\111=A3/  000= 0 TO 6.144/ 0= CONTINOUS MODE
				ADSwrite[2] = 0xE3; //1110,0011: 111= 860SPS SAMPLE PER SECOND/ 0= DEFAULT(COMPRATOR HYSTERESIS)/ 0= DEFAULT(ALLERT RDY PIN ACTIVE LOW)/ 0= DEFAULT(NO LATCHING COMPRATOR)/ 11= DEFAULT(DISABLE COMPRATOR)
				HAL_I2C_Master_Transmit(&hi2c1, 0x48<<1, ADSwrite, 3, 60);
				ADSwrite[0] = 0x00; //0000,0000: conversion register
				HAL_I2C_Master_Transmit(&hi2c1, 0x48<<1, ADSwrite, 1 ,60);
				HAL_Delay(45);	// data sheet says		
				HAL_I2C_Master_Receive(&hi2c1, 0x48<<1, ADSwrite, 2, 60);
				v = (ADSwrite[0] << 8 | ADSwrite[1] );
				voltage[0] = v * 6.144 / 32768.0;

			 	pressure0 = (voltage[0] - 0.218) / 0.045;
				if (pressure0 < 0)
						pressure0 = 0;
				
				//note : i2c acts as blocking mode , so unlike our UART which need a bit of time , i2c does not want a break time.
				// begin procces again to get the data from A1 / if the A0 pin of ADC burnt , we change to use the A0 , In this way we increase the lifetime of our system
				
				ADSwrite[0] = 0x01; //0000,0001: config register
				ADSwrite[1] = 0xD0; //1101,0000: 1= begin conv/  101= A1 TO GND : 110=A2\111=A3/  000= 0 TO 6.144/ 0= CONTINOUS MODE
				ADSwrite[2] = 0xE3; //1110,0011: 111= 860SPS SAMPLE PER SECOND/ 0= DEFAULT(COMPRATOR HYSTERESIS)/ 0= DEFAULT(ALLERT RDY PIN ACTIVE LOW)/ 0= DEFAULT(NO LATCHING COMPRATOR)/ 11= DEFAULT(DISABLE COMPRATOR)
				HAL_I2C_Master_Transmit(&hi2c1, 0x48<<1, ADSwrite, 3, 60);
				ADSwrite[0] = 0x00; //0000,0000: conversion register
				HAL_I2C_Master_Transmit(&hi2c1, 0x48<<1, ADSwrite, 1 ,60);
				HAL_Delay(45);			
				HAL_I2C_Master_Receive(&hi2c1, 0x48<<1, ADSwrite, 2, 60);
				v = (ADSwrite[0] << 8 | ADSwrite[1] );
				voltage[0] = v * 6.144 / 32768.0;

			 	pressure1 = (voltage[0] - 0.218) / 0.045;
				if (pressure1 < 0)
						pressure1 = 0;
				
				// the best way of indicating the not broken pin is to select the pin that has the higher value.
				if (pressure0 > pressure1)
				{
						return pressure0;
				}
				else
				{
						return pressure1;
				}
}
	
	
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart->Instance == USART1)
	{
		
			//MainBuf[0] = '\0';
			memcpy (MainBuf, RxBuf, Size);//memcpy (MainBuf, RxBuf, MainBuf_SIZE); //
			
		
		
//		oldPos = newPos;  // Update the last position before copying new data

//		/* If the data in large and it is about to exceed the buffer size, we have to route it to the start of the buffer
//		 * This is to maintain the circular buffer
//		 * The old data in the main buffer will be overlapped
//		 */
//		if (oldPos + Size > MainBuf_SIZE)  // If the current position + new data size is greater than the main buffer
//		{
//			uint16_t datatocopy = MainBuf_SIZE - oldPos;  // find out how much space is left in the main buffer
//			memcpy ((uint8_t *)MainBuf + oldPos, RxBuf, datatocopy);  // copy data in that remaining space

//			oldPos = 0;  // point to the start of the buffer
//			memcpy ((uint8_t *)MainBuf, (uint8_t *)RxBuf + datatocopy, (Size - datatocopy));  // copy the remaining data
//			newPos = (Size - datatocopy);  // update the position
//		}

//		/* if the current position + new data size is less than the main buffer
//		 * we will simply copy the data into the buffer and update the position
//		 */
//		else
//		{
//			memcpy ((uint8_t *)MainBuf + oldPos, RxBuf, Size);
//			newPos = Size + oldPos;
//		}


		/* start the DMA again */
		//RxBuf[0] = '\0';
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *) RxBuf, RxBuf_SIZE);
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

	}


	/****************** PROCESS (Little : NOT a TIME-CONSUMING PROCESS) THE DATA HERE *********************/

	/* Let's say we want to check for the keyword "OK" within our incoming DATA */
	//for (int i=0; i<Size; i++)
	if ((MainBuf[0] == 0x5A) && (MainBuf[3] == 0x83))
	{
				switch(MainBuf[4])
				{					
						case 0x10:
								mode = MainBuf[8];
								loading = 1;
								page_control = 1;
								break;
							
						case 0x13:
								pressure_user = (MainBuf[8]) * 10;
								loading = 1;
								ready_icon_one_time = 1;
								break;
						
						case 0x14:
								volume = MainBuf[8];
								send_volume = 1;
								break;
						
						case 0x57:
								ready_stop = MainBuf[8];	//ready_stop = 1 : ready
								ready_stop_check = 1;
								loading = 1;
								error_cancel = 1;
								send_stop_one_time = 1;
								send_run_one_time = 1;
								break;
						
						case 0x19:
								inject_1cc = MainBuf[8]; // 1 = YES
								pressure_user = 30;
								one_send = 1;
								break; // :) tnx God
												
						case 0x29:
								buzzer = MainBuf[8];
								break;
						
						/*case 0x18:
								heater = MainBuf[8]; // heater = 0 > heater on
								break;*/
						
						case 0x44:                        // page control ready stop
								page_control = 0 ;
								page_control_check = 1;
								break;
						
						case 0x55:
								calibration_page_ready_stop = MainBuf[8]; // calibration = 1 : start //calibration = 0 : stop
								zarib_t_c = 1;
								break;
						
						case 0x59:
								calibration_page_control =  MainBuf[8];
								one_time = 1;
								break;
						
						default:
								break;
				}
	}



}

void error_buzzer ()
{
		error_buzzer_flag = 1;
	
		switch (timer_error_buzzer) 
			{
        case 0:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);	//BUZZER_on
            break;
				case 1:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);	//BUZZER_on
            break;
        case 2:
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);	//BUZZER_off
            break;
				case 3:
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);	//BUZZER_off
            break;
        case 4:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);	//BUZZER_on
            break;
				case 5:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);	//BUZZER_on
            break;
        case 6:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);	//BUZZER_off
            break;
				case 7:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);	//BUZZER_off
            break;
        case 8:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);	//BUZZER_on
            break;
				case 9:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);	//BUZZER_on
            break;
        case 10:
				{
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);	//BUZZER_off
						timer_error_buzzer = 0;
            break;
				}
        default:
				{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);	//BUZZER_off
						timer_error_buzzer = 0;
            break;
				}
    }
		
				
		
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxBuf, RxBuf_SIZE);
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
	
	HAL_Delay(2000); // DELAY FOR 1 SECOND FOR WAITING TO LCD TO BOOTUP : NECESSARY!!!!	
			
	EEPROM_Read(0, 0, eeprom, 4); // this library has PROBLEM WITH EEPROM PAGES and it write at eeprom at same place. use it so simply.
	
	if ((eeprom[0] == 0xFF) && (eeprom[1] == 0xFF))
	{
			eeprom[0] = 0;
			eeprom[1] = 0;
	}
	
	if (eeprom[2] == 0xFF)
			eeprom[2] = 0;
	
	if (eeprom[3] == 0xFF)
			eeprom[3] = 0;
	
	HAL_Delay(30);
	cycle_w = ((uint16_t)eeprom[0] << 8) | eeprom[1];
	cycle_w++;

	eeprom[0] = (uint8_t)(cycle_w >> 8);  // High byte
	eeprom[1] = (uint8_t)cycle_w;  // Low byte
	
	var_write[4] = 0x40;
	var_write[7] = eeprom[1];
	var_write[6] = eeprom[0]; // in DGUS DWIN select var type = unsinged int (2byte)
	HAL_Delay(50);
	HAL_UART_Transmit_DMA(&huart1, var_write, 8);
	HAL_Delay(50);
	
	timer_min = eeprom[2];
	var_write[4] = 0x41;
	var_write[6] = 0;
	var_write[7] = timer_min;
	HAL_Delay(50);
	HAL_UART_Transmit_DMA (&huart1, var_write, 8);
	HAL_Delay(50);
	
	timer_hour = eeprom[3];
	var_write[4] = 0x42;
	var_write[7] = timer_hour;
	var_write[6] = 0; // in DGUS DWIN select var type = unsinged int (2byte)
	HAL_Delay(50);
	HAL_UART_Transmit_DMA (&huart1, var_write, 8);	
	var_write[6] = 0;
	
	EEPROM_Write(0,	0, eeprom, 4);
	
	//HAL_UART_Transmit_DMA(&huart1, barcode_write, 14);		
	
	pressure_user_check = pressure_user;
	//volume_check = volume;
	time_check = time;
	
	mode_a = mode; // both of them is zero . just to emphasize
	inject_1cc_check = inject_1cc;
	
	HAL_Delay(100); // DELAY FOR 1 SECOND FOR WAITING TO LCD TO BOOTUP : NECESSARY!!!!
	HAL_UART_Transmit_DMA(&huart1, let_start, 10); // start the LCD by permission of micro
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);		// RELAY NTC BYPASS OFF
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
///////////////////////// RELAY NTC BYPASS////////////////////////////////////////////////////////////////////////////////////
				if ((timer_relay_ntc >= 9) && (relay_ntc_one_time == 1))// 1.8 seconds
				{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);	// RELAY ON	
						relay_ntc_one_time = 0;
				}
				
				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
///////////////////////// RELAY NTC BYPASS////////////////////////////////////////////////////////////////////////////////////
			
			if ( (inject_1cc == 1) && ( ( pedal + ready_stop >= 1) || (check_1cc == 1) ) ) // for stabling the pedal for 1cc
				{ 
						 check_1cc = 1;
				}
				
///////////////////////// Begin Stories between Pedal and Touch button + Recieve pressure + motor control ///////////////////// 
				var_write[6] = 0;
				pressure = ads1115_pressure_adc();
								
				ready_stop = ready_stop * page_control; 
				// product of two dependant varibale. it is impossible : ready_stop = 1 , page_control = 0. you can not change ready_stop out of control_page

				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
				if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == GPIO_PIN_SET)
				{
						pedal = 1;
						send_run_one_time = 1;
				}
				else
						pedal = 0;
			
				//pedal = pedal * page_control ; >> EXPLAIN IN THE NEXT LINE
				// but here these are independant. you can make pedal = 1 , page control = 0 , if you use this code , when exit the page_control you will
				// be back to page_control again . so instead of product * we use if inside the if
				if (page_control == 1)
				{
					if (pedal == 1) 
					{
						
						// for avoiding the conflict when both pedal & touch start-stop are actived so when user deactivate the pedal , but touch start
						// was on , system stops = with the help of pedal_sequence
						//or when pedal is activated but user wants to stop from touch : in this method user can not stop when pedal is activated.
						// because when pedal is pressed . we transfer to another page which stop-start button touch is deactived so can not change.
						// this method is from DWIN furoum. also there are other ways for deactivating the touch button ex : changing adjust method 
						// ++ or -- to -- and ++ in incremental adjustment with UART cmd 0xB0 or fully deactivating the touch of incremental
					  // adjustment with UART cmd 0xB0 --- but working with 0xB0 is a bit complicated. this method requires much of background pics.
									
						if (pedal_check != pedal) // just send one time UART command
						{
							if ( (mode >= 1) && (mode <= 5) ) // face english
							{
									HAL_Delay(30);
									let_start[9] = 11; //or hex = 0x0B
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
							}
						
							if ( (mode >= 6) && (mode <= 22) ) // body english
							{
									HAL_Delay(30);
									let_start[9] = 13; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
							}
						
							if ( mode == 23 ) 								// vaginal english
							{
									HAL_Delay(30);
									let_start[9] = 15; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
							}
						
							if ( (mode >= 24) && (mode <= 28) ) // face farsi
							{
									HAL_Delay(30);
									let_start[9] = 17; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
							}
						
							if ( (mode >= 29) && (mode <= 45) )	// body farsi
							{
									HAL_Delay(30);
									let_start[9] = 19; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
							}
						
							if ( mode == 46 )										// vaginal farsi
							{
									HAL_Delay(30);
									let_start[9] = 21; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
							}
						
							pedal_sequence = 1;
							ready_stop_write[7] = 1;
							HAL_Delay(30);
							HAL_UART_Transmit_DMA(&huart1, ready_stop_write, 8);
							
							pedal_check = pedal; // just send one time UART command
							//ready_stop_check = 1; // it is for when we have separate calibration status and ready_stop and when stop , it turns off please_wait
						}
					}	
				
					else
					{
						if (pedal_sequence == 1)
						{
								if ( (mode >= 1) && (mode <= 5) ) 			// face eng
								{
										HAL_Delay(30);
										let_start[9] = 10; 
										HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
								if ( (mode >= 6) && (mode <= 22) )		//body eng
								{
										HAL_Delay(30);
										let_start[9] = 12; 
										HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
								if ( mode == 23 )											// vag eng
								{
										HAL_Delay(30);
										let_start[9] = 14; 
										HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
								if ( (mode >= 24) && (mode <= 28) )		// face far
								{
										HAL_Delay(30);
										let_start[9] = 16; 
										HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
								if ( (mode >= 29) && (mode <= 45) )		// body far
								{
										HAL_Delay(30);
										let_start[9] = 18; 
										HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
								if ( mode == 46 )										// vag far
								{
										HAL_Delay(30);
										let_start[9] = 20; 
										HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
							
								pedal_sequence = 0;
								pedal = 0;
								ready_stop = 0;
								if (inject_1cc == 0)
								{
								if (beep == 1)
								ready_stop_write[7] = 3;
								else
								ready_stop_write[7] = 0;	
								
								HAL_Delay(30);
								HAL_UART_Transmit_DMA(&huart1, ready_stop_write, 8);
								}
						}
						
						pedal_check = pedal;
					}
					
					if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
				}
				
				else // : page_control = 0
				{	
					pedal = 0;
					pedal_check = pedal;
					
					if (page_control_check == 1) // just one time
					{
							ready_stop_write[7] = 0;
							HAL_Delay(30);
							HAL_UART_Transmit_DMA(&huart1, ready_stop_write, 8);
							
							page_control_check = 0;
						
							HAL_Delay(30);
							var_write[4] = 0x19;
							var_write[7] = 0;
							HAL_UART_Transmit_DMA(&huart1, var_write, 8);
							inject_1cc = 0;
					}
				}

				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
///////////////////////////////////////// End Stories between Pedal and Touch button + Recieve pressure + motor control /////////////////////  		

////////////////////////////////////////////// Begin 1cc /////////////////////////////////////////////////////////
				
				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				

				if ((inject_1cc == 1) && (one_send == 1))
				{
						HAL_Delay(30);						
						var_write[4] = 0x58;
						var_write[7] = 1;
						HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						HAL_Delay(30);						
						var_write[4] = 0x14;
						var_write[7] = 0;
						HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						HAL_Delay(30);						
						var_write[4] = 0x12;
						var_write[7] = 1;
						HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						HAL_Delay(30);	
					
						bar_write[7] = 1;
						HAL_UART_Transmit_DMA (&huart1, bar_write, 8);
						time = 10; // 10s
					
						pressure_user_check	= 0;			
						one_send = 0;
				}
				
				if (timer_1cc >= 5)
				{
							pedal = 0;
							ready_stop = 0;
							check_1cc = 0;
					
							HAL_Delay(30);						
							var_write[4] = 0x57;
							var_write[7] = 0;
							HAL_UART_Transmit_DMA(&huart1, var_write, 8);
					
							if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == GPIO_PIN_RESET)
							{
									timer_1cc = 0;								
							}
							
							HAL_Delay(30);
							ready_stop_write[7] = 3;
							HAL_UART_Transmit_DMA(&huart1, ready_stop_write, 8);
				}
				
				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
////////////////////////////////////////////// End 1cc   /////////////////////////////////////////////////////////
			
///////////////////////////////////////////// Begin inject 1cc change status /////////////////////////////////////////
				if (inject_1cc != inject_1cc_check)
				{
						if (inject_1cc == 0)
						{
									//pr recommendation
									HAL_Delay(30);						///if not works maybe because delays > change to 60
									var_write[4] = 0x13;
									var_write[7] = pressure_selection[mode];
									HAL_UART_Transmit_DMA (&huart1, var_write, 8);
									//vul recommend
					  			HAL_Delay(30);
									var_write[4] = 0x14;
									var_write[7] = volume_selection[mode];
									HAL_UART_Transmit_DMA (&huart1, var_write, 8);
									// vul number
									HAL_Delay(30);
									var_write[4] = 0x58;
									var_write[7] = volume_selection[mode]*5;
									HAL_UART_Transmit_DMA (&huart1, var_write, 8);
									//time recommend
					  			HAL_Delay(30);
									var_write[4] = 0x12;
									var_write[7] = time_selection[mode];
									HAL_UART_Transmit_DMA (&huart1, var_write, 8);// bar becomes 0 zero
									HAL_Delay(30);
									bar_write[7] = 1;
									HAL_UART_Transmit_DMA (&huart1, bar_write, 8);
						}
						inject_1cc_check = inject_1cc;
				}
///////////////////////////////////////////// Begin inject 1cc change status /////////////////////////////////////////

//////////////////////////////////////////////// Begin Long Time Calibrating Error ////////////////////////////////////


						if (timer_long_time_error >= 75) // after 15 seconds
						{
								HAL_Delay(30);								
								ready_stop_write[7] = 4;																// "LONG TIME ERROR"
								HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);	
								long_time_error = 0;																		 
								timer_long_time_error = 0;	
								ready_stop = 0;
								pedal = 0;
							
								long_time_error_flag = 1;
							
								if ( (mode >= 1) && (mode <= 5) ) // face english
								{
									HAL_Delay(30);
									let_start[9] = 11; //or hex = 0x0B
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 6) && (mode <= 22) ) // body english
								{
									HAL_Delay(30);
									let_start[9] = 13; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( mode == 23 ) 								// vaginal english
								{
									HAL_Delay(30);
									let_start[9] = 15; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 24) && (mode <= 28) ) // face farsi
								{
									HAL_Delay(30);
									let_start[9] = 17; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 29) && (mode <= 45) )	// body farsi
								{
									HAL_Delay(30);
									let_start[9] = 19; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( mode == 46 )										// vaginal farsi
								{
									HAL_Delay(30);
									let_start[9] = 21; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
								
								while (long_time_error_flag) 
								{
										error_buzzer();
										if (long_time_error_flag_counter > 13)
										{
											
												error_buzzer_flag = 0;
												long_time_error_flag = 0;
												long_time_error_flag_counter = 0;
												if (mode >= 24)
												{
														HAL_Delay(30);
														let_start[9] = 28; 
														HAL_UART_Transmit_DMA(&huart1, let_start, 10);
														calibration_page_control = 1;
												}
												else
												{
														HAL_Delay(30);
														let_start[9] = 29; 
														HAL_UART_Transmit_DMA(&huart1, let_start, 10);
														calibration_page_control = 1;
												}
										}
								}
						}
		
//////////////////////////////////////////////// End Long Time Calibrating Error ////////////////////////////////////
				
///////////////////////////////////////// Begin Pressure Higher than 70 WARNING ALL STOP //////////////////////////////////////////////////////
				
				if (pressure >= 75)
				{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //VALVE off
						ready_stop = 0;
						pedal = 0;  // useless if user pressing the pedal
						ready_stop_write[7] = 0;
						HAL_Delay(30);
						HAL_UART_Transmit_DMA(&huart1, ready_stop_write, 8);
						
						pressure_error_write[4] = 0x18; //high pressure error 
						pressure_error_write[7] = 1; 		//icon animation blinking
						HAL_Delay(30);	
						HAL_UART_Transmit_DMA(&huart1, pressure_error_write, 8);	
						error_check = 1; // in motor_pwm function it will be zero and stop the showing error when again starts
						high_pressure_error_flag = 1;
				}
				
				 if ( error_cancel == 1)
				 {
						pressure_error_write[4] = 0x18; //high pressure error 
						pressure_error_write[7] = 0; 		//icon animation blinking
						HAL_Delay(30);	
						HAL_UART_Transmit_DMA(&huart1, pressure_error_write, 8);
						error_cancel = 0;
				 }
				 
				if ((page_control == 1) && (ready_stop_write[7] == 4))
				{
						pedal = 0;
				}
				
				if (high_pressure_error_flag)
				{
								HAL_Delay(30);								
								ready_stop_write[7] = 4;																// "LONG TIME ERROR"
								HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);	
							
								if ( (mode >= 1) && (mode <= 5) ) // face english
								{
									HAL_Delay(30);
									let_start[9] = 11; //or hex = 0x0B
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 6) && (mode <= 22) ) // body english
								{
									HAL_Delay(30);
									let_start[9] = 13; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( mode == 23 ) 								// vaginal english
								{
									HAL_Delay(30);
									let_start[9] = 15; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 24) && (mode <= 28) ) // face farsi
								{
									HAL_Delay(30);
									let_start[9] = 17; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 29) && (mode <= 45) )	// body farsi
								{
									HAL_Delay(30);
									let_start[9] = 19; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( mode == 46 )										// vaginal farsi
								{
									HAL_Delay(30);
									let_start[9] = 21; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
								
								while (high_pressure_error_flag)
								{
										error_buzzer();
										if (timer_high_pressure_error > 13)
										{
											
												error_buzzer_flag = 0;
												high_pressure_error_flag = 0;
												timer_high_pressure_error = 0;
												if (mode >= 24)
												{
														HAL_Delay(30);
														let_start[9] = 28; 
														HAL_UART_Transmit_DMA(&huart1, let_start, 10);
														calibration_page_control = 1; // this command must be after UART because UART is not blocking and if make this variable befor that it goes to while("")
												}
												else
												{
														HAL_Delay(30);
														let_start[9] = 29; 
														HAL_UART_Transmit_DMA(&huart1, let_start, 10);
														calibration_page_control = 1;
												}
										}
								}
				}
///////////////////////////////////////// End Pressure Higher than 70 WARNING ALL STOP //////////////////////////////////////////////////////
				
///////////////////////////////////////// Begin Mode Changig write UART recom & specs ///////////////////// 
		
				if (mode_a != mode)
				{
						pressure_user = (pressure_selection[mode]) * 10;
						volume = volume_selection[mode];
						time = time_selection[mode]; // OR : pressure_user * volume / 2; if (time < 2) time = 2;
						time_check = time;
					
						//pr recommendation
						HAL_Delay(30);						///if not works maybe because delays > change to 60
						var_write[4] = 0x13;
						var_write[7] = pressure_selection[mode];
						HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						//vul recommend
					  HAL_Delay(30);
						var_write[4] = 0x14;
						var_write[7] = volume_selection[mode];
						HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						// vul number
						HAL_Delay(30);
						var_write[4] = 0x58;
						var_write[7] = volume_selection[mode]*5;
						HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						//time recommend
					  HAL_Delay(30);
						var_write[4] = 0x12;
						var_write[7] = time_selection[mode];
						HAL_UART_Transmit_DMA (&huart1, var_write, 8);

						//info
						HAL_Delay(30);
						var_write[4] = 0x30;
					  var_write[7] = mode;
						HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						
						// bar becomes 0 zero
						HAL_Delay(30);
						bar_write[7] = 1;
						HAL_UART_Transmit_DMA (&huart1, bar_write, 8);
						
						mode_a = mode;
						
				}

///////////////////////////////////////// End Mode Changig write UART recom & specs ///////////////////// 	
 
////////////////////// Begin loading pressure status : successfor or calibrating + start beep sound for injection ///////////////////// 	
				
				if (pressure_user_check != pressure_user) // if user changes the pressure
				{
					timer_injection = 0; // the main timer : time + buzzer + bar
					if(ready_icon_one_time == 1)
					{
							HAL_Delay(30);
							ready_stop_write[7] = 0;																// "ready"
							HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);
							ready_icon_one_time = 0;
					}
					if  ((ready_stop + pedal) >= 1) 
					{
						
						if ((calibration_set == 0) && (loading == 1)) 							// loadign = 1 : whether user changes the pressure or motor is proccessing
						{		
								HAL_Delay(30);								
								ready_stop_write[7] = 2;																// "please wait"
								HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);	
							
								loading = 0; 																						// just one time
								beep = 0; 																							// beep for general enabling the buzzer
								beep_timer = 0; 																				// beep_timer is for controlling when buzzer must be ringing
								HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 	//BUZZER
								long_time_error = 1;																		// check how much time spent for calibrating - if too long == error too long
							
								rdy = 0; // these 3 variables are for stop after beeing ready / for beter undestanding just make table and run time proccess
								x = 0;
								y = 1;								
						}
						
						if ((calibration_set == 1) && (loading == 0))								// after loading & motor set and ready to go
						{
								
								if (inject_1cc == 0)
								{
										HAL_Delay(30);
										ready_stop_write[7] = 3;																// "ready"
										HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);
								}
								
								bar_write[7] = 1; // this =1 is for a technical reason.
								HAL_Delay(30);
								HAL_UART_Transmit_DMA (&huart1, bar_write, 8);
							
								pressure_user_check = pressure_user;										// just one time					
								beep = 1;																								// buzzer start
								long_time_error = 0;																		// check how much time spent for calibrating - if too long == error too long
								timer_long_time_error = 0;															// ""
							
								ready_stop = 0; // we can not make pedal = 0; user mast make pedal = 0 so we use this code and the if at the Begin MOTOR_PWM
								pedal = 0;
								rdy = 1; // these 3 variables are for stop after beeing ready / for beter undestanding just make table and run time proccess
								y = 0;
							
								HAL_Delay(30);
								var_write[4] = 0x57;	
								var_write[7] = 0;																// "stop"
								HAL_UART_Transmit_DMA (&huart1, var_write, 8);
								
								
						}
					}						
				}

				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
////////////////////// End loading pressure status : successful or calibrating + start beep sound for injection ///////////////////// 	

/////////////////////////////////////////////// Begin MOTOR_PWM /////////////////////////////////////////////////////////////////////

if ((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == GPIO_PIN_RESET) && (rdy == 1))		// agar az pedal == 0 bejaye gpio_read estefade konim kar nemikonad. be nazaram in vasat yeki dari pedal ro avaz mikone ya bahse zamani hast beyne avaz shodan vaziat pedal
{
		x = 1;
}

if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
calibration_set = motor_pwm (pressure);	

if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
							
				
/////////////////////////////////////////////// End MOTOR_PWM /////////////////////////////////////////////////////////////////////					

/////////////////////////////////////////////// Begin Error pressure ////////////////////////////////////////////////////////////////										

				if ( check == 0 )
				{
						pr1 = pressure;
				}
				
				if ( check == 1 )
				{
						pr2 = pressure;
				}
				
				check ++;
				
				if ( check == 2)
						check = 0;
				
				if ( ((fabs(pr1 - pr2)) >= 6) && ((ready_stop + pedal) >= 1) && ( timer_p_t_check >= 4 ) ) // if after 1 second of starting subtract of the two pr higher than 7
				{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //VALVE
						ready_stop = 0;
						pedal = 0;  // useless if user pressing the pedal
						ready_stop_write[7] = 4;
						HAL_Delay(30);
						HAL_UART_Transmit_DMA(&huart1, ready_stop_write, 8);
						
						pressure_error_write[4] = 0x17; //needle error
						pressure_error_write[7] = 1; 
						HAL_Delay(30);	
						HAL_UART_Transmit_DMA(&huart1, pressure_error_write, 8);	
						error_check = 1; // in motor_pwm function it will be zero and stop the showing error when again starts
					
						nail_error_flag = 1;	
						}		
						if (nail_error_flag)
						{
								HAL_Delay(30);								
								ready_stop_write[7] = 4;																// "LONG TIME ERROR"
								HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);	
							
								if ( (mode >= 1) && (mode <= 5) ) // face english
								{
									HAL_Delay(30);
									let_start[9] = 11; //or hex = 0x0B
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 6) && (mode <= 22) ) // body english
								{
									HAL_Delay(30);
									let_start[9] = 13; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( mode == 23 ) 								// vaginal english
								{
									HAL_Delay(30);
									let_start[9] = 15; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 24) && (mode <= 28) ) // face farsi
								{
									HAL_Delay(30);
									let_start[9] = 17; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( (mode >= 29) && (mode <= 45) )	// body farsi
								{
									HAL_Delay(30);
									let_start[9] = 19; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
						
							if ( mode == 46 )										// vaginal farsi
								{
									HAL_Delay(30);
									let_start[9] = 21; 
									HAL_UART_Transmit_DMA(&huart1, let_start, 10);
								}
								
								while (nail_error_flag)
								{
										error_buzzer();
										if (timer_nail_error > 13)
										{
											
												error_buzzer_flag = 0;
												nail_error_flag = 0;
												timer_nail_error = 0;
												if (mode >= 24)
												{
														HAL_Delay(30);
														let_start[9] = 28; 
														HAL_UART_Transmit_DMA(&huart1, let_start, 10);
														calibration_page_control = 1;
												}
												else
												{
														HAL_Delay(30);
														let_start[9] = 29; 
														HAL_UART_Transmit_DMA(&huart1, let_start, 10);
														calibration_page_control = 1;
												}
										}
								}
						
				}		
				
/////////////////////////////////////////////// End Error pressure ///////////////////////////////////////	

/////////////////////////////////////////////// Begin Ready_stop = 0  ///////////////////////////////////////	
				
				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
				
				if ((ready_stop + pedal) == 0)
				{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //BUZZER
					
						/*if (ready_stop_check == 1)  // it is for when we have separate calibration status and ready_stop and when stop , it turns off please_wait
						{
								ready_stop_check = 0;						
								calibration_write[7] = 0;
								HAL_Delay(30);
								HAL_UART_Transmit_DMA (&huart1, calibration_write, 8);
						}
						*/
						
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //VALVE
						TIM3->CCR1 = 0;       // pwm = 0;
						timer_p_t_check = 0 ; // timer for needle pressure error
						calibration_set = 0; //for 1cc
					
						if  (send_stop_one_time == 1)
						{
							if (beep)
							{
								if (inject_1cc == 0)
								{
										HAL_Delay(30);
										ready_stop_write[7] = 3;																// "stop"
										HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);
								}
								send_stop_one_time = 0;	
							}	
							else
							{
										HAL_Delay(30);
										ready_stop_write[7] = 0;																// "ready"
										HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);
										send_stop_one_time = 0;								
							}
						}	
							long_time_error = 0;
							timer_long_time_error = 0;
				}
				
				if ( ((ready_stop + pedal) >= 1) && (send_run_one_time == 1))
				{
								if ( (beep) && (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) ==  GPIO_PIN_SET))
								{
										HAL_Delay(30);
										ready_stop_write[7] = 1;																// "run"
										HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);
								}
								if (beep == 0)
								{
										HAL_Delay(30);
										ready_stop_write[7] = 2;																// "wait"
										HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);	
								}
								if ( (beep) && (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) ==  GPIO_PIN_RESET))
								{
										if (inject_1cc == 0)
										{
										HAL_Delay(30);
										ready_stop_write[7] = 3;																// "STOP"
										HAL_UART_Transmit_DMA (&huart1, ready_stop_write, 8);
										}
										timer_injection = 0;
										bar_write[7] = 1;
								}
								
								send_run_one_time = 0;
				}
				
				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
/////////////////////////////////////////////// End Ready_stop = 0 ///////////////////////////////////////////////////////////				

////////////////////////////////////////////// Begin time calculation ////////////////////////////////////////////////////////		
				if (inject_1cc == 0)
				{
						time = round(volume * pressure_user / 10) ;	
						if (time < 2)
								time = 2;
			  }
				
				if (time_check != time)
				{
						time_check = time;
					
						HAL_Delay(30);
						var_write[4] = 0x12;
						var_write[7] = time;
						HAL_UART_Transmit_DMA (&huart1, var_write, 8); 
						
						timer_injection = 0; // resets the main timer when either pressure_user or volume is changed
						
						HAL_Delay(30); // reseting Bar on LCD
						bar_write[7] = 1; // = 1 : just for technical reasons					
						HAL_UART_Transmit_DMA (&huart1, bar_write, 8);
				}
				
////////////////////////////////////////////// END  time calculation ////////////////////////////////////////////////////////		

////////////////////////////////////////////// Begin Send Volume NUMBER /////////////////////////////////////////////////////
				if (send_volume == 1)
				{						
						var_write[4] = 0x58;
						var_write[7] = volume * 5;
						HAL_Delay(30);
						HAL_UART_Transmit_DMA (&huart1, var_write, 8); 
						send_volume = 0;
				}
////////////////////////////////////////////// END Send Volume NUMBER   /////////////////////////////////////////////////////
				
///////////Begin Every 200ms :	injection time update LCD + eeprom update + beep sound every "time" for 200ms ////////////////	
			
				if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}
				
			if ( (inject_1cc == 1) && ( ( pedal + ready_stop >= 1) || (check_1cc == 1) ) )  // for stabling the pedal for 1cc
				{ 
						 check_1cc = 1;
				} 	
			
			
			if (1)//timer_check == 1)			// Here we have 3 UART transmits :	3 * 40 = 120 ms < 200 ms == OK!
			{
						timer_check = 0;
				
//						HAL_Delay(30);
//						var_write[4] = 0x30;
//						var_write[7] = pressure;
//						HAL_UART_Transmit_DMA (&huart1, var_write, 8); 
				
						if (pressure >= 7) // timer injection calculator when device is started and we have a pressure higher than 5 kpa
								timer_200ms ++;
						
						if (timer_200ms >= 64000)
								timer_200ms = 0;	
						if (timer_min >= 60)
								timer_min = 0;
						if (timer_hour >= 254)
								timer_hour = 0;
						
						if (timer_200ms == 300)
						{		
								timer_200ms = 0;
								timer_min ++;
								eeprom[2] = timer_min;
								EEPROM_Write(0,	0, eeprom, 4);
								var_write[4] = 0x41;
								var_write[7] = timer_min;						
								HAL_Delay(30);	
								HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						}
				
						if (timer_min == 60)
						{		
								timer_min = 0;
								timer_hour ++;
								eeprom[3] = timer_hour;
								EEPROM_Write(0,	0, eeprom, 4);			
								
								var_write[4] = 0x42;
								var_write[7] = timer_hour;
								var_write[6] = 0; // in DGUS DWIN select var type = unsinged int (2byte)
								HAL_Delay(30);
								HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						}
					
						// beep for general enabling the buzzer ... beep_timer is for controlling when buzzer must be ringing
						if ( (beep == 1) && ((ready_stop + pedal + x) >= 2) && (beep_timer == 1)) // main buzzer "if": when Caliration_set = 1 or when cycle ended /// x + ready_stop = 2 or x + pedal = 2
						{
								if(beep_check == 1) // beep_check is 1 for first time.
								{
										HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);	//BUZZER								
								}
								else
								{
										HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //BUZZER
										beep_timer = 0; // only for first time
								}
								beep_check = (-1) * beep_check;			// just one time							
						}
								
						if (timer_injection > (round(time * 5 ))) //here we calculate how many 200ms does "time" have ? : time is integer & (5 * time) / 0.2 = numbers if timer_injection
						{
								timer_injection = 0;
								beep_timer = 1; // for Ending
						}
						if (timer_injection == 1)
						{
								beep_timer = 1; // for Starting
						}
						
						// BAR TIME
						if ( ( (timer_injection == (round(time * bar_write[7] * 0.5)))) && (beep + x == 2) ) // x !
						{
								bar_write[7] ++;
								HAL_Delay(30);
								HAL_UART_Transmit_DMA (&huart1, bar_write, 8);
								
									
								if (bar_write[7] > 10) // must be 10 otherwise calculations are all wrong.
										bar_write[7] = 0; 
						}
					}
			
				if (timer_heater <= 50)
					 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // HEATER ON
				if ( (timer_heater > 50) && (timer_heater < 200) ) 
					 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // HEATER OFF			
				if (timer_heater >= 200)
						timer_heater = 0;			//HEATER  10s = 50 ON -- 30s = 150 OFF >> 50 + 150 = 200;
				
				if (timer_p_t_check >= 64000)
					 timer_p_t_check = 0; // for needle error pressure
							
				if (timer_calibration >= 64000)
					 timer_calibration = 0; // for calibration page
				
				if (timer_injection >= 64000)
						timer_injection = 0; // main timer
				
				if (timer_long_time_error >= 64000)
						timer_long_time_error = 0;	// long time error pressure after 30s
						
				if (long_time_error_flag_counter >= 64000)
						long_time_error_flag_counter = 0;
				
			
			if (check_1cc == 1) // for when release the pedal the injection wont stop
			{
					pedal = 1;
			}

////////////////////////////////////// Begin Error_buzzer /////////////////////////////////////////////
			//if ( (long_time_error_flag == 1) || (High_pressure_error_flag == 1) || (Nail_pressure_error_flag == 1) )
			
			
////////////////////////////////////// End 	Error_buzzer 	////////////////////////////////////////////
			
//////////////////////////////////////End 200 ms procedure //////////////////////////////////////////						
								
//////////////////////////////////////Begin calibration page //////////////////////////////////////	

		while (calibration_page_control == 1)
		{
				mode = 100;
				if (buzzer_last_state != buzzer)
				{	
						HAL_Delay(30);
						if (buzzer == 1)
						{
								mute_write[9] = 0x30;
								HAL_UART_Transmit_DMA(&huart1, mute_write, 10);	
						}
						else
						{
								mute_write[9] = 0x38;
								HAL_UART_Transmit_DMA(&huart1, mute_write, 10);
						}
						buzzer_last_state = buzzer;						
				}
				
			if (calibration_page_ready_stop == 1)
			{
						if (timer_calibration < 25)  //5 seconds
						{																	
										HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // valve is off
										TIM3->PSC = 1125-1;
										TIM3->ARR  = 8000 -1;	// f = 8 hz
										TIM3->CCR1 = 8000;		// dt = 100%
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);														
						}
						
						if ((timer_calibration >= ((25 * zarib_t_c) + (3 * (zarib_t_c - 1)))) && (timer_calibration <= (28 * zarib_t_c)))
						{
										HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // valve is on
										TIM3->PSC = 1125-1;
										TIM3->ARR  = 8000 -1;	// f = 8 hz
										TIM3->CCR1 = 0;
							pressure_calibration = ads1115_pressure_adc(); // location of this command is very important : LINE 1731
							var_write[4] = 0x31;
							var_write[7] = pressure_calibration;
							HAL_UART_Transmit_DMA (&huart1, var_write, 8);
						}
						
						if ((timer_calibration > (28 * zarib_t_c)) && (timer_calibration < ((25 * (zarib_t_c + 1)) + (3 * zarib_t_c))) )    // very important to define a range NOT a == number
						{										
										HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // valve is off
										
										if ( pressure_calibration > 90)
										{
												TIM3->PSC = 1125-1;
												TIM3->ARR  = 8000 -1;	// f = 8 hz
												TIM3->CCR1 = 8000;		// dt = 50%
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
												
										}
										
										if ((pressure_calibration > 80) && (pressure_calibration <= 90))
										{
												TIM3->PSC = 1125-1;
												TIM3->ARR  = 8000 -1;	// f = 8 hz
												TIM3->CCR1 = 5000;		// dt = %
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
										}
										
										if ((pressure_calibration > 70) && (pressure_calibration <= 80))
										{
												TIM3->PSC = 1125-1;
												TIM3->ARR  = 8000 -1;	// f = 8 hz
												TIM3->CCR1 = 3000;		// dt = %
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
										}
										
										if ((pressure_calibration > 50) && (pressure_calibration <= 70))
										{											
												TIM3->PSC = 1125-1;
												TIM3->ARR  = 8000 -1;	// f = 8 hz
												TIM3->CCR1 = 2000;		// dt = %
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);											
										}
										
										if ((pressure_calibration >= 15) && (pressure_calibration <= 50))
										{
												cal_p_r_s_write[7] = 2; // SUCCESSFUL
												HAL_Delay(30);
												HAL_UART_Transmit_DMA(&huart1, cal_p_r_s_write, 8);
											
												TIM3->PSC = 1125-1;
												TIM3->ARR  = 8000 -1;	// f = 8 hz
												TIM3->CCR1 = 0;
											
												calibration_page_ready_stop = 0; // exiting the loop
												timer_calibration = 0;
												zarib_t_c = 0;
												
										}
																		
										if ((pressure_calibration < 15) && (pressure_calibration >= 10))
										{
												TIM3->PSC = 1125-1;
												TIM3->ARR  = 8000 -1;	// f = 8 hz
												TIM3->CCR1 = 2000;		// dt = %
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
										}
										if ((pressure_calibration < 10) && (pressure_calibration >= 5))
										{
												TIM3->PSC = 1125-1;
												TIM3->ARR  = 8000 -1;	// f = 8 hz
												TIM3->CCR1 = 1000;		// dt = %
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
										}
										if (pressure_calibration < 5)
										{
												TIM3->PSC = 1125-1;
												TIM3->ARR  = 8000 -1;	// f = 8 hz
												TIM3->CCR1 = 8000;		// dt = %
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
										}																			
										
										 one_one = 1;
								}
								
								if ( (timer_calibration == (((25 * (zarib_t_c + 1)) + (3 * zarib_t_c)))) && (one_one == 1) ) //we need one_time because this loop last 200 ms
										{
												zarib_t_c ++;
												one_one = 0; // wow tnx God
										}
								
								if (zarib_t_c >= 20)
								{
										cal_p_r_s_write[7] = 3; // UN SUCCESSFUL
										HAL_Delay(30);
										HAL_UART_Transmit_DMA(&huart1, cal_p_r_s_write, 8);
											
										calibration_page_ready_stop = 0; // exiting the loop
										timer_calibration = 0;
										zarib_t_c = 1;
										TIM3->PSC = 1125-1;
										TIM3->ARR  = 8000 -1;	// f = 8 hz
										TIM3->CCR1 = 0;
										
								}	
								
						
								
						}
			
						if ( calibration_page_ready_stop == 0)
						{
								timer_calibration = 0;
								zarib_t_c = 1;
								TIM3->PSC = 1125-1;
								TIM3->ARR  = 8000 -1;	// f = 8 hz
								TIM3->CCR1 = 0;
								
						}
					}
			
				if ((calibration_page_control == 0) && (one_time == 1))
				{
						calibration_page_ready_stop = 0;
						cal_p_r_s_write[7] = 0; // stop
						HAL_Delay(30);
						HAL_UART_Transmit_DMA(&huart1, cal_p_r_s_write, 8);
						one_time = 0;
				}
//////////////////////////////////////END calibration page //////////////////////////////////////

/////////////////////////////////////Begin LCD Buzzer sound /////////////////////////////////////				
//it is on the above section
////////////////////////////////////END LCD Buzzer sound ////////////////////////////////////////				
				

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1124;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 12800-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1125-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 16000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA3 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


/*
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) 
{
	if(huart->Instance == huart1.Instance)
	{
				
	}
}
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == htim2.Instance)
    {   
				timer_check = 1;
				timer_heater ++;	
				
				if ((ready_stop + pedal) >= 1)
					 timer_p_t_check ++ ;													// for cheking needle error pressure
				
				if (calibration_page_ready_stop == 1)
						timer_calibration ++;												// for calibration page
				
				if( (beep == 1) && ((ready_stop + pedal) >= 1))
					timer_injection ++;														// the main timer : for time & bar & buzzer
					
				if (long_time_error == 1)
					timer_long_time_error ++;			

				if ( (inject_1cc == 1) && ( ( pedal + ready_stop >= 1) || (check_1cc == 1) ) ) 
				{ 
						 check_1cc = 1;
						 if (beep == 1)
						 {
								timer_1cc ++;
						 }
				}
				if (timer_relay_ntc <= 20)
				{
						timer_relay_ntc ++;
				}
				
				if (long_time_error_flag)
						long_time_error_flag_counter ++;
				
				if(high_pressure_error_flag)
						timer_high_pressure_error ++;
				
				if(nail_error_flag)					
						timer_nail_error ++;
				
				if (error_buzzer_flag)
					timer_error_buzzer ++;
    }
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
