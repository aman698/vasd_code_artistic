# VASD Display – Setup & Testing Guide

Raspberry Pi Pico firmware for a **Vehicle Actuated Speed Display (VASD)** LED matrix panel.  
Supports **two communication paths** and **two command formats**.

---

## 1. What This Project Does

| Feature | Description |
|---------|-------------|
| LED panel | 4×4 matrix of 32×32 RGB panels (128×128 pixels) |
| W5500 Ethernet | TCP server – receive commands over network |
| USR-K5 UART | Serial1 – receive same commands over serial |
| VSDS PDF protocol | Pipe format `\|T\|...\|` / `\|C\|...\|` (Artistic Infratech) |
| Legacy protocol | Comma format `SPEED,...` / `TEXT,...` etc. |
| Speed screen | Large speed number + OVERSPEED / UNDERSPEED label |

On boot, the **USB Serial monitor** (115200 baud) prints:

```
PORT:23
My IP:192.168.0.7
USR-K5 UART on TX:20 RX:21
```

GPIO **22** heartbeat LED blinks ~2× per second when the firmware is running.

---

## 2. Hardware Wiring

### 2.1 W5500 → Raspberry Pi Pico

| W5500 pin | Pico GPIO |
|-----------|-----------|
| CS        | **17**    |
| RST       | **24**    |
| SCK       | **18**    |
| MOSI      | **19**    |
| MISO      | **16**    |
| 3.3V      | 3V3       |
| GND       | GND       |

### 2.2 USR-K5 → Raspberry Pi Pico

| USR-K5 | Pico | Notes |
|--------|------|-------|
| RX     | **GPIO 20** (TX1) | Pico transmits → K5 receives |
| TX     | **GPIO 21** (RX1) | K5 transmits → Pico receives |
| GND    | GND | Common ground required |

**Serial settings:** 115200 baud, 8N1

### 2.3 Other Pico pins

| Function      | GPIO |
|---------------|------|
| Heartbeat LED | 22   |
| Hard reset btn| 27   (INPUT_PULLUP – hold LOW 10 s to factory reset) |
| LED panel     | See `config.h` (GPIO 0–12) |

### 2.4 Network (default)

| Setting | Value |
|---------|-------|
| IP address | **192.168.0.7** |
| TCP port   | **23** |
| Subnet     | PC must be on same network (e.g. 192.168.0.x) |

---

## 3. Software Setup (Arduino IDE)

1. Install **Arduino IDE** (or Arduino CLI).
2. Add board package: **Raspberry Pi Pico / RP2040** (Earle Philhower core).
3. Select board: **Raspberry Pi Pico**.
4. Required libraries (install via Library Manager if missing):
   - **Ethernet** (included with RP2040 core)
   - **SPI**, **EEPROM**, **HardwareSerial**
   - **DMD_RGB** / **DMD_STM32** (LED panel library used by this project)
5. Open `vasd.ino` from this folder.
6. Upload to Pico (BOOTSEL + USB if needed).
7. Open **Serial Monitor** at **115200** to confirm IP and port.

---

## 4. USR-K5 Module Setup

Configure the USR-K5 (web UI or AT commands) so it matches the Pico:

| Parameter | Value |
|-----------|-------|
| Baud rate | **115200** |
| Data bits | 8 |
| Parity    | None |
| Stop bits | 1 |

**Option A – Direct serial test (easiest)**  
Connect PC USB‑serial adapter to USR-K5 serial port (or Pico UART pins) and send commands from a terminal.

**Option B – Network through USR-K5**  
Set USR-K5 to **TCP Server** or **transparent transmission** mode so data received on its Ethernet port is forwarded to UART (GPIO 20/21). Send commands from a TCP client to the K5 IP; they arrive on Pico Serial1.

---

## 5. How to Connect & Send Commands

### 5.1 Test via W5500 (Ethernet TCP)

**Windows – PuTTY**

1. Connection type: **Raw** or **Telnet**
2. Host: `192.168.0.7`
3. Port: `23`
4. Open connection
5. Type a command and press **Enter**

**Windows – PowerShell**

```powershell
$client = New-Object System.Net.Sockets.TcpClient("192.168.0.7", 23)
$stream = $client.GetStream()
$writer = New-Object System.IO.StreamWriter($stream)
$reader = New-Object System.IO.StreamReader($stream)
$writer.WriteLine("SPEED,65,0")
$writer.Flush()
Start-Sleep -Milliseconds 500
$reader.ReadToEnd()
$client.Close()
```

**Linux / Mac**

```bash
nc 192.168.0.7 23
# then type:
SPEED,65,0
```

### 5.2 Test via USR-K5 (Serial)

Use **PuTTY**, **Tera Term**, or **Arduino Serial Monitor** (if wired to USB-serial on UART):

- Port: your COM port  
- Baud: **115200**  
- Send command + **Enter** (newline)

---

## 6. Command Formats Overview

The firmware auto-detects the format:

- Starts with `|` → **VSDS PDF protocol**
- Contains `,` → **Legacy comma protocol**

**Always end commands with Enter** (`\n` or `\r\n`).

---

## 7. SPEED Commands (Legacy – Comma Format)

Speed display uses the **legacy protocol only** (not in PDF pipe format).

### Syntax

```
SPEED,<speed>,<limit>
```

| Field | Meaning |
|-------|---------|
| speed | Vehicle speed (0–999 km/h) |
| limit | `0` = **OVERSPEED** (red), `1` = **UNDERSPEED** (green) |

### Examples

| Command | Result |
|---------|--------|
| `SPEED,45,0` | Shows **45** in red + **OVERSPEED** |
| `SPEED,45,1` | Shows **45** in green + **UNDERSPEED** |
| `SPEED,120,0` | Shows **120** in red + **OVERSPEED** |
| `SPEED,8,1` | Shows **8** in green + **UNDERSPEED** |

### Reply

Board responds on the same channel:

```
45 0
```

(speed, then limit)

### Test sequence (speed)

```
SPEED,65,0
SPEED,40,1
SPEED,999,0
```

---

## 8. TEXT Commands

### 8.1 VSDS PDF protocol (recommended for PDF compliance)

#### Show text

```
|T|x-y|text|color|font|1|0|
```

| Field | Description |
|-------|-------------|
| T | Text command |
| x-y | Position (e.g. `0-0` = x=0, y=0) |
| text | String to print |
| color | 1–7 (see color table below) |
| font | Font size 1–34 |
| 1 | Text effect (fixed = 1) |
| 0 | Reserved (fixed = 0) |

**Reply:** `OK` or `ERR`

#### PDF color table

| Code | Color |
|------|-------|
| 1 | RED |
| 2 | GREEN |
| 3 | YELLOW |
| 4 | BLUE |
| 5 | MAGENTA |
| 6 | CYAN |
| 7 | WHITE |

#### PDF text examples

```
|T|0-0|99|1|7|1|0|
```
Show **99** in red at top-left, font size 7.

```
|T|0-40|HELLO|2|2|1|0|
```
Show **HELLO** in green at (0,40), font size 2.

```
|T|10-50|TEST|4|12|1|0|
```
Show **TEST** in blue at (10,50), font size 12.

**Note:** Single digit 0–9 auto-shifts X by +15 pixels (per PDF).

#### Clear text (PDF)

```
|C|0-40|HELLO|1|2|1|0|
```
Erases **HELLO** at (0,40) using font 2.

#### Full screen clear (PDF)

```
|C|0-0|CLEAR|0|0|1|0|
```
Clears entire display. **Do not change** the `CLEAR|0|0|1|0` part.

---

### 8.2 Legacy comma TEXT

```
TEXT,x,y,message,color,fontSize
```

**Legacy color table** (different from PDF):

| Code | Color |
|------|-------|
| 1 | BLACK |
| 2 | BLUE |
| 3 | RED |
| 4 | GREEN |
| 5 | CYAN |
| 6 | MAGENTA |
| 7 | YELLOW |
| 8 | WHITE |
| 9 | ORANGE |

#### Examples

```
TEXT,10,50,HELLO,3,12
```
Red **HELLO** at (10,50), font 12.

```
TEXT,0,0,WELCOME,8,7
```
White **WELCOME** at top-left, font 7.

---

## 9. Other Legacy Commands

| Command | Format | Action |
|---------|--------|--------|
| Clear region | `CLEAR,x1,y1,x2,y2` | Black rectangle |
| Brightness | `LUMEN,value` | 0–255 brightness |
| Temperature | `TEMP` | Returns chip temperature |
| Set IP | `SET,a,b,c,d` | Saves IP to EEPROM, reboots |
| Set port | `SETS,port` | Saves TCP port to EEPROM, reboots |
| Alive | (internal) | `START,ALIVE,...` heartbeat string |

### Configuration examples

```
SET,192,168,0,7
SETS,23
```

### Clear region example

```
CLEAR,0,0,128,32
```

---

## 10. Step-by-Step First Test

### Test 1 – Confirm boot

1. Power Pico + W5500 + panel.
2. Open USB Serial Monitor @ 115200.
3. Verify: `My IP:192.168.0.7` and heartbeat LED blinking.

### Test 2 – Speed on Ethernet

1. Set PC IP to `192.168.0.100` (same subnet).
2. Connect TCP to `192.168.0.7:23`.
3. Send: `SPEED,55,0`
4. Display should show **55** red + **OVERSPEED**.

### Test 3 – Text on Ethernet (PDF format)

1. Same TCP connection.
2. Send: `|T|0-0|TEST|1|7|1|0|`
3. Reply: `OK`
4. Display shows **TEST** in red.

### Test 4 – Full clear

```
|C|0-0|CLEAR|0|0|1|0|
```

### Test 5 – USR-K5 serial

1. Send same commands through Serial1 @ 115200.
2. Same results; replies on serial.

---

## 11. Factory Reset

Hold **GPIO 27 (HARDRST)** **LOW for 10 seconds**:

- IP reset to **192.168.0.7**
- Port reset to default (**23**)
- Board reboots

Or send (if you can still connect on old IP):

```
SET,192,168,0,7
SETS,23
```

---

## 12. Troubleshooting

| Problem | Check |
|---------|-------|
| No IP on serial | W5500 wiring, RST on GPIO 24, 3.3V power |
| Cannot connect TCP | PC on 192.168.0.x subnet, firewall, correct port |
| USR-K5 no response | TX/RX not swapped, 115200 baud, common GND |
| Command ignored | End with Enter; check `\|` vs `,` format |
| PDF text wrong color | Use PDF color table (1=RED), not legacy table |
| SPEED no OK reply | Legacy speed returns `speed limit` not `OK` – this is normal |
| Old IP after flash | EEPROM still has old IP – hard reset or `SET,...` |
| Display blank after TEXT | Try `SPEED,...` or check font/color values |

---

## 13. Quick Reference Card

```
─── SPEED (legacy, comma) ───
SPEED,65,0          → 65 km/h OVERSPEED (red)
SPEED,65,1          → 65 km/h UNDERSPEED (green)

─── TEXT (PDF, pipe) ───
|T|0-0|99|1|7|1|0|              → text, red, font 7
|T|0-40|HELLO|2|2|1|0|          → HELLO, green, font 2
|C|0-0|CLEAR|0|0|1|0|           → clear screen
|C|0-40|HELLO|1|2|1|0|          → erase HELLO

─── TEXT (legacy, comma) ───
TEXT,10,50,HELLO,3,12          → red text, font 12

─── Network ───
IP:   192.168.0.7
Port: 23
UART: 115200, TX=20, RX=21
```

---

## 14. Project Files

| File | Purpose |
|------|---------|
| `vasd.ino` | Main firmware |
| `config.h` | Pins, IP, panel, colors |
| `VSDS PROTOCOL_AIPL.pdf` | Official VSDS pipe command spec |
| `fonts/` | Display font data |
| `bitmap.h` | Image bitmap data |

---

*Artistic Infratech VASD – Raspberry Pi Pico + W5500 + USR-K5*
