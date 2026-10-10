# STM32G431 Vibration Diagnostic Module - Rev 0.1

<details>
<summary><h2>Rev 0.1</h2></summary>

Custom PCB and STM32 firmware for real-time vibroacoustic diagnostics of rotating machines (designed for submersible pumps). It records vibration with a MEMS accelerometer and sound with a MEMS microphone, runs an FFT on-device and logs everything to a microSD card.

<img width="1200" height="1140" alt="assembled" src="https://github.com/user-attachments/assets/cab76f23-5bf8-4af9-bfb5-b04e78b15da9" />

## Features

- **Vibration:** LSM6DS3 at 6.66 kHz ODR, ±4 g, 4096 samples per axis (X/Y/Z)
- **Sound:** INMP441 I²S microphone at 16 kHz, captured with DMA
- **On-device FFT:** radix-2 Cooley–Tukey real FFT running on the Cortex-M4F
- **Logging:** raw signals, FFT magnitudes and audio buffers saved to microSD (FatFs over SPI)
- **Industrial interfaces:** 24 V input, RS-485 port, ESD/TVS protection on power, SWD and RS-485 lines

## Hardware

| Block | Part | MCU interface |
|---|---|---|
| MCU | STM32G431RBT6 (Cortex-M4F, 170 MHz, 24 MHz HSE) | – |
| Accelerometer | LSM6DS3 | I2C3 (PC8/PC9), INT1/INT2 → PC6/PC7 |
| Microphone | INMP441 | I2S2 (PB12/PB13/PB15) + DMA |
| Temp/humidity | SHT31 (reserved for future use) | I2C2 (PA8/PA9) |
| Storage | microSD | SPI3 (PC10–PC12, CS PD2) |
| Fieldbus | ST3485 RS-485 transceiver | USART1 (PC4/PC5), RE/DE PA7 |
| Power | MP1584EN buck (24 V → 5 V) or micro-USB, AMS1117 3.3 V LDO | jumper-selectable |
| Debug | 10-pin 1.27 mm SWD header (ST-LINK) | PA13/PA14/PB3 |

2-layer 96 × 98 mm board, designed in EasyEDA and fabricated by JLCPCB.

<details>
<summary><b>Schematics</b></summary>

**MCU**

<img width="1275" height="807" alt="05_mcu" src="https://github.com/user-attachments/assets/36de0939-b84c-42eb-aae7-30ebafb0095a" />

**Buck converter 24 V → 5 V**

<img width="1391" height="594" alt="01_buck_24v_to_5v" src="https://github.com/user-attachments/assets/54fc9c3c-1003-4d03-9f01-eb9a482553e9" />

**USB power input**

<img width="926" height="656" alt="02_usb_power" src="https://github.com/user-attachments/assets/d4e6b614-43d7-4223-aefc-30a1bcd17341" />

**3.3 V LDO and supply select**

<img width="786" height="570" alt="03_ldo_3v3" src="https://github.com/user-attachments/assets/fce08842-5349-4036-be84-0149615bc405" />

**VREF and analog supply**

<img width="956" height="350" alt="04_vref_analog_supply" src="https://github.com/user-attachments/assets/cadc8ece-f3c1-4537-847b-355fc9b09364" />

**Reset and LSE/HSE oscillators**

<img width="802" height="1108" alt="06_reset_clocks" src="https://github.com/user-attachments/assets/701470b0-1712-4c03-8207-69dd0b5f7cf4" />


**Sensors (LSM6DS3, SHT31, INMP441)**

<img width="1139" height="714" alt="10_sensors" src="https://github.com/user-attachments/assets/fc06bb9a-e281-4b04-b5ba-8a0ad0716379" />

**microSD**

<img width="987" height="445" alt="09_microsd" src="https://github.com/user-attachments/assets/7715ccd0-2b67-4531-b787-cdf7ce550315" />

**SWD header**

<img width="1478" height="367" alt="08_swd" src="https://github.com/user-attachments/assets/37643c0e-e3eb-4284-a653-c4e390c9816f" />

**Status LEDs**

<img width="1453" height="435" alt="07_status_leds" src="https://github.com/user-attachments/assets/6e41bb4e-8a46-4804-929b-e04e1b675942" />

</details>

<details>
<summary><b>PCB layout</b></summary>

<table><tr><th>Top</th><th>Bottom</th></tr>
<tr><td><img width="742" height="752" alt="top" src="https://github.com/user-attachments/assets/e76ffd3c-d5de-4bb3-8e58-ac3d9db9658a" /></td><td><img width="737" height="752" alt="bottom" src="https://github.com/user-attachments/assets/58fa2b1a-609a-47b6-9633-0ed489bd107a" /></td></tr></table>

</details>

## Fault detection method

Characteristic bearing frequencies (BPFO, BPFI, BSF, FTF) are calculated from the bearing geometry and shaft speed. FFT peaks higher than 10× the spectral median are matched to these frequencies and their harmonics within ±0.5 Hz. The method was prototyped in MATLAB on the SCA bearing dataset.

## Results

Tested on an accelerated-wear rig with SKF 30205 tapered roller bearings (abrasive added to the grease). At 1000 rpm the cage frequency (FTF) appeared from stage 1 and the outer-race frequency (BPFO) appeared at stage 4. At 1800 rpm, BPFI and BSF also appeared. A teardown of the bearings confirmed the cage and outer-race damage.

<img width="1600" height="942" alt="wear_test_1000rpm" src="https://github.com/user-attachments/assets/0c6549ef-9291-43c2-b5c5-fbb5c7777af1" />

## Repository layout

```
Core/Src/LSM6DS3.c     LSM6DS3 register-level driver (HAL I²C)
Core/Src/fft.c         Real FFT (Cooley–Tukey)
Core/Src/fft_util.c    FFT, peak search and SD logging helpers
MIDWARE/FATFS_SD/      SD-card-over-SPI driver for FatFs
inz_g431.ioc           STM32CubeMX configuration
```

To build, open `inz_g431.ioc` in STM32CubeIDE, generate the project and build it. Flash over the SWD header with an ST-LINK.

---
Developed as a B.Eng. thesis at Wrocław University of Science and Technology (2025).

</details>

<details>
<summary><h2>Rev 0.2 - in development</h2></summary>

The board described above is Rev 0.1. Rev 0.2 splits the design into a main board and a separate arc-shaped sensor board (138.7 mm wide). The two boards are connected by two 8-pin cables.

### Main board

- **Same core as Rev 0.1:** STM32G431RBT6, microSD, RS-485 (ST3485), SWD, MP1584EN 24 V → 5 V buck
- **24 V input protection:** fuse, P-MOSFET reverse-polarity protection (ZXMP6A17), SMAJ24A TVS
- **2 relay outputs** (COM/NO/NC) with optocoupler-isolated drive
- **2 opto-isolated digital outputs** (ALRT1/ALRT2) powered from an external logic supply
- **Analog outputs:** two 12-bit DAC channels (MCP4921 + OPA197) giving 0–10 V for vibration RMS and temperature. The RMS channel can be switched by jumper to a 4–20 mA loop (XTR111).
- **PT1000 input** to the MCU ADC
- **3.3 V voltage reference** (NCP51460) for the ADC and DACs
- USART header and test points

### Sensor board

- LSM6DS3TR-C accelerometer, INMP441 I²S microphone, SHT31 temperature/humidity sensor
- 2× DRV5023 Hall-effect sensors
- Connects to the main board through two cables: one for I²C + I²S, one for the Hall and SPI lines

<details>
<summary><b>Rev 0.2 schematics</b></summary>

**Main board**

<img width="6782" height="4382" alt="SCH_diagnostic_module_stm32g431_1-Sheet_1_2026-10-10" src="https://github.com/user-attachments/assets/76332ebb-def2-4f56-8db1-2525f314527a" />

**Sensor board**

<img width="3332" height="2362" alt="SCH_Schematic1_1-P1_2026-10-10" src="https://github.com/user-attachments/assets/8f59630f-bdb0-4e3b-b649-cc29b4671287" />

</details>

<details>
<summary><b>Rev 0.2 PCB layout</b></summary>

<table><tr><th>Main board – top</th><th>Main board – bottom</th></tr>
<tr><td><img width="2160" height="1746" alt="PCB_PCB3_2026-10-10 (1)" src="https://github.com/user-attachments/assets/fdf20f7e-37fe-41a4-bcab-2720b8355048" /></td><td><img width="2160" height="1746" alt="PCB_PCB3_2026-10-10 (2)" src="https://github.com/user-attachments/assets/adde5fbb-c598-4577-83ae-4db68313e575" /></td></tr>
<tr><th>Sensor board – top</th><th>Sensor board – bottom</th></tr>
<tr><td><img width="2160" height="897" alt="PCB_PCB5_2026-10-10" src="https://github.com/user-attachments/assets/46f1b609-c6d9-4206-990c-8448aafca04b" /></td><td><img width="2160" height="897" alt="PCB_PCB5_2026-10-10 (1)" src="https://github.com/user-attachments/assets/4c6192e8-23fa-41e1-b3ab-d3d4309f810f" /></td></tr></table>

</details>

</details>
