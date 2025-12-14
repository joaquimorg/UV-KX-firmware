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
#include "driver/st7565.h"
#include "printf.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "ui/ui.h"
#include "audio.h"

#include "ui/gui.h"

#ifdef ENABLE_FEAT_F4HWN
    #include "driver/system.h"
#endif

center_line_t center_line = CENTER_LINE_NONE;

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

static void DrawLevelBar(uint8_t xpos, uint8_t line, uint8_t level, uint8_t bars)
{
#ifndef ENABLE_FEAT_F4HWN
    const char hollowBar[] = {
        0b01111111,
        0b01000001,
        0b01000001,
        0b01111111
    };
#endif
    
    uint8_t *p_line = gFrameBuffer[line];
    level = MIN(level, bars);

    for(uint8_t i = 0; i < level; i++) {
#ifdef ENABLE_FEAT_F4HWN
        if(gSetting_set_met)
        {
            const char hollowBar[] = {
                0b01111111,
                0b01000001,
                0b01000001,
                0b01111111
            };

            if(i < bars - 4) {
                for(uint8_t j = 0; j < 4; j++)
                    p_line[xpos + i * 5 + j] = (~(0x7F >> (i + 1))) & 0x7F;
            }
            else {
                memcpy(p_line + (xpos + i * 5), &hollowBar, ARRAY_SIZE(hollowBar));
            }
        }
        else
        {
            const char hollowBar[] = {
                0b00111110,
                0b00100010,
                0b00100010,
                0b00111110
            };

            const char simpleBar[] = {
                0b00111110,
                0b00111110,
                0b00111110,
                0b00111110
            };

            if(i < bars - 4) {
                memcpy(p_line + (xpos + i * 5), &simpleBar, ARRAY_SIZE(simpleBar));
            }
            else {
                memcpy(p_line + (xpos + i * 5), &hollowBar, ARRAY_SIZE(hollowBar));
            }
        }
#else
        if(i < bars - 4) {
            for(uint8_t j = 0; j < 4; j++)
                p_line[xpos + i * 5 + j] = (~(0x7F >> (i+1))) & 0x7F;
        }
        else {
            memcpy(p_line + (xpos + i * 5), &hollowBar, ARRAY_SIZE(hollowBar));
        }
#endif
    }
}
#endif

#ifdef ENABLE_AUDIO_BAR

// Approximation of a logarithmic scale using integer arithmetic
uint8_t log2_approx(unsigned int value) {
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
#endif


uint8_t convertRSSIToSLevel(int16_t rssi_dBm) {
    if (rssi_dBm <= -121) {
        return 0; // S0
    }
    else if (rssi_dBm <= -115) {
        return 1; // S1
    }
    else if (rssi_dBm <= -109) {
        return 2; // S2
    }
    else if (rssi_dBm <= -103) {
        return 3; // S3
    }
    else if (rssi_dBm <= -97) {
        return 4; // S4
    }
    else if (rssi_dBm <= -91) {
        return 5; // S5
    }
    else if (rssi_dBm <= -85) {
        return 6; // S6
    }
    else if (rssi_dBm <= -79) {
        return 7; // S7
    }
    else if (rssi_dBm <= -73) {
        return 8; // S8
    }
    else if (rssi_dBm <= -67) {
        return 9; // S9
    }
    else {
        return 10; // Greater than S9
    }
}

int16_t convertRSSIToPlusDB(int16_t rssi_dBm) {
    if (rssi_dBm > -67) {
        return (rssi_dBm + 67); // Convert to +dB value
    }
    return 0; // Return 0 if not greater than S9
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

void DisplayRSSIBar_old(const bool now)
{
#if defined(ENABLE_RSSI_BAR)

    const unsigned int txt_width    = 7 * 8;                 // 8 text chars
    const unsigned int bar_x        = 2 + txt_width + 4;     // X coord of bar graph

#ifdef ENABLE_FEAT_F4HWN
    /*
    const char empty[] = {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    };
    */

    unsigned int line;
    if (isMainOnly())
    {
        line = 5;
    }
    else
    {
        line = 3;
    }

    //char rx[4];
    //sprintf(String, "%d", RxBlink);
    //UI_PrintStringSmallBold(String, 80, 0, RxLine);

    if(RxLine >= 0 && center_line != CENTER_LINE_IN_USE)
    {
        if (RxBlink == 0 || RxBlink == 1) {
            UI_PrintStringSmallBold("RX", 8, 0, RxLine);
            if (RxBlink == 1) RxBlink = 2;
        } else {
            for (uint8_t i = 8; i < 24; i++)
            {
                gFrameBuffer[RxLine][i] = 0x00;
            }
            RxBlink = 1;
        }
        ST7565_BlitLine(RxLine);
    }
#else
    const unsigned int line = 3;
#endif
    uint8_t           *p_line        = gFrameBuffer[line];
    char               str[16];

#ifndef ENABLE_FEAT_F4HWN
    const char plus[] = {
        0b00011000,
        0b00011000,
        0b01111110,
        0b01111110,
        0b01111110,
        0b00011000,
        0b00011000,
    };
#endif

    if ((gEeprom.KEY_LOCK && gKeypadLocked > 0) || center_line != CENTER_LINE_RSSI)
        return;     // display is in use

    if (gCurrentFunction == FUNCTION_TRANSMIT ||
        gScreenToDisplay != DISPLAY_MAIN
#ifdef ENABLE_DTMF_CALLING
        || gDTMF_CallState != DTMF_CALL_STATE_NONE
#endif
        )
        return;     // display is in use

    if (now)
        memset(p_line, 0, LCD_WIDTH);

#ifdef ENABLE_FEAT_F4HWN
    int16_t rssi_dBm =
        BK4819_GetRSSI_dBm()
#ifdef ENABLE_AM_FIX
        + ((gSetting_AM_fix && gRxVfo->Modulation == MODULATION_AM) ? AM_fix_get_gain_diff() : 0)
#endif
        + dBmCorrTable[gRxVfo->Band];

    rssi_dBm = -rssi_dBm;

    if(rssi_dBm > 141) rssi_dBm = 141;
    if(rssi_dBm < 53) rssi_dBm = 53;

    uint8_t s_level = 0;
    uint8_t overS9dBm = 0;
    uint8_t overS9Bars = 0;

    if(rssi_dBm >= 93) {
        s_level = map(rssi_dBm, 141, 93, 1, 9);
    }
    else {
        s_level = 9;
        overS9dBm = map(rssi_dBm, 93, 53, 0, 40);
        overS9Bars = map(overS9dBm, 0, 40, 0, 4);
    }
#else
    const int16_t s0_dBm   = -gEeprom.S0_LEVEL;                  // S0 .. base level
    const int16_t rssi_dBm =
        BK4819_GetRSSI_dBm()
#ifdef ENABLE_AM_FIX
        + ((gSetting_AM_fix && gRxVfo->Modulation == MODULATION_AM) ? AM_fix_get_gain_diff() : 0)
#endif
        + dBmCorrTable[gRxVfo->Band];

    int s0_9 = gEeprom.S0_LEVEL - gEeprom.S9_LEVEL;
    const uint8_t s_level = MIN(MAX((int32_t)(rssi_dBm - s0_dBm)*100 / (s0_9*100/9), 0), 9); // S0 - S9
    uint8_t overS9dBm = MIN(MAX(rssi_dBm + gEeprom.S9_LEVEL, 0), 99);
    uint8_t overS9Bars = MIN(overS9dBm/10, 4);
#endif

#ifdef ENABLE_FEAT_F4HWN
    if (gSetting_set_gui)
    {
        sprintf(str, "%3d", -rssi_dBm);
        UI_PrintStringSmallNormal(str, LCD_WIDTH + 8, 0, line - 1);
    }
    else
    {
        sprintf(str, "% 4d %s", -rssi_dBm, "dBm");
        if(isMainOnly())
            GUI_DisplaySmallest(str, 2, 41, false, true);
        else
            GUI_DisplaySmallest(str, 2, 25, false, true);
    }

    if(overS9Bars == 0) {
        sprintf(str, "S%d", s_level);
    }
    else {
        sprintf(str, "+%02d", overS9dBm);
    }

    UI_PrintStringSmallNormal(str, LCD_WIDTH + 38, 0, line - 1);
#else
    if(overS9Bars == 0) {
        sprintf(str, "% 4d S%d", -rssi_dBm, s_level);
    }
    else {
        sprintf(str, "% 4d  %2d", -rssi_dBm, overS9dBm);
        memcpy(p_line + 2 + 7*5, &plus, ARRAY_SIZE(plus));
    }

    UI_PrintStringSmallNormal(str, 2, 0, line);
#endif
    DrawLevelBar(bar_x, line, s_level + overS9Bars, 13);
    if (now)
        ST7565_BlitLine(line);
#else
    int16_t rssi = BK4819_GetRSSI();
    uint8_t Level;

    if (rssi >= gEEPROM_RSSI_CALIB[gRxVfo->Band][3]) {
        Level = 6;
    } else if (rssi >= gEEPROM_RSSI_CALIB[gRxVfo->Band][2]) {
        Level = 4;
    } else if (rssi >= gEEPROM_RSSI_CALIB[gRxVfo->Band][1]) {
        Level = 2;
    } else if (rssi >= gEEPROM_RSSI_CALIB[gRxVfo->Band][0]) {
        Level = 1;
    } else {
        Level = 0;
    }

    uint8_t *pLine = (gEeprom.RX_VFO == 0)? gFrameBuffer[2] : gFrameBuffer[6];
    if (now)
        memset(pLine, 0, 23);
    DrawSmallAntennaAndBars(pLine, Level);
    if (now)
        UI_UpdateDisplay();
#endif

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
    UI_PrintStringSmallNormal(buf, 2, 0, 3);
    if(now)
        ST7565_BlitLine(3);
}
#endif

void UI_MAIN_TimeSlice500ms(void)
{
    if(gScreenToDisplay==DISPLAY_MAIN) {
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

    if(gLowBattery && !gLowBatteryConfirmed) {
        UI_SetInfoMessage(UI_INFO_LOW_BATTERY);
        UI_UpdateDisplay();
        return;
    }

    if (gEeprom.KEY_LOCK && gKeypadLocked > 0)
    {   // tell user how to unlock the keyboard
        UI_DrawPopupWindow(20, 20, 88, 28, "Info");
        UI_SetFont(FONT_8B_TR);
        UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 36, true, false, false, "Long press #");
        UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 44, true, false, false, "to unlock");
        UI_UpdateDisplay();
        return;
    }

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
        }
    }

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
    } else {
        // show the channel name
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 1, 0, 6, false, false, false, String);
    }

    UI_DrawString(UI_TEXT_ALIGN_LEFT, 2, 0, 14, true, true, false, vfoA == 0 ? "A" : "B");
    if (rxVFO1) {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 12, 0, 14, true, true, false, UI_RX_STR);
    } else if (txVFO1) {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 12, 0, 14, true, true, false, UI_TX_STR);
    }

    if (IS_MR_CHANNEL(gEeprom.ScreenChannel[vfoA]))
    {   // channel mode
        UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 0, 0, 22, true, false, false, "M-%03u", gEeprom.ScreenChannel[vfoA] + 1);
    }
    else if (IS_FREQ_CHANNEL(gEeprom.ScreenChannel[vfoA]))
    {   // frequency mode
        // show the frequency band number
        UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 0, 0, 22, true, false, false, "F-%03u", 1 + gEeprom.ScreenChannel[vfoA] - FREQ_CHANNEL_FIRST);
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
        int i = vfoInfoA->TX_OFFSET_FREQUENCY_DIRECTION % 3;
        UI_SetFont(UI_FONT_BN_TN);
        UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 40, 0, 19, false, txVFO1, false, "%s", i == 1 ? "+" : (i == 2 ? "-" : ""));
    } else if (vfoInfoA->FrequencyReverse)
    {
        UI_SetFont(FONT_8B_TR);
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 40, 0, 19, false, txVFO1, false, "R");
    }

    UI_DrawFrequencyBig(invertFreqVFO1, displayFreqVFO1, 111, 19);


    // draw VFO2 area
    uint8_t vfoBY = 28;

    UI_SetBlackColor();
    UI_DrawBox(0, vfoBY, 128, 7);
    
    UI_SetFont(FONT_5_TR);
    SETTINGS_FetchChannelName(String, gEeprom.ScreenChannel[vfoB]);
    if (String[0] == 0)
    {   // no channel name, show the channel number instead
        //UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 1, 0, vfoBY + 6, false, false, false, "CH-%03u", gEeprom.ScreenChannel[vfoB] + 1);
        // TODO
        // Show Band name
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
    const char* powerB = UI_GetStrValue(UI_POWER_STR, currentPowerB);
    UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, 127, vfoBY + 6, false, false, false, "%.*s %.*s %.*s", UI_StringLengthNL(gModulationStr[modB]), gModulationStr[modB], UI_StringLengthNL(bandwidthB), bandwidthB, UI_StringLengthNL(powerB), powerB);

    UI_SetFont(FONT_8B_TR);
    UI_DrawString(UI_TEXT_ALIGN_LEFT, 2, 0, vfoBY + 15, true, false, true, vfoB == 0 ? "A" : "B");
    if (rxVFO2)
    {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, 12, 0, vfoBY + 15, true, true, false, UI_RX_STR);
    } else {
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

    // Status info
    if (gChargingWithTypeC) 
    {
        UI_DrawIc8Charging(118, 52, true);
    }
    else 
    {
        UI_DrawBatteryIcon(BATTERY_VoltsToPercent(gBatteryVoltageAverage), 114, 52);
    }

    UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, 108, 64, true, false, false, "A/B");

    if (gCurrentFunction == FUNCTION_POWER_SAVE)
    {
        UI_DrawPs(78, 59, true);
    }

    UI_SetFont(FONT_5_TR);
    UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, 128, 64, true, false, false, "%i%%", BATTERY_VoltsToPercent(gBatteryVoltageAverage));

    DisplayRSSIBar(false);

    UI_UpdateDisplay();
}

// ***************************************************************************
