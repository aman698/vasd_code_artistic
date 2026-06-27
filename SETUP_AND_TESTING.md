# VASD Display – Setup & Testing Guide

Raspberry Pi Pico firmware for a **Vehicle Actuated Speed Display (VASD)** LED matrix panel.  
Commands follow the **VSDS PDF protocol** (Artistic Infratech). Same commands work on **W5500 Ethernet** and **USR-K5 serial**.

---

## 1. What This Project Does

| Feature | Description |
|---------|-------------|
| LED panel | 4×4 matrix of 32×32 RGB panels (128×128 pixels) |
| W5500 Ethernet | TCP server – send commands over network |
| USR-K5 UART | Serial1 (GPIO 20/21) – same commands over serial |
| Speed display | Large speed number + **OVERSPEED** / **UNDERSPEED** label |
| IP & port | Stored in EEPROM – survives power off |

On boot, **USB Serial monitor** (115200 baud) prints:

```
PORT:23
My IP:192.168.0.7
USR-K5 UART on TX:20 RX:21
```

GPIO **22** heartbeat LED blinks when firmware is running.

---

## 2. Supported Commands (Summary)

| What you want | Command format | Reply |
|---------------|----------------|-------|
| Show speed | `\|T\|0-0\|<speed>\|<color>\|7\|1\|0\|` | `OK` |
| Clear speed screen | `\|C\|0-0\|<speed>\|<color>\|7\|1\|0\|` | `OK` |
| Clear full display | `\|C\|0-0\|CLEAR\|0\|0\|1\|0\|` | `OK` |
| Change IP | `SET,<a>,<b>,<c>,<d>` | `OK` then reboot |
| Change TCP port | `SETS,<port>` | `OK` then reboot |

**Always press Enter** after each command (`\n` or `\r\n`).

Works on **both** W5500 (TCP) and USR-K5 (115200 serial).

---

## 3. Default Network Settings

Change these in `config.h` **before first upload**, or use `SET` / `SETS` commands after upload (saved to EEPROM).

| Setting | Default | Where to change |
|---------|---------|-----------------|
| IP address | `192.168.0.7` | `config.h` → `ip_array[]` or command `SET,...` |
| TCP port | `23` | `config.h` → `SERVER_PORT` or command `SETS,...` |
| UART baud | `115200` | `config.h` → `USR_K5_BAUDRATE` |
| Subnet | PC must be `192.168.0.x` | Your PC network settings |

---

## 4. Hardware Wiring

### 4.1 W5500 → Raspberry Pi Pico

| W5500 pin | Pico GPIO |
|-----------|-----------|
| CS        | **17**    |
| RST       | **24**    |
| SCK       | **18**    |
| MOSI      | **19**    |
| MISO      | **16**    |
| 3.3V      | 3V3       |
| GND       | GND       |

### 4.2 USR-K5 → Raspberry Pi Pico

| USR-K5 | Pico | Notes |
|--------|------|-------|
| RX     | **GPIO 20** (TX1) | Pico TX → K5 RX |
| TX     | **GPIO 21** (RX1) | K5 TX → Pico RX |
| GND    | GND | Common ground required |

**Serial settings:** 115200 baud, 8N1

### 4.3 Other Pico pins

| Function       | GPIO |
|----------------|------|
| Heartbeat LED  | 22   |
| Hard reset btn | 27 (hold LOW 10 s → factory reset) |
| LED panel      | See `config.h` (GPIO 0–12) |

---

## 5. Software Setup (Arduino IDE)

1. Install **Arduino IDE** (or Arduino CLI).
2. Add board package: **Raspberry Pi Pico / RP2040** (Earle Philhower core).
3. Select board: **Raspberry Pi Pico**.
4. Required libraries:
   - **Ethernet**, **SPI**, **EEPROM**, **HardwareSerial** (RP2040 core)
   - **DMD_RGB** / **DMD_STM32** (LED panel)
5. Open `vasd.ino` and upload.
6. Open **Serial Monitor** @ **115200** – confirm IP and port.

---

## 6. How to Connect

### 6.1 W5500 – Ethernet TCP

**PuTTY:** Raw/Telnet → Host `192.168.0.7` → Port `23` → type command → Enter

**PowerShell:**

```powershell
$client = New-Object System.Net.Sockets.TcpClient("192.168.0.7", 23)
$stream = $client.GetStream()
$writer = New-Object System.IO.StreamWriter($stream)
$reader = New-Object System.IO.StreamReader($stream)
$writer.WriteLine("|T|0-0|65|1|7|1|0|")
$writer.Flush()
Start-Sleep -Milliseconds 500
$reader.ReadToEnd()
$client.Close()
```

**Linux / Mac:**

```bash
nc 192.168.0.7 23
# then type:
|T|0-0|65|1|7|1|0|
```

### 6.2 USR-K5 – Serial

Use **PuTTY**, **Tera Term**, or USB-serial on Pico UART pins:

- Baud: **115200**
- Send command + **Enter**

Configure USR-K5 to **115200, 8N1**. For network-through-K5, set K5 to transparent/TCP mode so data reaches Pico Serial1.

---

## 7. TEXT Command – Show Speed

Per PDF, **`|T|` = entering speed** (not free text). Shows full speed screen: headline, large number, OVERSPEED/UNDERSPEED.

### Format

```
|T|0-0|<speed>|<color>|7|1|0|
```

### Fields – what to change

| Field | Fixed value | You change | Example |
|-------|-------------|------------|---------|
| `T` | Text/speed command | No | `T` |
| `0-0` | Position (PDF fixed) | No | `0-0` |
| `<speed>` | Speed value | **Yes** – 0–999 km/h | `65` |
| `<color>` | Display color | **Yes** – 1 to 7 (see table) | `1` |
| `7` | Font size (PDF fixed) | No | `7` |
| `1` | Text effect (PDF fixed) | No | `1` |
| `0` | Reserved (PDF fixed) | No | `0` |

### Color table – change `<color>` to get different colours

| Code | Color | Label shown |
|------|-------|-------------|
| **1** | RED | OVERSPEED |
| **2** | GREEN | UNDERSPEED |
| **3** | YELLOW | OVERSPEED |
| **4** | BLUE | OVERSPEED |
| **5** | MAGENTA | OVERSPEED |
| **6** | CYAN | OVERSPEED |
| **7** | WHITE | OVERSPEED |

> **Tip:** Use color **1** (red) for overspeed, **2** (green) for underspeed – matches PDF behaviour.

### Examples – copy and edit the speed or color

```
|T|0-0|65|1|7|1|0|     → 65 km/h, RED, OVERSPEED
|T|0-0|65|2|7|1|0|     → 65 km/h, GREEN, UNDERSPEED
|T|0-0|120|1|7|1|0|    → 120 km/h, RED, OVERSPEED
|T|0-0|45|3|7|1|0|     → 45 km/h, YELLOW, OVERSPEED
|T|0-0|8|2|7|1|0|      → 8 km/h, GREEN, UNDERSPEED
```

**Reply:** `OK` or `ERR`

---

## 8. CLEAR Commands

### 8.1 Full display clear

```
|C|0-0|CLEAR|0|0|1|0|
```

Do **not** change `CLEAR|0|0|1|0` – PDF requires these exact values.

### 8.2 Clear speed screen

Use the **same speed and color** as when you showed it:

```
|C|0-0|65|1|7|1|0|
```

Clears the speed display (same fields as `|T|` except command is `C`).

### Clear command fields

| Field | Full clear | Speed clear |
|-------|------------|-------------|
| Command | `C` | `C` |
| Position | `0-0` | `0-0` |
| Text | `CLEAR` | `<speed>` (same as shown) |
| Color | `0` | `<color>` (same as shown) |
| Font | `0` | `7` |
| Effect | `1` | `1` |
| Reserve | `0` | `0` |

**Reply:** `OK` or `ERR`

---

## 9. IP & Port Commands (EEPROM)

Settings are **saved to EEPROM** and loaded on every boot.

### Change IP address

```
SET,192,168,0,7
```

| Part | Meaning | Example to change |
|------|---------|-------------------|
| `SET` | Set IP command | Fixed |
| 1st number | IP octet 1 | `192` |
| 2nd number | IP octet 2 | `168` |
| 3rd number | IP octet 3 | `0` |
| 4th number | IP octet 4 | `7` → change to `100` for `192.168.0.100` |

**Example – set IP to 192.168.0.100:**

```
SET,192,168,0,100
```

Reply: `OK` → board reboots → connect to new IP.

### Change TCP port

```
SETS,23
```

Change `23` to any port (e.g. `5023`):

```
SETS,5023
```

Reply: `OK` → board reboots → connect on new port.

### EEPROM layout (reference)

| EEPROM address | Stores |
|----------------|--------|
| 0–3 | IP address (4 bytes) |
| 5 | TCP port |

---

## 10. Step-by-Step First Test

### Test 1 – Boot check

1. Power Pico + W5500 + panel.
2. USB Serial Monitor @ 115200.
3. Confirm: `My IP:192.168.0.7`, `PORT:23`, heartbeat LED blinking.

### Test 2 – Speed on W5500

1. PC on same subnet (e.g. `192.168.0.100`).
2. TCP connect to `192.168.0.7:23`.
3. Send: `|T|0-0|55|1|7|1|0|`
4. Display: **55** red + **OVERSPEED**, reply: `OK`

### Test 3 – Color change

```
|T|0-0|55|2|7|1|0|
```

Display: **55** green + **UNDERSPEED**

### Test 4 – Full clear

```
|C|0-0|CLEAR|0|0|1|0|
```

Display clears, reply: `OK`

### Test 5 – USR-K5 serial

Send the same commands @ **115200** on Serial1. Same display, reply on serial.

### Test 6 – Change IP

```
SET,192,168,0,50
```

Reconnect to `192.168.0.50:23` after reboot.

---

## 11. Factory Reset

Hold **GPIO 27 (HARDRST)** **LOW for 10 seconds**:

- IP → `192.168.0.7`
- Port → `23` (default)
- Board reboots

Or send (if still reachable):

```
SET,192,168,0,7
SETS,23
```

---

## 12. Troubleshooting

| Problem | Check |
|---------|-------|
| No IP on serial | W5500 wiring, RST GPIO 24, 3.3V power |
| Cannot connect TCP | PC on same subnet, firewall, correct IP/port |
| USR-K5 no response | TX/RX swapped?, 115200 baud, common GND |
| Command ignored | End with Enter; command must start with `\|` or `SET`/`SETS` |
| Reply `ERR` on `\|T\|` | Use `0-0`, font `7`, color 1–7, numeric speed |
| Wrong color | Use PDF color table (1=RED, 2=GREEN) |
| Old IP after flash | EEPROM keeps old IP – factory reset or `SET,...` |
| Display blank | Send valid `\|T\|` command; check panel power |

---

## 13. Quick Reference Card

Copy this block – edit speed, color, IP, or port as needed:

```
─── SHOW SPEED (change speed & color) ───
|T|0-0|65|1|7|1|0|          → 65 red OVERSPEED
|T|0-0|65|2|7|1|0|          → 65 green UNDERSPEED
|T|0-0|120|1|7|1|0|         → 120 red OVERSPEED

─── CLEAR ───
|C|0-0|65|1|7|1|0|          → clear speed screen
|C|0-0|CLEAR|0|0|1|0|       → clear full display

─── IP & PORT (saved to EEPROM) ───
SET,192,168,0,7             → set IP, reboot
SETS,23                     → set port, reboot

─── CONNECTION ───
W5500:  192.168.0.7 : 23
USR-K5: 115200 baud, TX=GPIO20, RX=GPIO21
```

### Color quick pick

| Want | Use color code |
|------|----------------|
| Red overspeed | `1` |
| Green underspeed | `2` |
| Yellow | `3` |
| Blue | `4` |
| Magenta | `5` |
| Cyan | `6` |
| White | `7` |

---

## 14. Project Files

| File | Purpose |
|------|---------|
| `vasd.ino` | Main firmware – commands & display |
| `config.h` | Default IP, port, pins, panel, colors |
| `VSDS PROTOCOL_AIPL.pdf` | Official VSDS command spec |
| `fonts/` | Display fonts (headline + speed digits) |

---

*Artistic Infratech VASD – Raspberry Pi Pico + W5500 + USR-K5*
