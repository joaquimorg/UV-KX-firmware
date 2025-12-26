#include "ui/gui.h"

#include <stdarg.h>
#include <string.h>

#include "app/uart.h"
#include "font/font_10_tr.h"
#include "font/font_5_tr.h"
#include "font/font_8_tr.h"
#include "font/font_8b_tr.h"
#include "font/font_bn_tn.h"
#include "font/icons.h"
#include "printf.h"

#define UI_CHAR_BUFFER_SIZE 600

UI_Context gUiCtx;

static char ui_buffer[UI_CHAR_BUFFER_SIZE];

static void UI_DrawXbm(u8g2_uint_t x, u8g2_uint_t y, bool color,
                       u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bits)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }
    u8g2_SetDrawColor(gUiCtx.lcd, color ? UI_BLACK : UI_WHITE);
    u8g2_DrawXBM(gUiCtx.lcd, x, y, w, h, bits);
}

static void UI_SelectionList_AdjustWindow(UI_SelectionList *list)
{
    if (list->u8sl.total == 0 || list->u8sl.visible == 0) {
        list->u8sl.current_pos = 0;
        list->u8sl.first_pos = 0;
        return;
    }

    if (list->u8sl.current_pos >= list->u8sl.total) {
        list->u8sl.current_pos = (uint8_t)(list->u8sl.total - 1);
    }

    uint8_t middle = (uint8_t)(list->u8sl.visible / 2);
    if (list->u8sl.current_pos >= middle && list->u8sl.current_pos < list->u8sl.total - middle) {
        list->u8sl.first_pos = (uint8_t)(list->u8sl.current_pos - middle);
    } else if (list->u8sl.current_pos >= list->u8sl.total - middle) {
        list->u8sl.first_pos = (uint8_t)(list->u8sl.total - list->u8sl.visible);
    } else {
        list->u8sl.first_pos = 0;
    }
}

void UI_Init(u8g2_t *lcd)
{
    gUiCtx.lcd = lcd;
    gUiCtx.only_upper_case = false;
    gUiCtx.info_message = UI_INFO_NONE;
    gUiCtx.message_result = 0;
    gUiCtx.menu_pos = 1;
}

void UI_ClearDisplay(void)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    u8g2_ClearBuffer(gUiCtx.lcd);
}

void UI_UpdateDisplay(void)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    if (gUiCtx.info_message != UI_INFO_NONE) {
        UI_DrawPopupWindow(20, 20, 88, 24, "Info");
        UI_SetFont(UI_FONT_8B_TR);
        UI_DrawString(UI_TEXT_ALIGN_CENTER, 22, 106, 38, true, false, false,
                      UI_GetStrValue(UI_INFO_MESSAGE_STR, (uint8_t)gUiCtx.info_message - 1));
    }

    u8g2_SendBuffer(gUiCtx.lcd);

    sendScreenBuffer(u8g2_GetBufferPtr(gUiCtx.lcd), 1024);
}

void UI_TimeOut(void)
{
    if (gUiCtx.info_message != UI_INFO_NONE) {
        gUiCtx.info_message = UI_INFO_NONE;
    }
}

void UI_SetInfoMessage(UI_InfoMessageType message)
{
    gUiCtx.info_message = message;
}

UI_InfoMessageType UI_GetInfoMessage(void)
{
    return gUiCtx.info_message;
}

void UI_SetFont(UI_Font font)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    switch (font) {
    case UI_FONT_5_TR:
        u8g2_SetFont(gUiCtx.lcd, u8g2_font_5_tr);
        gUiCtx.only_upper_case = true;
        break;
    case UI_FONT_8_TR:
        u8g2_SetFont(gUiCtx.lcd, u8g2_font_8_tr);
        gUiCtx.only_upper_case = false;
        break;
    case UI_FONT_8B_TR:
        u8g2_SetFont(gUiCtx.lcd, u8g2_font_8_tr); //u8g2_font_8b_tr save space
        gUiCtx.only_upper_case = false;
        break;
    case UI_FONT_10_TR:
        u8g2_SetFont(gUiCtx.lcd, u8g2_font_10_tr);
        gUiCtx.only_upper_case = true;
        break;
    case UI_FONT_BN_TN:
        u8g2_SetFont(gUiCtx.lcd, u8g2_font_bn_tn);
        gUiCtx.only_upper_case = true;
        break;
    }
}

void UI_SetBlackColor(void)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }
    u8g2_SetDrawColor(gUiCtx.lcd, UI_BLACK);
}

void UI_SetWhiteColor(void)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }
    u8g2_SetDrawColor(gUiCtx.lcd, UI_WHITE);
}

void UI_DrawStrf(u8g2_uint_t x, u8g2_uint_t y, const char *fmt, ...)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    char text[40] = {0};
    va_list va;
    va_start(va, fmt);
    vsnprintf(text, sizeof(text), fmt, va);
    va_end(va);

    u8g2_DrawStr(gUiCtx.lcd, x, y, text);
}

void UI_DrawString(UI_TextAlign align,
                   u8g2_uint_t xstart,
                   u8g2_uint_t xend,
                   u8g2_uint_t y,
                   bool is_black,
                   bool is_fill,
                   bool is_box,
                   const char *str)
{
    if (gUiCtx.lcd == NULL || str == NULL) {
        return;
    }

    u8g2_uint_t startX = xstart;
    u8g2_uint_t endX = xend;
    u8g2_uint_t stringWidth = u8g2_GetStrWidth(gUiCtx.lcd, str);

    u8g2_uint_t xx;
    u8g2_uint_t yy;
    u8g2_uint_t ww;
    u8g2_uint_t hh;

    const u8g2_uint_t border_width = 0;

    u8g2_uint_t padding_h = 2;
    u8g2_uint_t padding_v = 1;
    if (is_box) {
        padding_v++;
    }

    u8g2_uint_t h = u8g2_GetAscent(gUiCtx.lcd);

    if (endX > startX) {
        if (align == UI_TEXT_ALIGN_CENTER) {
            if (stringWidth < (endX - startX)) {
                startX = (u8g2_uint_t)(((startX + endX) / 2U) - (stringWidth / 2U));
                endX = stringWidth;
            }
        } else if (align == UI_TEXT_ALIGN_RIGHT) {
            startX = endX - stringWidth;
        }
    }

    xx = startX;
    xx -= padding_h;
    xx -= border_width;
    ww = (u8g2_uint_t)((endX > startX ? (endX - startX) : stringWidth) + (2U * padding_h) + (2U * border_width));

    yy = y;
    yy -= h;
    yy -= padding_v;
    yy -= border_width;
    hh = (u8g2_uint_t)(h + (2U * padding_v) + (2U * border_width));

    u8g2_SetDrawColor(gUiCtx.lcd, is_black ? UI_BLACK : UI_WHITE);
    if (is_fill) {
        if (align == UI_TEXT_ALIGN_CENTER) {
            u8g2_DrawBox(gUiCtx.lcd, xstart, yy, xend - xstart, hh);
        } else {
            u8g2_DrawBox(gUiCtx.lcd, xx, yy, ww, hh);
        }
        u8g2_SetDrawColor(gUiCtx.lcd, is_black ? UI_WHITE : UI_BLACK);
    } else if (is_box) {
        u8g2_DrawFrame(gUiCtx.lcd, xx, yy, ww, hh);
    }

    u8g2_DrawStr(gUiCtx.lcd, startX, y, str);
}

void UI_DrawStringf(UI_TextAlign align,
                    u8g2_uint_t xstart,
                    u8g2_uint_t xend,
                    u8g2_uint_t y,
                    bool is_black,
                    bool is_fill,
                    bool is_box,
                    const char *fmt,
                    ...)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    char text[60] = {0};
    va_list va;
    va_start(va, fmt);
    vsnprintf(text, sizeof(text), fmt, va);
    va_end(va);

    UI_DrawString(align, xstart, xend, y, is_black, is_fill, is_box, text);
}

const char *UI_GetStrValue(const char *str, uint8_t index)
{
    return u8x8_GetStringLineStart(index, str);
}

int UI_StringLengthNL(const char *str)
{
    int length = 0;
    while (str[length] != '\n' && str[length] != '\0') {
        ++length;
    }
    return length;
}

void UI_DrawPopupWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char *title)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    UI_SetWhiteColor();
    u8g2_DrawBox(gUiCtx.lcd, x - 1, y - 1, w + 3, h + 4);
    UI_SetBlackColor();
    u8g2_DrawFrame(gUiCtx.lcd, x, y, (uint8_t)(w + 1), (uint8_t)(h + 2));
    u8g2_DrawFrame(gUiCtx.lcd, x, y, w, (uint8_t)(h + 1));

    // manual rounded corners (no u8g2_circle)
    u8g2_SetDrawColor(gUiCtx.lcd, UI_WHITE);
    u8g2_DrawPixel(gUiCtx.lcd, x, y);
    u8g2_DrawPixel(gUiCtx.lcd, x + 1, y);
    u8g2_DrawPixel(gUiCtx.lcd, x, y + 1);
    u8g2_DrawPixel(gUiCtx.lcd, (uint8_t)(x + w), y);
    u8g2_DrawPixel(gUiCtx.lcd, (uint8_t)(x + w - 1), y);
    u8g2_DrawPixel(gUiCtx.lcd, (uint8_t)(x + w), (uint8_t)(y + 1));
    u8g2_DrawPixel(gUiCtx.lcd, x, (uint8_t)(y + h + 1));
    u8g2_DrawPixel(gUiCtx.lcd, x + 1, (uint8_t)(y + h + 1));
    u8g2_DrawPixel(gUiCtx.lcd, x, (uint8_t)(y + h));
    u8g2_DrawPixel(gUiCtx.lcd, (uint8_t)(x + w), (uint8_t)(y + h + 1));
    u8g2_DrawPixel(gUiCtx.lcd, (uint8_t)(x + w - 1), (uint8_t)(y + h + 1));
    u8g2_DrawPixel(gUiCtx.lcd, (uint8_t)(x + w), (uint8_t)(y + h));
    u8g2_SetDrawColor(gUiCtx.lcd, UI_BLACK);

    u8g2_DrawBox(gUiCtx.lcd, (uint8_t)(x + 1), (uint8_t)(y + 1), (uint8_t)(w - 1), 6);

    UI_SetFont(UI_FONT_8B_TR);
    UI_DrawString(UI_TEXT_ALIGN_CENTER, x, (uint8_t)(x + w), (uint8_t)(y + 6), false, false, false, title);
}

void UI_DrawIc8Battery50(u8g2_uint_t x, u8g2_uint_t y, bool color)
{
    UI_DrawXbm(x, y, color, batt_50_width, batt_50_height, batt_50_bits);
}

void UI_DrawIc8Charging(u8g2_uint_t x, u8g2_uint_t y, bool color)
{
    UI_DrawXbm(x, y, color, charging_width, charging_height, charging_bits);
}

void UI_DrawSmeter(u8g2_uint_t x, u8g2_uint_t y, bool color)
{
    UI_DrawXbm(x, y, color, smeter_width, smeter_height, smeter_bits);
}

void UI_DrawMmeter(u8g2_uint_t x, u8g2_uint_t y, bool color)
{
    UI_DrawXbm(x, y, color, mmeter_width, mmeter_height, mmeter_bits);
}

void UI_DrawDotline(u8g2_uint_t x, u8g2_uint_t y, bool color)
{
    UI_DrawXbm(x, y, color, dotline_width, dotline_height, dotline_bits);
}

void UI_DrawPs(u8g2_uint_t x, u8g2_uint_t y, bool color)
{
    UI_DrawXbm(x, y, color, batt_ps_width, batt_ps_height, batt_ps_bits);
}

void UI_DrawSave(u8g2_uint_t x, u8g2_uint_t y, bool color)
{
    UI_DrawXbm(x, y, color, memory_width, memory_height, memory_bits);
}

void UI_DrawLock(u8g2_uint_t x, u8g2_uint_t y, bool inverse)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    if (inverse) {
        u8g2_SetDrawColor(gUiCtx.lcd, UI_BLACK);
        u8g2_DrawBox(gUiCtx.lcd, x, y, lock_width + 2U, lock_height + 2U);
        u8g2_SetDrawColor(gUiCtx.lcd, UI_WHITE);
        u8g2_DrawXBM(gUiCtx.lcd, x + 1U, y + 1U, lock_width, lock_height, lock_bits);
    } else {
        u8g2_SetDrawColor(gUiCtx.lcd, UI_BLACK);
        u8g2_DrawXBM(gUiCtx.lcd, x, y, lock_width, lock_height, lock_bits);
    }
}

void UI_DrawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }
    u8g2_DrawBox(gUiCtx.lcd, x, y, w, h);
}

void UI_DrawFrequencyBig(bool invert, uint32_t freq, u8g2_uint_t xend, u8g2_uint_t y)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    UI_SetFont(UI_FONT_BN_TN);

    if (freq >= 100000000U) {
        UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, xend, y, true, invert, false, "%1lu.%03lu.%03lu",
                       freq / 100000000UL, (freq / 100000UL) % 1000UL, (freq % 100000UL) / 100UL);
    } else if (freq >= 10000000U) {
        UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, xend, y, true, invert, false, "%3lu.%03lu",
                       freq / 100000UL, (freq % 100000UL) / 100UL);
    } else {
        UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, xend, y, true, invert, false, "%2lu.%03lu",
                       freq / 100000UL, (freq % 100000UL) / 100UL);
    }
    UI_SetBlackColor();

    UI_SetFont(UI_FONT_10_TR);
    UI_DrawStringf(UI_TEXT_ALIGN_LEFT, (u8g2_uint_t)(xend + 2), 0, y, true, invert, false, "%02lu", freq % 100UL);
}

void UI_DrawFrequencySmall(bool invert, uint32_t freq, u8g2_uint_t xend, u8g2_uint_t y)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    UI_SetFont(UI_FONT_10_TR);
    if (freq >= 100000000U) {
        UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, xend, y, true, invert, false, "%1lu.%03lu.%03lu.%02lu",
                       freq / 100000000UL, (freq / 100000UL) % 1000UL, (freq % 100000UL) / 100UL, freq % 100UL);
    } else if (freq >= 10000000U) {
        UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, xend, y, true, invert, false, "%3lu.%03lu.%02lu",
                       freq / 100000UL, (freq % 100000UL) / 100UL, freq % 100UL);
    } else {
        UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, xend, y, true, invert, false, "%2lu.%03lu.%02lu",
                       freq / 100000UL, (freq % 100000UL) / 100UL, freq % 100UL);
    }
}

void UI_DrawRSSI(uint8_t s_level, u8g2_uint_t x, u8g2_uint_t y)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    UI_DrawSmeter(x, y, UI_BLACK);
    UI_SetBlackColor();

    u8g2_uint_t currentX = x;
    for (uint8_t i = 0; i < s_level && i < 9; ++i) {
        u8g2_DrawBox(gUiCtx.lcd, currentX, y + 6, 3, 3);
        currentX += 4;
    }
}

void UI_DrawBatteryIcon(uint8_t level, u8g2_uint_t x, u8g2_uint_t y)
{
    if (gUiCtx.lcd == NULL) {
        return;
    }

    UI_DrawIc8Battery50(x, y, UI_BLACK);
    UI_SetBlackColor();

    uint8_t fill = (uint8_t)((level * 10U) / 100U);
    u8g2_DrawBox(gUiCtx.lcd, x + 1, y + 1, fill, 3);
}

const char *UI_GenerateCTDCList(const uint16_t *options, size_t count, bool is_ctcss)
{
    char *ptr = ui_buffer;
    size_t remaining = UI_CHAR_BUFFER_SIZE;
    int written;

    ui_buffer[0] = '\0';

    for (size_t i = 0; i < count; i++) {
        if (is_ctcss) {
            written = snprintf(ptr, remaining, "%u.%u%s", options[i] / 10U, options[i] % 10U, (i + 1 == count) ? "" : "\n");
        } else {
            written = snprintf(ptr, remaining, "D%03o%s", options[i], (i + 1 == count) ? "" : "\n");
        }

        if (written < 0 || (size_t)written >= remaining) {
            break;
        }

        ptr += written;
        remaining -= (size_t)written;
    }

    return ui_buffer;
}

const char *UI_GetFrequencyString(uint32_t frequency, uint8_t precision, bool is_khz)
{
    if (is_khz) {
        if (precision == 0) {
            snprintf(ui_buffer, UI_CHAR_BUFFER_SIZE, "%lu.%03lu KHz",
                     frequency / 1000UL,
                     (frequency % 1000UL) / 10UL);
        } else {
            snprintf(ui_buffer, UI_CHAR_BUFFER_SIZE, "%lu.%03lu KHz",
                     frequency / 1000UL,
                     frequency % 1000UL);
        }
    } else {
        if (precision == 0) {
            snprintf(ui_buffer, UI_CHAR_BUFFER_SIZE, "%lu.%03lu Hz",
                     frequency / 1000UL,
                     (frequency % 1000UL) / 10UL);
        } else {
            snprintf(ui_buffer, UI_CHAR_BUFFER_SIZE, "%lu.%03lu Hz",
                     frequency / 1000UL,
                     frequency % 1000UL);
        }
    }
    return ui_buffer;
}

static u8g2_uint_t UI_DrawSelectionListLine(UI_SelectionList *list, u8g2_uint_t y, uint8_t idx, const char *info)
{
    if (list == NULL || list->ui == NULL || list->ui->lcd == NULL) {
        return 0;
    }

    uint8_t is_invert = 0;

    u8g2_uint_t line_height = (u8g2_uint_t)(u8g2_GetAscent(list->ui->lcd) - u8g2_GetDescent(list->ui->lcd) + 2);

    if (idx == list->u8sl.current_pos) {
        is_invert = 1;
    }

    const char *line = u8x8_GetStringLineStart(idx, list->slines);
    if (line == NULL) {
        return line_height;
    }

    if (list->show_line_numbers) {
        UI_SetFont(UI_FONT_5_TR);
        UI_DrawStringf(UI_TEXT_ALIGN_LEFT, list->start_x_pos, 0, y, is_invert, true, false, "%02u", idx + 1);
    }

    UI_SetFont(is_invert ? UI_FONT_8B_TR : UI_FONT_8_TR);

    if (list->show_line_numbers) {
        UI_DrawString(UI_TEXT_ALIGN_LEFT, (u8g2_uint_t)(list->start_x_pos + 14), list->max_width, y, is_invert, true, false, line);
        if (info != NULL && is_invert) {
            UI_SetFont(UI_FONT_8B_TR);
            if (list->suffix == NULL) {
                UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, (u8g2_uint_t)(list->max_width - 2), y, !is_invert, true, false, info);
            } else {
                UI_DrawStringf(UI_TEXT_ALIGN_RIGHT, 0, (u8g2_uint_t)(list->max_width - 2), y, !is_invert, true, false, "%.*s %s",
                               UI_StringLengthNL(info), info, list->suffix);
            }
        }
    } else {
        if (list->suffix == NULL) {
            UI_DrawString(UI_TEXT_ALIGN_CENTER, list->start_x_pos, list->max_width, y, is_invert, true, false, line);
        } else {
            UI_DrawStringf(UI_TEXT_ALIGN_CENTER, list->start_x_pos, list->max_width, y, is_invert, true, false, "%.*s %s",
                           UI_StringLengthNL(line), line, list->suffix);
        }
    }

    return line_height;
}

static void UI_DrawSelectionList(UI_SelectionList *list, u8g2_uint_t y, const char *info)
{
    if (list == NULL) {
        return;
    }

    for (uint8_t i = 0; i < list->u8sl.visible; i++) {
        y += UI_DrawSelectionListLine(list, y, (uint8_t)(i + list->u8sl.first_pos), info);
    }
}

void UI_SelectionList_Init(UI_SelectionList *list)
{
    if (list == NULL) {
        return;
    }

    memset(&list->u8sl, 0, sizeof(list->u8sl));
    list->slines = NULL;
    list->suffix = NULL;
    list->max_width = 75;
    list->start_x_pos = 2;
    list->show_line_numbers = true;
    list->ui = &gUiCtx;
}

void UI_SelectionList_Next(UI_SelectionList *list)
{
    if (list == NULL) {
        return;
    }

    list->u8sl.current_pos++;
    if (list->u8sl.current_pos >= list->u8sl.total) {
        list->u8sl.current_pos = 0;
        list->u8sl.first_pos = 0;
        return;
    }

    UI_SelectionList_AdjustWindow(list);
}

void UI_SelectionList_Prev(UI_SelectionList *list)
{
    if (list == NULL) {
        return;
    }

    if (list->u8sl.current_pos == 0) {
        list->u8sl.current_pos = (uint8_t)(list->u8sl.total - 1);
        list->u8sl.first_pos = (list->u8sl.total > list->u8sl.visible) ? (uint8_t)(list->u8sl.total - list->u8sl.visible) : 0;
    } else {
        list->u8sl.current_pos--;
        UI_SelectionList_AdjustWindow(list);
    }
}

void UI_SelectionList_Set(UI_SelectionList *list, uint8_t start_pos, uint8_t display_lines, uint8_t maxw, const char *sl, const char *sf)
{
    if (list == NULL) {
        return;
    }

    list->u8sl.visible = display_lines;

    list->u8sl.total = u8x8_GetStringLineCnt(sl);
    if (list->u8sl.total <= list->u8sl.visible) {
        list->u8sl.visible = list->u8sl.total;
    }

    list->u8sl.current_pos = start_pos;
    UI_SelectionList_AdjustWindow(list);

    list->slines = sl;
    list->suffix = sf;
    list->max_width = maxw;
}

void UI_SelectionList_SetCurrentPos(UI_SelectionList *list, uint8_t pos)
{
    if (list == NULL) {
        return;
    }

    list->u8sl.current_pos = pos;
    UI_SelectionList_AdjustWindow(list);
}

uint8_t UI_SelectionList_GetListPos(const UI_SelectionList *list)
{
    if (list == NULL) {
        return 0;
    }
    return list->u8sl.current_pos;
}

uint8_t UI_SelectionList_GetTotal(const UI_SelectionList *list)
{
    if (list == NULL) {
        return 0;
    }
    return list->u8sl.total;
}

void UI_SelectionList_Draw(UI_SelectionList *list, uint8_t y, const char *info)
{
    if (list == NULL || list->ui == NULL || list->ui->lcd == NULL) {
        return;
    }

    u8g2_SetFontPosBaseline(list->ui->lcd);
    UI_DrawSelectionList(list, y, info);
}

void UI_SelectionList_SetStartXPos(UI_SelectionList *list, uint8_t x)
{
    if (list == NULL) {
        return;
    }
    list->start_x_pos = x;
}

void UI_SelectionList_SetMaxWidth(UI_SelectionList *list, uint8_t w)
{
    if (list == NULL) {
        return;
    }
    list->max_width = w;
}

void UI_SelectionList_SetShowLineNumbers(UI_SelectionList *list, bool show)
{
    if (list == NULL) {
        return;
    }
    list->show_line_numbers = show;
}

const char *UI_SelectionList_GetStringLine(UI_SelectionList *list)
{
    if (list == NULL) {
        return NULL;
    }
    return u8x8_GetStringLineStart(list->u8sl.current_pos, list->slines);
}

void UI_SelectionList_SetSuffix(UI_SelectionList *list, const char *sf)
{
    if (list == NULL) {
        return;
    }
    list->suffix = sf;
}

void UI_SelectionListPopup_Init(UI_SelectionListPopup *popup)
{
    if (popup == NULL) {
        return;
    }

    UI_SelectionList_Init(&popup->base);
    UI_SelectionList_SetShowLineNumbers(&popup->base, false);
    popup->title = NULL;
}

void UI_SelectionListPopup_Draw(UI_SelectionListPopup *popup, bool is_settings)
{
    if (popup == NULL) {
        return;
    }

    uint8_t popupWidth;
    uint8_t popupHeight;
    uint8_t x;
    uint8_t y;

    if (is_settings) {
        popupWidth = 90;
        popupHeight = 52;
        x = 36;
        y = (uint8_t)((UI_H - popupHeight) / 2U);
    } else {
        popupWidth = 72;
        popupHeight = 34;
        x = (uint8_t)((UI_W - popupWidth) / 2U);
        y = (uint8_t)((UI_H - popupHeight) / 2U);
    }

    UI_DrawPopupWindow(x, y, popupWidth, popupHeight, popup->title);

    UI_SelectionList_SetMaxWidth(&popup->base, (uint8_t)(x + popupWidth - 4));
    UI_SelectionList_SetStartXPos(&popup->base, (uint8_t)(x + 4));
    UI_SelectionList_Draw(&popup->base, (uint8_t)(y + 14), NULL);
}

void UI_SelectionListPopup_SetTitle(UI_SelectionListPopup *popup, const char *title)
{
    if (popup == NULL) {
        return;
    }
    popup->title = title;
}
