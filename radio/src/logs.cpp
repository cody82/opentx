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
#include "ff.h"

FIL g_oLogFile = {0};
const pm_char * g_logError = NULL;
uint8_t logDelay;

#if defined(PCBTARANIS) || defined(PCBFLAMENCO) || defined(PCBHORUS)
  #define get2PosState(sw) (switchState(SW_ ## sw ## 0) ? -1 : 1)
#else
  #define get2PosState(sw) (switchState(SW_ ## sw) ? -1 : 1)
#endif

#define get3PosState(sw) (switchState(SW_ ## sw ## 0) ? -1 : (switchState(SW_ ## sw ## 2) ? 1 : 0))

const pm_char * openLogs()
{
  // Determine and set log file filename
  FRESULT result;
  char filename[34]; // /LOGS/modelnamexxx-2013-01-01.log

  if (!sdMounted())
    return STR_NO_SDCARD;

  if (sdGetFreeSectors() == 0)
    return STR_SDCARD_FULL;

  // check and create folder here
  strcpy_P(filename, STR_LOGS_PATH);
  const char * error = sdCheckAndCreateDirectory(filename);
  if (error) {
    return error;
  }

  filename[sizeof(LOGS_PATH)-1] = '/';
  memcpy(&filename[sizeof(LOGS_PATH)], g_model.header.name, sizeof(g_model.header.name));
  filename[sizeof(LOGS_PATH)+sizeof(g_model.header.name)] = '\0';

  uint8_t i = sizeof(LOGS_PATH)+sizeof(g_model.header.name)-1;
  uint8_t len = 0;
  while (i>sizeof(LOGS_PATH)-1) {
    if (!len && filename[i])
      len = i+1;
    if (len) {
      if (filename[i])
        filename[i] = idx2char(filename[i]);
      else
        filename[i] = '_';
    }
    i--;
  }

  if (len == 0) {
#if defined(EEPROM)
    uint8_t num = g_eeGeneral.currModel + 1;
#else
    // TODO
    uint8_t num = 1;
#endif
    strcpy_P(&filename[sizeof(LOGS_PATH)], STR_MODEL);
    filename[sizeof(LOGS_PATH) + PSIZE(TR_MODEL)] = (char)((num / 10) + '0');
    filename[sizeof(LOGS_PATH) + PSIZE(TR_MODEL) + 1] = (char)((num % 10) + '0');
    len = sizeof(LOGS_PATH) + PSIZE(TR_MODEL) + 2;
  }

  char * tmp = &filename[len];

#if defined(RTCLOCK)
  tmp = strAppendDate(&filename[len]);
#endif

  strcpy_P(tmp, STR_LOGS_EXT);

  result = f_open(&g_oLogFile, filename, FA_OPEN_ALWAYS | FA_WRITE);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  if (f_size(&g_oLogFile) == 0) {
    writeHeader();
  }
  else {
    result = f_lseek(&g_oLogFile, f_size(&g_oLogFile)); // append
    if (result != FR_OK) {
      return SDCARD_ERROR(result);
    }
  }

  return NULL;
}

tmr10ms_t lastLogTime = 0;

void closeLogs()
{
  if (f_close(&g_oLogFile) != FR_OK) {
    // close failed, forget file
    g_oLogFile.fs = 0;
  }
  lastLogTime = 0;
}

#if !defined(CPUARM)
getvalue_t getConvertedTelemetryValue(getvalue_t val, uint8_t unit)
{
  convertUnit(val, unit);
  return val;
}
#endif

void writeHeader()
{
#if defined(RTCLOCK)
  f_puts("Date,Time,", &g_oLogFile);
#else
  f_puts("Time,", &g_oLogFile);
#endif

#if defined(FRSKY)
#if !defined(CPUARM)
  f_puts("Buffer,RX,TX,A1,A2,", &g_oLogFile);
#if defined(FRSKY_HUB)
  if (IS_USR_PROTO_FRSKY_HUB()) {
    f_puts("GPS Date,GPS Time,Long,Lat,Course,GPS Speed(kts),GPS Alt,Baro Alt(", &g_oLogFile);
    f_puts(TELEMETRY_BARO_ALT_UNIT, &g_oLogFile);
    f_puts("),Vertical Speed,Air Speed(kts),Temp1,Temp2,RPM,Fuel," TELEMETRY_CELLS_LABEL "Current,Consumption,Vfas,AccelX,AccelY,AccelZ,", &g_oLogFile);
  }
#endif
#if defined(WS_HOW_HIGH)
  if (IS_USR_PROTO_WS_HOW_HIGH()) {
    f_puts("WSHH Alt,", &g_oLogFile);
  }
#endif
#endif

#if defined(CPUARM)
  char label[TELEM_LABEL_LEN+7];
  for (int i=0; i<MAX_SENSORS; i++) {
    TelemetrySensor & sensor = g_model.telemetrySensors[i];
    if (sensor.logs) {
      memset(label, 0, sizeof(label));
      zchar2str(label, sensor.label, TELEM_LABEL_LEN);
      if (sensor.unit != UNIT_RAW && sensor.unit != UNIT_GPS && sensor.unit != UNIT_DATETIME) {
        strcat(label, "(");
        strncat(label, STR_VTELEMUNIT+1+3*sensor.unit, 3);
        strcat(label, ")");
      }
      strcat(label, ",");
      f_puts(label, &g_oLogFile);
    }
  }
#endif
#endif

#if defined(PCBTARANIS) || defined(PCBHORUS)
  for (uint8_t i=1; i<NUM_STICKS+NUM_POTS+1; i++) {
    const char * p = STR_VSRCRAW + i * STR_VSRCRAW[0] + 2;
    for (uint8_t j=0; j<STR_VSRCRAW[0]-1; ++j) {
      if (!*p) break;
      f_putc(*p, &g_oLogFile);
      ++p;
    }
    f_putc(',', &g_oLogFile);
  }
  #define STR_SWITCHES_LOG_HEADER  "SA,SB,SC,SD,SE,SF,SG,SH"
  f_puts(STR_SWITCHES_LOG_HEADER ",LS" "\n", &g_oLogFile);
#else
  f_puts("Rud,Ele,Thr,Ail,P1,P2,P3,THR,RUD,ELE,3POS,AIL,GEA,TRN\n", &g_oLogFile);
#endif
}

uint32_t getLogicalSwitchesStates(uint8_t first)
{
  uint32_t result = 0;
  for (uint8_t i=0; i<32; i++) {
    result |= (getSwitch(SWSRC_FIRST_LOGICAL_SWITCH+first+i) << i);
  }
  return result;
}

void writeLogs()
{
  static const pm_char * error_displayed = NULL;

  if (isFunctionActive(FUNCTION_LOGS) && logDelay > 0) {
    tmr10ms_t tmr10ms = get_tmr10ms();
    if (lastLogTime == 0 || (tmr10ms_t)(tmr10ms - lastLogTime) >= (tmr10ms_t)logDelay*10) {
      lastLogTime = tmr10ms;

      if (!g_oLogFile.fs) {
        const pm_char * result = openLogs();
        if (result != NULL) {
          if (result != error_displayed) {
            error_displayed = result;
            POPUP_WARNING(result);
          }
          return;
        }
      }

#if defined(RTCLOCK)
      {
        static struct gtm utm;
        static gtime_t lastRtcTime = 0;
        if (g_rtcTime != lastRtcTime) {
          lastRtcTime = g_rtcTime;
          gettime(&utm);
        }
        f_printf(&g_oLogFile, "%4d-%02d-%02d,%02d:%02d:%02d.%02d0,", utm.tm_year+1900, utm.tm_mon+1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec, g_ms100);
      }
#else
      f_printf(&g_oLogFile, "%d,", tmr10ms);
#endif

#if defined(FRSKY)
#if !defined(CPUARM)
      f_printf(&g_oLogFile, "%d,%d,%d,", telemetryStreaming, RAW_FRSKY_MINMAX(telemetryData.rssi[0]), RAW_FRSKY_MINMAX(telemetryData.rssi[1]));
      for (uint8_t i=0; i<MAX_FRSKY_A_CHANNELS; i++) {
        int16_t converted_value = applyChannelRatio(i, RAW_FRSKY_MINMAX(telemetryData.analog[i]));
        f_printf(&g_oLogFile, "%d.%02d,", converted_value/100, converted_value%100);
      }

#if defined(FRSKY_HUB)
      TELEMETRY_BARO_ALT_PREPARE();

      if (IS_USR_PROTO_FRSKY_HUB()) {
        f_printf(&g_oLogFile, "%4d-%02d-%02d,%02d:%02d:%02d,%03d.%04d%c,%03d.%04d%c,%03d.%02d," TELEMETRY_GPS_SPEED_FORMAT TELEMETRY_GPS_ALT_FORMAT TELEMETRY_BARO_ALT_FORMAT TELEMETRY_VSPEED_FORMAT TELEMETRY_ASPEED_FORMAT "%d,%d,%d,%d," TELEMETRY_CELLS_FORMAT TELEMETRY_CURRENT_FORMAT "%d," TELEMETRY_VFAS_FORMAT "%d,%d,%d,",
            telemetryData.hub.year+2000,
            telemetryData.hub.month,
            telemetryData.hub.day,
            telemetryData.hub.hour,
            telemetryData.hub.min,
            telemetryData.hub.sec,
            telemetryData.hub.gpsLongitude_bp,
            telemetryData.hub.gpsLongitude_ap,
            telemetryData.hub.gpsLongitudeEW ? telemetryData.hub.gpsLongitudeEW : '-',
            telemetryData.hub.gpsLatitude_bp,
            telemetryData.hub.gpsLatitude_ap,
            telemetryData.hub.gpsLatitudeNS ? telemetryData.hub.gpsLatitudeNS : '-',
            telemetryData.hub.gpsCourse_bp,
            telemetryData.hub.gpsCourse_ap,
            TELEMETRY_GPS_SPEED_ARGS
            TELEMETRY_GPS_ALT_ARGS
            TELEMETRY_BARO_ALT_ARGS
            TELEMETRY_VSPEED_ARGS
            TELEMETRY_ASPEED_ARGS
            telemetryData.hub.temperature1,
            telemetryData.hub.temperature2,
            telemetryData.hub.rpm,
            telemetryData.hub.fuelLevel,
            TELEMETRY_CELLS_ARGS
            TELEMETRY_CURRENT_ARGS
            telemetryData.hub.currentConsumption,
            TELEMETRY_VFAS_ARGS
            telemetryData.hub.accelX,
            telemetryData.hub.accelY,
            telemetryData.hub.accelZ);
      }
#endif

#if defined(WS_HOW_HIGH)
      if (IS_USR_PROTO_WS_HOW_HIGH()) {
        f_printf(&g_oLogFile, "%d,", TELEMETRY_RELATIVE_BARO_ALT_BP);
      }
#endif
#endif

#if defined(CPUARM)
      for (int i=0; i<MAX_SENSORS; i++) {
        TelemetrySensor & sensor = g_model.telemetrySensors[i];
        TelemetryItem & telemetryItem = telemetryItems[i];
        if (sensor.logs) {
          if (sensor.unit == UNIT_GPS) {
            if (telemetryItem.gps.longitude && telemetryItem.gps.latitude)
              f_printf(&g_oLogFile, "%f %f,", float(telemetryItem.gps.longitude)/1000000, float(telemetryItem.gps.latitude)/1000000);
            else
              f_printf(&g_oLogFile, ",");
          }
          else if (sensor.unit == UNIT_DATETIME) {
            if (telemetryItem.datetime.datestate)
              f_printf(&g_oLogFile, "%4d-%02d-%02d %02d:%02d:%02d,", telemetryItem.datetime.year, telemetryItem.datetime.month, telemetryItem.datetime.day, telemetryItem.datetime.hour, telemetryItem.datetime.min, telemetryItem.datetime.sec);
            else
              f_printf(&g_oLogFile, ",");
          }
          else if (sensor.prec == 2) {
            div_t qr = div(telemetryItem.value, 100);
            if (telemetryItem.value < 0) f_printf(&g_oLogFile, "-");
            f_printf(&g_oLogFile, "%d.%02d,", abs(qr.quot), abs(qr.rem));
          }
          else if (sensor.prec == 1) {
            div_t qr = div(telemetryItem.value, 10);
            if (telemetryItem.value < 0) f_printf(&g_oLogFile, "-");
            f_printf(&g_oLogFile, "%d.%d,", abs(qr.quot), abs(qr.rem));
          }
          else {
            f_printf(&g_oLogFile, "%d,", telemetryItem.value);
          }
        }
      }
#endif
#endif

      for (uint8_t i=0; i<NUM_STICKS+NUM_POTS; i++) {
        f_printf(&g_oLogFile, "%d,", calibratedStick[i]);
      }

#if defined(PCBFLAMENCO)
      int result = f_printf(&g_oLogFile, "%d,%d,%d,%d\n",
          get3PosState(SA),
          get3PosState(SB),
          // get3PosState(SC),
          get2PosState(SE),
          get3PosState(SF));
#elif defined(PCBTARANIS) || defined(PCBHORUS)
      int result = f_printf(&g_oLogFile, "%d,%d,%d,%d,%d,%d,%d,%d,%lu%lu\n",
          get3PosState(SA),
          get3PosState(SB),
          get3PosState(SC),
          get3PosState(SD),
          get3PosState(SE),
          get2PosState(SF),
          get3PosState(SG),
          get2PosState(SH),
          getLogicalSwitchesStates(32),
          getLogicalSwitchesStates(0));
#else
      int result = f_printf(&g_oLogFile, "%d,%d,%d,%d,%d,%d,%d\n",
          get2PosState(THR),
          get2PosState(RUD),
          get2PosState(ELE),
          get3PosState(ID),
          get2PosState(AIL),
          get2PosState(GEA),
          get2PosState(TRN));
#endif

      if (result<0 && !error_displayed) {
        error_displayed = STR_SDCARD_ERROR;
        POPUP_WARNING(STR_SDCARD_ERROR);
        closeLogs();
      }
    }
  }
  else {
    error_displayed = NULL;
    if (g_oLogFile.fs) {
      closeLogs();
    }
  }
}



