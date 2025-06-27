
# Sistem de Irigații Inteligent

Proiect de diplomă – Facultatea de Automatică și Calculatoare, UPT  
Autor: Eugen-Alexandru Lunguleasa  
Coordonator științific: As.dr.ing. Marian-Emanuel Ionașcu  
Sesiunea: Iunie 2025

---

## 🛠️ Pașii de compilare

### Cerințe minime:
- 2x Plăci de dezvoltare NodeMCU ESP32
- Arduino IDE (cu extensie pentru ESP32)
- Biblioteci Arduino necesare:
  - `DHT.h`
  - `WiFi.h`
  - `WebServer.h`

### Configurare Arduino IDE:
1. Deschide Arduino IDE
2. Instalează placa ESP32:
   - `File > Preferences > Additional Board URLs`:  
     Adaugă: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Mergi la `Tools > Board > Boards Manager`, caută **ESP32** și instalează
3. Selectează placa: `Tools > Board > ESP32 Dev Module`
4. Selectează portul corect: `Tools > Port > COMx`

---

## ⚙️ Pașii de instalare și punere în funcțiune

### 1. Conectare hardware:

| Componentă               | ESP32 Master         | ESP32 Slave          |
|--------------------------|----------------------|----------------------|
| Senzor DHT22             | GPIO 32              | -                    |
| Senzor ploaie FC-37      | GPIO 23              | -                    |
| Senzor umiditate sol     | GPIO 34              | -                    |
| Releu 2 canale IN1/2     | -                    | GPIO 25 și GPIO 26   |
| Electrovana 12V NC       | -                    | RELAY COM            |
| Pompa apă 3–6V           | -                    | RELAY COM            |

Alimentare:
- ESP32: 5V stabilizat
- Pompa: 5V
- Electrovalva: 12V

---

### 2. Upload cod

- În Arduino IDE, încarcă fișierul `esp32_master.ino` pe placa ESP32 Master
- Încarcă fișierul `esp32_slave.ino` pe placa ESP32 Slave

---

### 3. Lansare și utilizare

1. La pornire, **ESP32 Master** creează un Access Point Wi-Fi
2. **ESP32 Slave** se conectează la această rețea
3. ESP32 Master:
   - citește senzorii de sol, ploaie, temperatură și umiditate
   - trimite comenzi HTTP `/start` sau `/stop` către Slave
4. ESP32 Slave:
   - activează/dezactivează pompa și electrovalva pe baza comenzilor

---

## 🌐 Funcționare

Sistemul este complet autonom și funcționează fără conexiune la internet. Toate deciziile se iau local. Interfața de control poate fi accesată prin rețeaua Wi-Fi locală a ESP32 Master.

---

## 📌 Observații

- Sistemul este scalabil: se pot adăuga mai multe unități Slave
