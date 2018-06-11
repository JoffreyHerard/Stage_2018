/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <assert.h>
#include "hw.h"
#include "radio.h"
//#include "lora.h"
#include "timeServer.h"
#include "delay.h"
#include "low_power_manager.h"
#include "vcom.h"

#define  USE_BAND_868
#define USE_MODEM_LORA

#define RF_FREQUENCY                                868000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm


#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
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


uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;


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

/**
 * Main application entry point.
 */
#define NOT_FOUND (-1)
static const char LORAWAN_DEVICE_EUI_STRING[]=                   "3434373170368E0E";

/*CUSTOM DEFINE*/

#define NAME_MSG_LORA 0
#define KIND_MSG_LORA 1
#define FREQUENCY_MSG_LORA 2
#define SLOTS_MSG_LORA 3
#define ID_SRC_MSG_LORA 4
#define ID_DEST_MSG_LORA 5
#define DATA_MSG_LORA 6
#define LISTENINGTIME_MSG_LORA 7

//unsigned int RF_FREQUENCY=868000000;

/*CUSTOM VAR */
bool discovered=false;
bool registered=false;
bool isListening=true;
char MyLoRaGW[127];
int tryRegister= 0;
int tryDiscover= 0;
int slot=10;
int listeningTime=10;
int data= 42;
/*CUSTOM FUNCTION*/

void switchToLoRa( void );
void sendLoRa( char* data, int length );
char** str_split( char* a_str, const char a_delim );
void notDiscovered( void );
void notRegistered( void );
void sendData( void );
void change_frequency(int newFrequency);
/**
 * Main application entry point.
 */
int main( void )
{
  HAL_Init( );
  SystemClock_Config( );
  DBG_Init( );
  HW_Init( );
  /*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id , LPM_Disable );
  /* Led Timers*/
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

  PRINTF("STARTING.... \n\r");

  PRINTF("WAITING TO BE DISCOVERED \n\r");
  /*ENABLE_IRQ( );
  while(!discovered){
        DelayMs(10);
        notDiscovered();
  }
  PRINTF("WAITING TO BE REGISTERED \n\r");
  while(discovered && !registered){
	  	notRegistered();
	  	DelayMs(10);
  }
  PRINTF("ATTACHED \n\r");*/
  while( 1 )
  {
      DISABLE_IRQ( );

#ifndef LOW_POWER_DISABLE
      LPM_EnterLowPower( );
#endif
      ENABLE_IRQ( );
      DelayMs(10);
      if(!discovered){
			DelayMs(10);
			notDiscovered();
      }
      if(discovered && !registered){
			notRegistered();
			DelayMs(10);
      }
      Radio.Rx( RX_TIMEOUT_VALUE );
  }
}

/*BEGIN LORA FUNCTION*/

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
    //PRINTF("LoRa: OnTxDone\n\r");
}
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
    char * c = (char*)Buffer;
    PRINTF(c);
    PRINTF("\n\r");
    /*Redistribution du message a la bonne fonction */
    char ** result = str_split(c, ',');
    if (strcmp(result[KIND_MSG_LORA],"2")==0 && strcmp(result[ID_DEST_MSG_LORA],LORAWAN_DEVICE_EUI_STRING)==0){

    	/*Reception Accept messge*/
    	PRINTF("Accept message received ");
    	discovered=true;
    	strcpy(MyLoRaGW,result[ID_SRC_MSG_LORA]);
    }
    if (strcmp(result[KIND_MSG_LORA], "4")== 0  && strcmp(result[ID_DEST_MSG_LORA],LORAWAN_DEVICE_EUI_STRING)==0){
    	/*Reception registering or Request Data  phase for me */
    	if(!registered){
    		registered=true;
    	}else{
    		sendData();
    	}

    }
    PRINTF("\n\r");
    PRINTF("LoRa: OnRxDone\n\r");
    PRINTF("LoRa: RssiValue=%d dBm, SnrValue=%d\n\r", rssi, snr);
}
void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;

    PRINTF("LoRa: OnTxTimeout\n\r");
}
void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
    PRINTF("LoRa: OnRxError\n\r");
}
void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
    PRINTF("LoRa: OnRxTimeout\n\r");
}

/*END LORA FUNCTION*/


/* BEGIN PERSONNAL  FUNCTION  */
void switchToLoRa( void ){
	    //LORA_Init( NULL, NULL);
	  	// Radio initialization
		RadioEvents.TxDone = OnTxDone;
		RadioEvents.RxDone = OnRxDone;
		RadioEvents.TxTimeout = OnTxTimeout;
		RadioEvents.RxTimeout = OnRxTimeout;
		RadioEvents.RxError = OnRxError ;

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
}
void sendLoRa(char* data, int length){
	int iter;
	for(iter=0;iter<length;iter++){
		Buffer[iter]=data[iter];
	}
	Radio.Send(Buffer,length);
}
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
void notDiscovered( void ){
	char* name_tmp= "Discover,";
	char* kind_tmp = "1,";
	char* frequency_tmp= "1,";
	char slot_tmp[20];
	itoa(slot,slot_tmp,10);
	char id[20];
	strcpy(id,LORAWAN_DEVICE_EUI_STRING);
	char dest[20];
	strcpy(dest,"None");
	char datas[20];
	itoa(data,datas,10);
	char listening[20];
	itoa(slot*3,listening,10);

	char messageAnswer[512] ;
	memset( messageAnswer, '\0', sizeof(char)*512 );
	strcat(messageAnswer,name_tmp);
	strcat(messageAnswer,kind_tmp);
	strcat(messageAnswer,frequency_tmp);
	strcat(messageAnswer,slot_tmp);
	strcat(messageAnswer,",");
	strcat(messageAnswer,id);
	strcat(messageAnswer,",");
	strcat(messageAnswer,dest);
	strcat(messageAnswer,",");
	strcat(messageAnswer,datas);
	strcat(messageAnswer,",");
	strcat(messageAnswer,listening);
	PRINTF("Send Discover Message");
	PRINTF("\n\r");

	DelayMs( 1 );
	sendLoRa(messageAnswer, strlen(messageAnswer));
	memset( messageAnswer, '\0', sizeof(char)*512 );
}
void notRegistered( void ){
	char* name_tmp= "Register,";
	char* kind_tmp = "3,";
	char* frequency_tmp= "1,";
	char slot_tmp[20];
	itoa(slot,slot_tmp,10);
	char id[20];
	strcpy(id,LORAWAN_DEVICE_EUI_STRING);
	char dest[20];
	strcpy(dest,MyLoRaGW);
	char datas[20];
	itoa(data,datas,10);
	char listening[20];
	itoa(slot*3,listening,10);

	char messageAnswer[512] ;
	memset( messageAnswer, '\0', sizeof(char)*512 );
	strcat(messageAnswer,name_tmp);
	strcat(messageAnswer,kind_tmp);
	strcat(messageAnswer,frequency_tmp);
	strcat(messageAnswer,slot_tmp);
	strcat(messageAnswer,",");
	strcat(messageAnswer,id);
	strcat(messageAnswer,",");
	strcat(messageAnswer,dest);
	strcat(messageAnswer,",");
	strcat(messageAnswer,datas);
	strcat(messageAnswer,",");
	strcat(messageAnswer,listening);
	PRINTF("Send Register Message");
	PRINTF("\n\r");
	DelayMs( 1 );
	sendLoRa(messageAnswer, strlen(messageAnswer));
	memset( messageAnswer, '\0', sizeof(char)*512 );
}
void sendData( void ){
	char* name_tmp= "DataRes,";
	char* kind_tmp = "5,";
	char* frequency_tmp= "1,";
	char slot_tmp[20];
	itoa(slot,slot_tmp,10);
	char id[20];
	strcpy(id,LORAWAN_DEVICE_EUI_STRING);
	char dest[20];
	strcpy(dest,MyLoRaGW);
	char datas[20];
	itoa(data,datas,10);
	char listening[20];
	itoa(slot*3,listening,10);

	char messageAnswer[512] ;
	memset( messageAnswer, '\0', sizeof(char)*512 );
	strcat(messageAnswer,name_tmp);
	strcat(messageAnswer,kind_tmp);
	strcat(messageAnswer,frequency_tmp);
	strcat(messageAnswer,slot_tmp);
	strcat(messageAnswer,",");
	strcat(messageAnswer,id);
	strcat(messageAnswer,",");
	strcat(messageAnswer,dest);
	strcat(messageAnswer,",");
	strcat(messageAnswer,datas);
	strcat(messageAnswer,",");
	strcat(messageAnswer,listening);
	PRINTF("Send Register Message");
	PRINTF("\n\r");
	DelayMs( 1 );
	sendLoRa(messageAnswer, strlen(messageAnswer));
	memset( messageAnswer, '\0', sizeof(char)*512 );
}

void change_frequency(int newFrequency){

}
/* END PERSONNAL  FUNCTION  */
