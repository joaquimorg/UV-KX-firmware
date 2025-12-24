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

#include "driver/eeprom.h"
#include "printf.h"
#include "settings.h"
#include "misc.h"
#include "ui/welcome.h"
#include "version.h"
#include "ui/gui.h"

void UI_DisplayReleaseKeys(void)
{
    UI_ClearDisplay();
    UI_SetBlackColor();

    UI_DrawPopupWindow(20, 20, 88, 28, "Info");
    UI_SetFont(FONT_8B_TR);
    UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 36, true, false, false, "RELEASE");
    UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 44, true, false, false, "ALL KEYS");
    UI_UpdateDisplay();
}

void UI_DisplayWelcome(void)
{
    UI_ClearDisplay();
    UI_SetBlackColor();
    UI_SetFont(FONT_8B_TR);

    UI_DrawString(UI_TEXT_ALIGN_LEFT, 5, 0, 10, true, false, false, "Hello !");

    UI_DrawString(UI_TEXT_ALIGN_LEFT, 5, 0, 20, true, false, false, "UV-Kx Open Firmware");

    UI_DrawBatteryIcon(BATTERY_VoltsToPercent(gBatteryVoltageAverage), 20, 30);

    UI_SetFont(FONT_8_TR);
    UI_DrawStrf(8, 42, "%i%% %u.%02uV", BATTERY_VoltsToPercent(gBatteryVoltageAverage), gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100);

    UI_DrawString(UI_TEXT_ALIGN_LEFT, 64, 0, 42, true, false, false, "EEPROM");
    UI_DrawString(UI_TEXT_ALIGN_LEFT, 64, 0, 51, true, false, false, "FM");

    UI_SetFont(FONT_5_TR);
    UI_DrawString(UI_TEXT_ALIGN_LEFT, 110, 0, 42, true, false, false, "64");
    UI_DrawString(UI_TEXT_ALIGN_LEFT, 110, 0, 51, true, false, false, "YES");

    UI_DrawBox(0, 57, 128, 7);
    UI_DrawString(UI_TEXT_ALIGN_CENTER, 0, 128, 63, false, false, false, AUTHOR_STRING_2 " - " VERSION_STRING_2 " - " EDITION_STRING " EDITION");

    UI_UpdateDisplay();


    /*char WelcomeString0[16];
    char WelcomeString1[16];
    char WelcomeString2[16];
    char WelcomeString3[20];

    memset(gStatusLine,  0, sizeof(gStatusLine));

#if defined(ENABLE_FEAT_F4HWN_CTR) || defined(ENABLE_FEAT_F4HWN_INV)
        //ST7565_ContrastAndInv();
#endif
    UI_ClearDisplay();

#ifdef ENABLE_FEAT_F4HWN
    //ST7565_BlitStatusLine();
    UI_UpdateDisplay();
    
    if (gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_NONE || gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_SOUND) {
        //ST7565_FillScreen(0x00);
#else
    if (gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_NONE || gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_FULL_SCREEN) {
        //ST7565_FillScreen(0xFF);
#endif
    } else {
        memset(WelcomeString0, 0, sizeof(WelcomeString0));
        memset(WelcomeString1, 0, sizeof(WelcomeString1));

        EEPROM_ReadBuffer(0x0EB0, WelcomeString0, 16);
        EEPROM_ReadBuffer(0x0EC0, WelcomeString1, 16);

        sprintf(WelcomeString2, "%u.%02uV %u%%",
                gBatteryVoltageAverage / 100,
                gBatteryVoltageAverage % 100,
                BATTERY_VoltsToPercent(gBatteryVoltageAverage));

        if (gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_VOLTAGE)
        {
            strcpy(WelcomeString0, "VOLTAGE");
            strcpy(WelcomeString1, WelcomeString2);
        }
        else if(gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_ALL)
        {
            if(strlen(WelcomeString0) == 0 && strlen(WelcomeString1) == 0)
            {
                strcpy(WelcomeString0, "WELCOME");
                strcpy(WelcomeString1, WelcomeString2);
            }
            else if(strlen(WelcomeString0) == 0 || strlen(WelcomeString1) == 0)
            {
                if(strlen(WelcomeString0) == 0)
                {
                    strcpy(WelcomeString0, WelcomeString1);
                }
                strcpy(WelcomeString1, WelcomeString2);
            }
        }
        else if(gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_MESSAGE)
        {
            if(strlen(WelcomeString0) == 0)
            {
                strcpy(WelcomeString0, "WELCOME");
            }

            if(strlen(WelcomeString1) == 0)
            {
                strcpy(WelcomeString1, "BIENVENUE");
            }
        }

        UI_PrintString(WelcomeString0, 0, 127, 0, 10);
        UI_PrintString(WelcomeString1, 0, 127, 2, 10);

#ifdef ENABLE_FEAT_F4HWN
        UI_PrintStringSmallNormal(Version, 0, 128, 4);

        UI_DrawLineBuffer(gFrameBuffer, 0, 31, 127, 31, 1); // Be ware, status zone = 8 lines, the rest = 56 ->total 64

        for (uint8_t i = 18; i < 110; i++)
        {
            gFrameBuffer[4][i] ^= 0xFF;
        }

        sprintf(WelcomeString3, "%s Edition", Edition);
        UI_PrintStringSmallNormal(WelcomeString3, 0, 127, 6);

#else
        UI_PrintStringSmallNormal(Version, 0, 127, 6);
#endif
        UI_UpdateDisplay();

    }*/
}
