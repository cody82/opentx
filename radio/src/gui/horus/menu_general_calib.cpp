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

#include "../../opentx.h"

#define XPOT_DELTA                     10
#define XPOT_DELAY                     10 /* cycles */
#define POT_BAR_WIDTH                  7
#define POT_BAR_HEIGHT                 (BOX_WIDTH-9)
#define POT_BAR_INTERVAL               20
#define POT_BAR_BOTTOM                 200
#define LBOX_CENTERX                   (BOX_WIDTH/2 + 17)
#define RBOX_CENTERX                   (LCD_W-LBOX_CENTERX)

void drawPotsBars()
{
  // Optimization by Mike Blandford
  unsigned int x, i, len ;  // declare temporary variables
  for (x=LCD_W/2-(POT_BAR_INTERVAL*NUM_POTS/2), i=NUM_STICKS; i<NUM_STICKS+NUM_POTS; x+=POT_BAR_INTERVAL, i++) {
    if (IS_POT_AVAILABLE(i)) {
      len = ((calibratedStick[i]+RESX)*POT_BAR_HEIGHT/(RESX*2))+1l;  // calculate once per loop
      // TODO 220 constant
      lcdDrawSolidFilledRect(x, POT_BAR_BOTTOM-len, POT_BAR_WIDTH, len, TEXT_COLOR);
      putsStickName(x-2, POT_BAR_BOTTOM+5, i, TEXT_COLOR|TINSIZE);
    }
  }
}

void drawSticksPositions()
{
  int16_t calibStickVert = calibratedStick[CONVERT_MODE(1)];
  if (g_model.throttleReversed && CONVERT_MODE(1) == THR_STICK)
    calibStickVert = -calibStickVert;
  drawStick(LBOX_CENTERX, calibratedStick[CONVERT_MODE(0)], calibStickVert);

  calibStickVert = calibratedStick[CONVERT_MODE(2)];
  if (g_model.throttleReversed && CONVERT_MODE(2) == THR_STICK)
    calibStickVert = -calibStickVert;
  drawStick(RBOX_CENTERX, calibratedStick[CONVERT_MODE(3)], calibStickVert);
}

bool menuCommonCalib(evt_t event)
{
  for (uint8_t i=0; i<NUM_STICKS+NUM_POTS; i++) { // get low and high vals for sticks and trims
    int16_t vt = anaIn(i);
    reusableBuffer.calib.loVals[i] = min(vt, reusableBuffer.calib.loVals[i]);
    reusableBuffer.calib.hiVals[i] = max(vt, reusableBuffer.calib.hiVals[i]);
    if (i >= POT1 && i <= POT_LAST) {
      if (IS_POT_WITHOUT_DETENT(i)) {
        reusableBuffer.calib.midVals[i] = (reusableBuffer.calib.hiVals[i] + reusableBuffer.calib.loVals[i]) / 2;
      }
      uint8_t idx = i - POT1;
      int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
      if (IS_POT_MULTIPOS(i) && count <= XPOTS_MULTIPOS_COUNT) {
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == 0 || vt < reusableBuffer.calib.xpotsCalib[idx].lastPosition - XPOT_DELTA || vt > reusableBuffer.calib.xpotsCalib[idx].lastPosition + XPOT_DELTA) {
          reusableBuffer.calib.xpotsCalib[idx].lastPosition = vt;
          reusableBuffer.calib.xpotsCalib[idx].lastCount = 1;
        }
        else {
          if (reusableBuffer.calib.xpotsCalib[idx].lastCount < 255) reusableBuffer.calib.xpotsCalib[idx].lastCount++;
        }
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == XPOT_DELAY) {
          int16_t position = reusableBuffer.calib.xpotsCalib[idx].lastPosition;
          bool found = false;
          for (int j=0; j<count; j++) {
            int16_t step = reusableBuffer.calib.xpotsCalib[idx].steps[j];
            if (position >= step-XPOT_DELTA && position <= step+XPOT_DELTA) {
              found = true;
              break;
            }
          }
          if (!found) {
            if (count < XPOTS_MULTIPOS_COUNT) {
              reusableBuffer.calib.xpotsCalib[idx].steps[count] = position;
            }
            reusableBuffer.calib.xpotsCalib[idx].stepsCount += 1;
          }
        }
      }
    }
  }

  switch (event) {
    case EVT_ENTRY:
    case EVT_KEY_BREAK(KEY_EXIT):
      calibrationState = 0;
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      calibrationState++;
      break;
  }

  switch (calibrationState) {
    case 0:
      // START CALIBRATION
      if (!READ_ONLY()) {
        lcd_putsCenter(MENU_CONTENT_TOP, STR_MENUTOSTART);
      }
      break;

    case 1:
      // SET MIDPOINT
      lcd_putsCenter(MENU_CONTENT_TOP, STR_SETMIDPOINT, INVERS);
      lcd_putsCenter(MENU_CONTENT_TOP+FH, STR_MENUWHENDONE);

      for (int i=0; i<NUM_STICKS+NUM_POTS; i++) {
        reusableBuffer.calib.loVals[i] = 15000;
        reusableBuffer.calib.hiVals[i] = -15000;
        reusableBuffer.calib.midVals[i] = anaIn(i);
        if (i<NUM_XPOTS) {
          reusableBuffer.calib.xpotsCalib[i].stepsCount = 0;
          reusableBuffer.calib.xpotsCalib[i].lastCount = 0;
        }
      }
      break;

    case 2:
      // MOVE STICKS/POTS
      STICK_SCROLL_DISABLE();
      lcd_putsCenter(MENU_CONTENT_TOP, STR_MOVESTICKSPOTS, INVERS);
      lcd_putsCenter(MENU_CONTENT_TOP+FH, STR_MENUWHENDONE);

      for (int i=0; i<NUM_STICKS+NUM_POTS; i++) {
        if (abs(reusableBuffer.calib.loVals[i]-reusableBuffer.calib.hiVals[i]) > 50) {
          g_eeGeneral.calib[i].mid = reusableBuffer.calib.midVals[i];
          int16_t v = reusableBuffer.calib.midVals[i] - reusableBuffer.calib.loVals[i];
          g_eeGeneral.calib[i].spanNeg = v - v/STICK_TOLERANCE;
          v = reusableBuffer.calib.hiVals[i] - reusableBuffer.calib.midVals[i];
          g_eeGeneral.calib[i].spanPos = v - v/STICK_TOLERANCE;
        }
      }
      break;

    case 3:
      for (int i=POT1; i<=POT_LAST; i++) {
        int idx = i - POT1;
        int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
        if (IS_POT_MULTIPOS(i)) {
          if (count > 1 && count <= XPOTS_MULTIPOS_COUNT) {
            for (int j=0; j<count; j++) {
              for (int k=j+1; k<count; k++) {
                if (reusableBuffer.calib.xpotsCalib[idx].steps[k] < reusableBuffer.calib.xpotsCalib[idx].steps[j]) {
                  SWAP(reusableBuffer.calib.xpotsCalib[idx].steps[j], reusableBuffer.calib.xpotsCalib[idx].steps[k]);
                }
              }
            }
            StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[i];
            calib->count = count - 1;
            for (int j=0; j<calib->count; j++) {
              calib->steps[j] = (reusableBuffer.calib.xpotsCalib[idx].steps[j+1] + reusableBuffer.calib.xpotsCalib[idx].steps[j]) >> 5;
            }
          }
          else {
            // g_eeGeneral.potsConfig &= ~(0x03<<(2*idx));
          }
        }
      }
      g_eeGeneral.chkSum = evalChkSum();
      storageDirty(EE_GENERAL);
      calibrationState = 4;
      break;

    default:
      calibrationState = 0;
      break;
  }

  drawSticksPositions();
  drawPotsBars();

  for (int i=POT1; i<=POT_LAST; i++) {
    uint8_t steps = 0;
    if (calibrationState == 2) {
      steps = reusableBuffer.calib.xpotsCalib[i-POT1].stepsCount;
    }
    else if (IS_POT_MULTIPOS(i)) {
      StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[i];
      steps = calib->count + 1;
    }
    if (steps > 0 && steps <= XPOTS_MULTIPOS_COUNT) {
      lcdDrawNumber(LCD_W/2-(POT_BAR_INTERVAL*NUM_POTS/2)+(POT_BAR_INTERVAL*(i-POT1)), POT_BAR_BOTTOM+15, steps, TEXT_COLOR|TINSIZE, 0, "[", "]");
    }
  }

  return true;
}

bool menuGeneralCalib(evt_t event)
{
  SIMPLE_MENU(STR_MENUCALIBRATION, menuTabGeneral, e_Calib, 0, DEFAULT_SCROLLBAR_X);
  menuVerticalPosition = -1;

  return menuCommonCalib(READ_ONLY() ? 0 : event);
}

bool menuFirstCalib(evt_t event)
{
  if (event == EVT_KEY_BREAK(KEY_EXIT) || calibrationState == 4) {
    calibrationState = 0;
    chainMenu(menuMainView);
    return false;
  }
  else {
    drawScreenTemplate(STR_MENUCALIBRATION);
    return menuCommonCalib(event);
  }
}