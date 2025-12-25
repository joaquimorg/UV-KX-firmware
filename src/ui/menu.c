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
//#include "driver/st7565.h"
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
        {"SQL",         MENU_SQL           },
        {"STEP ",       MENU_STEP          },
        {"POWER",       MENU_TXP           }, // was "TXP"
        {"RX DCS",      MENU_R_DCS         }, // was "R_DCS"
        {"RX CTCS",     MENU_R_CTCS        }, // was "R_CTCS"
        {"TX DCS",      MENU_T_DCS         }, // was "T_DCS"
        {"TX CTCS",     MENU_T_CTCS        }, // was "T_CTCS"
        {"TX ODIR",     MENU_SFT_D         }, // was "SFT_D"
        {"TXOFFSET",    MENU_OFFSET        }, // was "OFFSET"
        {"W/N",         MENU_W_N           },
        {"BUSYLOCK",    MENU_BCL           }, // was "BCL"
        {"COMPANDR",    MENU_COMPAND       }, //compander
        {"MODE",        MENU_AM            }, // was "AM"
    #ifdef ENABLE_FEAT_F4HWN
        {"TX LOCK",     MENU_TX_LOCK       },
    #endif
    #ifdef ENABLE_SCANLIST
        {"SC ADD1",     MENU_S_ADD1        },
        {"SC ADD2",     MENU_S_ADD2        },
        {"SC ADD3",     MENU_S_ADD3        },
    #endif
        //{"CH SAVE",     MENU_MEM_CH        }, // was "MEM-CH"
        //{"CH DELETE",   MENU_DEL_CH        }, // was "DEL-CH"
        //{"CH NAME",     MENU_MEM_NAME      },

    #ifdef ENABLE_SCANLIST
        {"S LIST",      MENU_S_LIST        },
        {"S LIST1",     MENU_SLIST1        },
        {"S LIST2",     MENU_SLIST2        },
        {"S LIST3",     MENU_SLIST3        },
    #endif
        {"SCN REV",     MENU_SC_REV        },
    #ifndef ENABLE_FEAT_F4HWN
        #ifdef ENABLE_NOAA
            {"NOAA-S",  MENU_NOAA_S    },
        #endif
    #endif
        {"F1 SHORT",    MENU_F1SHRT        },
        {"F1 LONG",     MENU_F1LONG        },
        {"F2 SHORT",    MENU_F2SHRT        },
        {"F2 LONG",     MENU_F2LONG        },
        {"M LONG",      MENU_MLONG         },
        {"KEY LCK",     MENU_AUTOLK        }, // was "AUTOLk"
        {"TXT OUT",     MENU_TOT           }, // was "TOT"
        {"BAT SAVE",    MENU_SAVE          }, // was "SAVE"
        //{"BATTXT",  MENU_BAT_TXT       },
        {"MIC",         MENU_MIC           },
        {"MIC BAR",     MENU_MIC_BAR       },
        //{"CH DISP",  MENU_MDF           }, // was "MDF"
        //{"PONMSG",  MENU_PONMSG        },
        {"BL TIME",     MENU_ABR           }, // was "ABR"
        {"BL MIN",      MENU_ABR_MIN       },
        {"BL MAX",      MENU_ABR_MAX       },
        {"BL TXRX",     MENU_ABR_ON_TX_RX  },
        {"BEEP",        MENU_BEEP          },
    #ifdef ENABLE_VOICE
        {"VOICE",       MENU_VOICE         },
    #endif
        {"ROGER",       MENU_ROGER         },
        {"STE",         MENU_STE           },
        {"RP STE",      MENU_RP_STE        },
        {"1 CALL",      MENU_1_CALL        },
    #ifdef ENABLE_ALARM
        {"ALARM T",     MENU_AL_MOD        },
    #endif
    #ifdef ENABLE_DTMF_CALLING
        {"ANI ID",      MENU_ANI_ID        },
    #endif
        {"UPCODE",      MENU_UPCODE        },
        {"DWCODE",      MENU_DWCODE        },
        {"PTT ID",      MENU_PTT_ID        },
        {"D ST",        MENU_D_ST          },
    #ifdef ENABLE_DTMF_CALLING
        {"D RESP",      MENU_D_RSP         },
        {"D HOLD",      MENU_D_HOLD        },
    #endif
        {"D PREL",      MENU_D_PRE         },
    #ifdef ENABLE_DTMF_CALLING
        {"D DECD",      MENU_D_DCD         },
        {"D LIST",      MENU_D_LIST        },
    #endif
        {"D LIVE",      MENU_D_LIVE_DEC    }, // live DTMF decoder
    #ifndef ENABLE_FEAT_F4HWN
        #ifdef ENABLE_AM_FIX
            {"AM FIX",  MENU_AM_FIX        },
        #endif
    #endif
        {"VOX",         MENU_VOX           },
    #ifdef ENABLE_FEAT_F4HWN
        {"SYSINF",      MENU_VOL           }, // was "VOL"
    #else
        {"BATVOL",      MENU_VOL           }, // was "VOL"
    #endif
        {"RX MODE",     MENU_TDR           },
    #ifdef ENABLE_FEAT_F4HWN
        {"SET PWR",     MENU_SET_PWR       },
        {"SET PTT",     MENU_SET_PTT       },
        {"SET TOT",     MENU_SET_TOT       },
        {"SET EOT",     MENU_SET_EOT       },
        //{"SET CTR",  MENU_SET_CTR       },
        //{"SETINV",  MENU_SET_INV       },
        {"SET LCK",     MENU_SET_LCK       },
        //{"SET MET",  MENU_SET_MET       },
        //{"SETGUI",  MENU_SET_GUI       },
        {"SET TMR",     MENU_SET_TMR       },
    #ifdef ENABLE_FEAT_F4HWN_SLEEP
        {"SET OFF",     MENU_SET_OFF      },
    #endif
    #ifdef ENABLE_FEAT_F4HWN_NARROWER
        {"SET NFM",     MENU_SET_NFM       },
    #endif
    #ifdef ENABLE_FEAT_F4HWN_VOL
        {"SET VOL",     MENU_SET_VOL       },
    #endif
    #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
        {"SET KEY",     MENU_SET_KEY       },
    #endif
    #ifdef ENABLE_NOAA
        {"SET NWR",     MENU_NOAA_S    },
    #endif
    #endif
        // hidden menu items from here on
        // enabled if pressing both the PTT and upper side button at power-on
        {"F LOCK",      MENU_F_LOCK        },
    #ifndef ENABLE_FEAT_F4HWN
    //    {"TX 200",      MENU_200TX         }, // was "200TX"
    //    {"TX 350",      MENU_350TX         }, // was "350TX"
    //    {"TX 500",      MENU_500TX         }, // was "500TX"
    #endif
    //    {"350 EN",      MENU_350EN         }, // was "350EN"
    #ifdef ENABLE_F_CAL_MENU
        {"FR CALI",     MENU_F_CALI        }, // reference xtal calibration
    #endif
        {"BAT CAL",     MENU_BATCAL        }, // battery voltage calibration
        {"BAT TYP",     MENU_BATTYP        }, // battery type 1600/2200mAh
        {"RESET",       MENU_RESET         }, // might be better to move this to the hidden menu items ?

        {"",            0xff               }  // end of list - DO NOT delete or move this this
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
 "A TX"         // always TX on main, but RX on both
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

/*const char gSubMenu_PONMSG[][8] =
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
};*/

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

/*const char gSubMenu_BAT_TXT[][8] =
{
 "NONE",
 "VOLTAGE",
 "PERCENT"
};
*/

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

#ifdef ENABLE_SCANLIST
const char gSubMenu_SLIST[][9] =
{
    "NO LIST",
    "LIST 1",
    "LIST 2",
    "LIST 3",
    "LISTS 1-3",
    "ALL"
};
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

void UI_GenerateChannelStringEx(char *pString, const bool bShowPrefix, const uint8_t ChannelNumber)
{
    if (gInputBoxIndex > 0) {
        for (unsigned int i = 0; i < 3; i++) {
            pString[i] = (gInputBox[i] == 10) ? '-' : gInputBox[i] + '0';
        }

        pString[3] = 0;
        return;
    }

    if (bShowPrefix) {
        // BUG here? Prefixed NULLs are allowed
        sprintf(pString, "CH-%03u", ChannelNumber + 1);
    } else if (ChannelNumber == 0xFF) {
        strcpy(pString, "NULL");
    } else {
        sprintf(pString, "%03u", ChannelNumber + 1);
    }
}

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
int32_t gSubMenuSelectionOffset = 0;

// edit box
char    edit_original[17]; // a copy of the text before editing so that we can easily test for changes/difference
char    edit[17];
int     edit_index;


UI_SelectionList menuList;
UI_SelectionList subMenuList;

#define MENU_LIST_BUFFER_SIZE 1536

static char gMenuListBuffer[MENU_LIST_BUFFER_SIZE];

static bool UI_MENU_AppendChar(char *buf, size_t cap, size_t *out, char c)
{
    if (*out + 1 >= cap) {
        return false;
    }
    buf[(*out)++] = c;
    buf[*out] = '\0';
    return true;
}

static bool UI_MENU_AppendStr(char *buf, size_t cap, size_t *out, const char *src)
{
    if (src == NULL) {
        return true;
    }
    while (*src != '\0') {
        if (!UI_MENU_AppendChar(buf, cap, out, *src++)) {
            return false;
        }
    }
    return true;
}

static bool UI_MENU_AppendUIntWidth(char *buf, size_t cap, size_t *out, uint32_t v, uint8_t width)
{
    char tmp[10];
    uint8_t len = 0;
    do {
        tmp[len++] = (char)('0' + (v % 10U));
        v /= 10U;
    } while (v && len < (uint8_t)sizeof(tmp));

    if (width < len) {
        width = len;
    }
    if (*out + width >= cap) {
        return false;
    }
    while (width > len) {
        buf[(*out)++] = '0';
        width--;
    }
    while (len > 0) {
        buf[(*out)++] = tmp[--len];
    }
    buf[*out] = '\0';
    return true;
}

static bool UI_MENU_AppendInt(char *buf, size_t cap, size_t *out, int32_t v)
{
    if (v < 0) {
        if (!UI_MENU_AppendChar(buf, cap, out, '-')) {
            return false;
        }
        v = (int32_t)-(int64_t)v;
    }
    return UI_MENU_AppendUIntWidth(buf, cap, out, (uint32_t)v, 0);
}

static bool UI_MENU_AppendOctalWidth(char *buf, size_t cap, size_t *out, uint32_t v, uint8_t width)
{
    char tmp[12];
    uint8_t len = 0;
    do {
        tmp[len++] = (char)('0' + (v & 7U));
        v >>= 3U;
    } while (v && len < (uint8_t)sizeof(tmp));

    if (width < len) {
        width = len;
    }
    if (*out + width >= cap) {
        return false;
    }
    while (width > len) {
        buf[(*out)++] = '0';
        width--;
    }
    while (len > 0) {
        buf[(*out)++] = tmp[--len];
    }
    buf[*out] = '\0';
    return true;
}

static const char* UI_MENU_GetMenuListLines(void)
{
    static char s_menuLines[ARRAY_SIZE(MenuList) *
                                (sizeof(MenuList[0].name) + 1U) +
                            1U] = {0};

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

#ifdef ENABLE_SCANLIST
static void UI_MENU_DrawScanListPopup(void)
{
    const uint8_t popup_x = 36;
    const uint8_t popup_y = 6;
    const uint8_t popup_w = 90;
    const uint8_t popup_h = 52;
    const uint8_t text_x1 = 40;
    const uint8_t text_x2 = 122;
    const uint8_t line_h = 10;
    char line[32];
    const int32_t selection = gSubMenuSelection;
    const uint8_t list_index = (uint8_t)(UI_MENU_GetCurrentMenuId() - MENU_SLIST1);

    UI_DrawPopupWindow(popup_x, popup_y, popup_w, popup_h, UI_SelectionList_GetStringLine(&menuList));

    UI_SetFont(FONT_8B_TR);
    if (selection < 0) {
        strcpy(line, "NULL");
    } else {
        UI_GenerateChannelStringEx(line, true, selection);
    }
    UI_DrawString(UI_TEXT_ALIGN_CENTER, text_x1, text_x2, popup_y + 14, true, false, false, line);

    UI_SetFont(FONT_8_TR);
    if (selection < 0) {
        strcpy(line, "--");
    } else {
        SETTINGS_FetchChannelName(line, selection);
        if (line[0] == '\0') {
            strcpy(line, "--");
        }
    }
    UI_DrawString(UI_TEXT_ALIGN_CENTER, text_x1, text_x2, (uint8_t)(popup_y + 14 + line_h), true, false, false, line);

    if (selection >= 0 && gEeprom.SCAN_LIST_ENABLED[list_index]) {
        for (uint8_t pri = 1; pri <= 2; ++pri) {
            const uint8_t channel = (pri == 1) ? gEeprom.SCANLIST_PRIORITY_CH1[list_index] : gEeprom.SCANLIST_PRIORITY_CH2[list_index];
            if (IS_MR_CHANNEL(channel)) {
                snprintf(line, sizeof(line), "PRI%u:%u", pri, (unsigned int)channel + 1U);
                UI_DrawString(UI_TEXT_ALIGN_CENTER, text_x1, text_x2,
                    (uint8_t)(popup_y + 14 + (line_h * (pri + 1))), true, false, false, line);
            }
        }
    }
}
#endif

static const char* squelchStr = "OFF\n1\n2\n3\n4\n5\n6\n7\n8\n9";

static const char* UI_MENU_JoinFixedList(const char* arr, size_t elem_size, size_t count)
{
    char *buffer = gMenuListBuffer;
    const size_t cap = sizeof(gMenuListBuffer);
    size_t out = 0;

    for (size_t i = 0; i < count; ++i) {
        const char* item = arr + (i * elem_size);
        const size_t len = strnlen(item, elem_size);

        if (len == 0) {
            continue;
        }

        if (out + len + 1 >= cap) {
            break;
        }

        memcpy(&buffer[out], item, len);
        out += len;

        if ((i + 1) < count && out + 1 < cap) {
            buffer[out++] = '\n';
        }
    }

    buffer[out] = '\0';
    return buffer;
}

static const char* UI_MENU_GetStepList(void)
{
    char *buffer = gMenuListBuffer;
    const size_t cap = sizeof(gMenuListBuffer);
    size_t out = 0;

    for (uint8_t i = 0; i < STEP_N_ELEM; ++i) {
        const STEP_Setting_t step_idx = FREQUENCY_GetStepIdxFromSortedIdx(i);
        const uint16_t step = gStepFrequencyTable[step_idx];
        const uint16_t whole = step / 100;
        const uint16_t frac = step % 100;
        if (!UI_MENU_AppendUIntWidth(buffer, cap, &out, whole, 0) ||
            !UI_MENU_AppendChar(buffer, cap, &out, '.') ||
            !UI_MENU_AppendUIntWidth(buffer, cap, &out, frac, 2) ||
            !UI_MENU_AppendStr(buffer, cap, &out, "kHz")) {
            buffer[cap - 1] = '\0';
            break;
        }

        if ((i + 1) < STEP_N_ELEM && out + 1 < cap) {
            buffer[out++] = '\n';
            buffer[out] = '\0';
        }
    }

    return buffer;
}

static const char* UI_MENU_JoinPtrList(const char* const* arr, size_t count)
{
    char *buffer = gMenuListBuffer;
    const size_t cap = sizeof(gMenuListBuffer);
    size_t out = 0;

    gSubMenuSelectionOffset = 0;

    for (size_t i = 0; i < count; ++i) {
        const char* item = arr[i];
        if (item == NULL) {
            continue;
        }
        const size_t len = strlen(item);
        if (len == 0) {
            continue;
        }

        if (out + len + 1 >= cap) {
            break;
        }

        memcpy(&buffer[out], item, len);
        out += len;

        if ((i + 1) < count && out + 1 < cap) {
            buffer[out++] = '\n';
        }
    }

    buffer[out] = '\0';
    return buffer;
}

static const char* UI_MENU_JoinSideFunctions(void)
{
    char *buffer = gMenuListBuffer;
    const size_t cap = sizeof(gMenuListBuffer);
    size_t out = 0;

    for (size_t i = 0; i < gSubMenu_SIDEFUNCTIONS_size; ++i) {
        const char* item = gSubMenu_SIDEFUNCTIONS[i].name;
        const size_t len = strlen(item);

        if (len == 0) {
            continue;
        }

        if (out + len + 1 >= cap) {
            break;
        }

        memcpy(&buffer[out], item, len);
        out += len;

        if ((i + 1) < gSubMenu_SIDEFUNCTIONS_size && out + 1 < cap) {
            buffer[out++] = '\n';
        }
    }

    buffer[out] = '\0';
    return buffer;
}

typedef enum {
    MENU_LIST_NUMERIC,
    MENU_LIST_TIME_MMSS,
    MENU_LIST_TIME_HM,
    MENU_LIST_MIC_DB,
    MENU_LIST_SCAN_REV,
    MENU_LIST_DCS,
    MENU_LIST_CTCSS
} UI_MENU_ListMode;

static const char* UI_MENU_BuildList(UI_MENU_ListMode mode, int32_t min, int32_t max,
    int32_t base_offset, uint32_t step,
    const char* first_item, const char* last_item,
    const char* prefix, const char* suffix)
{
    size_t out = 0;

    if (min > max) {
        gMenuListBuffer[0] = '\0';
        return gMenuListBuffer;
    }

    gSubMenuSelectionOffset = (min > 0) ? -min : 0;

    if (prefix == NULL) {
        prefix = "";
    }
    if (suffix == NULL) {
        suffix = "";
    }

    for (int32_t value = min; value <= max; ++value) {
        const char* item_text = NULL;
        const bool allow_item_text = (mode == MENU_LIST_NUMERIC ||
            mode == MENU_LIST_TIME_MMSS ||
            mode == MENU_LIST_TIME_HM);

        if (allow_item_text) {
            if (value == min && first_item != NULL) {
                item_text = first_item;
            }
            else if (value == max && last_item != NULL) {
                item_text = last_item;
            }
        }

        if (item_text != NULL) {
            if (!UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, item_text)) {
                gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                break;
            }
        } else {
            switch (mode) {
            case MENU_LIST_NUMERIC:
                if (!UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, prefix) ||
                    !UI_MENU_AppendInt(gMenuListBuffer, sizeof(gMenuListBuffer), &out, value) ||
                    !UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, suffix)) {
                    gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                    value = max;
                }
                break;
            case MENU_LIST_TIME_MMSS:
            {
                const uint32_t seconds = (uint32_t)(value + base_offset) * step;
                const uint32_t minutes = seconds / 60U;
                const uint32_t secs = seconds % 60U;
                if (!UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, minutes, 2) ||
                    !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 'm') ||
                    !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, ':') ||
                    !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, secs, 2) ||
                    !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 's')) {
                    gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                    value = max;
                }
                break;
            }
            case MENU_LIST_TIME_HM:
            {
                const uint32_t total_minutes = (uint32_t)value * step;
                const uint32_t hours = total_minutes / 60U;
                const uint32_t minutes = total_minutes % 60U;
                if (!UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, hours, 0) ||
                    !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 'h') ||
                    !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, ':') ||
                    !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, minutes, 2) ||
                    !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 'm')) {
                    gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                    value = max;
                }
                break;
            }
            case MENU_LIST_MIC_DB:
            {
                const uint8_t mic = gMicGain_dB2[value];
                if (!UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, '+') ||
                    !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, mic / 2U, 0) ||
                    !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, '.') ||
                    !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, mic % 2U, 1) ||
                    !UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, "dB")) {
                    gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                    value = max;
                }
                break;
            }
            case MENU_LIST_SCAN_REV:
                if (value == 0) {
                    if (!UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, "STOP")) {
                        gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                        value = max;
                    }
                } else if (value < 81) {
                    const uint32_t ms = (uint32_t)value * 250U;
                    const uint32_t seconds = ms / 1000U;
                    const uint32_t millis = ms % 1000U;
                    if (!UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, "CAR ") ||
                        !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, seconds, 2) ||
                        !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 's') ||
                        !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, ':') ||
                        !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, millis, 3) ||
                        !UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, "ms")) {
                        gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                        value = max;
                    }
                } else {
                    const uint32_t seconds = (uint32_t)(value - 80) * 5U;
                    const uint32_t minutes = seconds / 60U;
                    const uint32_t secs = seconds % 60U;
                    if (!UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, "OUT ") ||
                        !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, minutes, 2) ||
                        !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 'm') ||
                        !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, ':') ||
                        !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, secs, 2) ||
                        !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 's')) {
                        gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                        value = max;
                    }
                }
                break;
            case MENU_LIST_DCS:
            {
                const uint32_t count = ARRAY_SIZE(DCS_Options);
                if (value == 0) {
                    if (!UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, gSubMenu_OFF_ON[0])) {
                        gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                        value = max;
                    }
                } else if ((uint32_t)value <= count) {
                    if (!UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 'D') ||
                        !UI_MENU_AppendOctalWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out,
                                                  DCS_Options[value - 1], 3) ||
                        !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 'N')) {
                        gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                        value = max;
                    }
                } else if ((uint32_t)(value - (int32_t)count) <= count) {
                    if (!UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 'D') ||
                        !UI_MENU_AppendOctalWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out,
                                                  DCS_Options[value - 1 - (int32_t)count], 3) ||
                        !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, 'I')) {
                        gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                        value = max;
                    }
                }
                break;
            }
            case MENU_LIST_CTCSS:
            {
                const uint32_t count = ARRAY_SIZE(CTCSS_Options);
                if (value == 0) {
                    if (!UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, gSubMenu_OFF_ON[0])) {
                        gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                        value = max;
                    }
                } else if ((uint32_t)value <= count) {
                    const uint16_t tone = CTCSS_Options[value - 1];
                    if (!UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, tone / 10U, 0) ||
                        !UI_MENU_AppendChar(gMenuListBuffer, sizeof(gMenuListBuffer), &out, '.') ||
                        !UI_MENU_AppendUIntWidth(gMenuListBuffer, sizeof(gMenuListBuffer), &out, tone % 10U, 1) ||
                        !UI_MENU_AppendStr(gMenuListBuffer, sizeof(gMenuListBuffer), &out, "Hz")) {
                        gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
                        value = max;
                    }
                }
                break;
            }
            default:
                break;
            }
        }

        if (out + 1 >= sizeof(gMenuListBuffer)) {
            gMenuListBuffer[sizeof(gMenuListBuffer) - 1] = '\0';
            break;
        }

        if (value < max && out + 1 < sizeof(gMenuListBuffer)) {
            gMenuListBuffer[out++] = '\n';
            gMenuListBuffer[out] = '\0';
        }
    }

    return gMenuListBuffer;
}

static const char* UI_MENU_GetOptionLinesForId(int menuId)
{
    gSubMenuSelectionOffset = 0;

    int32_t        mMin;
    int32_t        mMax;
    char* buf = gMenuListBuffer;
    MENU_GetLimits(UI_MENU_GetCurrentMenuId(), &mMin, &mMax);

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
    case MENU_R_DCS:
    case MENU_T_DCS:
        return UI_MENU_BuildList(MENU_LIST_DCS, mMin, mMax, 0, 0U, NULL, NULL, NULL, NULL);
    case MENU_R_CTCS:
    case MENU_T_CTCS:
        return UI_MENU_BuildList(MENU_LIST_CTCSS, mMin, mMax, 0, 0U, NULL, NULL, NULL, NULL);
    case MENU_SC_REV:
        return UI_MENU_BuildList(MENU_LIST_SCAN_REV, mMin, mMax, 0, 0U, NULL, NULL, NULL, NULL);
    case MENU_TOT:
        return UI_MENU_BuildList(MENU_LIST_TIME_MMSS, mMin, mMax, 1, 5U, NULL, NULL, NULL, NULL);
    case MENU_MIC:
        return UI_MENU_BuildList(MENU_LIST_MIC_DB, mMin, mMax, 0, 0U, NULL, NULL, NULL, NULL);
    case MENU_ABR:
        return UI_MENU_BuildList(MENU_LIST_TIME_MMSS, mMin, mMax, 0, 5U, gSubMenu_OFF_ON[0], gSubMenu_OFF_ON[1], NULL, NULL);
    case MENU_ABR_MIN:
    case MENU_ABR_MAX:
        if (gIsInSubMenu) {
            BACKLIGHT_SetBrightness(gSubMenuSelection);
        }
        return UI_MENU_BuildList(MENU_LIST_NUMERIC, mMin, mMax, 0, 0U, NULL, NULL, NULL, NULL);
#ifdef ENABLE_FEAT_F4HWN_SLEEP
    case MENU_SET_OFF:
        return UI_MENU_BuildList(MENU_LIST_TIME_HM, mMin, mMax, 0, 1U, gSubMenu_OFF_ON[0], NULL, NULL, NULL);
#endif
#ifdef ENABLE_VOICE
    case MENU_VOICE:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_VOICE, sizeof(gSubMenu_VOICE[0]), ARRAY_SIZE(gSubMenu_VOICE));
#endif
    case MENU_ROGER:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_ROGER, sizeof(gSubMenu_ROGER[0]), ARRAY_SIZE(gSubMenu_ROGER));
        /*case MENU_PONMSG:
            return UI_MENU_JoinFixedList((const char*)gSubMenu_PONMSG, sizeof(gSubMenu_PONMSG[0]), ARRAY_SIZE(gSubMenu_PONMSG));*/
    case MENU_RESET:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_RESET, sizeof(gSubMenu_RESET[0]), ARRAY_SIZE(gSubMenu_RESET));
    case MENU_F_LOCK:
        return UI_MENU_JoinPtrList(gSubMenu_F_LOCK, ARRAY_SIZE(gSubMenu_F_LOCK));
        /*case MENU_BAT_TXT:
            return UI_MENU_JoinFixedList((const char*)gSubMenu_BAT_TXT, sizeof(gSubMenu_BAT_TXT[0]), ARRAY_SIZE(gSubMenu_BAT_TXT));*/
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
#ifdef ENABLE_SCANLIST
    case MENU_S_LIST:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_SLIST, sizeof(gSubMenu_SLIST[0]), ARRAY_SIZE(gSubMenu_SLIST));
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
#ifdef ENABLE_SCANLIST
    case MENU_S_ADD1:
    case MENU_S_ADD2:
    case MENU_S_ADD3:
#endif
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
    //case MENU_350TX:
    //case MENU_200TX:
    //case MENU_500TX:
#endif
    //case MENU_350EN:

#ifdef ENABLE_FEAT_F4HWN
    case MENU_SET_TMR:
#endif

    case MENU_TX_LOCK:
        return UI_MENU_JoinFixedList((const char*)gSubMenu_OFF_ON, sizeof(gSubMenu_OFF_ON[0]), ARRAY_SIZE(gSubMenu_OFF_ON));
    default:
        break;
    }

    // ************************************************************************************
    //

    switch (menuId)
    {

    case MENU_OFFSET:
        if (!gIsInSubMenu || gInputBoxIndex == 0)
        {
            snprintf(buf, sizeof(gMenuListBuffer), "%3d.%.4u MHz", gSubMenuSelection / 100000, abs(gSubMenuSelection) % 100000);
        }
        else
        {
            const char* ascii = INPUTBOX_GetAscii();
            snprintf(buf, sizeof(gMenuListBuffer), "%.3s.%.3s MHz", ascii, ascii + 3);
        }
        break;
    case MENU_AUTOLK:
        return UI_MENU_BuildList(MENU_LIST_TIME_MMSS, mMin, mMax, 0, 15U, gSubMenu_OFF_ON[0], NULL, NULL, NULL);
    case MENU_SAVE:
        return UI_MENU_BuildList(MENU_LIST_NUMERIC, mMin, mMax, 0, 0U, gSubMenu_OFF_ON[0], NULL, "1:", NULL);
        break;
    case MENU_RP_STE:
        return UI_MENU_BuildList(MENU_LIST_NUMERIC, mMin, mMax, 0, 0U, gSubMenu_OFF_ON[0], NULL, NULL, "*100ms");
        break;
    case MENU_VOX:
#ifdef ENABLE_VOX
        snprintf(buf, sizeof(gMenuListBuffer), gSubMenuSelection == 0 ? gSubMenu_OFF_ON[0] : "%u", gSubMenuSelection);
#else
        return gSubMenu_NA;
#endif
        break;
    case MENU_VOL:
        snprintf(buf, sizeof(gMenuListBuffer), "%s\n%s\n%s\n%u.%02uV %u%%",
            AUTHOR_STRING_2,
            VERSION_STRING_2,
            EDITION_STRING,
            gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100,
            BATTERY_VoltsToPercent(gBatteryVoltageAverage)
        );
        break;
        //case MENU_MEM_CH:
    case MENU_1_CALL:
        //case MENU_DEL_CH:
    {
        const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 0);
        UI_GenerateChannelStringEx(buf, valid, gSubMenuSelection);
        size_t len = strnlen(buf, sizeof(gMenuListBuffer));
        if (valid && !gAskForConfirmation) {
            const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);
            len += snprintf(&buf[len], sizeof(gMenuListBuffer) - len,
                            "\n%u.%05u", frequency / 100000, frequency % 100000);
        }
        char* name = NULL;
        SETTINGS_FetchChannelName(name, gSubMenuSelection);
        if (name != NULL) {
            len += snprintf(&buf[len], sizeof(gMenuListBuffer) - len, "\n%.8s", name);
        }
    }
    break;

    /*case MENU_MEM_NAME:
    {
        const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 0);
        UI_GenerateChannelStringEx(buf, valid, gSubMenuSelection);

    }
    break;*/

    case MENU_UPCODE:
        memcpy(buf, gEeprom.DTMF_UP_CODE, 8);
        buf[8] = '\n';
        memcpy(buf + 9, gEeprom.DTMF_UP_CODE + 8, 8);
        buf[17] = '\0';
        break;

    case MENU_DWCODE:
        memcpy(buf, gEeprom.DTMF_DOWN_CODE, 8);
        buf[8] = '\n';
        memcpy(buf + 9, gEeprom.DTMF_DOWN_CODE + 8, 8);
        buf[17] = '\0';
        break;

    case MENU_D_PRE:
        return UI_MENU_BuildList(MENU_LIST_NUMERIC, mMin, mMax, 0, 0U, NULL, NULL, NULL, "*10ms");
        break;
#ifdef ENABLE_SCANLIST
    case MENU_SLIST1:
    case MENU_SLIST2:
    case MENU_SLIST3:
        if (gSubMenuSelection < 0) {
            strcpy(buf, "NULL");
        }
        else {
            UI_GenerateChannelStringEx(buf, true, gSubMenuSelection);
        }
        break;
#endif
        // ************************************************************************************
    default:
        snprintf(buf, sizeof(gMenuListBuffer), "* %ld *", (long)gSubMenuSelection);
        break;
    }

    return buf;
}

const char* getCurrentOption() {
    const char* lines = UI_MENU_GetOptionLinesForId(UI_MENU_GetCurrentMenuId());
    if (u8x8_GetStringLineCnt(lines) == 1)
        return lines;
    int32_t index = gSubMenuSelection + gSubMenuSelectionOffset;
    if (index < 0) {
        index = 0;
    }
    return UI_GetStrValue(lines, (uint8_t)index);
}

void setSubMenu(void) {
    const char* lines = UI_MENU_GetOptionLinesForId(UI_MENU_GetCurrentMenuId());
    const uint8_t total = u8x8_GetStringLineCnt(lines);
    const uint8_t visible = MIN(total, 5);
    UI_SelectionList_Set(&subMenuList, gSubMenuSelection + gSubMenuSelectionOffset, visible > 0 ? visible : 1, 122, lines, NULL);
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

    if ((UI_MENU_GetCurrentMenuId() == MENU_RESET /*||
        UI_MENU_GetCurrentMenuId() == MENU_MEM_CH ||
        UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME ||
        UI_MENU_GetCurrentMenuId() == MENU_DEL_CH*/) && gAskForConfirmation)
    {   // display confirmation

        UI_DrawPopupWindow(20, 20, 88, 24, "Info");
        UI_SetFont(FONT_8B_TR);
        UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 36, true, false, false, (gAskForConfirmation == 1) ? "SURE?" : "WAIT!");

    }
    else if (gIsInSubMenu) {
        // gSubMenuSelection
        const int current_menu_id = UI_MENU_GetCurrentMenuId();
    #ifdef ENABLE_SCANLIST
        if (current_menu_id == MENU_SLIST1 ||
            current_menu_id == MENU_SLIST2 ||
            current_menu_id == MENU_SLIST3) {
            UI_MENU_DrawScanListPopup();
        }
        else
    #endif
        {
            UI_DrawPopupWindow(36, 6, 90, 52, UI_SelectionList_GetStringLine(&menuList));
            setSubMenu();
            UI_SelectionList_Draw(&subMenuList, 20, NULL);
        }
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
