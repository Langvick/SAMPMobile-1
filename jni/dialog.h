#pragma once

#include "game/game.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "vendor/imgui/imgui_internal.h"
#include "keyboard.h"
#include "chatwindow.h"

#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>

#define SCROLL_BAR_ELEMENTS_MAX 20

enum dialogStyles : uint8_t {
	DIALOG_STYLE_MSGBOX = 0,
	DIALOG_STYLE_INPUT,
	DIALOG_STYLE_LIST,
	DIALOG_STYLE_PASSWORD,
	DIALOG_STYLE_TABLIST,
	DIALOG_STYLE_TABLIST_HEADERS
};

struct dialogData {
	uint16_t usDialogId = 0;
	dialogStyles ucDialogType = DIALOG_STYLE_MSGBOX;
	std::string szTitle = std::string("Title");
	std::string szMainText = std::string("MainText");
	std::string szLeftButton = std::string("Left");
	std::string szRightButton = std::string("Right");
};

class CDialogWindow {
public:
	CDialogWindow() {

	};
	
	~CDialogWindow() {

	};

	void Show(dialogData *inputDialog);
	void Clear();
	
	bool GetState() {
		return m_bIsActive;
	};
	
	void Render();

	ImVec4 CovertToImGuiColor(ImU32 uiColor, bool bUseAlpha);
	void PushTextWithColour(std::string szText, ImU32 uiColor);
	void PushColouredText(splittedText *pSplittedText);
	void PushListItemButton(uint16_t usListCount);
	void SwitchContextId(uint16_t usListCount);
	void PushColouredList(splittedText *pSplittedText);
	void SetPasswordText(char* szText);

private:
	bool m_bIsActive = false;
	dialogData *currentDialogData = nullptr;
	splittedText *splittedTitle = nullptr;
	splittedText *splittedMainText = nullptr;
	uint16_t lastPushedId = 0;
	size_t mainTextLines = 0;
	char m_szPasswordInputText[32*2];
	
public:
	uint8_t m_byteInputState = 0;
};