#include <SPI.h>
#include <HardwareSerial.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <stdio.h>
#include "config.h"

#include "DMD_RGB.h"

#include "fonts/Bahnschrift_Condensed30_b_033_126.h"
#include "fonts/Microsoft_Sans_Serif84_033_126.h"


struct adc {
  volatile uint32_t CS, RESULT,
           FCS, FIFO, DIV, INTR, INTE, INTF,
           INTS;
};
#define ADC ((struct adc*)0x4004C000)

EthernetClient client;
EthernetServer server = EthernetServer(SERVER_PORT);

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


void replyPrintln(const String& msg) {
  if (activeChannel == CHANNEL_ETH && client.connected()) {
    client.println(msg);
  } else if (activeChannel == CHANNEL_SERIAL) {
    Serial1.println(msg);
  }
}

DMD_RGB <PANELTYPE, PANELCOLORMODE> PANEL(
  addressMuxList,
  PIN_nOE,
  PIN_SCLK,
  RGBPinsList,
  DISPLAYS_ACROSS,
  DISPLAYS_DOWN,
  ENABLE_DUAL_BUFFER);

DMD_Standard_Font fontHeadline(Bahnschrift_Condensed30_b_033_126);
DMD_Standard_Font fontSpeed(Microsoft_Sans_Serif84_033_126);

void HeartBeat();
void checkHardReset();
String pollCompleteCommand();
bool isVsdsProtocol(const String& str);
bool isNetworkConfigCommand(const String& str);
int parseVsdsCommand(String str);
int parseCommand(String str);
void clearVsdsText(int x, int y, String text, int fontSize);

bool isVsdsSpeedValue(const String& text) {
  if (text.length() == 0) {
    return false;
  }
  for (unsigned i = 0; i < text.length(); i++) {
    if (text.charAt(i) < '0' || text.charAt(i) > '9') {
      return false;
    }
  }
  int speed = text.toInt();
  return speed >= 0 && speed <= 999;
}

void saveNetworkSettingsAndReboot() {
  EEPROM.commit();
  replyPrintln("OK");
  delay(100);
  NVIC_SystemReset();
}

void clearVsdsText(int x, int y, String text, int fontSize) {
  (void)fontSize;
  char buf[100];
  text.toCharArray(buf, sizeof(buf));
  PANEL.setTextColor(BLACK, BLACK);
  PANEL.drawStringX(x, y, buf, BLACK, 0);
}

void Headline() {
  PANEL.selectFont(&fontHeadline);
  PANEL.setTextColor(YELLOW, BLACK);
  PANEL.drawStringX(6, 2, "SPEED-KMph", YELLOW, 0);
  PANEL.swapBuffers(true);
}

void displaySpeed(int vehicleSpeed, int colorCode) {
  uint16_t pixelColor = vsdsColorToPixel(colorCode);
  bool underspeed = (colorCode == VSDS_COLOR_GREEN);

  PANEL.clearScreen(true);
  Headline();
  PANEL.selectFont(&fontSpeed);

  int n = vehicleSpeed, digitCount = 0;
  do {
    n /= 10;
    ++digitCount;
  } while (n != 0);

  int x = 0, y = 28;
  char s[20];
  sprintf(s, "%ld", (long)vehicleSpeed);
  int first_digit = s[0] - '0';

  if (digitCount == 1) {
    x = 45;
  } else if (digitCount == 2) {
    x = 25;
  } else if (digitCount == 3) {
    x = (first_digit == 1) ? 15 : 5;
  }

  char sp[10];
  sprintf(sp, "%d", vehicleSpeed);

  PANEL.setTextColor(pixelColor, BLACK);
  PANEL.drawStringX(x, y, sp, pixelColor, 0);
  PANEL.selectFont(&fontHeadline);
  PANEL.setTextColor(pixelColor, BLACK);
  if (underspeed) {
    PANEL.drawStringX(6, 100, "UNDERSPEED", pixelColor, 0);
  } else {
    PANEL.drawStringX(10, 100, "OVERSPEED", pixelColor, 0);
  }
  PANEL.swapBuffers(true);
}

void checkHardReset() {
  static unsigned long st_time = millis();
  if (digitalRead(HARDRST) == LOW) {
    if (millis() - st_time > 10000) {
      if (digitalRead(HARDRST) == LOW) {
        EEPROM.write(0, 192);
        EEPROM.write(1, 168);
        EEPROM.write(2, 0);
        EEPROM.write(3, 7);
        EEPROM.write(5, 255);
        EEPROM.commit();
        NVIC_SystemReset();
      }
    }
  }
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
      bool network = isNetworkConfigCommand(commandLineBuffer);
      if (vsds || network) {
        String complete = commandLineBuffer;
        commandLineBuffer = "";
        activeChannel = commandLineChannel;
        return complete;
      }
    }
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

bool isNetworkConfigCommand(const String& str) {
  String trimmed = trimMessage(str);
  return trimmed.startsWith("SET,") || trimmed.startsWith("SETS,");
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
    if (pos != "0-0" || fontSize != 7 || colorCode < 1 || colorCode > 7 || !isVsdsSpeedValue(text)) {
      replyPrintln("ERR");
      return -1;
    }
    displaySpeed(text.toInt(), colorCode);
    replyPrintln("OK");
    return 0;
  }

  if (cmd == "C") {
    if (text == "CLEAR" && pos == "0-0" && colorCode == 0 && fontSize == 0) {
      PANEL.clearScreen(true);
      replyPrintln("OK");
      return 0;
    }
    if (pos == "0-0" && fontSize == 7 && isVsdsSpeedValue(text)) {
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

  int commaIndex = str.indexOf(',');
  if (commaIndex < 0) {
    return -1;
  }

  String firstValue = str.substring(0, commaIndex);

  if (firstValue == "SET") {
    int c1 = str.indexOf(',');
    int c2 = str.indexOf(',', c1 + 1);
    int c3 = str.indexOf(',', c2 + 1);
    int c4 = str.indexOf(',', c3 + 1);
    if (c4 < 0) {
      replyPrintln("ERR");
      return -1;
    }
    EEPROM.write(0, str.substring(c1 + 1, c2).toInt());
    EEPROM.write(1, str.substring(c2 + 1, c3).toInt());
    EEPROM.write(2, str.substring(c3 + 1, c4).toInt());
    EEPROM.write(3, str.substring(c4 + 1).toInt());
    saveNetworkSettingsAndReboot();
    return 0;
  }

  if (firstValue == "SETS") {
    int c1 = str.indexOf(',');
    if (c1 < 0) {
      replyPrintln("ERR");
      return -1;
    }
    EEPROM.write(5, str.substring(c1 + 1).toInt());
    saveNetworkSettingsAndReboot();
    return 0;
  }

  replyPrintln("ERR");
  return -1;
}

void HeartBeat()
{
  digitalWrite(HeartBeatLed, HIGH);
  delay(50);
  digitalWrite(HeartBeatLed, LOW);
  delay(50);
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
