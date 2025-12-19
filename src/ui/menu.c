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
#include <stdlib.h>
#include <stdio.h>

#include "app/dtmf.h"
#include "app/menu.h"
#include "bitmaps.h"
#include "board.h"
#include "dcs.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "driver/st7565.h"
#include "printf.h"
#include "frequencies.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#ifdef ENABLE_FEAT_F4HWN
#include "version.h"
#endif
#include "helper.h"
#include "inputbox.h"
#include "menu.h"
#include "ui.h"
#include "ui/gui.h"


const t_menu_item MenuList[] =
{
    //   text,          menu ID
        {"SQL",     MENU_SQL           },
        {"STEP ",   MENU_STEP          },
        {"POWER",   MENU_TXP           }, // was "TXP"
        {"RX DCS",  MENU_R_DCS         }, // was "R_DCS"
        {"RX CTCS", MENU_R_CTCS        }, // was "R_CTCS"
        {"TX DCS",  MENU_T_DCS         }, // was "T_DCS"
        {"TX CTCS", MENU_T_CTCS        }, // was "T_CTCS"
        {"TX ODIR", MENU_SFT_D         }, // was "SFT_D"
        {"TX OFFS", MENU_OFFSET        }, // was "OFFSET"
        {"W/N",     MENU_W_N           },
        {"BUSYCL",  MENU_BCL           }, // was "BCL"
        {"COMPND",  MENU_COMPAND       },
        {"MODE",    MENU_AM            }, // was "AM"
    #ifdef ENABLE_FEAT_F4HWN
        {"TX LOCK", MENU_TX_LOCK       },
    #endif
        {"SC ADD1", MENU_S_ADD1        },
        {"SC ADD2", MENU_S_ADD2        },
        {"SC ADD3", MENU_S_ADD3        },
        {"CH SAVE", MENU_MEM_CH        }, // was "MEM-CH"
        {"CH DELE", MENU_DEL_CH        }, // was "DEL-CH"
        {"CH NAME", MENU_MEM_NAME      },

        {"S LIST",  MENU_S_LIST        },
        {"S LIST1", MENU_SLIST1        },
        {"S LIST2", MENU_SLIST2        },
        {"S LIST3", MENU_SLIST3        },
        {"SCN REV", MENU_SC_REV        },
    #ifndef ENABLE_FEAT_F4HWN
        #ifdef ENABLE_NOAA
            {"NOAA-S",  MENU_NOAA_S    },
        #endif
    #endif
        {"F1 SHORT", MENU_F1SHRT        },
        {"F1 LONG",  MENU_F1LONG        },
        {"F2 SHORT", MENU_F2SHRT        },
        {"F2 LONG",  MENU_F2LONG        },
        {"M LONG",   MENU_MLONG         },

        {"KEY LCK",  MENU_AUTOLK        }, // was "AUTOLk"
        {"TXT OUT",  MENU_TOT           }, // was "TOT"
        {"BAT SAVE", MENU_SAVE          }, // was "SAVE"
        //{"BATTXT",  MENU_BAT_TXT       },
        {"MIC",      MENU_MIC           },
        {"MIC BAR",  MENU_MIC_BAR       },
        //{"CH DISP",  MENU_MDF           }, // was "MDF"
        //{"PONMSG",  MENU_PONMSG        },
        {"BL TIME",  MENU_ABR           }, // was "ABR"
        {"BL MIN",   MENU_ABR_MIN       },
        {"BL MAX",   MENU_ABR_MAX       },
        {"BL TXRX",  MENU_ABR_ON_TX_RX  },
        {"BEEP",     MENU_BEEP          },
    #ifdef ENABLE_VOICE
        {"VOICE",    MENU_VOICE         },
    #endif
        {"ROGER",    MENU_ROGER         },
        {"STE",      MENU_STE           },
        {"RP STE",   MENU_RP_STE        },
        {"1 CALL",   MENU_1_CALL        },
    #ifdef ENABLE_ALARM
        {"ALARM T",  MENU_AL_MOD        },
    #endif
    #ifdef ENABLE_DTMF_CALLING
        {"ANI ID",   MENU_ANI_ID        },
    #endif
        {"UPCODE",   MENU_UPCODE        },
        {"DWCODE",   MENU_DWCODE        },
        {"PTT ID",   MENU_PTT_ID        },
        {"D ST",     MENU_D_ST          },
    #ifdef ENABLE_DTMF_CALLING
        {"D RESP",   MENU_D_RSP         },
        {"D HOLD",   MENU_D_HOLD        },
    #endif
        {"D PREL",   MENU_D_PRE         },
    #ifdef ENABLE_DTMF_CALLING
        {"D DECD",   MENU_D_DCD         },
        {"D LIST",   MENU_D_LIST        },
    #endif
        {"D LIVE",   MENU_D_LIVE_DEC    }, // live DTMF decoder
    #ifndef ENABLE_FEAT_F4HWN
        #ifdef ENABLE_AM_FIX
            {"AM FIX",  MENU_AM_FIX        },
        #endif
    #endif
        {"VOX",      MENU_VOX           },
    #ifdef ENABLE_FEAT_F4HWN
        {"SYSINF",   MENU_VOL           }, // was "VOL"
    #else
        {"BATVOL",   MENU_VOL           }, // was "VOL"
    #endif
        {"RX MODE",  MENU_TDR           },
    #ifdef ENABLE_FEAT_F4HWN
        {"SET PWR",  MENU_SET_PWR       },
        {"SET PTT",  MENU_SET_PTT       },
        {"SET TOT",  MENU_SET_TOT       },
        {"SET EOT",  MENU_SET_EOT       },
        //{"SET CTR",  MENU_SET_CTR       },
        //{"SETINV",  MENU_SET_INV       },
        {"SET LCK",  MENU_SET_LCK       },
        //{"SET MET",  MENU_SET_MET       },
        //{"SETGUI",  MENU_SET_GUI       },
        {"SET TMR",  MENU_SET_TMR       },
    #ifdef ENABLE_FEAT_F4HWN_SLEEP
        {"SET OFF",  MENU_SET_OFF      },
    #endif
    #ifdef ENABLE_FEAT_F4HWN_NARROWER
        {"SET NFM",  MENU_SET_NFM       },
    #endif
    #ifdef ENABLE_FEAT_F4HWN_VOL
        {"SET VOL",  MENU_SET_VOL       },
    #endif
    #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
        {"SET KEY",  MENU_SET_KEY       },
    #endif
    #ifdef ENABLE_NOAA
        {"SET NWR",  MENU_NOAA_S    },
    #endif
    #endif
        // hidden menu items from here on
        // enabled if pressing both the PTT and upper side button at power-on
        {"F LOCK",   MENU_F_LOCK        },
    #ifndef ENABLE_FEAT_F4HWN
        {"TX 200",   MENU_200TX         }, // was "200TX"
        {"TX 350",   MENU_350TX         }, // was "350TX"
        {"TX 500",   MENU_500TX         }, // was "500TX"
    #endif
        {"350 EN",   MENU_350EN         }, // was "350EN"
    #ifdef ENABLE_F_CAL_MENU
        {"FR CALI",  MENU_F_CALI        }, // reference xtal calibration
    #endif
        {"BAT CAL",  MENU_BATCAL        }, // battery voltage calibration
        {"BAT TYP",  MENU_BATTYP        }, // battery type 1600/2200mAh
        {"RESET",    MENU_RESET         }, // might be better to move this to the hidden menu items ?

        {"",                              0xff               }  // end of list - DO NOT delete or move this this
};

const uint8_t FIRST_HIDDEN_MENU_ITEM = MENU_F_LOCK;

const char gSubMenu_TXP[][6] =
{
 "USER",
 "LOW 1",
 "LOW 2",
 "LOW 3",
 "LOW 4",
 "LOW 5",
 "MID",
 "HIGH"
};

const char gSubMenu_SFT_D[][4] =
{
 "OFF",
 "+",
 "-"
};

const char gSubMenu_W_N[][7] =
{
 "WIDE",
 "NARROW"
};

const char gSubMenu_OFF_ON[][4] =
{
 "OFF",
 "ON"
};

const char gSubMenu_NA[4] =
{
 "N/A"
};

const char* const gSubMenu_RXMode[] =
{
 "A ONLY",      // TX and RX on main only
 "A/B RX",      // Watch both and respond
 "CROSS A/B",   // TX on main, RX on secondary
 "A TX A/B RX"  // always TX on main, but RX on both
};

#ifdef ENABLE_VOICE
const char gSubMenu_VOICE[][4] =
{
 "OFF",
 "CHI",
 "ENG"
};
#endif

#ifdef ENABLE_ALARM
const char gSubMenu_AL_MOD[][5] =
{
 "SITE",
 "TONE"
};
#endif

#ifdef ENABLE_DTMF_CALLING
const char gSubMenu_D_RSP[][11] =
{
 "DO NOTHING",
 "RING",
 "REPLY",
 "BOTH"
};
#endif

const char* const gSubMenu_PTT_ID[] =
{
 "OFF",
 "UP CODE",
 "DOWN CODE",
 "UP+DOWN",
 "APOLLO"
};

const char gSubMenu_PONMSG[][8] =
{
#ifdef ENABLE_FEAT_F4HWN
 "ALL",
 "SOUND",
#else
 "FULL",
#endif
 "MESSAGE",
 "VOLTAGE",
 "NONE"
};

const char gSubMenu_ROGER[][6] =
{
 "OFF",
 "ROGER",
 "MDC"
};

const char gSubMenu_RESET[][4] =
{
 "VFO",
 "ALL"
};

const char* const gSubMenu_F_LOCK[] =
{
 "DEFAULT+",
 "FCC HAM",
#ifdef ENABLE_FEAT_F4HWN_CA
 "CA HAM",
#endif
 "CE HAM",
 "GB HAM",
 "137../..430",
 "137../..438",
#ifdef ENABLE_FEAT_F4HWN_PMR
 "PMR 446",
#endif
#ifdef ENABLE_FEAT_F4HWN_GMRS_FRS_MURS
 "GMRS FRS MURS",
#endif
 "DISABLE ALL",
 "UNLOCK ALL",
};

const char gSubMenu_RX_TX[][6] =
{
 "OFF",
 "TX",
 "RX",
 "TX/RX"
};

const char gSubMenu_BAT_TXT[][8] =
{
 "NONE",
 "VOLTAGE",
 "PERCENT"
};

const char gSubMenu_BATTYP[][9] =
{
 "1600mAh",
 "2200mAh",
 "3500mAh"
};


#ifdef ENABLE_FEAT_F4HWN
const char gSubMenu_SET_PWR[][6] =
{
 "< 20m",
 "125m",
 "250m",
 "500m",
 "1",
 "2",
 "5"
};

const char gSubMenu_SET_PTT[][8] =
{
 "CLASSIC",
 "ONEPUSH"
};

const char gSubMenu_SET_TOT[][7] =  // Use by SET_EOT too
{
 "OFF",
 "SOUND",
 "VISUAL",
 "ALL"
};

const char gSubMenu_SET_LCK[][9] =
{
 "KEYS",
 "KEYS+PTT"
};

const char gSubMenu_SET_MET[][8] =
{
 "TINY",
 "CLASSIC"
};

#ifdef ENABLE_FEAT_F4HWN_NARROWER
const char gSubMenu_SET_NFM[][9] =
{
 "NARROW",
 "NARROWER"
};
#endif

#ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
const char gSubMenu_SET_KEY[][9] =
{
 "KEY_MENU",
 "KEY_UP",
 "KEY_DOWN",
 "KEY_EXIT",
 "KEY_STAR"
};
#endif
#endif

const t_sidefunction gSubMenu_SIDEFUNCTIONS[] =
{
    {"NONE",            ACTION_OPT_NONE},
#ifdef ENABLE_FLASHLIGHT
    {"FLASH LIGHT",    ACTION_OPT_FLASHLIGHT},
#endif
    {"POWER",           ACTION_OPT_POWER},
    {"MONITOR",         ACTION_OPT_MONITOR},
    {"SCAN",            ACTION_OPT_SCAN},
#ifdef ENABLE_VOX
    {"VOX",             ACTION_OPT_VOX},
#endif
#ifdef ENABLE_ALARM
    {"ALARM",           ACTION_OPT_ALARM},
#endif
#ifdef ENABLE_FMRADIO
    {"FM RADIO",        ACTION_OPT_FM},
#endif
#ifdef ENABLE_TX1750
    {"1750Hz",          ACTION_OPT_1750},
#endif
#ifdef ENABLE_REGA
    {"REGA ALARM",     ACTION_OPT_REGA_ALARM},
    {"REGA TEST",      ACTION_OPT_REGA_TEST},
#endif
    {"LOCK KEYPAD",    ACTION_OPT_KEYLOCK},
    {"VFO A/B",    ACTION_OPT_A_B},
    {"VFO MEM",        ACTION_OPT_VFO_MR},
    {"MODE",            ACTION_OPT_SWITCH_DEMODUL},
#ifdef ENABLE_BLMIN_TMP_OFF
    {"BLMIN TMP OFF",  ACTION_OPT_BLMIN_TMP_OFF},      //BackLight Minimum Temporay OFF
#endif
#ifdef ENABLE_FEAT_F4HWN
    {"RX MODE",         ACTION_OPT_RXMODE},
    {"MAIN ONLY",       ACTION_OPT_MAINONLY},
    {"PTT",             ACTION_OPT_PTT},
    {"WIDE/NARROW",    ACTION_OPT_WN},
    #if !defined(ENABLE_SPECTRUM) || !defined(ENABLE_FMRADIO)
    {"MUTE",            ACTION_OPT_MUTE},
    #endif
    #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
        {"POWER HIGH",    ACTION_OPT_POWER_HIGH},
        {"REMOVE OFFSET",  ACTION_OPT_REMOVE_OFFSET},
    #endif
#endif
};

const uint8_t gSubMenu_SIDEFUNCTIONS_size = ARRAY_SIZE(gSubMenu_SIDEFUNCTIONS);

bool    gIsInSubMenu;
uint8_t gMenuCursor;

int UI_MENU_GetCurrentMenuId() {
    if (gMenuCursor < ARRAY_SIZE(MenuList))
        return MenuList[gMenuCursor].menu_id;

    return MenuList[ARRAY_SIZE(MenuList) - 1].menu_id;
}

uint8_t UI_MENU_GetMenuIdx(uint8_t id)
{
    for (uint8_t i = 0; i < ARRAY_SIZE(MenuList); i++)
        if (MenuList[i].menu_id == id)
            return i;
    return 0;
}

int32_t gSubMenuSelection;

// edit box
char    edit_original[17]; // a copy of the text before editing so that we can easily test for changes/difference
char    edit[17];
int     edit_index;


UI_SelectionList menuList;
UI_SelectionList subMenuList;

static const char* UI_MENU_GetMenuListLines(void)
{
    static char s_menuLines[ARRAY_SIZE(MenuList) * 12] = { 0 }; // 10 chars + '\n' + '\0' headroom

    size_t out = 0;
    const size_t cap = sizeof(s_menuLines);

    for (uint8_t i = 0; i < gMenuListCount && MenuList[i].name[0] != '\0'; i++) {
        const char* name = MenuList[i].name;
        const size_t len = strnlen(name, sizeof(MenuList[i].name));

        if (len == 0) {
            continue;
        }

        if (out + len + 1 >= cap) {
            break;
        }

        memcpy(&s_menuLines[out], name, len);
        out += len;

        if ((i + 1) < gMenuListCount && out + 1 < cap) {
            s_menuLines[out++] = '\n';
        }
    }

    s_menuLines[out] = '\0';
    return s_menuLines;
}

static const char* squelchStr = "OFF\n1\n2\n3\n4\n5\n6\n7\n8\n9";

static const char* UI_MENU_JoinFixedList(const char* arr, size_t elem_size, size_t count)
{
    static char buffer[512];
    size_t out = 0;

    for (size_t i = 0; i < count; ++i) {
        const char* item = arr + (i * elem_size);
        const size_t len = strnlen(item, elem_size);

        if (len == 0) {
            continue;
        }

        if (out + len + 1 >= sizeof(buffer)) {
            break;
        }

        memcpy(&buffer[out], item, len);
        out += len;

        if ((i + 1) < count && out + 1 < sizeof(buffer)) {
            buffer[out++] = '\n';
        }
    }

    buffer[out] = '\0';
    return buffer;
}

static const char* UI_MENU_GetStepList(void)
{
    static char buffer[STEP_N_ELEM * 12];
    size_t out = 0;

    for (uint8_t i = 0; i < STEP_N_ELEM; ++i) {
        const STEP_Setting_t step_idx = FREQUENCY_GetStepIdxFromSortedIdx(i);
        const uint16_t step = gStepFrequencyTable[step_idx];
        const uint16_t whole = step / 100;
        const uint16_t frac = step % 100;
        const int written = snprintf(&buffer[out], sizeof(buffer) - out, "%u.%02ukHz", (unsigned int)whole, (unsigned int)frac);

        if (written <= 0 || (size_t)written >= sizeof(buffer) - out) {
            buffer[sizeof(buffer) - 1] = '\0';
            break;
        }

        out += (size_t)written;

        if ((i + 1) < STEP_N_ELEM && out + 1 < sizeof(buffer)) {
            buffer[out++] = '\n';
            buffer[out] = '\0';
        }
    }

    return buffer;
}

static const char* UI_MENU_JoinPtrList(const char* const* arr, size_t count)
{
    static char buffer[512];
    size_t out = 0;

    for (size_t i = 0; i < count; ++i) {
        const char* item = arr[i];
        if (item == NULL) {
            continue;
        }
        const size_t len = strlen(item);
        if (len == 0) {
            continue;
        }

        if (out + len + 1 >= sizeof(buffer)) {
            break;
        }

        memcpy(&buffer[out], item, len);
        out += len;

        if ((i + 1) < count && out + 1 < sizeof(buffer)) {
            buffer[out++] = '\n';
        }
    }

    buffer[out] = '\0';
    return buffer;
}

static const char* UI_MENU_JoinSideFunctions(void)
{
    static char buffer[512];
    size_t out = 0;

    for (size_t i = 0; i < gSubMenu_SIDEFUNCTIONS_size; ++i) {
        const char* item = gSubMenu_SIDEFUNCTIONS[i].name;
        const size_t len = strlen(item);

        if (len == 0) {
            continue;
        }

        if (out + len + 1 >= sizeof(buffer)) {
            break;
        }

        memcpy(&buffer[out], item, len);
        out += len;

        if ((i + 1) < gSubMenu_SIDEFUNCTIONS_size && out + 1 < sizeof(buffer)) {
            buffer[out++] = '\n';
        }
    }

    buffer[out] = '\0';
    return buffer;
}

static const char* UI_MENU_GetOptionLinesForId(int menuId)
{
    switch (menuId)
    {
    case MENU_SQL:
        return squelchStr;
    case MENU_STEP:
        return UI_MENU_GetStepList();
    case MENU_AM:
        return UI_MENU_JoinFixedList((const char*)gModulationStr, sizeof(gModulationStr[0]), ARRAY_SIZE(gModulationStr));
    case MENU_TXP:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_TXP, sizeof(gSubMenu_TXP[0]), ARRAY_SIZE(gSubMenu_TXP));
    case MENU_SFT_D:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_SFT_D, sizeof(gSubMenu_SFT_D[0]), ARRAY_SIZE(gSubMenu_SFT_D));
    case MENU_W_N:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_W_N, sizeof(gSubMenu_W_N[0]), ARRAY_SIZE(gSubMenu_W_N));
    case MENU_COMPAND:
    case MENU_ABR_ON_TX_RX:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_RX_TX, sizeof(gSubMenu_RX_TX[0]), ARRAY_SIZE(gSubMenu_RX_TX));
    case MENU_TDR:
        return UI_MENU_JoinPtrList(gSubMenu_RXMode, ARRAY_SIZE(gSubMenu_RXMode));
#ifdef ENABLE_VOICE
    case MENU_VOICE:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_VOICE, sizeof(gSubMenu_VOICE[0]), ARRAY_SIZE(gSubMenu_VOICE));
#endif
    case MENU_ROGER:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_ROGER, sizeof(gSubMenu_ROGER[0]), ARRAY_SIZE(gSubMenu_ROGER));
    case MENU_PONMSG:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_PONMSG, sizeof(gSubMenu_PONMSG[0]), ARRAY_SIZE(gSubMenu_PONMSG));
    case MENU_RESET:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_RESET, sizeof(gSubMenu_RESET[0]), ARRAY_SIZE(gSubMenu_RESET));
    case MENU_F_LOCK:
        return UI_MENU_JoinPtrList(gSubMenu_F_LOCK, ARRAY_SIZE(gSubMenu_F_LOCK));
    case MENU_BAT_TXT:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_BAT_TXT, sizeof(gSubMenu_BAT_TXT[0]), ARRAY_SIZE(gSubMenu_BAT_TXT));
    case MENU_BATTYP:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_BATTYP, sizeof(gSubMenu_BATTYP[0]), ARRAY_SIZE(gSubMenu_BATTYP));
#ifdef ENABLE_ALARM
    case MENU_AL_MOD:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_AL_MOD, sizeof(gSubMenu_AL_MOD[0]), ARRAY_SIZE(gSubMenu_AL_MOD));
#endif
#ifdef ENABLE_DTMF_CALLING
    case MENU_D_RSP:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_D_RSP, sizeof(gSubMenu_D_RSP[0]), ARRAY_SIZE(gSubMenu_D_RSP));
#endif

    case MENU_MIC_BAR:
#ifdef ENABLE_AUDIO_BAR
        return UI_MENU_JoinFixedList((const char*)gSubMenu_OFF_ON, sizeof(gSubMenu_OFF_ON[0]), ARRAY_SIZE(gSubMenu_OFF_ON));
#else
        return gSubMenu_NA;
#endif
    case MENU_PTT_ID:
        return UI_MENU_JoinPtrList(gSubMenu_PTT_ID, ARRAY_SIZE(gSubMenu_PTT_ID));
#ifdef ENABLE_FEAT_F4HWN
    case MENU_SET_PWR:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_SET_PWR, sizeof(gSubMenu_SET_PWR[0]), ARRAY_SIZE(gSubMenu_SET_PWR));
    case MENU_SET_PTT:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_SET_PTT, sizeof(gSubMenu_SET_PTT[0]), ARRAY_SIZE(gSubMenu_SET_PTT));
    case MENU_SET_TOT:
    case MENU_SET_EOT:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_SET_TOT, sizeof(gSubMenu_SET_TOT[0]), ARRAY_SIZE(gSubMenu_SET_TOT));
    case MENU_SET_LCK:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_SET_LCK, sizeof(gSubMenu_SET_LCK[0]), ARRAY_SIZE(gSubMenu_SET_LCK));
#ifdef ENABLE_FEAT_F4HWN_NARROWER
    case MENU_SET_NFM:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_SET_NFM, sizeof(gSubMenu_SET_NFM[0]), ARRAY_SIZE(gSubMenu_SET_NFM));
#endif
#ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
    case MENU_SET_KEY:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_SET_KEY, sizeof(gSubMenu_SET_KEY[0]), ARRAY_SIZE(gSubMenu_SET_KEY));
#endif
#endif
    case MENU_F1SHRT:
    case MENU_F1LONG:
    case MENU_F2SHRT:
    case MENU_F2LONG:
    case MENU_MLONG:
        return UI_MENU_JoinSideFunctions();

#ifndef ENABLE_FEAT_F4HWN
#ifdef ENABLE_AM_FIX
    case MENU_AM_FIX:
#endif
#endif
    case MENU_BCL:
    case MENU_BEEP:
    case MENU_S_ADD1:
    case MENU_S_ADD2:
    case MENU_S_ADD3:
    case MENU_STE:
    case MENU_D_ST:
#ifdef ENABLE_DTMF_CALLING
    case MENU_D_DCD:
#endif
    case MENU_D_LIVE_DEC:
#ifdef ENABLE_NOAA
    case MENU_NOAA_S:
#endif
#ifndef ENABLE_FEAT_F4HWN
    case MENU_350TX:
    case MENU_200TX:
    case MENU_500TX:
#endif
    case MENU_350EN:

#ifdef ENABLE_FEAT_F4HWN
    case MENU_SET_TMR:
#endif

    case MENU_TX_LOCK:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_OFF_ON, sizeof(gSubMenu_OFF_ON[0]), ARRAY_SIZE(gSubMenu_OFF_ON));
    default:
        break;
    }

    static char numeric[15];
    switch (menuId)
    {
    case MENU_AUTOLK:
        if (gSubMenuSelection == 0)
            strcpy(numeric, gSubMenu_OFF_ON[0]);
        else
        {
            snprintf(numeric, sizeof(numeric), "%02dm:%02ds", ((gSubMenuSelection * 15) / 60), ((gSubMenuSelection * 15) % 60));
        }
        break;
    case MENU_TOT:
        snprintf(numeric, sizeof(numeric), "%02dm:%02ds", (((gSubMenuSelection + 1) * 5) / 60), (((gSubMenuSelection + 1) * 5) % 60));
        break;
    case MENU_SAVE:
        snprintf(numeric, sizeof(numeric), gSubMenuSelection == 0 ? gSubMenu_OFF_ON[0] : "1:%u", gSubMenuSelection);
        break;
    case MENU_MIC:
    {   // display the mic gain in actual dB rather than just an index number
        const uint8_t mic = gMicGain_dB2[gSubMenuSelection];
        snprintf(numeric, sizeof(numeric), "+%u.%01udB", mic / 2, mic % 2);
    }
    break;
    case MENU_ABR:
        if (gSubMenuSelection == 0)
        {
            return gSubMenu_OFF_ON[0];
        }
        else if (gSubMenuSelection < 61)
        {
            snprintf(numeric, sizeof(numeric), "%02dm:%02ds", (((gSubMenuSelection) * 5) / 60), (((gSubMenuSelection) * 5) % 60));
        }
        else
        {
            return gSubMenu_OFF_ON[1];
        }
        break;
    case MENU_ABR_MIN:
    case MENU_ABR_MAX:
        snprintf(numeric, sizeof(numeric), "%d", gSubMenuSelection);
        if (gIsInSubMenu)
            BACKLIGHT_SetBrightness(gSubMenuSelection);
        break;
    case MENU_RP_STE:
        snprintf(numeric, sizeof(numeric), gSubMenuSelection == 0 ? gSubMenu_OFF_ON[0] : "%u*100ms", gSubMenuSelection);
        break;
    case MENU_VOX:
#ifdef ENABLE_VOX
        snprintf(numeric, sizeof(numeric), gSubMenuSelection == 0 ? gSubMenu_OFF_ON[0] : "%u", gSubMenuSelection);
#else
        return gSubMenu_NA;
#endif
        break;
    case MENU_VOL:
        snprintf(numeric, sizeof(numeric), "%s\n%s",
            AUTHOR_STRING_2,
            VERSION_STRING_2
        );
        break;
#ifdef ENABLE_FEAT_F4HWN_SLEEP
    case MENU_SET_OFF:
        if (gSubMenuSelection == 0)
        {
            return gSubMenu_OFF_ON[0];
        }
        else if (gSubMenuSelection < 121)
        {
            snprintf(numeric, sizeof(numeric), "%dh:%02dm", (gSubMenuSelection / 60), (gSubMenuSelection % 60));
        }
        break;
#endif            
    default:
        snprintf(numeric, sizeof(numeric), "%ld", (long)gSubMenuSelection);
        break;
    }

    return numeric;
}

const char* getCurrentOption() {
    const char* lines = UI_MENU_GetOptionLinesForId(UI_MENU_GetCurrentMenuId());
    if (u8x8_GetStringLineCnt(lines) == 1)
        return lines;
    return UI_GetStrValue(lines, gSubMenuSelection);
}

void setSubMenu(void) {
    const char* lines = UI_MENU_GetOptionLinesForId(UI_MENU_GetCurrentMenuId());
    const uint8_t total = u8x8_GetStringLineCnt(lines);
    const uint8_t visible = MIN(total, 5);
    UI_SelectionList_Set(&subMenuList, gSubMenuSelection, visible > 0 ? visible : 1, 122, lines, NULL);
}

void UI_DisplayMenu(void)
{
    // clear the screen
    UI_ClearDisplay();
    // redraw the menu
    UI_SetBlackColor();

    UI_DrawBox(0, 0, 128, 7);

    UI_SetFont(FONT_8B_TR);
    UI_DrawString(UI_TEXT_ALIGN_LEFT, 2, 0, 6, false, false, false, "MENU");
    UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", 1 + gMenuCursor, gMenuListCount);

    UI_SetBlackColor();
    UI_SelectionList_SetCurrentPos(&menuList, gMenuCursor);

    UI_SelectionList_Draw(&menuList, 15, getCurrentOption());

    if (gIsInSubMenu) {
        // gSubMenuSelection
        UI_DrawPopupWindow(36, 6, 90, 52, UI_SelectionList_GetStringLine(&menuList));

        setSubMenu();

        UI_SelectionList_Draw(&subMenuList, 20, NULL);
    }

    UI_UpdateDisplay();
}

void UI_MenuInit(void)
{
    UI_SelectionList_Init(&menuList);
    UI_SelectionList_Set(&menuList, gMenuCursor, 6, 127, UI_MENU_GetMenuListLines(), NULL);

    UI_SelectionList_Init(&subMenuList);
    //UI_SelectionList_SetMaxWidth(&subMenuList, 90);
    UI_SelectionList_SetShowLineNumbers(&subMenuList, false);
    UI_SelectionList_SetStartXPos(&subMenuList, 40);
}
