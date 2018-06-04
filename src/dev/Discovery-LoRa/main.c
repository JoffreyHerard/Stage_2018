/******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.5
  * @date    30-March-2018
  * @brief   this is the main!
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "low_power_manager.h"
#include "lora.h"
#include "bsp.h"
#include "timeServer.h"
#include "vcom.h"
#include "version.h"
/* LoRa RAW Includes ------------------------------------------------------------------*/
#include <string.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "delay.h"
#include "low_power_manager.h"
#include "vcom.h"


/*LoRaWAN VAR / CALLBACK / etc */

#define LPP_DATATYPE_DIGITAL_INPUT  0x0
#define LPP_DATATYPE_DIGITAL_OUTPUT 0x1
#define LPP_DATATYPE_HUMIDITY       0x68
#define LPP_DATATYPE_TEMPERATURE    0x67
#define LPP_DATATYPE_BAROMETER      0x73
#define LPP_APP_PORT 99
/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            10000
/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE LORAWAN_ADR_ON
/*!
 * LoRaWAN Default data Rate Data Rate
 * @note Please note that LORAWAN_DEFAULT_DATA_RATE is used only when ADR is disabled
 */
#define LORAWAN_DEFAULT_DATA_RATE DR_0
/*!
 * LoRaWAN application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_APP_PORT                            2
/*!
 * LoRaWAN default endNode class port
 */
#define LORAWAN_DEFAULT_CLASS                       CLASS_A
/*!
 * LoRaWAN default confirm state
 */
#define LORAWAN_DEFAULT_CONFIRM_MSG_STATE           LORAWAN_UNCONFIRMED_MSG
/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFF_SIZE                           64
/*!
 * User application data
 */
static uint8_t AppDataBuff[LORAWAN_APP_DATA_BUFF_SIZE];

/*!
 * User application data structure
 */
static lora_AppData_t AppData={ AppDataBuff,  0 ,0 };
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* call back when LoRa endNode has received a frame*/
static void LORA_RxData( lora_AppData_t *AppData);

/* call back when LoRa endNode has just joined*/
static void LORA_HasJoined( void );

/* call back when LoRa endNode has just switch the class*/
static void LORA_ConfirmClass ( DeviceClass_t Class );

/* call back when server needs endNode to send a frame*/
static void LORA_TxNeeded ( void );

/* LoRa endNode send request*/
static void Send( void );

/* start the tx process*/
static void LoraStartTx(TxEventType_t EventType);

/* tx timer callback function*/
static void OnTxTimerEvent( void );

/* Private variables ---------------------------------------------------------*/
/* load Main call backs structure*/
static LoRaMainCallback_t LoRaMainCallbacks = { HW_GetBatteryLevel,
                                                HW_GetTemperatureLevel,
                                                HW_GetUniqueId,
                                                HW_GetRandomSeed,
                                                LORA_RxData,
                                                LORA_HasJoined,
                                                LORA_ConfirmClass,
                                                LORA_TxNeeded};

/*!
 * Specifies the state of the application LED
 */
static uint8_t AppLedStateOn = RESET;

static TimerEvent_t TxTimer;

#ifdef USE_B_L072Z_LRWAN1
/*!
 * Timer to handle the application Tx Led to toggle
 */
static TimerEvent_t TxLedTimer;
static void OnTimerLedEvent( void );
#endif
/* !
 *Initialises the Lora Parameters
 */
static  LoRaParam_t LoRaParamInit= {LORAWAN_ADR_STATE,
                                    LORAWAN_DEFAULT_DATA_RATE,
                                    LORAWAN_PUBLIC_NETWORK};



/*LoRa VAR / CALLBACK / etc */



#define RF_FREQUENCY                                868000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 64 // Define the payload size here
#define LED_PERIOD_MS               200

#define LEDS_OFF   do{ \
                   LED_Off( LED_BLUE ) ;   \
                   LED_Off( LED_RED ) ;    \
                   LED_Off( LED_GREEN1 ) ; \
                   LED_Off( LED_GREEN2 ) ; \
                   } while(0) ;

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

 /* Led Timers objects*/
/* Private function prototypes -----------------------------------------------*/
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

void switchToLoRa( void );
void sendLoRa(void);
void switchToLoRaWAN( void );
void sendLoRaWAN(void);


int main( void )
{

  /* STM32 HAL library initialization*/
  HAL_Init();

  /* Configure the system clock*/
  SystemClock_Config();

  /* Configure the debug mode*/
  DBG_Init();

  /* Configure the hardware*/
  HW_Init();
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id , LPM_Disable );

  /* Configure the Lora Stack*/
  LORA_Init( &LoRaMainCallbacks, &LoRaParamInit);

  LORA_Join();

  LoraStartTx( TX_ON_TIMER) ;
  /*Tant que pas join*/
  while ( LORA_JoinStatus () != LORA_SET)
  {
      /*Not joined, try again later*/
      LORA_Join();
  }
  PRINTF("On est en LoRaWAn ! ");
  /*Passage en LoRa*/
  LORA_Init( NULL, NULL);

    // Radio initialization
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init( &RadioEvents );

  Radio.SetChannel( RF_FREQUENCY );



  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000000 );

  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
  Radio.Rx( RX_TIMEOUT_VALUE );
  // Send the next PING frame
  Buffer[0] = 'D';
  Buffer[1] = 'I';
  Buffer[2] = 'S';
  Buffer[3] = 'C';
  Buffer[4] = 'O';
  Buffer[5] = 'V';
  Buffer[6] = 'E';
  Buffer[7] = 'R';

  // We fill the buffer with numbers for the payload
  /*for( int i = 8; i < BufferSize; i++ )
  {
    Buffer[i] = i - 4;
  }*/
  PRINTF("...PING\n\r");
  while(1){
  DelayMs( 1 );
  Radio.Send( Buffer, 8 );
  }
  /*Retour en LoRaWAN*/




  /* Configure the Lora Stack*/
  LORA_Init( &LoRaMainCallbacks, &LoRaParamInit);

  LORA_Join();

  LoraStartTx( TX_ON_TIMER) ;
  /*Tant que pas join*/
  while ( LORA_JoinStatus () != LORA_SET)
  {
      /*Not joined, try again later*/
      LORA_Join();
  }

  PRINTF("On est en LoRaWAn ! AGAAAAAIN");

  /*Maintenant on slack*/
  while( 1 )
  {
    PRINTF("Slack\n\r");
    DISABLE_IRQ( );
    /* if an interrupt has occurred after DISABLE_IRQ, it is kept pending
    * and cortex will not enter low power anyway  */

#ifndef LOW_POWER_DISABLE
    LPM_EnterLowPower( );
#endif

    ENABLE_IRQ();

    /* USER CODE BEGIN 2 */
    /* USER CODE END 2 */
  }
}



/*BEGIN  LORAWAN FUNCTION */

static void LORA_HasJoined( void )
{
#if( OVER_THE_AIR_ACTIVATION != 0 )
  PRINTF("JOINED\n\r");
#endif
  LORA_RequestClass( LORAWAN_DEFAULT_CLASS );
}

static void Send( void )
{
  /* USER CODE BEGIN 3 */
  uint16_t pressure = 0;
  int16_t temperature = 0;
  uint16_t humidity = 0;
  uint8_t batteryLevel;
  sensor_t sensor_data;

  if ( LORA_JoinStatus () != LORA_SET)
  {
    /*Not joined, try again later*/
    LORA_Join();
    return;
  }

  DBG_PRINTF("SEND REQUEST\n\r");
#ifndef CAYENNE_LPP
  int32_t latitude, longitude = 0;
  uint16_t altitudeGps = 0;
#endif

#ifdef USE_B_L072Z_LRWAN1
  TimerInit( &TxLedTimer, OnTimerLedEvent );

  TimerSetValue(  &TxLedTimer, 200);

  LED_On( LED_RED1 ) ;

  TimerStart( &TxLedTimer );
#endif

  BSP_sensor_Read( &sensor_data );



  temperature = ( int16_t )( sensor_data.temperature * 100 );     /* in �C * 100 */
  pressure    = ( uint16_t )( sensor_data.pressure * 100 / 10 );  /* in hPa / 10 */
  humidity    = ( uint16_t )( sensor_data.humidity * 10 );        /* in %*10     */
  latitude = sensor_data.latitude;
  longitude= sensor_data.longitude;
  uint32_t i = 0;

  batteryLevel = HW_GetBatteryLevel( );                     /* 1 (very low) to 254 (fully charged) */

  AppData.Port = LORAWAN_APP_PORT;


 /* not REGION_XX915 */
  AppData.Buff[i++] = AppLedStateOn;
  AppData.Buff[i++] = ( pressure >> 8 ) & 0xFF;
  AppData.Buff[i++] = pressure & 0xFF;
  AppData.Buff[i++] = ( temperature >> 8 ) & 0xFF;
  AppData.Buff[i++] = temperature & 0xFF;
  AppData.Buff[i++] = ( humidity >> 8 ) & 0xFF;
  AppData.Buff[i++] = humidity & 0xFF;
  AppData.Buff[i++] = batteryLevel;
  AppData.Buff[i++] = ( latitude >> 16 ) & 0xFF;
  AppData.Buff[i++] = ( latitude >> 8 ) & 0xFF;
  AppData.Buff[i++] = latitude & 0xFF;
  AppData.Buff[i++] = ( longitude >> 16 ) & 0xFF;
  AppData.Buff[i++] = ( longitude >> 8 ) & 0xFF;
  AppData.Buff[i++] = longitude & 0xFF;
  AppData.Buff[i++] = ( altitudeGps >> 8 ) & 0xFF;
  AppData.Buff[i++] = altitudeGps & 0xFF;
  AppData.BuffSize = i;

  LORA_send( &AppData, LORAWAN_DEFAULT_CONFIRM_MSG_STATE);

  /* USER CODE END 3 */
}


static void LORA_RxData( lora_AppData_t *AppData )
{
  /* USER CODE BEGIN 4 */
  DBG_PRINTF("PACKET RECEIVED ON PORT %d\n\r", AppData->Port);

  switch (AppData->Port)
  {
    case 3:
    /*this port switches the class*/
    if( AppData->BuffSize == 1 )
    {
      switch (  AppData->Buff[0] )
      {
        case 0:
        {
          LORA_RequestClass(CLASS_A);
          break;
        }
        case 1:
        {
          LORA_RequestClass(CLASS_B);
          break;
        }
        case 2:
        {
          LORA_RequestClass(CLASS_C);
          break;
        }
        default:
          break;
      }
    }
    break;
    case LORAWAN_APP_PORT:
    if( AppData->BuffSize == 1 )
    {
      AppLedStateOn = AppData->Buff[0] & 0x01;
      if ( AppLedStateOn == RESET )
      {
        PRINTF("LED OFF\n\r");
        LED_Off( LED_BLUE ) ;
      }
      else
      {
        PRINTF("LED ON\n\r");
        LED_On( LED_BLUE ) ;
      }
    }
    break;
  case LPP_APP_PORT:
  {
    AppLedStateOn= (AppData->Buff[2] == 100) ?  0x01 : 0x00;
    if ( AppLedStateOn == RESET )
    {
      PRINTF("LED OFF\n\r");
      LED_Off( LED_BLUE ) ;

    }
    else
    {
      PRINTF("LED ON\n\r");
      LED_On( LED_BLUE ) ;
    }
    break;
  }
  default:
    break;
  }
  /* USER CODE END 4 */
}

static void OnTxTimerEvent( void )
{
  Send( );
  /*Wait for next tx slot*/
  TimerStart( &TxTimer);
}

static void LoraStartTx(TxEventType_t EventType)
{
  if (EventType == TX_ON_TIMER)
  {
    /* send everytime timer elapses */
    TimerInit( &TxTimer, OnTxTimerEvent );
    TimerSetValue( &TxTimer,  APP_TX_DUTYCYCLE);
    OnTxTimerEvent();
  }
  else
  {
    /* send everytime button is pushed */
    GPIO_InitTypeDef initStruct={0};

    initStruct.Mode =GPIO_MODE_IT_RISING;
    initStruct.Pull = GPIO_PULLUP;
    initStruct.Speed = GPIO_SPEED_HIGH;

    HW_GPIO_Init( USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, &initStruct );
    HW_GPIO_SetIrq( USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, 0, Send );
  }
}

static void LORA_ConfirmClass ( DeviceClass_t Class )
{
  PRINTF("switch to class %c done\n\r","ABC"[Class] );

  /*Optionnal*/
  /*informs the server that switch has occurred ASAP*/
  AppData.BuffSize = 0;
  AppData.Port = LORAWAN_APP_PORT;

  LORA_send( &AppData, LORAWAN_UNCONFIRMED_MSG);
}

static void LORA_TxNeeded ( void )
{
  AppData.BuffSize = 0;
  AppData.Port = LORAWAN_APP_PORT;

  LORA_send( &AppData, LORAWAN_UNCONFIRMED_MSG);
}

#ifdef USE_B_L072Z_LRWAN1
static void OnTimerLedEvent( void )
{
  LED_Off( LED_RED1 ) ;
}
#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


/*END LORAWAN FUNCTION */





/*BEGIN LORA FUNCTION*/

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
    PRINTF("LoRa: OnTxDone\n\r");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;

    PRINTF("LoRa: OnRxDone\n\r");
    PRINTF("LoRa: RssiValue=%d dBm, SnrValue=%d\n\r", rssi, snr);
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;

    PRINTF("LoRa: OnTxTimeout\n\r");
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
    PRINTF("LoRa: OnRxTimeout\n\r");
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
    PRINTF("LoRa: OnRxError\n\r");
}



/*END LORA FUNCTION*/






/* BEGIN PERSONNAL  FUNCTION  */
void switchToLoRa( void ){

}
void sendLoRa(void){

}
void switchToLoRaWAN( void ){

}
void sendLoRaWAN(void){

}



/* END PERSONNAL  FUNCTION  */
