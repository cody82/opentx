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

#include "opentx.h"

#define PIN_ANALOG  0x0003
#define PIN_PORTA   0x0000
#define PIN_PORTB   0x0100
#define PIN_PORTC   0x0200

// Sample time should exceed 1uS
#define SAMPTIME       2   // sample time = 28 cycles
#define SAMPTIME_LONG  3   // sample time = 56 cycles

#if defined(REV9E) && defined(HORUS_STICKS)
  const int8_t ana_direction[NUMBER_ANALOG] = {1,-1,1,-1,  -1,-1,-1,1, -1,1,1,1,  -1};
#elif defined(REV9E)
  const int8_t ana_direction[NUMBER_ANALOG] = {1,1,-1,-1,  -1,-1,-1,1, -1,1,1,1,  -1};
#elif defined(REVPLUS)
  const int8_t ana_direction[NUMBER_ANALOG] = {1,-1,1,-1,  -1,1,-1,  -1,1,  1};
#elif defined(REV4a)
  const int8_t ana_direction[NUMBER_ANALOG] = {1,-1,1,-1,  -1,-1,0,  -1,1,  1};
#else
  const int8_t ana_direction[NUMBER_ANALOG] = {1,-1,1,-1,  -1,1,0,   -1,1,  1};
#endif

#if defined(REV9E)
    #define NUMBER_ANALOG_ADC1      10
    #define NUMBER_ANALOG_ADC3      3
    // mapping from adcValues order to enum Analogs
    const uint8_t ana_mapping[NUMBER_ANALOG] = { 0 /*STICK1*/, 1 /*STICK2*/, 2 /*STICK3*/, 3 /*STICK4*/,
                                                 10 /*POT1*/, 4 /*POT2*/, 5 /*POT3*/, 6 /*POT4*/,
                                                 11 /*SLIDER1*/, 12 /*SLIDER2*/, 7 /*SLIDER3*/, 8 /*SLIDER4*/,
                                                 9 /*TX_VOLTAGE*/ };
#else
    #define NUMBER_ANALOG_ADC1      10
#endif

uint16_t adcValues[NUMBER_ANALOG] __DMA;

void adcInit()
{
#if defined(REV9E)
  configure_pins(ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_SLIDER3, PIN_ANALOG | PIN_PORTA);
  configure_pins(ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SLIDER4, PIN_ANALOG | PIN_PORTB);
  configure_pins(ADC_GPIO_PIN_POT3 | ADC_GPIO_PIN_POT4 | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT, PIN_ANALOG | PIN_PORTC);
  configure_pins(ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2, PIN_ANALOG | PIN_PORTF);
#elif defined(REVPLUS)
  configure_pins(ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT1, PIN_ANALOG | PIN_PORTA);
  configure_pins(ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3, PIN_ANALOG | PIN_PORTB);
  configure_pins(ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT, PIN_ANALOG | PIN_PORTC);
#else
  configure_pins(ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT1, PIN_ANALOG | PIN_PORTA);
  configure_pins(ADC_GPIO_PIN_POT2, PIN_ANALOG|PIN_PORTB);
  configure_pins(ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT, PIN_ANALOG | PIN_PORTC);
#endif

  ADC1->CR1 = ADC_CR1_SCAN;
  ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC1->SQR1 = (NUMBER_ANALOG_ADC1-1) << 20; // bits 23:20 = number of conversions
#if defined(REV9E)
  ADC1->SQR2 = (ADC_CHANNEL_POT4<<0) + (ADC_CHANNEL_SLIDER3<<5) + (ADC_CHANNEL_SLIDER4<<10) + (ADC_CHANNEL_BATT<<15); // conversions 7 and more
  ADC1->SQR3 = (ADC_CHANNEL_STICK_LH<<0) + (ADC_CHANNEL_STICK_LV<<5) + (ADC_CHANNEL_STICK_RV<<10) + (ADC_CHANNEL_STICK_RH<<15) + (ADC_CHANNEL_POT2<<20) + (ADC_CHANNEL_POT3<<25); // conversions 1 to 6
#else
  ADC1->SQR2 = (ADC_CHANNEL_POT3<<0) + (ADC_CHANNEL_SLIDER1<<5) + (ADC_CHANNEL_SLIDER2<<10) + (ADC_CHANNEL_BATT<<15); // conversions 7 and more
  ADC1->SQR3 = (ADC_CHANNEL_STICK_LH<<0) + (ADC_CHANNEL_STICK_LV<<5) + (ADC_CHANNEL_STICK_RV<<10) + (ADC_CHANNEL_STICK_RH<<15) + (ADC_CHANNEL_POT1<<20) + (ADC_CHANNEL_POT2<<25); // conversions 1 to 6
#endif
  ADC1->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) + (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24);
  ADC1->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) + (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27);

  ADC->CCR = 0;

  ADC1_DMA_Stream->CR = DMA_SxCR_PL | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC;
  ADC1_DMA_Stream->PAR = CONVERT_PTR_UINT(&ADC1->DR);
  ADC1_DMA_Stream->M0AR = CONVERT_PTR_UINT(adcValues);
  ADC1_DMA_Stream->NDTR = NUMBER_ANALOG_ADC1;
  ADC1_DMA_Stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;

#if defined(REV9E)
  ADC3->CR1 = ADC_CR1_SCAN;
  ADC3->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC3->SQR1 = (NUMBER_ANALOG_ADC3-1) << 20;   // NUMBER_ANALOG Channels
  ADC3->SQR2 = 0;
  ADC3->SQR3 = (ADC_CHANNEL_POT1<<0) + (ADC_CHANNEL_SLIDER1<<5) + (ADC_CHANNEL_SLIDER2<<10); // conversions 1 to 3
  ADC3->SMPR1 = 0;
  ADC3->SMPR2 = (SAMPTIME_LONG<<(3*ADC_CHANNEL_POT1)) + (SAMPTIME_LONG<<(3*ADC_CHANNEL_SLIDER1)) + (SAMPTIME_LONG<<(3*ADC_CHANNEL_SLIDER2));

  ADC3_DMA_Stream->CR = DMA_SxCR_PL | DMA_SxCR_CHSEL_1 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC;
  ADC3_DMA_Stream->PAR = CONVERT_PTR_UINT(&ADC3->DR);
  ADC3_DMA_Stream->M0AR = CONVERT_PTR_UINT(adcValues + NUMBER_ANALOG_ADC1);
  ADC3_DMA_Stream->NDTR = NUMBER_ANALOG_ADC3;
  ADC3_DMA_Stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;
#endif
}

void adcSingleRead()
{
  ADC1_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  ADC1->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADC1_DMA->HIFCR = ADC1_DMA_FLAGS; // Write ones to clear bits
  ADC1_DMA_Stream->CR |= DMA_SxCR_EN; // Enable DMA
  ADC1->CR2 |= (uint32_t) ADC_CR2_SWSTART;

#if defined(REV9E)
  ADC3_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  ADC3->SR &= ~(uint32_t) ( ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR );
  ADC3_DMA->LIFCR = ADC3_DMA_FLAGS; // Write ones to clear bits
  ADC3_DMA_Stream->CR |= DMA_SxCR_EN; // Enable DMA
  ADC3->CR2 |= (uint32_t)ADC_CR2_SWSTART;
#endif // defined(REV9E)

#if defined(REV9E)
  for (unsigned int i=0; i<10000; i++) {
    if ((ADC1_DMA->HISR & ADC1_DMA_FLAG_TC) && (ADC3_DMA->LISR & ADC3_DMA_FLAG_TC)) {
      break;
    }
  }
  ADC1_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  ADC3_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
#else
  for (unsigned int i = 0; i < 10000; i++) {
    if (ADC1_DMA->HISR & ADC1_DMA_FLAG_TC) {
      break;
    }
  }
  ADC1_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
#endif
}

void adcRead()
{
  uint16_t temp[NUMBER_ANALOG] = { 0 };

  for (int i=0; i<4; i++) {
    adcSingleRead();
    for (uint8_t x=0; x<NUMBER_ANALOG; x++) {
      uint16_t val = adcValues[x];
#if defined(JITTER_MEASURE)
      if (JITTER_MEASURE_ACTIVE()) {
        rawJitter[x].measure(val);
      }
#endif
      temp[x] += val;
    }
  }

  for (uint8_t x=0; x<NUMBER_ANALOG; x++) {
    adcValues[x] = temp[x] >> 2;
  }
}

// TODO
void adcStop()
{
}

#if !defined(SIMU)
uint16_t getAnalogValue(uint8_t index)
{
  if (IS_POT(index) && !IS_POT_AVAILABLE(index)) {
    // Use fixed analog value for non-existing and/or non-connected pots.
    // Non-connected analog inputs will slightly follow the adjacent connected analog inputs, 
    // which produces ghost readings on these inputs.
    return 0;
  }
#if defined(REV9E)
  index = ana_mapping[index];
#endif
  if (ana_direction[index] < 0)
    return 4095 - adcValues[index];
  else
    return adcValues[index];
}
#endif // #if !defined(SIMU)
