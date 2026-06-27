#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic warning "-Wnarrowing"
#pragma GCC diagnostic warning "-Woverflow"

/*
   PIN DEFINES FOR PANEL
*/
#define PIN_A 6
#define PIN_B 7
#define PIN_C 8
#define PIN_D 9
#define PIN_E -1
#define PIN_nOE 10
#define PIN_SCLK 12
uint8_t addressMuxList[] = { PIN_A , PIN_B , PIN_C };
uint8_t RGBPinsList[] = { 11, 0, 1, 2, 3, 4, 5 }; // CLK, R0, G0, B0, R1, G1, B1

/*
 * Panel Specifications
 * Number of Panels in X Direction
 * Number of Panels in Y Direction
 * Dual Buffering Enable/Disable (Use swapBuffers(true) command)
 * 
 * Type of Panel Being Used
 * Colour Mode Used
 * Maximum Brightness
 * Minimum Brightness
 * 
 */
 
#define DISPLAYS_ACROSS 4
#define DISPLAYS_DOWN 4
#define ENABLE_DUAL_BUFFER false
#define PANELTYPE RGB32x32_S8_maxmurugan //RGB32x32plainS16
#define PANELCOLORMODE COLOR_4BITS
#define MaxPanelBrightness 255
#define MinPanelBrightness 0

/*
 * COLOUR DEFINES
 */
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFA00
#define WHITE    0xFFFF
#define ORANGE   0xFCBC

/*
   VSDS PDF color codes (Artistic Infratech protocol)
   1=RED 2=GREEN 3=YELLOW 4=BLUE 5=MAGENTA 6=CYAN 7=WHITE
*/
#define VSDS_COLOR_RED     1
#define VSDS_COLOR_GREEN   2
#define VSDS_COLOR_YELLOW  3
#define VSDS_COLOR_BLUE    4
#define VSDS_COLOR_MAGENTA 5
#define VSDS_COLOR_CYAN    6
#define VSDS_COLOR_WHITE   7

#define ETHERNET_SS_PIN 17
#define W5500_RST_PIN 24
#define W5500_SCK_PIN 18
#define W5500_MOSI_PIN 19
#define W5500_MISO_PIN 16
#define HeartBeatLed 22
#define HARDRST 27

/*
   USR-K5 UART (Serial1 on Pico)
   TX = GPIO 20, RX = GPIO 21
*/
#define USR_K5_TX_PIN 20
#define USR_K5_RX_PIN 21
#define USR_K5_BAUDRATE 115200

/*
   ETHERNET CONFIGS (W5500)
*/

#define SERVER_PORT 23
byte SERVER_MAC[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
const int ip_array[4] = {192, 168, 0, 7};

/*
   SERIAL CONFIGS
*/
#define SERIAL_BAUDRATE 115200
#define SERIAL_INTERFACE Serial
#define COM_PORT Serial1



#endif
