/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <string.h>
#include <stdlib.h>  // abs()

#include "app/chFrScanner.h"
#include "app/dtmf.h"
#ifdef ENABLE_AM_FIX
    #include "am_fix.h"
#endif
#include "bitmaps.h"
#include "board.h"
#include "driver/bk4819.h"
//#include "driver/st7565.h"
#include "printf.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
//#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "ui/ui.h"
#include "audio.h"

#include "ui/gui.h"

#ifdef ENABLE_FEAT_F4HWN
    #include "driver/system.h"
#endif

center_line_t center_line = CENTER_LINE_NONE;

static uint8_t  gLastRxVfo = 0;
static bool     gLastRxVfoValid = false;
static uint16_t gLastRxBlinkCountdown = 0; // 500ms units, counts down after last RX

#ifdef ENABLE_FEAT_F4HWN
    static int8_t RxBlink;
    static int8_t RxBlinkLed = 0;
    static int8_t RxBlinkLedCounter;
    static int8_t RxLine;
    static uint32_t RxOnVfofrequency;

    bool isMainOnlyInputDTMF = false;

    static bool isMainOnly()
    {
        return (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF) && (gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF);
    }
#endif

const int8_t dBmCorrTable[7] = {
            -15, // band 1
            -25, // band 2
            -20, // band 3
            -4, // band 4
            -7, // band 5
            -6, // band 6
             -1  // band 7
        };

const char *VfoStateStr[] = {
       [VFO_STATE_NORMAL]="",
       [VFO_STATE_BUSY]="BUSY",
       [VFO_STATE_BAT_LOW]="BAT LOW",
       [VFO_STATE_TX_DISABLE]="TX DISABLE",
       [VFO_STATE_TIMEOUT]="TIMEOUT",
       [VFO_STATE_ALARM]="ALARM",
       [VFO_STATE_VOLTAGE_HIGH]="VOLT HIGH"
};

// ***************************************************************************
/*
#if defined ENABLE_AUDIO_BAR || defined ENABLE_RSSI_BAR
#ifdef ENABLE_FEAT_F4HWN
static const uint8_t kBarHollowMet[]  = {0b01111111, 0b01000001, 0b01000001, 0b01111111};
static const uint8_t kBarHollow[]     = {0b00111110, 0b00100010, 0b00100010, 0b00111110};
static const uint8_t kBarSolid[]      = {0b00111110, 0b00111110, 0b00111110, 0b00111110};
#else
static const uint8_t kBarHollow[]     = {0b01111111, 0b01000001, 0b01000001, 0b01111111};
#endif
#endif
*/
static void DrawSmallAntennaAndBars(uint8_t *p, unsigned int level)
{
    if(level>6)
        level = 6;

    memcpy(p, BITMAP_Antenna, ARRAY_SIZE(BITMAP_Antenna));

    for(uint8_t i = 1; i <= level; i++) {
        char bar = (0xff << (6-i)) & 0x7F;
        memset(p + 2 + i*3, bar, 2);
    }
}
#if defined ENABLE_AUDIO_BAR || defined ENABLE_RSSI_BAR

/*static void DrawLevelBar(uint8_t xpos, uint8_t line, uint8_t level, uint8_t bars)
{
    uint8_t *p_line = gFrameBuffer[line];
    level = MIN(level, bars);

    for(uint8_t i = 0; i < level; i++) {
#ifdef ENABLE_FEAT_F4HWN
        uint8_t *p_bar = p_line + xpos + i * 5;
        if(gSetting_set_met)
        {
            if(i < bars - 4) {
                const uint8_t barPixel = (~(0x7F >> (i + 1))) & 0x7F;
                memset(p_bar, barPixel, 4);
            }
            else {
                memcpy(p_bar, kBarHollowMet, ARRAY_SIZE(kBarHollowMet));
            }
        }
        else
        {
            if(i < bars - 4) {
                memcpy(p_bar, kBarSolid, ARRAY_SIZE(kBarSolid));
            }
            else {
                memcpy(p_bar, kBarHollow, ARRAY_SIZE(kBarHollow));
            }
        }
#else
        uint8_t *p_bar = p_line + xpos + i * 5;
        if(i < bars - 4) {
            const uint8_t barPixel = (~(0x7F >> (i + 1))) & 0x7F;
            memset(p_bar, barPixel, 4);
        }
        else {
            memcpy(p_bar, kBarHollow, ARRAY_SIZE(kBarHollow));
        }
#endif
    }
}*/
#endif

#ifdef ENABLE_AUDIO_BAR
/*
// Approximation of a logarithmic scale using integer arithmetic
static inline uint8_t log2_approx(unsigned int value) {
    uint8_t log = 0;
    while (value >>= 1) {
        log++;
    }
    return log;
}

void UI_DisplayAudioBar(void)
{
    if (gSetting_mic_bar)
    {
        if(gLowBattery && !gLowBatteryConfirmed)
            return;

#ifdef ENABLE_FEAT_F4HWN
        RxBlinkLed = 0;
        RxBlinkLedCounter = 0;
        BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, false);
        unsigned int line;
        if (isMainOnly())
        {
            line = 5;
        }
        else
        {
            line = 3;
        }
#else
        const unsigned int line = 3;
#endif

        if (gCurrentFunction != FUNCTION_TRANSMIT ||
            gScreenToDisplay != DISPLAY_MAIN
#ifdef ENABLE_DTMF_CALLING
            || gDTMF_CallState != DTMF_CALL_STATE_NONE
#endif
            )
        {
            return;  // screen is in use
        }

#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
        if (gAlarmState != ALARM_STATE_OFF)
            return;
#endif
        static uint8_t barsOld = 0;
        const uint8_t thresold = 18; // arbitrary thresold
        //const uint8_t barsList[] = {0, 0, 0, 1, 2, 3, 4, 5, 6, 8, 10, 13, 16, 20, 25, 25};
        const uint8_t barsList[] = {0, 0, 0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 21, 25, 25, 25};
        uint8_t logLevel;
        uint8_t bars;

        unsigned int voiceLevel  = BK4819_GetVoiceAmplitudeOut();  // 15:0

        voiceLevel = (voiceLevel >= thresold) ? (voiceLevel - thresold) : 0;
        logLevel = log2_approx(MIN(voiceLevel * 16, 32768u) + 1);
        bars = barsList[logLevel];
        barsOld = (barsOld - bars > 1) ? (barsOld - 1) : bars;

        uint8_t *p_line = gFrameBuffer[line];
        memset(p_line, 0, LCD_WIDTH);

        DrawLevelBar(2, line, barsOld, 25);

        if (gCurrentFunction == FUNCTION_TRANSMIT)
            UI_UpdateDisplay();
    }
}
    */
#endif


uint8_t convertRSSIToSLevel(int16_t rssi_dBm)
{
    static const int16_t sLevelThresholds[] = {
        -121, -115, -109, -103, -97, -91, -85, -79, -73, -67
    };

    for (uint8_t level = 0; level < ARRAY_SIZE(sLevelThresholds); level++) {
        if (rssi_dBm <= sLevelThresholds[level]) {
            return level;
        }
    }

    return 10;
}

static inline int16_t convertRSSIToPlusDB(int16_t rssi_dBm)
{
    return (rssi_dBm > -67) ? (rssi_dBm + 67) : 0;
}

void DisplayRSSIBar(const bool now)
{
    uint8_t posX = 1;
    uint8_t posY = 52;
    uint8_t sValue = 0;
    int16_t plusDB = 0;

    if(FUNCTION_IsRx()) {
        int16_t rssi_dBm = BK4819_GetRSSI_dBm()
        #ifdef ENABLE_AM_FIX
            + ((gSetting_AM_fix && gRxVfo->Modulation == MODULATION_AM) ? AM_fix_get_gain_diff() : 0)
        #endif
            + dBmCorrTable[gRxVfo->Band];
            
        sValue = convertRSSIToSLevel(rssi_dBm); // Convert RSSI to S-level
        if (sValue == 10) {
            plusDB = convertRSSIToPlusDB(rssi_dBm); // Convert to +dB value if greater than S9
        }
    }

    UI_DrawRSSI(sValue, posX, posY + 1);

    UI_SetFont(FONT_8_TR);
    if (sValue > 0) {
        if (sValue == 10) {
            UI_DrawString(UI_TEXT_ALIGN_LEFT, posX + 38, 0, posY + 5, true, false, false, "S9");
            UI_DrawStringf(UI_TEXT_ALIGN_LEFT, posX + 38, 0, posY + 12, true, false, false, "+%idB", plusDB);
        }
        else {
            UI_DrawStringf(UI_TEXT_ALIGN_LEFT, posX + 38, 0, posY + 5, true, false, false, "S%i", sValue);
        }
    }
    if (now) 
    {
        UI_UpdateDisplay();
    }
}


#ifdef ENABLE_AGC_SHOW_DATA
void UI_MAIN_PrintAGC(bool now)
{
    char buf[20];
    memset(gFrameBuffer[3], 0, 128);
    union {
        struct {
            uint16_t _ : 5;
            uint16_t agcSigStrength : 7;
            int16_t gainIdx : 3;
            uint16_t agcEnab : 1;
        };
        uint16_t __raw;
    } reg7e;
    reg7e.__raw = BK4819_ReadRegister(0x7E);
    uint8_t gainAddr = reg7e.gainIdx < 0 ? 0x14 : 0x10 + reg7e.gainIdx;
    union {
        struct {
            uint16_t pga:3;
            uint16_t mixer:2;
            uint16_t lna:3;
            uint16_t lnaS:2;
        };
        uint16_t __raw;
    } agcGainReg;
    agcGainReg.__raw = BK4819_ReadRegister(gainAddr);
    int8_t lnaShortTab[] = {-28, -24, -19, 0};
    int8_t lnaTab[] = {-24, -19, -14, -9, -6, -4, -2, 0};
    int8_t mixerTab[] = {-8, -6, -3, 0};
    int8_t pgaTab[] = {-33, -27, -21, -15, -9, -6, -3, 0};
    int16_t agcGain = lnaShortTab[agcGainReg.lnaS] + lnaTab[agcGainReg.lna] + mixerTab[agcGainReg.mixer] + pgaTab[agcGainReg.pga];

    sprintf(buf, "%d%2d %2d %2d %3d", reg7e.agcEnab, reg7e.gainIdx, -agcGain, reg7e.agcSigStrength, BK4819_GetRSSI());
    //UI_PrintStringSmallNormal(buf, 2, 0, 3);
    //if(now)
        //ST7565_BlitLine(3);
}
#endif

void UI_MAIN_TimeSlice500ms(void)
{
    if(gScreenToDisplay==DISPLAY_MAIN) {
        static bool sLastBlinkPhase = false;
        static bool sLastBlinkActive = false;

        if (gLastRxBlinkCountdown > 0) {
            gLastRxBlinkCountdown--;
        }

        const bool blinkActive = gLastRxVfoValid && (gLastRxBlinkCountdown > 0);
        const bool blinkPhase = blinkActive && (((gFlashLightBlinkCounter / 50u) & 1u) == 0u); // ~500ms toggle (10ms units)

        if ((blinkActive != sLastBlinkActive) || (blinkActive && blinkPhase != sLastBlinkPhase)) {
            gUpdateDisplay = true;
        }
        sLastBlinkPhase = blinkPhase;
        sLastBlinkActive = blinkActive;

#ifdef ENABLE_AGC_SHOW_DATA
        UI_MAIN_PrintAGC(true);
        return;
#endif
        DisplayRSSIBar(true);

        if(FUNCTION_IsRx()) {
            
        }
#ifdef ENABLE_FEAT_F4HWN // Blink Green Led for white...
        else if(gSetting_set_eot > 0 && RxBlinkLed == 2)
        {
            if(RxBlinkLedCounter <= 8)
            {
                if(RxBlinkLedCounter % 2 == 0)
                {
                    if(gSetting_set_eot > 1 )
                    {
                        BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, false);
                    }
                }
                else
                {
                    if(gSetting_set_eot > 1 )
                    {
                        BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, true);
                    }

                    if(gSetting_set_eot == 1 || gSetting_set_eot == 3)
                    {
                        switch(RxBlinkLedCounter)
                        {
                            case 1:
                            AUDIO_PlayBeep(BEEP_400HZ_30MS);
                            break;

                            case 3:
                            AUDIO_PlayBeep(BEEP_400HZ_30MS);
                            break;

                            case 5:
                            AUDIO_PlayBeep(BEEP_500HZ_30MS);
                            break;

                            case 7:
                            AUDIO_PlayBeep(BEEP_600HZ_30MS);
                            break;
                        }
                    }
                }
                RxBlinkLedCounter += 1;
            }
            else
            {
                RxBlinkLed = 0;
            }
        }
#endif
    }
}

// ***************************************************************************

void UI_DisplayMain(void)
{
    char               String[22];

    center_line = CENTER_LINE_NONE;

    // clear the screen
    UI_ClearDisplay();    

    unsigned int activeTxVFO = gRxVfoIsActive ? gEeprom.RX_VFO : gEeprom.TX_VFO;
    unsigned int vfoA = gEeprom.TX_VFO == 0 ? 0 : 1;
    unsigned int vfoB = gEeprom.TX_VFO == 0 ? 1 : 0;

    uint32_t displayFreqVFO1 = gEeprom.VfoInfo[vfoA].pRX->Frequency;
    uint32_t displayFreqVFO2 = gEeprom.VfoInfo[vfoB].pRX->Frequency;

    const VFO_Info_t *vfoInfoA = &gEeprom.VfoInfo[vfoA];
    const VFO_Info_t *vfoInfoB = &gEeprom.VfoInfo[vfoB];

    bool rxVFO1 = false;
    bool rxVFO2 = false;
    bool txVFO1 = false;

    if (gCurrentFunction == FUNCTION_TRANSMIT)
    {   // transmitting
        txVFO1 = true;
        displayFreqVFO1 = gEeprom.VfoInfo[vfoA].pTX->Frequency;
    }
    else
    {   // receiving .. 

        if (FUNCTION_IsRx())
        {
            rxVFO1 = (gEeprom.RX_VFO == vfoA && VfoState[vfoA] == VFO_STATE_NORMAL);
            rxVFO2 = (gEeprom.RX_VFO == vfoB && VfoState[vfoB] == VFO_STATE_NORMAL);

            if ((rxVFO1 || rxVFO2) && gEeprom.RX_VFO < 2) {
                gLastRxVfo = gEeprom.RX_VFO;
                gLastRxVfoValid = true;
                gLastRxBlinkCountdown = 240; // 120s (500ms steps)
            }
        }
    }

    const bool blinkActive = gLastRxVfoValid && (gLastRxBlinkCountdown > 0);
    const bool lastRxBlinkPhase = blinkActive && (((gFlashLightBlinkCounter / 50u) & 1u) == 0u); // ~500ms toggle (10ms units)

    // draw VFO1 area

    UI_SetBlackColor();
    UI_DrawBox(0, 0, 128, 7);

    UI_SetFont(FONT_8B_TR);

    SETTINGS_FetchChannelName(String, gEeprom.ScreenChannel[vfoA]);
    if (String[0] == 0)
    {   // no channel name, show the channel number instead
        // UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 1, 0, 6, false, false, false, "CH-%03u", gEeprom.ScreenChannel[vfoA] + 1);
        // TODO
        // Show Band name
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 1, 0, 6, false, false, false, "VFO");
    } else {
        // show the channel name
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 1, 0, 6, false, false, false, String);
    }

    const bool lastRxIsVfoA = (gLastRxVfoValid && gLastRxVfo == vfoA);
    const bool vfoALabelFill = (!blinkActive || !lastRxIsVfoA) ? true : lastRxBlinkPhase;
    if (vfoALabelFill) {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 2, 0, 14, true, true, false, vfoA == 0 ? "A" : "B");
    }
    if (rxVFO1) {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 12, 0, 14, true, true, false, UI_RX_STR);
    } else if (txVFO1) {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 12, 0, 14, true, true, false, UI_TX_STR);
    }

    if (IS_MR_CHANNEL(gEeprom.ScreenChannel[vfoA]))
    {   // channel mode
        UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 1, 0, 22, true, false, false, "M-%03u", gEeprom.ScreenChannel[vfoA] + 1);
    }
    else if (IS_FREQ_CHANNEL(gEeprom.ScreenChannel[vfoA]))
    {   // frequency mode
        // show the frequency band number
        UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 1, 0, 22, true, false, false, "F-%03u", 1 + gEeprom.ScreenChannel[vfoA] - FREQ_CHANNEL_FIRST);
    }
    
    UI_SetFont(FONT_5_TR);

    uint8_t codeXend = 127;

    const FREQ_Config_t *pConfigRX = vfoInfoA->pRX;
    const FREQ_Config_t *pConfigTX = vfoInfoA->pTX;
    const unsigned int code_type = pConfigRX->CodeType;
    
    if ( pConfigRX->CodeType == CODE_TYPE_OFF && pConfigTX->CodeType == CODE_TYPE_OFF ) {        
        UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, codeXend, 26, true, false, false, "%d.%02uK", vfoInfoA->StepFrequency / 100, vfoInfoA->StepFrequency % 100);
    } else {        
        // RX code
        if ( pConfigRX->CodeType == CODE_TYPE_CONTINUOUS_TONE )
        {
            UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, codeXend, 26, true, g_CTCSS_Lost, false, "%s %u.%u%s", UI_RX_STR, CTCSS_Options[pConfigRX->Code] / 10, CTCSS_Options[pConfigRX->Code] % 10, UI_HZ_STR);
        } else if ( pConfigRX->CodeType == CODE_TYPE_DIGITAL )
        {
            UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, codeXend, 26, true, g_CDCSS_Lost, false, "%s %03oN", UI_RX_STR, DCS_Options[pConfigRX->Code]);
        } else if ( pConfigRX->CodeType == CODE_TYPE_REVERSE_DIGITAL )
        {
            UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, codeXend, 26, true, g_CDCSS_Lost, false, "%s %03oI", UI_RX_STR, DCS_Options[pConfigRX->Code]);
        }
        if (pConfigRX->CodeType != CODE_TYPE_OFF) {
            codeXend -= 48;
        }

        // TX code
        if ( pConfigTX->CodeType == CODE_TYPE_CONTINUOUS_TONE )
        {
            UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, codeXend, 26, true, txVFO1, false, "%s %u.%u%s", UI_TX_STR, CTCSS_Options[pConfigTX->Code] / 10, CTCSS_Options[pConfigTX->Code] % 10, UI_HZ_STR);
        } else if ( pConfigTX->CodeType == CODE_TYPE_DIGITAL )
        {
            UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, codeXend, 26, true, txVFO1, false, "%s %03oN", UI_TX_STR, DCS_Options[pConfigTX->Code]);
        } else if ( pConfigTX->CodeType == CODE_TYPE_REVERSE_DIGITAL )
        {
            UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, codeXend, 26, true, txVFO1, false, "%s %03oI", UI_TX_STR, DCS_Options[pConfigTX->Code]);
        }
    }

    const ModulationMode_t modA = vfoInfoA->Modulation;    
    const char* bandwidthA = UI_GetStrValue(UI_BANDWIDTH_STR, (uint8_t)vfoInfoA->CHANNEL_BANDWIDTH);
    uint8_t currentPowerA = vfoInfoA->OUTPUT_POWER % 8;
    if(currentPowerA == OUTPUT_POWER_USER)
    {
        currentPowerA = gSetting_set_pwr;
    }
    else
    {
        currentPowerA--;
    }
    const char* powerA = UI_GetStrValue(UI_POWER_STR, currentPowerA);
    UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, 127, 6, false, false, false, "%.*s %.*s %.*s", UI_StringLengthNL(gModulationStr[modA]), gModulationStr[modA], UI_StringLengthNL(bandwidthA), bandwidthA, UI_StringLengthNL(powerA), powerA);

    bool invertFreqVFO1 = txVFO1 || rxVFO1;
    
    //if (vfoInfoA->freq_config_RX.Frequency != vfoInfoA->freq_config_TX.Frequency)
    if (vfoInfoA->pRX->Frequency != vfoInfoA->pTX->Frequency)
    {   // show the TX offset symbol
        if (vfoInfoA->FrequencyReverse)
        {
            UI_SetFont(FONT_8B_TR);
            UI_DrawString(UI_TEXT_ALIGN_LEFT, 40, 0, 17, true, false, false, "R");
        } else {
            int i = vfoInfoA->TX_OFFSET_FREQUENCY_DIRECTION % 3;
            UI_SetFont(UI_FONT_BN_TN);
            UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 40, 0, 19, true, false, false, "%s", i == 1 ? "+" : (i == 2 ? "-" : ""));
        }
    }    

    UI_DrawFrequencyBig(invertFreqVFO1, displayFreqVFO1, 111, 19);


    // draw VFO2 area
    uint8_t vfoBY = 28;

    UI_SetBlackColor();
    UI_DrawBox(0, vfoBY, 128, 7);
    
    UI_SetFont(FONT_8_TR);
    SETTINGS_FetchChannelName(String, gEeprom.ScreenChannel[vfoB]);
    if (String[0] == 0)
    {   // no channel name, show the channel number instead
        //UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 1, 0, vfoBY + 6, false, false, false, "CH-%03u", gEeprom.ScreenChannel[vfoB] + 1);
        // TODO
        // Show Band name
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 1, 0, vfoBY + 6, false, false, false, "VFO");
    } else {
        // show the channel name
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 1, 0, vfoBY + 6, false, false, false, String);
    }

    const ModulationMode_t modB = vfoInfoB->Modulation;    
    const char* bandwidthB = UI_GetStrValue(UI_BANDWIDTH_STR, (uint8_t)vfoInfoB->CHANNEL_BANDWIDTH);
    uint8_t currentPowerB = vfoInfoB->OUTPUT_POWER % 8;
    if(currentPowerB == OUTPUT_POWER_USER)
    {
        currentPowerB = gSetting_set_pwr;
    }
    else
    {
        currentPowerB--;
    }    

    UI_SetFont(FONT_5_TR);
    const char* powerB = UI_GetStrValue(UI_POWER_STR, currentPowerB);
    UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, 127, vfoBY + 6, false, false, false, "%.*s %.*s %.*s", UI_StringLengthNL(gModulationStr[modB]), gModulationStr[modB], UI_StringLengthNL(bandwidthB), bandwidthB, UI_StringLengthNL(powerB), powerB);

    UI_SetFont(FONT_8B_TR);
    const bool lastRxIsVfoB = (gLastRxVfoValid && gLastRxVfo == vfoB);
    const bool vfoBLabelFill = (blinkActive && lastRxIsVfoB) ? !lastRxBlinkPhase : true;
    if (vfoBLabelFill) {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 2, 0, vfoBY + 15, true, false, true, vfoB == 0 ? "A" : "B");
    }
    if (rxVFO2)
    {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 12, 0, vfoBY + 15, true, true, false, UI_RX_STR);
    } else {
        UI_SetFont(FONT_5_TR);
        // not receiving on VFO2
        if (IS_MR_CHANNEL(gEeprom.ScreenChannel[vfoB]))
        {   // channel mode
            UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 12, 0, vfoBY + 15, true, false, false, "M-%03u", gEeprom.ScreenChannel[vfoB] + 1);
        }
        else if (IS_FREQ_CHANNEL(gEeprom.ScreenChannel[vfoB]))
        {   // frequency mode
            // show the frequency band number
            UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 12, 0, vfoBY + 15, true, false, false, "F-%03u", 1 + gEeprom.ScreenChannel[vfoB] - FREQ_CHANNEL_FIRST);
        }
    }
    UI_DrawFrequencySmall(rxVFO2, displayFreqVFO2, 126, vfoBY + 17);


    // draw bottom status area

    UI_SetFont(FONT_5_TR);
    // Status info
    if (gChargingWithTypeC) 
    {
        UI_DrawIc8Charging(114, 52, true);
    }
    else 
    {
        UI_DrawBatteryIcon(BATTERY_VoltsToPercent(gBatteryVoltageAverage), 114, 52);
    }
    UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, 128, 64, true, false, false, "%i%%", BATTERY_VoltsToPercent(gBatteryVoltageAverage));

    //

    uint8_t dw = (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) + (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) * 2;

    if(dw == 1 || dw == 3) { // DWR - dual watch
        if(gDualWatchActive) {
            UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, 108, 64, true, false, false, "A/B");            
        }
        else
        {            
            UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, 108, 64, true, false, false, gLastRxVfo == vfoA ? "A" : "B");
        }
    }
    else if(dw == 2) { // XB - crossband
        UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, 108, 64, true, false, false, "X");
    }
    else
    {        
        UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, 108, 64, true, false, false, "M");
    }    

    if (gCurrentFunction == FUNCTION_POWER_SAVE)
    {
        UI_DrawPs(78, 59, true);
    }
    
    // TODO : create icons for the following statuses
    if (gEeprom.KEY_LOCK) {
        UI_DrawLock(88, 50, false);
    }
    else if (gWasFKeyPressed) {
        UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, 97, 56, true, true, false, "F");        
    }    
    else if (gMute) {
        UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, 97, 56, true, true, false, "M");
    }

    DisplayRSSIBar(false);

    enum VfoState_t stateA = VfoState[0];
    enum VfoState_t stateB = VfoState[1];


    // TODO : use UI_SetInfoMessage
    if (stateA != VFO_STATE_NORMAL || stateB != VFO_STATE_NORMAL)
    {
        UI_DrawPopupWindow(20, 20, 88, 28, "Info");
        UI_SetFont(FONT_8B_TR);
        if (stateA != VFO_STATE_NORMAL)
        {
            UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 36, true, false, false, "VFO A");            
            UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 44, true, false, false, VfoStateStr[stateA]);
        } else if (stateB != VFO_STATE_NORMAL)
        {
            UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 36, true, false, false, "VFO B");
            UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 44, true, false, false, VfoStateStr[stateB]);   
        }        
    }

    if(gLowBattery && !gLowBatteryConfirmed) {
        UI_SetInfoMessage(UI_INFO_LOW_BATTERY);        
    }

    if (gEeprom.KEY_LOCK && gKeypadLocked > 0)
    {   // tell user how to unlock the keyboard
        UI_DrawPopupWindow(20, 20, 88, 28, "Info");
        UI_SetFont(FONT_8B_TR);
        UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 36, true, false, false, "Long press #");
        UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 44, true, false, false, "to unlock");        
    }

    UI_UpdateDisplay();
}

// ***************************************************************************
