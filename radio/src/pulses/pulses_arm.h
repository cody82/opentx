/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _PULSES_ARM_H_
#define _PULSES_ARM_H_

#if NUM_MODULES == 2
  #define MODULES_INIT(...) __VA_ARGS__, __VA_ARGS__
#else
  #define MODULES_INIT(...) __VA_ARGS__
#endif

#if defined(PCBHORUS)
  #define pulse_duration_t uint32_t
#else
  #define pulse_duration_t uint16_t
#endif

extern uint8_t s_current_protocol[NUM_MODULES];
extern uint8_t s_pulses_paused;
extern uint16_t failsafeCounter[NUM_MODULES];

PACK(struct PpmPulsesData {
  pulse_duration_t pulses[20];
  pulse_duration_t * ptr;
});

#if defined(PPM_PIN_SERIAL)
PACK(struct PxxSerialPulsesData {
  uint8_t  pulses[64];
  uint8_t  *ptr;
  uint16_t pcmValue;
  uint16_t pcmCrc;
  uint32_t pcmOnesCount;
  uint16_t serialByte;
  uint16_t serialBitCount;
});

PACK(struct Dsm2SerialPulsesData {
  uint8_t  pulses[64];
  uint8_t *ptr;
  uint8_t  serialByte ;
  uint8_t  serialBitCount;
});
#endif

#if defined(PPM_PIN_UART)
PACK(struct PxxUartPulsesData {
  uint8_t  pulses[64];
  uint8_t  * ptr;
  uint16_t pcmCrc;
});
#endif

#if defined(PPM_PIN_TIMER)
/* PXX uses 20 bytes (as of Rev 1.1 document) with 8 changes per byte + stop bit ~= 162 max pulses */
/* DSM2 uses 2 header + 12 channel bytes, with max 10 changes (8n2) per byte + 16 bits trailer ~= 156 max pulses */
/* Multimodule uses 3 bytes header + 22 channel bytes with max 11 changes per byte (8e2) + 16 bits trailer ~= 291 max pulses */
/* Multimodule reuses some of the DSM2 function and structs since the protocols are similar enough */
PACK(struct PxxTimerPulsesData {
  pulse_duration_t pulses[200];
  pulse_duration_t * ptr;
  uint16_t rest;
  uint16_t pcmCrc;
  uint32_t pcmOnesCount;
});

PACK(struct Dsm2TimerPulsesData {
#if defined(MULTIMODULE)
  pulse_duration_t pulses[300];
#else
  pulse_duration_t pulses[200];
#endif
  pulse_duration_t * ptr;
  uint16_t rest;
  uint8_t index;
});
#endif

#define CROSSFIRE_BAUDRATE             400000
#define CROSSFIRE_FRAME_PERIOD         4 // 4ms
#define CROSSFIRE_FRAME_LEN            (3+22+1)
#define CROSSFIRE_CHANNELS_COUNT       16
PACK(struct CrossfirePulsesData {
  uint8_t pulses[CROSSFIRE_FRAME_LEN];
});

union ModulePulsesData {
#if defined(PPM_PIN_SERIAL)
  PxxSerialPulsesData pxx;
  Dsm2SerialPulsesData dsm2;
#else
  PxxTimerPulsesData pxx;
  Dsm2TimerPulsesData dsm2;
#endif
#if defined(PPM_PIN_UART)
  PxxUartPulsesData pxx_uart;
#endif
  PpmPulsesData ppm;
  CrossfirePulsesData crossfire;
};

union TrainerPulsesData {
  PpmPulsesData ppm;
};

extern ModulePulsesData modulePulsesData[NUM_MODULES];
extern TrainerPulsesData trainerPulsesData;
extern const uint16_t CRCTable[];

void setupPulses(uint8_t port);
void setupPulsesDSM2(uint8_t port);
void setupPulsesMultimodule(uint8_t port);
void setupPulsesPXX(uint8_t port);
void setupPulsesPPM(uint8_t port);
void sendByteDsm2(uint8_t b);
void putDsm2Flush();
void putDsm2SerialBit(uint8_t bit);

void createCrossfireFrame(uint8_t * frame, int16_t * pulses);

#if defined(HUBSAN)
void Hubsan_Init();
#endif

inline void startPulses()
{
  s_pulses_paused = false;

#if defined(PCBTARANIS) || defined(PCBHORUS)
  setupPulses(INTERNAL_MODULE);
  setupPulses(EXTERNAL_MODULE);
#else
  setupPulses(EXTERNAL_MODULE);
#endif

#if defined(HUBSAN)
  Hubsan_Init();
#endif
}

inline bool pulsesStarted() { return s_current_protocol[0] != 255; }
inline void pausePulses() { s_pulses_paused = true; }
inline void resumePulses() { s_pulses_paused = false; }

#define SEND_FAILSAFE_NOW(idx) failsafeCounter[idx] = 1

inline void SEND_FAILSAFE_1S()
{
  for (int i=0; i<NUM_MODULES; i++) {
    failsafeCounter[i] = 100;
  }
}

#endif // _PULSES_ARM_H_
