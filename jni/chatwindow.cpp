#include "main.h"
#include "gui/gui.h"
#include "chatwindow.h"
#include "keyboard.h"
#include "settings.h"
#include "game/game.h"
#include "net/netgame.h"
#include "playerslist.h"

extern CGUI *pGUI;
extern CKeyBoard *pKeyBoard;
extern CSettings *pSettings;
extern CNetGame *pNetGame;
extern CPlayersList *pPlayersList;

void ChatWindowInputHandler(const char* str)
{
	if(!str || *str == '\0') return;
	if(!pNetGame) return;

	if(*str == '/')
		pNetGame->SendChatCommand(str);
	else
		pNetGame->SendChatMessage(str);
	return;
}

CChatWindow::CChatWindow() {
	Log("Initializng Chat Window..");
	
	m_fChatPosX = pGUI->ScaleX(pSettings->Get().fChatPosX);
	m_fChatPosY = pGUI->ScaleY(pSettings->Get().fChatPosY);
	// Log("Chat pos: %f, %f", m_fChatPosX, m_fChatPosY);
	
	m_fChatSizeX = pGUI->ScaleX(pSettings->Get().fChatSizeX);
	m_fChatSizeY = pGUI->ScaleY(pSettings->Get().fChatSizeY);
	// Log("Size: %f, %f", m_fChatSizeX, m_fChatSizeY);
	
	m_iMaxMessages = pSettings->Get().iChatMaxMessages;

	m_dwTextColor = 0xFFFFFFFF;
	m_dwInfoColor = 0x00C8C8FF;
	m_dwDebugColor = 0xBEBEBEFF;
	
	m_byteRepeatKeys = 1;
}

CChatWindow::~CChatWindow()
{
}

bool CChatWindow::OnTouchEvent(int type, bool multi, int x, int y) {
	static bool bWannaOpenChat = false;
	switch(type) {
		case TOUCH_PUSH: {
			if (x >= m_fChatPosX && x <= m_fChatPosX + m_fChatSizeX && y >= m_fChatPosY && y <= m_fChatPosY + m_fChatSizeY) {
				bWannaOpenChat = true;
			}
			break;
		}

		case TOUCH_POP: {
			if(bWannaOpenChat && x >= m_fChatPosX && x <= m_fChatPosX + m_fChatSizeX && y >= m_fChatPosY && y <= m_fChatPosY + m_fChatSizeY) {
				if(!pPlayersList->m_bIsActive)
					pKeyBoard->Open(&ChatWindowInputHandler);
			}
			bWannaOpenChat = false;
			break;
		}

		case TOUCH_MOVE: {
			break;
		}
	}

	return true;
}

void CChatWindow::Render() {
	ImVec2 pos = ImVec2(m_fChatPosX, m_fChatPosY);

	for(auto entry : m_ChatWindowEntries) {
		switch(entry.eType) {
			case CHAT_TYPE_CHAT: {
				if(entry.szNick[0] != 0) {
					RenderText(entry.szNick, pos.x, pos.y, entry.dwNickColor);
					pos.x += ImGui::CalcTextSize(entry.szNick).x + ImGui::CalcTextSize(" ").x;
				}
				RenderText(entry.utf8Message, pos.x, pos.y, entry.dwTextColor);
				break;
			}

			case CHAT_TYPE_INFO:
			case CHAT_TYPE_DEBUG: {
				RenderText(entry.utf8Message, pos.x, pos.y, entry.dwTextColor);
				break;
			}
		}

		pos.x = m_fChatPosX;
		pos.y += pGUI->GetFontSize();
	}
}

void CChatWindow::RenderText(const char* u8Str, float posX, float posY, uint32_t dwColor) {
	ImVec2 vecPosition = ImVec2(posX, posY);
	pGUI->Render2DColouredText(vecPosition, dwColor, true, std::string(u8Str));
}

void CChatWindow::AddChatMessage(char* szNick, uint32_t dwNickColor, char* szMessage) {
	FilterInvalidChars(szMessage);
	AddToChatWindowBuffer(CHAT_TYPE_CHAT, szMessage, szNick, m_dwTextColor, dwNickColor);
}

void CChatWindow::AddInfoMessage(char* szFormat, ...) {
	char tmp_buf[512];
	memset(tmp_buf, 0, sizeof(tmp_buf));

	va_list args;
	va_start(args, szFormat);
	vsprintf(tmp_buf, szFormat, args);
	va_end(args);

	FilterInvalidChars(tmp_buf);
	AddToChatWindowBuffer(CHAT_TYPE_INFO, tmp_buf, nullptr, m_dwInfoColor, 0);
}

void CChatWindow::AddDebugMessage(char *szFormat, ...) {
	char tmp_buf[512];
	memset(tmp_buf, 0, sizeof(tmp_buf));

	va_list args;
	va_start(args, szFormat);
	vsprintf(tmp_buf, szFormat, args);
	va_end(args);

	FilterInvalidChars(tmp_buf);
	AddToChatWindowBuffer(CHAT_TYPE_DEBUG, tmp_buf, nullptr, m_dwDebugColor, 0);
}

void CChatWindow::AddClientMessage(uint32_t dwColor, char* szStr) {
	FilterInvalidChars(szStr);
	AddToChatWindowBuffer(CHAT_TYPE_INFO, szStr, nullptr, dwColor, 0);
}

void CChatWindow::PushBack(CHAT_WINDOW_ENTRY &entry) {
	if(m_ChatWindowEntries.size() >= m_iMaxMessages)
		m_ChatWindowEntries.pop_front();

	m_ChatWindowEntries.push_back(entry);
	return;
}

void CChatWindow::AddToChatWindowBuffer(eChatMessageType type, char* szString, char* szNick, uint32_t dwTextColor, uint32_t dwNickColor) {
	int iBestLineLength = 0;
	
	CHAT_WINDOW_ENTRY entry;
	entry.eType = type;
	entry.dwNickColor = dwNickColor | 0x000000FF;
	entry.dwTextColor = dwTextColor | 0x000000FF;

	if(szNick) {
		strcpy(entry.szNick, szNick);
		strcat(entry.szNick, ":");
	} else {
		entry.szNick[0] = '\0';
	}

	if(type == CHAT_TYPE_CHAT && strlen(szString) > MAX_LINE_LENGTH) {
		iBestLineLength = MAX_LINE_LENGTH;

		while(szString[iBestLineLength] != ' ' && iBestLineLength) {
			iBestLineLength--;
		}

		if((MAX_LINE_LENGTH - iBestLineLength) > 12) {
			cp1251_to_utf8(entry.utf8Message, szString, MAX_LINE_LENGTH);
			PushBack(entry);

			entry.szNick[0] = '\0';
			cp1251_to_utf8(entry.utf8Message, szString+MAX_LINE_LENGTH);
			PushBack(entry);
		} else {
			cp1251_to_utf8(entry.utf8Message, szString, iBestLineLength);
			PushBack(entry);

			entry.szNick[0] = '\0';
			cp1251_to_utf8(entry.utf8Message, szString+(iBestLineLength+1));
			PushBack(entry);
		}
	} else {
		cp1251_to_utf8(entry.utf8Message, szString, MAX_MESSAGE_LENGTH);
		PushBack(entry);
	}
	return;
}

void CChatWindow::FilterInvalidChars(char *szString)
{
	while(*szString) {
		if(*szString > 0 && *szString < ' ') {
			*szString = ' ';
		}
		szString++;
	}
}