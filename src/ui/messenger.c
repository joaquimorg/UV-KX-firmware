
#ifdef ENABLE_MESSENGER

#include <string.h>
#include "app/messenger.h"
#include "printf.h"
#include "misc.h"
#include "settings.h"
#include "ui/gui.h"
#include "ui/messenger.h"

void UI_DisplayMSG(void) {

	//memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
	UI_ClearDisplay();
	UI_SetBlackColor();

	/*if (gEeprom.KEY_LOCK && gKeypadLocked > 0)
	{	// tell user how to unlock the keyboard
		UI_SetFont(FONT_8_TR);
		UI_DrawString(UI_TEXT_ALIGN_CENTER, 0, UI_W, 22, true, false, false, "Long press #");
		UI_DrawString(UI_TEXT_ALIGN_CENTER, 0, UI_W, 38, true, false, false, "to unlock");
		UI_UpdateDisplay();
		return;
	}*/

	UI_SetBlackColor();

    UI_DrawBox(0, 0, 128, 7);
	UI_DrawBox(0, 56, 128, 8);

    UI_SetFont(FONT_8B_TR);
    UI_DrawString(UI_TEXT_ALIGN_LEFT, 2, 0, 6, false, false, false, "MESSENGER");
	UI_DrawString(UI_TEXT_ALIGN_RIGHT, 0, 126, 6, true, true, false, keyboardType == NUMERIC ? "-123-" : "-ABC-");

	/*if ( msgStatus == SENDING ) {
		GUI_DisplaySmallest("SENDING", 100, 6, false, true);
	} else if ( msgStatus == RECEIVING ) {
		GUI_DisplaySmallest("RECEIVING", 100, 6, false, true);
	} else {
		GUI_DisplaySmallest("READY", 100, 6, false, true);
	}*/

	UI_SetFont(FONT_5_TR);

	uint8_t mPos = 14;
	const uint8_t mLine = 7;
	for (uint8_t i = 0; i < MAX_LINES; ++i) {
		UI_DrawString(UI_TEXT_ALIGN_LEFT, 2, 0, mPos, true, false, false, rxMessage[i]);
		mPos += mLine;
    }

	UI_SetFont(FONT_8_TR);
	UI_DrawStringf(UI_TEXT_ALIGN_LEFT, 2, 0, 62, false, false, false, "%s_", cMessage);

	UI_UpdateDisplay();
}

#endif
