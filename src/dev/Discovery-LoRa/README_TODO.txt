
PENSEZ À MODIFIER DANS LE FICHIER LORA.C LE DEFINE DU DUTYCYLE A METTRE A FALSE !!!!!
#if defined( REGION_EU868 )

#include "LoRaMacTest.h"

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                        false

#endif
