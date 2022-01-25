#ifndef ZOPTIONS_H
#define ZOPTIONS_H

#define ZMODEM_VERSION "1.0.1"

#define PIN_FACTORY_RESET GPIO_NUM_0
#define PIN_LED_HS GPIO_NUM_2
#define PIN_LED_WIFI GPIO_NUM_22
#define PIN_LED_DATA GPIO_NUM_33
#define PIN_DCD GPIO_NUM_14
#define PIN_CTS GPIO_NUM_13
#define PIN_RTS GPIO_NUM_15 // unused
#define PIN_RI GPIO_NUM_32
#define PIN_DSR GPIO_NUM_12
#define PIN_DTR GPIO_NUM_27

#define DEFAULT_BAUD_RATE 1200
#define DEFAULT_HS_RATE 38400
#define DEFAULT_SERIAL_CONFIG SERIAL_8N1
#define DEFAULT_TERMTYPE "ZTerm"
#define ESCAPE_BUF_LEN 10

#endif