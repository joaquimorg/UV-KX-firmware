#ifndef UI_GUI_H
#define UI_GUI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "u8g2.h"

#define UI_BLACK 1
#define UI_WHITE 0

#define UI_W 128
#define UI_H 64

typedef enum {
    UI_TEXT_ALIGN_LEFT,
    UI_TEXT_ALIGN_CENTER,
    UI_TEXT_ALIGN_RIGHT
} UI_TextAlign;

typedef enum {
    UI_FONT_5_TR,
    UI_FONT_8_TR,
    UI_FONT_8B_TR,
    UI_FONT_10_TR,
    UI_FONT_BN_TN
} UI_Font;

typedef enum {
    UI_INFO_NONE = 0,
    UI_INFO_LOW_BATTERY = 1,
    UI_INFO_TX_DISABLED = 2,
    UI_INFO_UART_COMM = 3
} UI_InfoMessageType;


#define UI_TX_STR   "TX"
#define UI_RX_STR   "RX"
#define UI_HZ_STR   "Hz"
#define UI_KHZ_STR  "KHz"
#define UI_VFO_STR  "VFO"
#define UI_DB_STR   "Db"

#define UI_INFO_MESSAGE_STR "BATTERY LOW\nTX DISABLED\nUART IN USE"
#define UI_BANDWIDTH_STR "WIDE\nNAR\nNAR+"
#define UI_POWER_STR "LOW1\nLOW2\nLOW3\nLOW4\nLOW5\nMID\nHIGH"

typedef struct {
    u8g2_t *lcd;
    bool only_upper_case;
    UI_InfoMessageType info_message;
    uint8_t message_result;
    uint8_t menu_pos;
} UI_Context;

extern UI_Context gUiCtx;

void UI_Init(u8g2_t *lcd);
void UI_ClearDisplay(void);
void UI_UpdateDisplay(void);
void UI_TimeOut(void);
void UI_SetInfoMessage(UI_InfoMessageType message);
UI_InfoMessageType UI_GetInfoMessage(void);
void UI_SetFont(UI_Font font);
void UI_SetBlackColor(void);
void UI_SetWhiteColor(void);
void UI_DrawStrf(u8g2_uint_t x, u8g2_uint_t y, const char *fmt, ...);
void UI_DrawString(UI_TextAlign align, u8g2_uint_t xstart, u8g2_uint_t xend, u8g2_uint_t y, bool is_black, bool is_fill, bool is_box, const char *str);
void UI_DrawWords(u8g2_uint_t xloc, u8g2_uint_t yloc, const char *msg);
void UI_DrawStringf(UI_TextAlign align, u8g2_uint_t xstart, u8g2_uint_t xend, u8g2_uint_t y, bool is_black, bool is_fill, bool is_box, const char *fmt, ...);
const char *UI_GetStrValue(const char *str, uint8_t index);
int UI_StringLengthNL(const char *str);
void UI_DrawPopupWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char *title);
uint8_t UI_KeycodeToNumber(uint8_t keycode);

void UI_DrawIc8Battery50(u8g2_uint_t x, u8g2_uint_t y, bool color);
void UI_DrawIc8Charging(u8g2_uint_t x, u8g2_uint_t y, bool color);
void UI_DrawSmeter(u8g2_uint_t x, u8g2_uint_t y, bool color);
void UI_DrawMmeter(u8g2_uint_t x, u8g2_uint_t y, bool color);
void UI_DrawDotline(u8g2_uint_t x, u8g2_uint_t y, bool color);
void UI_DrawPs(u8g2_uint_t x, u8g2_uint_t y, bool color);
void UI_DrawSave(u8g2_uint_t x, u8g2_uint_t y, bool color);
void UI_DrawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h);

void UI_DrawFrequencyBig(bool invert, uint32_t freq, u8g2_uint_t xend, u8g2_uint_t y);
void UI_DrawFrequencySmall(bool invert, uint32_t freq, u8g2_uint_t xend, u8g2_uint_t y);
int16_t UI_ConvertRSSIToPixels(int16_t rssi_dbm);
void UI_DrawRSSI(uint8_t s_level, u8g2_uint_t x, u8g2_uint_t y);
void UI_DrawBatteryIcon(uint8_t level, u8g2_uint_t x, u8g2_uint_t y);
const char *UI_GenerateCTDCList(const uint16_t *options, size_t count, bool is_ctcss);
const char *UI_GetFrequencyString(uint32_t frequency, uint8_t precision, bool is_khz);

typedef struct {
    u8sl_t u8sl;
    const char *slines;
    const char *suffix;
    uint8_t max_width;
    uint8_t start_x_pos;
    bool show_line_numbers;
    UI_Context *ui;
} UI_SelectionList;

void UI_SelectionList_Init(UI_SelectionList *list);
void UI_SelectionList_Next(UI_SelectionList *list);
void UI_SelectionList_Prev(UI_SelectionList *list);
void UI_SelectionList_Set(UI_SelectionList *list, uint8_t start_pos, uint8_t display_lines, uint8_t maxw, const char *sl, const char *sf);
void UI_SelectionList_SetCurrentPos(UI_SelectionList *list, uint8_t pos);
uint8_t UI_SelectionList_GetListPos(const UI_SelectionList *list);
uint8_t UI_SelectionList_GetTotal(const UI_SelectionList *list);
void UI_SelectionList_Draw(UI_SelectionList *list, uint8_t y, const char *info);
void UI_SelectionList_SetStartXPos(UI_SelectionList *list, uint8_t x);
void UI_SelectionList_SetMaxWidth(UI_SelectionList *list, uint8_t w);
void UI_SelectionList_SetShowLineNumbers(UI_SelectionList *list, bool show);
const char *UI_SelectionList_GetStringLine(UI_SelectionList *list);
void UI_SelectionList_SetSuffix(UI_SelectionList *list, const char *sf);

typedef struct {
    UI_SelectionList base;
    const char *title;
} UI_SelectionListPopup;

void UI_SelectionListPopup_Init(UI_SelectionListPopup *popup);
void UI_SelectionListPopup_Draw(UI_SelectionListPopup *popup, bool is_settings);
void UI_SelectionListPopup_SetTitle(UI_SelectionListPopup *popup, const char *title);

#define FONT_5_TR  UI_FONT_5_TR
#define FONT_8_TR  UI_FONT_8_TR
#define FONT_8B_TR UI_FONT_8B_TR
#define FONT_10_TR UI_FONT_10_TR
#define FONT_BN_TN UI_FONT_BN_TN

#endif
