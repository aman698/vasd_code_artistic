#include <SPI.h>
#include <HardwareSerial.h>
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

const uint16_t* bitmaps[] = {flag};


struct adc {
  volatile uint32_t CS, RESULT,
           FCS, FIFO, DIV, INTR, INTE, INTF,
           INTS;
};
#define ADC ((struct adc*)0x4004C000)

EthernetClient client;
EthernetServer server = EthernetServer(SERVER_PORT);
String  AliveN = "START,ALIVE,";

enum CommChannel { CHANNEL_NONE, CHANNEL_ETH, CHANNEL_SERIAL };
CommChannel activeChannel = CHANNEL_NONE;
String commandLineBuffer = "";
CommChannel commandLineChannel = CHANNEL_NONE;

uint16_t vsdsColorToPixel(int colorCode) {
  switch (colorCode) {
    case VSDS_COLOR_RED: return RED;
    case VSDS_COLOR_GREEN: return GREEN;
    case VSDS_COLOR_YELLOW: return YELLOW;
    case VSDS_COLOR_BLUE: return BLUE;
    case VSDS_COLOR_MAGENTA: return MAGENTA;
    case VSDS_COLOR_CYAN: return CYAN;
    case VSDS_COLOR_WHITE: return WHITE;
    default: return WHITE;
  }
}

void w5500Reset() {
  pinMode(W5500_RST_PIN, OUTPUT);
  digitalWrite(W5500_RST_PIN, LOW);
  delay(10);
  digitalWrite(W5500_RST_PIN, HIGH);
  delay(100);
}

void replyPrint(const String& msg) {
  if (activeChannel == CHANNEL_ETH && client.connected()) {
    client.print(msg);
  } else if (activeChannel == CHANNEL_SERIAL) {
    Serial1.print(msg);
  }
}

void replyPrintln(const String& msg) {
  if (activeChannel == CHANNEL_ETH && client.connected()) {
    client.println(msg);
  } else if (activeChannel == CHANNEL_SERIAL) {
    Serial1.println(msg);
  }
}

void replyPrint(int val) {
  replyPrint(String(val));
}

void replyPrintln(int val) {
  replyPrintln(String(val));
}

void replyPrintln(float val) {
  replyPrintln(String(val));
}

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

float getTemperature();
void HeartBeat();
void checkHardReset();
String getMessageEthernet();
String getMessageSerial1(bool raw = false);
String pollCompleteCommand();
String readIncomingData();
bool isVsdsProtocol(const String& str);
int parseVsdsCommand(String str);
int parseCommand(String str);
void printAlive();
void writeToDisplay(int x, int y, String text, int fontColour, int fontSize);
void writeToDisplayVsds(int x, int y, String text, int colorCode, int fontSize);
void clearVsdsText(int x, int y, String text, int fontSize);
void drawImage(int imageSize, int startX, int startY, int x, int y);
void clearDisplay(int x1, int x2, int y1, int y2);

void selectDisplayFont(int fontSize) {
  switch (fontSize) {
    case 1: PANEL.selectFont(&font1); break;
    case 2: PANEL.selectFont(&font2); break;
    case 3: PANEL.selectFont(&font3); break;
    case 4: PANEL.selectFont(&font4); break;
    case 5: PANEL.selectFont(&font5); break;
    case 6: PANEL.selectFont(&font6); break;
    case 7: PANEL.selectFont(&font7); break;
    case 8: PANEL.selectFont(&font8); break;
    case 9: PANEL.selectFont(&font9); break;
    case 10: PANEL.selectFont(&font10); break;
    case 11: PANEL.selectFont(&font11); break;
    case 12: PANEL.selectFont(&font12); break;
    case 13: PANEL.selectFont(&font13); break;
    case 14: PANEL.selectFont(&font14); break;
    case 15: PANEL.selectFont(&font15); break;
    case 16: PANEL.selectFont(&font16); break;
    case 17: PANEL.selectFont(&font17); break;
    case 18: PANEL.selectFont(&font18); break;
    case 19: PANEL.selectFont(&font19); break;
    case 20: PANEL.selectFont(&font20); break;
    case 21: PANEL.selectFont(&font21); break;
    case 22: PANEL.selectFont(&font22); break;
    case 23: PANEL.selectFont(&font23); break;
    case 24: PANEL.selectFont(&font24); break;
    case 25: PANEL.selectFont(&font25); break;
    case 26: PANEL.selectFont(&font26); break;
    case 27: PANEL.selectFont(&font27); break;
    case 28: PANEL.selectFont(&font28); break;
    case 29: PANEL.selectFont(&font29); break;
    case 30: PANEL.selectFont(&font30); break;
    case 31: PANEL.selectFont(&font31); break;
    case 32: PANEL.selectFont(&font32); break;
    case 33: PANEL.selectFont(&font84); break;
    case 34: PANEL.selectFont(&font34); break;
    default: PANEL.selectFont(&font7); break;
  }
}

void vsdsAdjustTextX(const String& text, int& x) {
  if (text.length() == 1 && text.charAt(0) >= '0' && text.charAt(0) <= '9') {
    x += 15;
  }
}

void writeToDisplayVsds(int x, int y, String text, int colorCode, int fontSize) {
  vsdsAdjustTextX(text, x);
  selectDisplayFont(fontSize);
  uint16_t pixelColor = vsdsColorToPixel(colorCode);
  char buf[100];
  text.toCharArray(buf, sizeof(buf));
  PANEL.setTextColor(pixelColor, BLACK);
  PANEL.drawStringX(x, y, buf, pixelColor, 0);
}

void clearVsdsText(int x, int y, String text, int fontSize) {
  selectDisplayFont(fontSize);
  char buf[100];
  text.toCharArray(buf, sizeof(buf));
  PANEL.setTextColor(BLACK, BLACK);
  PANEL.drawStringX(x, y, buf, BLACK, 0);
}

void clearDisplay(int x1, int y1, int x2, int y2) {
  int i = x1;
  while (y1 < y2) {
    while (i < x2) {
      PANEL.drawPixel(i, y1, BLACK);
      i++;
    }
    y1++;
    i = x1;
  }
}
void writeToDisplay(int x, int y, String text, int fontColour, int fontSize) {
  selectDisplayFont(fontSize);
  char buf[100];
  text.toCharArray(buf, sizeof(buf));
  int fclr = 0;

  switch (fontColour) {
    case 1: fclr = BLACK; break;
    case 2: fclr = BLUE; break;
    case 3: fclr = RED; break;
    case 4: fclr = GREEN; break;
    case 5: fclr = CYAN; break;
    case 6: fclr = MAGENTA; break;
    case 7: fclr = YELLOW; break;
    case 8: fclr = WHITE; break;
    case 9: fclr = ORANGE; break;
    default: fclr = fontColour; break;
  }

  PANEL.setTextColor(fclr, BLACK);
  PANEL.drawStringX(x, y, buf, fclr, 0);
}

void Headline() {
  PANEL.selectFont(&fontHeadline);
  PANEL.setTextColor(YELLOW, BLACK);
  PANEL.drawStringX(6, 2, "SPEED-KMph", YELLOW, 0);
  PANEL.swapBuffers(true);
}

void displaySpeed(int vehicleSpeed, int limit) {
  PANEL.clearScreen(true);
  Headline();
  PANEL.selectFont(&font33);

  int n = vehicleSpeed, digitCount = 0;
  do {
    n /= 10;
    ++digitCount;
  } while (n != 0);

  int x = 0, y = 0;
  n = vehicleSpeed;
  char s[20];
  sprintf (s, "%ld", n);
  int first_digit = s[0] - '0';

  if (digitCount == 1) {
    x = 45; y = 28;
  }
  else if (digitCount == 2) {
    x = 25; y = 28;
  }
  else if (digitCount == 3) {
    if (first_digit == 1) {
      x = 15; y = 28;
    } else if (first_digit > 1) {
      x = 5; y = 28;
    }
  }

  char sp[10] = "";
  sprintf(sp, "%d" , vehicleSpeed);
  if (limit == 1)
  {
    PANEL.setTextColor(GREEN, BLACK);
    PANEL.drawStringX(x, y, sp, GREEN, 0);
    PANEL.selectFont(&fontHeadline);
    PANEL.setTextColor(GREEN, BLACK);
    PANEL.drawStringX(6, 100, "UNDERSPEED", GREEN, 0);
    PANEL.swapBuffers(true);

  }
  else if (limit == 0) {
    PANEL.setTextColor(RED, BLACK);
    PANEL.drawStringX(x, y, sp, RED, 0);
    PANEL.selectFont(&fontHeadline);
    PANEL.setTextColor(RED, BLACK);
    PANEL.drawStringX(10, 100, "OVERSPEED", RED, 0);
    PANEL.swapBuffers(true);
  }
}

void checkHardReset() {
  static unsigned long st_time = millis();
  if (digitalRead(HARDRST) == LOW) {
    if (millis() - st_time > 10000) {
      if (digitalRead(HARDRST) == LOW) {
        EEPROM.write(0, 192); EEPROM.commit();
        EEPROM.write(1, 168); EEPROM.commit();
        EEPROM.write(2, 0); EEPROM.commit();
        EEPROM.write(3, 7); EEPROM.commit();
        EEPROM.write(5, 255); EEPROM.commit();
        NVIC_SystemReset();
      }
    }
  }
}

String getMessageEthernet() {
  String str = "";
  client = server.available();
  if (!client) {
    return str;
  }
  activeChannel = CHANNEL_ETH;
  while (client.available() > 0) {
    str += (char)client.read();
  }
  return str;
}

String getMessageSerial1(bool raw) {
  String str = "";
  while (Serial1.available()) {
    char ch = Serial1.read();
    if (!raw && ch == '\r') {
      continue;
    }
    if (!raw && ch == '\n') {
      break;
    }
    str += ch;
  }
  if (str.length() > 0) {
    activeChannel = CHANNEL_SERIAL;
  }
  return str;
}

void feedCommandChar(CommChannel channel, char ch) {
  if (commandLineBuffer.length() == 0) {
    commandLineChannel = channel;
  } else if (commandLineChannel != channel) {
    return;
  }

  activeChannel = channel;
  if (ch == '\r') {
    return;
  }
  if (ch == '\n') {
    return;
  }
  commandLineBuffer += ch;
}

String pollCompleteCommand() {
  client = server.available();
  if (client) {
    while (client.available() > 0) {
      char ch = client.read();
      if (ch == '\r') {
        continue;
      }
      if (ch == '\n') {
        if (commandLineBuffer.length() > 0 && commandLineChannel == CHANNEL_ETH) {
          String complete = commandLineBuffer;
          commandLineBuffer = "";
          activeChannel = CHANNEL_ETH;
          return complete;
        }
        continue;
      }
      feedCommandChar(CHANNEL_ETH, ch);
    }
  }

  while (Serial1.available()) {
    char ch = Serial1.read();
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      if (commandLineBuffer.length() > 0 && commandLineChannel == CHANNEL_SERIAL) {
        String complete = commandLineBuffer;
        commandLineBuffer = "";
        activeChannel = CHANNEL_SERIAL;
        return complete;
      }
      continue;
    }
    feedCommandChar(CHANNEL_SERIAL, ch);
  }

  if (commandLineBuffer.length() > 0) {
    bool ethPending = client && client.available() > 0;
    bool serPending = Serial1.available() > 0;
    if (!ethPending && !serPending) {
      bool vsds = isVsdsProtocol(commandLineBuffer);
      bool legacy = !vsds && commandLineBuffer.indexOf(',') > 0;
      if (vsds || legacy) {
        String complete = commandLineBuffer;
        commandLineBuffer = "";
        activeChannel = commandLineChannel;
        return complete;
      }
    }
  }

  return "";
}

String readIncomingData() {
  if (activeChannel == CHANNEL_ETH) {
    return getMessageEthernet();
  }
  if (activeChannel == CHANNEL_SERIAL) {
    return getMessageSerial1(true);
  }
  return "";
}

String trimMessage(String str) {
  str.trim();
  while (str.endsWith("\r") || str.endsWith("\n")) {
    str.remove(str.length() - 1);
  }
  return str;
}

bool getVsdsField(const String& str, int fieldIndex, String& result) {
  int current = -1;
  int start = -1;
  for (unsigned i = 0; i <= str.length(); i++) {
    if (i == str.length() || str.charAt(i) == '|') {
      if (start >= 0) {
        current++;
        if (current == fieldIndex) {
          result = str.substring(start, i);
          return true;
        }
      }
      start = -1;
    } else if (start < 0) {
      start = i;
    }
  }
  return false;
}

bool parseVsdsPosition(const String& pos, int& x, int& y) {
  int dash = pos.indexOf('-');
  if (dash < 0) {
    return false;
  }
  x = pos.substring(0, dash).toInt();
  y = pos.substring(dash + 1).toInt();
  return true;
}

bool isVsdsProtocol(const String& str) {
  String trimmed = trimMessage(str);
  return trimmed.startsWith("|") && trimmed.indexOf('|', 1) > 0;
}

int parseVsdsCommand(String str) {
  str = trimMessage(str);

  String cmd, pos, text, colorStr, fontStr, effectStr, reserveStr;
  if (!getVsdsField(str, 0, cmd)) return -1;
  if (!getVsdsField(str, 1, pos)) return -1;
  if (!getVsdsField(str, 2, text)) return -1;
  if (!getVsdsField(str, 3, colorStr)) return -1;
  if (!getVsdsField(str, 4, fontStr)) return -1;
  if (!getVsdsField(str, 5, effectStr)) return -1;
  if (!getVsdsField(str, 6, reserveStr)) return -1;

  int x = 0, y = 0;
  if (!parseVsdsPosition(pos, x, y)) return -1;

  int colorCode = colorStr.toInt();
  int fontSize = fontStr.toInt();
  int textEffect = effectStr.toInt();
  int textReserve = reserveStr.toInt();

  if (textEffect != 1 || textReserve != 0) {
    return -1;
  }

  if (cmd == "T") {
    writeToDisplayVsds(x, y, text, colorCode, fontSize);
    replyPrintln("OK");
    return 0;
  }

  if (cmd == "C") {
    if (text == "CLEAR" && pos == "0-0" && colorCode == 0 && fontSize == 0) {
      PANEL.clearScreen(true);
      replyPrintln("OK");
      return 0;
    }
    clearVsdsText(x, y, text, fontSize);
    replyPrintln("OK");
    return 0;
  }

  replyPrintln("ERR");
  return -1;
}

int parseCommand(String str) {
  str = trimMessage(str);
  if (str == "") {
    return -1;
  }
  if (isVsdsProtocol(str)) {
    return parseVsdsCommand(str);
  }

  if (str != "") {
    int commaIndex = str.indexOf(',');
    int secondCommaIndex = str.indexOf(',', commaIndex + 1);
    String firstValue = str.substring(0, commaIndex);
    String secondValue = str.substring(commaIndex + 1);

    if (firstValue == "SET") {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);
      int fourthCommaIndex1 = str.indexOf(',', thirdCommaIndex1 + 1);
      int fifthCommaIndex1 = str.indexOf(',', fourthCommaIndex1 + 1);

      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);
      String thirdValue1 = str.substring(secondCommaIndex1 + 1, thirdCommaIndex1);
      String fourthValue1 = str.substring(thirdCommaIndex1 + 1, fourthCommaIndex1);
      String fifthValue1  = str.substring(fourthCommaIndex1 + 1, fifthCommaIndex1);

      EEPROM.write(0, secondValue1.toInt()); EEPROM.commit();
      EEPROM.write(1, thirdValue1.toInt()); EEPROM.commit();
      EEPROM.write(2, fourthValue1.toInt()); EEPROM.commit();
      EEPROM.write(3, fifthValue1.toInt()); EEPROM.commit();

      NVIC_SystemReset();
    }
    else if (firstValue == "SETS") {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);
      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);
      EEPROM.write(5, secondValue1.toInt()); EEPROM.commit();
      NVIC_SystemReset();
    }
    else if (firstValue == "SETP") {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);

      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);

      EEPROM.write(4, secondValue1.toInt()); EEPROM.commit();
      NVIC_SystemReset();
    }
    if (firstValue == "SPEED") {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);

      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);
      String thirdValue1 = str.substring(secondCommaIndex1 + 1, thirdCommaIndex1);

      int vehicle_speed = secondValue1.toInt();
      int limit = thirdValue1.toInt();

      displaySpeed(vehicle_speed, limit);

      replyPrint(vehicle_speed);
      replyPrint(" ");
      replyPrint(limit);
    }
    else if (firstValue == "TEXT") {
      int packetIndex1 = str.indexOf(',');
      int packetIndex2 = str.indexOf(',', packetIndex1 + 1);
      int packetIndex3 = str.indexOf(',', packetIndex2 + 1);
      int packetIndex4 = str.indexOf(',', packetIndex3 + 1);
      int packetIndex5 = str.indexOf(',', packetIndex4 + 1);
      int packetIndex6 = str.indexOf(',', packetIndex5 + 1);

      String packetValue1 = str.substring(0, packetIndex1);
      String packetValue2 = str.substring(packetIndex1 + 1, packetIndex2);
      String packetValue3 = str.substring(packetIndex2 + 1, packetIndex3);
      String packetValue4 = str.substring(packetIndex3 + 1, packetIndex4);
      String packetValue5 = str.substring(packetIndex4 + 1, packetIndex5);
      String packetValue6 = str.substring(packetIndex5 + 1, packetIndex6);

      int xCoord = packetValue2.toInt();
      int yCoord = packetValue3.toInt();
      String usrMessage = packetValue4;
      int fontColour = packetValue5.toInt();
      int fontSize = packetValue6.toInt();

      writeToDisplay(xCoord, yCoord, usrMessage, fontColour, fontSize);
    }
    else if (firstValue == "CLEAR") {
      int packetIndex1 = str.indexOf(',');
      int packetIndex2 = str.indexOf(',', packetIndex1 + 1);
      int packetIndex3 = str.indexOf(',', packetIndex2 + 1);
      int packetIndex4 = str.indexOf(',', packetIndex3 + 1);
      int packetIndex5 = str.indexOf(',', packetIndex4 + 1);

      String packetValue1 = str.substring(0, packetIndex1);
      String packetValue2 = str.substring(packetIndex1 + 1, packetIndex2);
      String packetValue3 = str.substring(packetIndex2 + 1, packetIndex3);
      String packetValue4 = str.substring(packetIndex3 + 1, packetIndex4);
      String packetValue5 = str.substring(packetIndex4 + 1, packetIndex5);

      int x1 = packetValue2.toInt();
      int y1 = packetValue3.toInt();
      int x2 = packetValue4.toInt();
      int y2 = packetValue5.toInt();

      int i = x1;
      while (y1 < y2) {
        while (i < x2) {
          PANEL.drawPixel(i, y1, BLACK);
          i++;
        }
        y1++;
        i = x1;
      }
    }
    else if (firstValue == "TEMP") {
      int packetIndex1 = str.indexOf(',');
      String packetValue1 = str.substring(0, packetIndex1);
      if (packetValue1 == "TEMP") {
        replyPrintln(getTemperature());
      }
    }
    else if (firstValue == "IMAGE") {
      int packetIndex1 = str.indexOf(',');
      int packetIndex2 = str.indexOf(',', packetIndex1 + 1);
      int packetIndex3 = str.indexOf(',', packetIndex2 + 1);
      int packetIndex4 = str.indexOf(',', packetIndex3 + 1);
      int packetIndex5 = str.indexOf(',', packetIndex4 + 1);
      int packetIndex6 = str.indexOf(',', packetIndex5 + 1);

      String packetValue1 = str.substring(0, packetIndex1);
      String packetValue2 = str.substring(packetIndex1 + 1, packetIndex2);
      String packetValue3 = str.substring(packetIndex2 + 1, packetIndex3);
      String packetValue4 = str.substring(packetIndex3 + 1, packetIndex4);
      String packetValue5 = str.substring(packetIndex4 + 1, packetIndex5);
      String packetValue6 = str.substring(packetIndex5 + 1, packetIndex6);

      int imageSize = packetValue2.toInt();
      int x1 = packetValue3.toInt();
      int y1 = packetValue4.toInt();
      int xFinal = packetValue5.toInt();
      int yFinal = packetValue6.toInt();
      drawImage(imageSize, x1, y1, xFinal, yFinal);
    }
    else if (firstValue == "LUMEN") {
      int packetIndex1 = str.indexOf(',');
      int packetIndex2 = str.indexOf(',', packetIndex1 + 1);

      String packetValue1 = str.substring(0, packetIndex1);
      String packetValue2 = str.substring(packetIndex1 + 1, packetIndex2);

      PANEL.setBrightness(packetValue2.toInt());
    }
  }
  return -1;
}

void printAlive() {
  delay(50);
  //Date = String(day()) + ":" + String(month()) + ":" + String(year());
  //Times = String(hour()) + ":" + String(minute()) + ":" + String(second());
  AliveN = AliveN + ",";
  AliveN = AliveN + "E";
  AliveN = AliveN + "N";
  AliveN = AliveN + "D";
  AliveN = AliveN + "\0";

  replyPrintln(AliveN);
  Serial.println(AliveN);
  AliveN = "START,ALIVE,";
}

float getTemperature() {
  delay(200);
  int adc_ready = (ADC->CS & (1 << 8)) >> 8;
  float adc_voltage = (ADC->RESULT * 3.3) / 4095;
  float T = 27 - (adc_voltage - 0.706) / 0.001721;
  return (T - 10);
}

void HeartBeat()
{
  digitalWrite(HeartBeatLed, HIGH);
  delay(50);
  digitalWrite(HeartBeatLed, LOW);
  delay(50);
}

void drawImage(int imagesize, int startX, int startY, int x, int y) {
  //uint16_t* imageData = (uint16_t*)malloc(132000);
  uint16_t imageData[16385];
  for (int index = 0; index < 16385; index++) {
    imageData[index] = BLACK;
  }
  int ptr = imagesize, i = 0;
  String str = "";
  char *ch;
  do {
    str = readIncomingData();
    if (str != 0) {
      if (str == "!") {
        break;
      }
      char arr[str.length() + 1];
      strcpy(arr, str.c_str());
      imageData[i] = strtoul(arr, &ch, 16);
      replyPrintln(i);
      i++;
      ptr--;
    }
  } while (ptr);
  replyPrintln("Done");

  PANEL.drawRGBBitmap((PANEL.width() - 128) + startX, 0 + startY, imageData, x, y);
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial1.setTX(USR_K5_TX_PIN);
  Serial1.setRX(USR_K5_RX_PIN);
  Serial1.begin(USR_K5_BAUDRATE);
  EEPROM.begin(64);

  w5500Reset();
  SPI.setSCK(W5500_SCK_PIN);
  SPI.setTX(W5500_MOSI_PIN);
  SPI.setRX(W5500_MISO_PIN);
  SPI.begin();
  Ethernet.init(ETHERNET_SS_PIN);

  Serial.print("PORT:");
  int port = SERVER_PORT;
  if (EEPROM.read(5) < 254) {
    port = EEPROM.read(5);
  }
  Serial.println(port);
  server = EthernetServer(port);
  if (EEPROM.read(0) != 255 && EEPROM.read(1) != 255 && EEPROM.read(2) != 255 && EEPROM.read(3) != 255) {
    IPAddress SERVER_IP(EEPROM.read(0), EEPROM.read(1), EEPROM.read(2), EEPROM.read(3));
    Ethernet.begin(SERVER_MAC, SERVER_IP);
  }
  else {
    IPAddress SERVER_IP(ip_array[0], ip_array[1], ip_array[2], ip_array[3]);
    Ethernet.begin(SERVER_MAC, SERVER_IP);
  }
  server.begin();

  Serial.print("My IP:");
  Serial.println(Ethernet.localIP());
  Serial.print("USR-K5 UART on TX:");
  Serial.print(USR_K5_TX_PIN);
  Serial.print(" RX:");
  Serial.println(USR_K5_RX_PIN);

  pinMode(HeartBeatLed, OUTPUT);
  pinMode(HARDRST, INPUT_PULLUP);


  PANEL.init();
  PANEL.setBrightness(MaxPanelBrightness);
  //PANEL.fillScreen(WHITE);
  ADC->CS = 0x400B;
}

void loop() {
  HeartBeat();
  String str = pollCompleteCommand();
  if (str != "") {
    parseCommand(str);
    str = "";
  }
  checkHardReset();
}
