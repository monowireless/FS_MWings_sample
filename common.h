#pragma once

// load Arduino.h first
#ifdef ARDUINO
#include <Arduino.h> // explicitly include this for code analysis in IDE (like VSCode)
#endif
#include <MWings.h> // MW Wings Paser Library

//// CONFIGURATIONS
// DEBUG_UNUSE_AZURE : if defined, remove Azure code.
#define DEBUG_UNUSE_AZURE // define to skip Azure code.

//// SELECT BOARD (Choose ONE)
#define BRD_DEFAULT
// #define BRD_WIONODE
// #define BRD_TWELITE_SPOT

// default board
#if defined(BRD_DEFAULT)
#if defined(ESP8266)
#define BRD_WIONODE
#elif defined(ESP32)
#define BRD_TWELITE_SPOT
#endif
#endif

//// OTFFT DATYTYPE
using REAL = float; // double is not supported on ESP32

//// Prototypes
void on_pkt_ARIA(const ParsedAriaPacket& packet);
void on_pkt_CUE(const ParsedCuePacket& packet);
void on_pkt_PAL_AMB(const ParsedPalAmbPacket& packet);
void on_pkt_PAL_MOT(const ParsedPalMotPacket& packet);

void print_div100(uint16_t val);
void print_div100(int16_t val);

void azureCallback(String s);

//// for ESP8266 and WIONODE
#if defined(BRD_WIONODE)
#define ARCH_ESP8266 // ESP8266 (defined in Arduino board support)
// port defs for WIONODE
const uint8_t PORT0A = 1;
const uint8_t PORT0B = 3;
const uint8_t PORT1A = 4;
const uint8_t PORT1B = 5;
const uint8_t PORT_POWER = 15; // (common with RED_LED)

const uint8_t FUNC_BTN = 0;
const uint8_t BLUE_LED = 2;
const uint8_t RED_LED = PORT_POWER;

const uint8_t UART_TX = PORT0A;
const uint8_t UART_RX = PORT0B;
const uint8_t I2C_SDA = PORT1A;
const uint8_t I2C_SCL = PORT1B;
#endif

#if defined(ESP8266)
#if (defined(ARDUINO_ESP8266_MAJOR) && ARDUINO_ESP8266_MAJOR <= 2)
// Serial.setRxBufferSize() is not supported.
#define NO_SERIAL_RX_BUFFER
#endif
#endif

//// FOR ESP32
#if defined(BRD_TWELITE_SPOT)
#define ARCH_ESP32
const uint8_t PORT_TWE_RST = 5;
const uint8_t PORT_TWE_PGM = 4;
const uint8_t BLUE_LED = 18;
#endif
