#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#include "DMD_RGB.h"
#include <stdio.h>

#include "fonts/FONT8.h"
#include "fonts/FONT10.h"
#include "fonts/FONT12.h"
#include "fonts/FONT14.h"
#include "fonts/FONT16.h"
#include "fonts/FONT18.h"
#include "fonts/FONT20.h"
#include "fonts/FONT22.h"
#include "fonts/FONT24.h"
#include "fonts/FONT26.h"
#include "fonts/FONT28.h"
#include "fonts/FONT30.h"
#include "fonts/FONT32.h"
#include "fonts/FONT34.h"
#include "fonts/FONT36.h"
#include "fonts/FONT38.h"
#include "fonts/FONT40.h"
#include "fonts/FONT42.h"
#include "fonts/FONT44.h"
#include "fonts/FONT46.h"
#include "fonts/FONT48.h"
#include "fonts/FONT50.h"
#include "fonts/FONT52.h"
#include "fonts/FONT54.h"
#include "fonts/FONT56.h"
#include "fonts/FONT58.h"
#include "fonts/FONT60.h"
#include "fonts/FONT62.h"
#include "fonts/FONT64.h"
#include "fonts/FONT66.h"
#include "fonts/FONT68.h"
#include "fonts/FONT70.h"
#include "fonts/FONT86.h"
#include "fonts/FONT95.h"

#include "fonts/Bahnschrift_Condensed30_b_033_126.h"
#include "fonts/Bahnschrift_Condensed36_b_037_057.h"
#include "fonts/Microsoft_Sans_Serif84_033_126.h"
#include "st_fonts/SystemFont5x7.h"

DMD_RGB <PANELTYPE, PANELCOLORMODE> PANEL(
  addressMuxList,
  PIN_nOE,
  PIN_SCLK,
  RGBPinsList,
  DISPLAYS_ACROSS,
  DISPLAYS_DOWN,
  ENABLE_DUAL_BUFFER);

DMD_Standard_Font font1(FONT8);
DMD_Standard_Font font2(FONT10);
DMD_Standard_Font font3(FONT12);
DMD_Standard_Font font4(FONT14);
DMD_Standard_Font font5(FONT16);
DMD_Standard_Font font6(FONT18);
DMD_Standard_Font font7(FONT20);
DMD_Standard_Font font8(FONT22);
DMD_Standard_Font font9(FONT24);
DMD_Standard_Font font10(FONT26);
DMD_Standard_Font font11(FONT28);
DMD_Standard_Font font12(FONT30);
DMD_Standard_Font font13(FONT32);
DMD_Standard_Font font14(FONT34);
DMD_Standard_Font font15(FONT36);
DMD_Standard_Font font16(FONT38);
DMD_Standard_Font font17(FONT40);
DMD_Standard_Font font18(FONT42);
DMD_Standard_Font font19(FONT44);
DMD_Standard_Font font20(FONT46);
DMD_Standard_Font font21(FONT48);
DMD_Standard_Font font22(FONT50);
DMD_Standard_Font font23(FONT52);
DMD_Standard_Font font24(FONT54);
DMD_Standard_Font font25(FONT56);
DMD_Standard_Font font26(FONT58);
DMD_Standard_Font font27(FONT60);
DMD_Standard_Font font28(FONT62);
DMD_Standard_Font font29(FONT64);
DMD_Standard_Font font30(FONT66);
DMD_Standard_Font font31(FONT68);
DMD_Standard_Font font32(FONT70);
DMD_Standard_Font font33(FONT86);
DMD_Standard_Font font34(FONT95);


DMD_Standard_Font fontHeadline(Bahnschrift_Condensed30_b_033_126);
DMD_Standard_Font font36(Bahnschrift_Condensed36_b_037_057);
DMD_Standard_Font font84(Microsoft_Sans_Serif84_033_126);

const uint16_t* bitmaps[] = {flag};

struct adc {
    volatile uint32_t CS, RESULT,
             FCS, FIFO, DIV, INTR, INTE, INTF,
             INTS;
};
#define ADC ((struct adc*)0x4004C000)

void setup(){
  Serial.begin(115200);
  Serial1.begin(115200);
}
void loop(){
  
}