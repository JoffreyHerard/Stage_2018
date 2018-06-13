/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRaMac classA device implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

/*! \file classA/LoRaMote/main.c */

#include <string.h>
#include <math.h>
#include "board.h"
#include "radio.h"

#include "LoRaMac.h"
#include "Commissioning.h"

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            30000


/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND                        2000

/*!
 * Default datarate
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_0

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG_ON                    true

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON                              1

#if defined( USE_BAND_868 )

#include "LoRaMacTest.h"

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                        true

#define USE_SEMTECH_DEFAULT_CHANNEL_LINEUP          1

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )

#define LC4                { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10               { 868300000, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

#endif

#endif

/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT                            2

//#define RF_FREQUENCY                                868000000 // Hz
#define RF_FREQUENCY                                868000000 // Hz

/*!
 * User application data buffer size
 */
#if defined( USE_BAND_868 )

#define LORAWAN_APP_DATA_SIZE                       16

#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )

#define LORAWAN_APP_DATA_SIZE                       11

#endif

#define TX_OUTPUT_POWER                             20        // dBm

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

/*
// xrange node 1
#define  LORAWAN_DEVICE_EUI {0x08, 0x00, 0xF7,0xFF,0x96,0x4D,0x69,0xB2};
#define  LORAWAN_APPLICATION_EUI {0x70,0xB3,0xD5,0x7E,0xF0,0x00,0x49,0xE1};
#define  LORAWAN_APPLICATION_KEY {0x3E,0x92,0x5C,0xF0,0x0F,0x6A,0xD5,0x0C,0xFA,0xFE,0xD6,0xF2,0x67,0xBB,0x40,0x11};
*/


/*
// xrange node 2
#define  LORAWAN_DEVICE_EUI {0x05,0x00,0xFA,0xFF,0x71,0x62,0x8E,0x9D}
#define  LORAWAN_APPLICATION_EUI {0x70,0xB3,0xD5,0x7E,0xF0,0x00,0x49,0xE1};
#define  LORAWAN_APPLICATION_KEY {0x58,0xA8,0x03,0xF6,0xD4,0xAA,0xE1,0x47,0x38,0x61,0x29,0x66,0x48,0x69,0xCC,0x23};
*/


// xrange node 3 Bouygues
#define  LORAWAN_DEVICE_EUI {0x0B,0x00,0xF4,0xFF,0x6F,0x3E,0x90,0xC1};
//#define  LORAWAN_DEVICE_EUI {0xC1,0x90,0x3E,0x6F,0xFF,0xF4,0x00,0x0B};

#define  LORAWAN_APPLICATION_EUI {0x70,0xB3,0xD5,0x7E,0xF0,0x00,0x49,0xE1};
//#define  LORAWAN_APPLICATION_EUI {0xE1,0x49,0x00,0xF0,0x7E,0xD5,0xB3,0x70};

#define  LORAWAN_APPLICATION_KEY {0x30,0x4C,0x99,0x26,0x3E,0xA5,0xE6,0x43,0xB5,0xA0,0x8C,0xB3,0x25,0x4A,0x61,0xFA};
//#define  LORAWAN_APPLICATION_KEY {0xFA,0x61,0x4A,0x25,0xB3,0x8C,0xA0,0xB5,0x43,0xE6,0xA5,0x3E,0x26,0x99,0x4C,0x30};




static uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

#if( OVER_THE_AIR_ACTIVATION == 0 )

static uint8_t NwkSKey[] = LORAWAN_NWKSKEY;
static uint8_t AppSKey[] = LORAWAN_APPSKEY;

/*!
 * Device address
 */
static uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;

#endif

/*!
 * Application port
 */
static uint8_t AppPort = LORAWAN_APP_PORT;

/*!
 * User application data size
 */
static uint8_t AppDataSize = LORAWAN_APP_DATA_SIZE;

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_MAX_SIZE                           64

/*!
 * User application data
 */
static uint8_t AppData[LORAWAN_APP_DATA_MAX_SIZE];

/*!
 * Indicates if the node is sending confirmed or unconfirmed messages
 */
static uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;

/*!
 * Defines the application data transmission duty cycle
 */
static uint32_t TxDutyCycleTime;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxNextPacketTimer;

/*!
 * Specifies the state of the application LED
 */

/*!
 * Timer to handle the state of LED1
 */
//static TimerEvent_t Led1Timer;

/*!
 * Timer to handle the state of LED2
 */
//static TimerEvent_t Led2Timer;

/*!
 * Indicates if a new packet can be sent
 */
static bool NextTx = true;


// PERSO FLORENT
/*
* To swap message between gateway and end-node
*/
static bool MsgToGW = true; //true : message to gateway, false: to the end-node
//static bool FirstTime = true; // To send only one time the Join message
static bool SwapMsg = true; // To Swap Msg between GW and node; true: message only to either GW if MsgToGW=true, or Node if MsgToGW=false

////////////////////////

/*!
 * Device states
 */
static enum eDeviceState
{
    DEVICE_STATE_INIT,
    DEVICE_STATE_JOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_SLEEP,
    DEVICE_STATE_RELAY,
    DEVICE_STATE_RX
}DeviceState;

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

States_t StateRadio;

void LedOn( Gpio_t led ) // led doit être soit LED1, LED2 ou LED3 (
{
    GpioWrite( &led, 0);
}

void LedOff( Gpio_t led )
{
    GpioWrite( &led, 1);
}

/*void LedToggle( tLed led )
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
*/


void reset_device(void)
{
DeviceState = DEVICE_STATE_INIT;
}
/*!
 * LoRaWAN compliance tests support data
 */
struct ComplianceTest_s
{
    bool Running;
    uint8_t State;
    bool IsTxConfirmed;
    uint8_t AppPort;
    uint8_t AppDataSize;
    uint8_t *AppDataBuffer;
    uint16_t DownLinkCounter;
    bool LinkCheck;
    uint8_t DemodMargin;
    uint8_t NbGateways;
}ComplianceTest;

/*!
 * \brief   Prepares the payload of the frame
 */
static void PrepareTxFrame( uint8_t port, uint8_t value )
{
    //char Volt[5];
    uint16_t voltage;
    uint8_t batlevel;

    voltage=BoardGetBatteryVoltage();
    batlevel=BoardGetBatteryLevel();
    switch( port )
    {
    case 2:
        {

            //sprintf(Volt,"Volt: %i",voltage);
            //UartUsbPutBuffer(&UartUsb,Volt,strlen(Volt)); // Add Florent

            AppData[0] = (TxDutyCycleTime & 0x00FF);
            AppData[1] = (TxDutyCycleTime & 0xFF00) >> 8;
            AppData[2] = (TxDutyCycleTime & 0xFF0000) >> 16;
            AppData[3] = (TxDutyCycleTime & 0xFF000000) >> 24;
            // 32 bits BoardGetBatteryVoltage()
            // 8 bits BoardGetBatteryLevel
            AppData[4] = (voltage && 0x00FF);
            AppData[5] = (voltage & 0xFF00) >> 8;
            AppData[6] = (voltage & 0xFF0000) >> 16;
            AppData[7] = (voltage & 0xFF000000) >> 24;
            AppData[8] = batlevel;
            AppData[9] = value;        }
        break;

    case 224:
        if( ComplianceTest.LinkCheck == true )
        {
            ComplianceTest.LinkCheck = false;
            AppDataSize = 3;
            AppData[0] = 5;
            AppData[1] = ComplianceTest.DemodMargin;
            AppData[2] = ComplianceTest.NbGateways;
            ComplianceTest.State = 1;
        }
        else
        {
            switch( ComplianceTest.State )
            {
            case 4:
                ComplianceTest.State = 1;
                break;
            case 1:
                AppDataSize = 2;
                AppData[0] = ComplianceTest.DownLinkCounter >> 8;
                AppData[1] = ComplianceTest.DownLinkCounter;
                break;
            }
        }
        break;
    default:
        break;
    }
}

/*!
 * \brief   Prepares the payload of the frame
 *
 * \retval  [0: frame could be send, 1: error]
 */
static bool SendFrame( void )
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    if( LoRaMacQueryTxPossible( AppDataSize, &txInfo ) != LORAMAC_STATUS_OK )
    {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
    }
    else
    {
        if( IsTxConfirmed == false )
        {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = AppPort;
            mcpsReq.Req.Unconfirmed.fBuffer = AppData;
            mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
        else
        {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = AppPort;
            mcpsReq.Req.Confirmed.fBuffer = AppData;
            mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Confirmed.NbTrials = 8;
            mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
    }

    //UartUsbPutBuffer(&UartUsb,"Send",15);
    if( LoRaMacMcpsRequest( &mcpsReq ) == LORAMAC_STATUS_OK )
    {
        return false;
    }
    return true;
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    TimerStop( &TxNextPacketTimer );

    mibReq.Type = MIB_NETWORK_JOINED;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK )
    {
        if( mibReq.Param.IsNetworkJoined == true )
        {
            DeviceState = DEVICE_STATE_SEND;
            NextTx = true;
        }
        else
        {
            DeviceState = DEVICE_STATE_JOIN;
        }
    }
}

/*!
 * \brief Function executed on Led 1 Timeout event
 */
/*static void OnLed1TimerEvent( void )
{
    TimerStop( &Led1Timer );
    // Switch LED 1 OFF
    GpioWrite( &Led1, 1 );
}*/

/*!
 * \brief Function executed on Led 2 Timeout event
 */
/*
static void OnLed2TimerEvent( void )
{
    TimerStop( &Led2Timer );
    // Switch LED 2 OFF
    GpioWrite( &Led2, 1 );
}
*/
/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    if( mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
    {
        switch( mcpsConfirm->McpsRequest )
        {
            case MCPS_UNCONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                break;
            }
            case MCPS_CONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                // Check AckReceived
                // Check NbTrials
                break;
            }
            case MCPS_PROPRIETARY:
            {
                break;
            }
            default:
                break;
        }

        // Switch LED 1 ON
        //GpioWrite( &Led1, 0 );
        //TimerStart( &Led1Timer );
    }
    NextTx = true;
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        return;
    }

    switch( mcpsIndication->McpsIndication )
    {
        case MCPS_UNCONFIRMED:
        {
            break;
        }
        case MCPS_CONFIRMED:
        {
            break;
        }
        case MCPS_PROPRIETARY:
        {
            break;
        }
        case MCPS_MULTICAST:
        {
            break;
        }
        default:
            break;
    }

    // Check Multicast
    // Check Port
    // Check Datarate
    // Check FramePending
    // Check Buffer
    // Check BufferSize
    // Check Rssi
    // Check Snr
    // Check RxSlot

    if( ComplianceTest.Running == true )
    {
        ComplianceTest.DownLinkCounter++;
    }

    if( mcpsIndication->RxData == true )
    {
        switch( mcpsIndication->Port )
        {
        case 1: // The application LED can be controlled on port 1 or 2
        case 2:
            if( mcpsIndication->BufferSize == 1 )
            {
//                AppLedStateOn = mcpsIndication->Buffer[0] & 0x01;
//                GpioWrite( &Led3, ( ( AppLedStateOn & 0x01 ) != 0 ) ? 0 : 1 );
            }
            break;
        case 224:
            if( ComplianceTest.Running == false )
            {
                // Check compliance test enable command (i)
                if( ( mcpsIndication->BufferSize == 4 ) &&
                    ( mcpsIndication->Buffer[0] == 0x01 ) &&
                    ( mcpsIndication->Buffer[1] == 0x01 ) &&
                    ( mcpsIndication->Buffer[2] == 0x01 ) &&
                    ( mcpsIndication->Buffer[3] == 0x01 ) )
                {
                    IsTxConfirmed = false;
                    AppPort = 224;
                    AppDataSize = 2;
                    ComplianceTest.DownLinkCounter = 0;
                    ComplianceTest.LinkCheck = false;
                    ComplianceTest.DemodMargin = 0;
                    ComplianceTest.NbGateways = 0;
                    ComplianceTest.Running = true;
                    ComplianceTest.State = 1;

                    MibRequestConfirm_t mibReq;
                    mibReq.Type = MIB_ADR;
                    mibReq.Param.AdrEnable = true;
                    LoRaMacMibSetRequestConfirm( &mibReq );

#if defined( USE_BAND_868 )
                    LoRaMacTestSetDutyCycleOn( false );
#endif

                }
            }
            else
            {
                ComplianceTest.State = mcpsIndication->Buffer[0];
                switch( ComplianceTest.State )
                {
                case 0: // Check compliance test disable command (ii)
                    IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
                    AppPort = LORAWAN_APP_PORT;
                    AppDataSize = LORAWAN_APP_DATA_SIZE;
                    ComplianceTest.DownLinkCounter = 0;
                    ComplianceTest.Running = false;

                    MibRequestConfirm_t mibReq;
                    mibReq.Type = MIB_ADR;
                    mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                    LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( USE_BAND_868 )
                    LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif

                    break;
                case 1: // (iii, iv)
                    AppDataSize = 2;
                    break;
                case 2: // Enable confirmed messages (v)
                    IsTxConfirmed = true;
                    ComplianceTest.State = 1;
                    break;
                case 3:  // Disable confirmed messages (vi)
                    IsTxConfirmed = false;
                    ComplianceTest.State = 1;
                    break;
                case 4: // (vii)
                    AppDataSize = mcpsIndication->BufferSize;

                    AppData[0] = 4;
                    for( uint8_t i = 1; i < AppDataSize; i++ )
                    {
                        AppData[i] = mcpsIndication->Buffer[i] + 1;
                    }
                    break;
                case 5: // (viii)
                    {
                        MlmeReq_t mlmeReq;
                        mlmeReq.Type = MLME_LINK_CHECK;
                        LoRaMacMlmeRequest( &mlmeReq );
                    }
                    break;
                case 6: // (ix)
                    {
                        MlmeReq_t mlmeReq;

                        // Disable TestMode and revert back to normal operation
                        IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
                        AppPort = LORAWAN_APP_PORT;
                        AppDataSize = LORAWAN_APP_DATA_SIZE;
                        ComplianceTest.DownLinkCounter = 0;
                        ComplianceTest.Running = false;

                        MibRequestConfirm_t mibReq;
                        mibReq.Type = MIB_ADR;
                        mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                        LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( USE_BAND_868 )
                        LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif


                        mlmeReq.Type = MLME_JOIN;

                        mlmeReq.Req.Join.DevEui = DevEui;
                        mlmeReq.Req.Join.AppEui = AppEui;
                        mlmeReq.Req.Join.AppKey = AppKey;
                        mlmeReq.Req.Join.NbTrials = 3;

                        LoRaMacMlmeRequest( &mlmeReq );
                        DeviceState = DEVICE_STATE_SLEEP;
                    }
                    break;
                case 7: // (x)
                    {
                        if( mcpsIndication->BufferSize == 3 )
                        {
                            MlmeReq_t mlmeReq;
                            mlmeReq.Type = MLME_TXCW;
                            mlmeReq.Req.TxCw.Timeout = ( uint16_t )( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
                            LoRaMacMlmeRequest( &mlmeReq );
                        }
                        ComplianceTest.State = 1;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            break;
        }
    }

    // Switch LED 2 ON for each received downlink
   // GpioWrite( &Led2, 0 );
    // TimerStart( &Led2Timer );
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    switch( mlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                // Status is OK, node has joined the network
                DeviceState = DEVICE_STATE_SEND;
            }
            else
            {
                // Join was not successful. Try to join again
                DeviceState = DEVICE_STATE_JOIN;
            }
            break;
        }
        case MLME_LINK_CHECK:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                // Check DemodMargin
                // Check NbGateways
                if( ComplianceTest.Running == true )
                {
                    ComplianceTest.LinkCheck = true;
                    ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
                    ComplianceTest.NbGateways = mlmeConfirm->NbGateways;
                }
            }
            break;
        }
        default:
            break;
    }
    NextTx = true;
}

//Modification F. NOLOT
// Variable déclaré maintenant en global dans LoRaMac.c
//static RadioEvents_t RadioEvents;

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


int main( void )
{
    LoRaMacPrimitives_t LoRaMacPrimitives;
    LoRaMacCallback_t LoRaMacCallbacks;
    MibRequestConfirm_t mibReq;
    BoardInitMcu( );
    BoardInitPeriph( );

    DeviceState = DEVICE_STATE_INIT;

    uint8_t cnt=1; // Compteur pour envoyer des messages différents à chaque fois

    //RadioEvents.RxDone = OnRxDone; // Pour la liaison avec le noeud non joignable

    while( 1 )
    {
        switch( DeviceState )
        {
            case DEVICE_STATE_INIT:
            {
                LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
                LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
                LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
                LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
                LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks );
                TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );

/*                TimerInit( &Led1Timer, OnLed1TimerEvent );
                TimerSetValue( &Led1Timer, 25 );

                TimerInit( &Led2Timer, OnLed2TimerEvent );
                TimerSetValue( &Led2Timer, 25 );
*/
                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_PUBLIC_NETWORK;
                mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
                LoRaMacMibSetRequestConfirm( &mibReq );

#if defined( USE_BAND_868 )
                LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )
                LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4 );
                LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5 );
                LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6 );
                LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7 );
                LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8 );
                LoRaMacChannelAdd( 8, ( ChannelParams_t )LC9 );
                LoRaMacChannelAdd( 9, ( ChannelParams_t )LC10 );

                mibReq.Type = MIB_RX2_DEFAULT_CHANNEL;
                mibReq.Param.Rx2DefaultChannel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_RX2_CHANNEL;
                mibReq.Param.Rx2Channel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
                LoRaMacMibSetRequestConfirm( &mibReq );
#endif

#endif
                DeviceState = DEVICE_STATE_JOIN;

                break;
            }
            case DEVICE_STATE_JOIN:
            {
#if( OVER_THE_AIR_ACTIVATION != 0 )

                MlmeReq_t mlmeReq;

                // Initialize LoRaMac device unique ID
              // BoardGetUniqueId( DevEui );

                mlmeReq.Type = MLME_JOIN;

                mlmeReq.Req.Join.DevEui = DevEui;
                mlmeReq.Req.Join.AppEui = AppEui;
				mlmeReq.Req.Join.AppKey = AppKey;

                mlmeReq.Req.Join.NbTrials = 3;

                if( NextTx == true )
                {
                    LoRaMacMlmeRequest( &mlmeReq );
                }
                DeviceState = DEVICE_STATE_SLEEP;
#else

                // Choose a random device address if not already defined in Commissioning.h
                if( DevAddr == 0 ) {
                    // Random seed initialization
                    srand1( BoardGetRandomSeed( ) );

                    // Choose a random device address
                    DevAddr = randr( 0, 0x01FFFFFF );
                }

                mibReq.Type = MIB_NET_ID;
                mibReq.Param.NetID = LORAWAN_NETWORK_ID;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_DEV_ADDR;
                mibReq.Param.DevAddr = DevAddr;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NWK_SKEY;
				mibReq.Param.NwkSKey = NwkSKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_APP_SKEY;
				mibReq.Param.AppSKey =  AppSKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NETWORK_JOINED;
                mibReq.Param.IsNetworkJoined = true;
                LoRaMacMibSetRequestConfirm( &mibReq );

                DeviceState = DEVICE_STATE_SEND;

#endif

             //   FirstTime = false;

              break;
            }
            case DEVICE_STATE_SEND:
            {

                if( ComplianceTest.Running == true )
                {
                    // Schedule next packet transmission
                    TxDutyCycleTime = 5000; // 5000 ms
                }
                else
                {
                    // Schedule next packet transmission
                    TxDutyCycleTime = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
                }

               /* LedOn(Led1);
                LedOff(Led1);*/
                if( NextTx == true )
                {
//                    PrepareTxFrame( AppPort );
                    if ( MsgToGW == true) {
                        // LoraMac_radio_event_init();
                        PrepareTxFrame( 2 ,cnt );
                        cnt++;
                        NextTx = SendFrame( ); //Send DutyCycle, BatteryVoltage and BatteryLevel ...
                        DeviceState = DEVICE_STATE_CYCLE;
                     }
                    else { // Echange avec le noeud non joignable

                    /*RadioEvents.RxDone = OnRxDone; // Pour la liaison avec le noeud non joignable
                    RadioEvents.TxDone = OnTxDone;
                    RadioEvents.TxTimeout = OnTxTimeout;
                    RadioEvents.RxTimeout = OnRxTimeout;
                    RadioEvents.RxError = OnRxError;
*/
//Avec cette partie ci-dessous, non commentée, j'ai bien le TxDone et RxDone OK
//                     Radio.Init( &RadioEvents );

                    Radio.SetChannel( RF_FREQUENCY );
                    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

                    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
                    DelayMs(100);
                    PrepareTxFrame( 2 ,cnt );
                    Radio.Send( AppData, LORAWAN_APP_DATA_SIZE );
                    DelayMs(100);
                    cnt++;
             //       if (StateRadio == RX )
                    NextTx=true;
                    //Delay(10);

                    }

                    if (SwapMsg==true)
                        MsgToGW = MsgToGW^true;

                  //  UartUsbPutBuffer(&UartUsb,"OK2\n\r",15);

                    //UartUsbPutBuffer(&UartUsb,AppData,15);
                }

                break;
            }

            case DEVICE_STATE_RX:
            {
             Radio.Rx( 10000 );
             DelayMs(100);
/*             AppData[10]=0xFF;
             AppData[11]=0xFF;
             */
             DeviceState = DEVICE_STATE_CYCLE;
                break;
            }
            case DEVICE_STATE_CYCLE:
            {
                DeviceState = DEVICE_STATE_SLEEP;

                // Schedule next packet transmission
                TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
                TimerStart( &TxNextPacketTimer );
                break;
            }
            case DEVICE_STATE_SLEEP:
            {
                // Wake up through events
                TimerLowPowerHandler( );

//                cli_process();
                break;
            }

            default:
            {
                DeviceState = DEVICE_STATE_INIT;
                break;
            }
        }
    }
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    AppDataSize = size;
    AppData[10]=payload[10]; //RxTxBuffer dans le fichier sx1272
//    memcpy( AppData, payload, AppDataSize );
    AppData[11] = rssi;
    //SnrValue = snr;
    DeviceState = DEVICE_STATE_SEND;
    //UartUsbPutBuffer(&UartUsb,"\r\RX done",15);
}

void OnTxDone( void )
{
    Radio.Sleep( );
    DeviceState = DEVICE_STATE_RX;
    //UartUsbPutBuffer(&UartUsb,"\r\nTX done",15);

}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    //UartUsbPutBuffer(&UartUsb,"\r\nTX TIMEOUT",15);
   // State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    //UartUsbPutBuffer(&UartUsb,(unsigned char*)"\r\nRX TIMEOUT",11);
    //State = RX_TIMEOUT;
}

void OnRxError( void )
{
    Radio.Sleep( );
    //UartUsbPutBuffer(&UartUsb,(unsigned char*)"\r\nRX ERROR",11);
    //State = RX_ERROR;
}
