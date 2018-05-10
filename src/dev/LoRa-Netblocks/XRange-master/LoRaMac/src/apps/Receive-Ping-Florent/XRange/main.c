/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Ping-Pong implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include <time.h>


#include "board.h"
#include "radio.h"
#include "rtc-board.h"


#if defined( USE_BAND_868 )

#define RF_FREQUENCY                                868000000 // Hz
//#define RF_FREQUENCY                                869150000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             20        // dBm

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0        // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25e3      // Hz
#define FSK_DATARATE                                50e3      // bps
#define FSK_BANDWIDTH                               50e3      // Hz
#define FSK_AFC_BANDWIDTH                           83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

// RED
#define LED1_PIN                                    GPIO_PIN_8
#define LED1_GPIO_PORT                              GPIOA

// GREEN
#define LED2_PIN                                    GPIO_PIN_7
#define LED2_GPIO_PORT                              GPIOA

#define LED_NB                           2

#define LED_ON                           GPIO_PIN_SET
#define LED_OFF                          GPIO_PIN_RESET

typedef enum
{
    LED_YELLOW = 0,
    LED_GREEN = 1,
} tLed;

GPIO_TypeDef* LedPort[LED_NB] =
{
    LED1_GPIO_PORT,
    LED2_GPIO_PORT
};
const uint16_t LedPin[LED_NB] =
{
    LED1_PIN,
    LED2_PIN,

};

void LedOn( tLed led )
{
    HAL_GPIO_WritePin( LedPort[led], LedPin[led], LED_ON );
}

void LedOff( tLed led )
{
    HAL_GPIO_WritePin( LedPort[led], LedPin[led], LED_OFF );
}

void LedToggle( tLed led )
{
    LedPort[led]->ODR ^= LedPin[led];
}

void LedGreenBlink()
{
    LedToggle( LED_GREEN );
    Delay(2);
    LedToggle( LED_GREEN );
}

void LedYellowBlink()
{
    uint8_t tmp;
    for (tmp=0; tmp<10; tmp++) {
    LedToggle( LED_YELLOW );
    Delay(2);
    LedToggle( LED_YELLOW );
    }
}

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT
}States_t;

#define RX_TIMEOUT_VALUE                            20000 // En ms
#define BUFFER_SIZE                                 64 // Define the payload size here


uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
uint8_t OldBuffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

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

/**
 * Main application entry point.
 */
int main( void )
{
    //bool isMaster = true;
   // TimerTime_t MyTimestamp;
   // unsigned char tmp[30]="";
   // unsigned char BufferRssiSnrString[BUFFER_SIZE]="";
  // uint16_t DelayTimer = RX_TIMEOUT_VALUE;
 // Temps en minutes
    //char temps[255];

    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );

    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );

#if defined( USE_MODEM_LORA )

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

#elif defined( USE_MODEM_FSK )

    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );

    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif

//printf("\fStart Ping-Pong App\n\r");

/*
    LedOff( LED_YELLOW);
    LedOff( LED_GREEN);

    LedOn( LED_YELLOW);
    LedOn( LED_GREEN);
    Delay(5);
    LedOff( LED_YELLOW);
    LedOff( LED_GREEN);
    */


    /*MyTimestamp=RtcGetTimerValue();

    sprintf(BufferRssiSnrString,"\r\nInitialisation :%i",MyTimestamp);
    UartUsbPutBuffer(&UartUsb,BufferRssiSnrString,BUFFER_SIZE);
*/
    Radio.Rx(RX_TIMEOUT_VALUE);
    while( 1 )
    {
        switch(State) {
    case RX:

       // LedOn( LED_YELLOW);
    //    if ( Buffer[0]=='D' )
      //  {
       // LedOn( LED_GREEN);
/*
DEBUG ONLY
        sprintf(tmp,"\r\nTimestamp: %i",RtcGetTimerValue()); // Convert integer to string
        UartUsbPutBuffer(&UartUsb,tmp,30);

        memset(BufferRssiSnrString,0,sizeof(BufferRssiSnrString));
        BufferRssiSnrString[0]='\r';
        BufferRssiSnrString[1]='\n';
        BufferRssiSnrString[2]='X';
        for(int i=3; i<BUFFER_SIZE; i++){
        BufferRssiSnrString[i]=Buffer[i-3]; // Convert integer to string
        }

        DelayMs(100);
        sprintf(BufferRssiSnrString,"Reçu :%02X",MyTimestamp);
        UartUsbPutBuffer(&UartUsb,BufferRssiSnrString,BUFFER_SIZE);

        memset(BufferRssiSnrString,0,sizeof(BufferRssiSnrString));

        DelayMs(100); */
        Buffer[10]=Buffer[9]+10;
        DelayMs( 2000 );
        Radio.Send( Buffer, BUFFER_SIZE );
/*        DelayMs(100);
        memset(BufferRssiSnrString,0,sizeof(BufferRssiSnrString));
*/

        //}
//                LedOff( LED_GREEN);
//                LedOff( LED_YELLOW);

        /*
        if (strcmp( ( const char* )OldBuffer, ( const char* )Buffer) != 0 ) {
            memcpy1(OldBuffer,Buffer,BUFFER_SIZE);

           sprintf(BufferRssiSnrString,"%02X %02X %02X/n/r",Buffer[0],Buffer[1],Buffer[2]); // Convert integer to string
           */
            /*len=strlen(RssiSnrString);
            len_buf=strlen(Buffer);
            for (i=0; i<len; i++)
            {
                Buffer[len_buf+i]=RssiSnrString[i];
            } */
    /*            len_buf=strlen(BufferRssiSnrString);
            BufferRssiSnrString[len_buf]='\r';
            BufferRssiSnrString[len_buf+1]='\n';
            BufferRssiSnrString[len_buf+2]=NULL;
            */
            /*  len_buf=strlen(Buffer);
            Buffer[len_buf]='\r';
            Buffer[len_buf+1]='\n';
            Buffer[len_buf+2]=NULL;*/
 //           len_buf=strlen(BufferRssiSnrString);
            //Buffer[len_buf+i+2]=NULL;
 //        UartUsbPutBuffer(&UartUsb,BufferRssiSnrString,len_buf);
      //      UartUsbPutBuffer(&UartUsb,Buffer,len_buf);
//            DelayMs( 100 );
//            memset(Buffer,NULL,sizeof(Buffer));
    //        memset(BufferRssiSnrString,0,sizeof(BufferRssiSnrString));


        //}
    break;
    case TX:
        //UartUsbPutBuffer(&UartUsb,(unsigned char *)"TX ok",10);
        Radio.Rx(RX_TIMEOUT_VALUE);
        State = LOWPOWER;
    break;
    case RX_TIMEOUT:
       //UartUsbPutBuffer(&UartUsb,(unsigned char*)"\r\nTIMEOUT",9);
//        DelayTimer = DelayTimer+1000;
        Radio.Rx(RX_TIMEOUT_VALUE);
        State = LOWPOWER;
    break;
    case RX_ERROR:
        //UartUsbPutBuffer(&UartUsb,(unsigned char*)"\r\nERROR",9);
        Radio.Rx(RX_TIMEOUT_VALUE);
        State = LOWPOWER;
    break;
    case TX_TIMEOUT:
        //UartUsbPutBuffer(&UartUsb,(unsigned char*)"\r\nTX Timeout",10);
        Radio.Rx(RX_TIMEOUT_VALUE);
        State = LOWPOWER;
    break;
    case LOWPOWER:
    default:

    break;
    }

 //   TimerLowPowerHandler( );
 /*   LedOff( LED_YELLOW);
    LedOff( LED_GREEN);
*/
   //     TimerLowPowerHandler( );

    }
}

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
   // UartUsbPutBuffer(&UartUsb,(unsigned char*)"\r\nReceive OK",BUFFER_SIZE);

    State = RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
}
