#include "main.h"
#include "gui.h"
#include "game/game.h"
#include "net/netgame.h"
#include "game/RW/RenderWare.h"
#include "util/util.h"
#include "chatwindow.h"
#include "spawnscreen.h"
#include "playertags.h"
#include "../playerslist.h"
#include "dialog.h"
#include "keyboard.h"
#include "extrakeyboard.h"
#include "settings.h"

#include <sstream>
#include <iomanip>

extern CChatWindow *pChatWindow;
extern CSpawnScreen *pSpawnScreen;
extern CPlayerTags *pPlayerTags;
extern CDialogWindow *pDialogWindow;
extern CPlayersList *pPlayersList;
extern CSettings *pSettings;
extern CKeyBoard *pKeyBoard;
extern CExtraKeyBoard *pExtraKeyBoard;
extern CNetGame *pNetGame;
extern CGame *pGame;
extern CTextDrawPool *pTextDrawPool;
extern CGUI *pGUI;

const auto FAndG = cryptor::create("", 24);

void ImGui_ImplRenderWare_RenderDrawData(ImDrawData* draw_data);
bool ImGui_ImplRenderWare_Init();
void ImGui_ImplRenderWare_NewFrame();
void ImGui_ImplRenderWare_ShutDown();

CGUI::CGUI()
{
	Log("Initializing GUI..");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	ImGui_ImplRenderWare_Init();

	m_vecScale.x = io.DisplaySize.x * 0.00052083333f; // 1 / 1920
	m_vecScale.y = io.DisplaySize.y * 0.00092592592f; // 1 / 1080

	m_fFontSize = ScaleY(pSettings->Get().fFontSize);

	m_bMousePressed = false;
	m_vecMousePos = ImVec2(0, 0);

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScrollbarSize = ScaleY(55.0f);
	style.WindowBorderSize = 0.0f;
	ImGui::StyleColorsDark();

	char path[0xFF];
	sprintf(path, "%sSAMP/fonts/%s", g_pszStorage, pSettings->Get().szFont);

	static const ImWchar ranges[] = {
		0x0020, 0x0080,
		0x00A0, 0x00C0,
		0x0400, 0x0460,
		0x0490, 0x04A0,
		0x2010, 0x2040,
		0x20A0, 0x20B0,
		0x2110, 0x2130,
		0
	};

	m_pFont = io.Fonts->AddFontFromFileTTF(path, m_fFontSize, nullptr, ranges);
}

CGUI::~CGUI()
{
	ImGui_ImplRenderWare_ShutDown();
	ImGui::DestroyContext();
}

void CGUI::Render()
{	
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplRenderWare_NewFrame();
	ImGui::NewFrame();
	
	ImVec2 vecPos = ImVec2(ScaleX(5), ScaleY(5));
	RenderText(vecPos, 0xFFFFFFFF, true, FAndG.decrypt());
	
	if(pPlayerTags) {
		pPlayerTags->Render();
	}

	if(pChatWindow) {
		pChatWindow->Render();
	}

	if(pDialogWindow) {
		pDialogWindow->Render();
	}

	if(pPlayersList) {
		pPlayersList->Render();
	}

	if(pSpawnScreen) {
		pSpawnScreen->Render();
	}

	if(pKeyBoard) {
		pKeyBoard->Render();
	}

	if(pExtraKeyBoard) {
		pExtraKeyBoard->Render();
	}

	if(pNetGame) {
		CTextDrawPool *pTextDrawPool = pNetGame->GetTextDrawPool();
		if(pTextDrawPool) {
			// pTextDrawPool->RenderDebug();
		}
			
		CMenuPool *pMenuPool = pNetGame->GetMenuPool();
		if(pMenuPool) {
			// pMenuPool->RenderDebug();
		}
		
		CLabelPool *pLabelPool = pNetGame->GetLabelPool();
		if(pLabelPool) {
			pLabelPool->Draw();
		}
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplRenderWare_RenderDrawData(ImGui::GetDrawData());

	if(m_bNeedClearMousePos) {
		io.MousePos = ImVec2(-1, -1);
		m_bNeedClearMousePos = false;
	}
}

bool CGUI::OnTouchEvent(int type, bool multi, int x, int y)
{
	if(pGame->IsMenuActive()) {
		return true;
	}
	
	ImGuiIO& io = ImGui::GetIO();

	if(!pKeyBoard->OnTouchEvent(type, multi, x, y)) {
		return false;
	}

	if(!pChatWindow->OnTouchEvent(type, multi, x, y)) {
		return false;
	}
	
	if(pNetGame && pNetGame->GetMenuPool() && !pNetGame->GetMenuPool()->OnTouchEvent(type, multi, x, y)) {
		return false;
	}
	
	if(pNetGame && pNetGame->GetTextDrawPool() && !pNetGame->GetTextDrawPool()->OnTouchEvent(type, multi, x, y)) {
		return false;
	}

	switch(type) {
		case TOUCH_PUSH: {
			io.MousePos = ImVec2(x, y);
			io.MouseDown[0] = true;
			break;
		}
		
		case TOUCH_POP: {
			io.MouseDown[0] = false;
			m_bNeedClearMousePos = true;
			break;
		}
		
		case TOUCH_MOVE: {
			io.MousePos = ImVec2(x, y);
			break;
		}
	}

	return true;
}

void CGUI::RenderText(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end)
{
	int iOffset = pSettings->Get().iFontOutline;
	if(bOutline) {
		// left
		posCur.x -= iOffset;
		ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.x += iOffset;
		
		// right 
		posCur.x += iOffset;
		ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.x -= iOffset;
		
		// above
		posCur.y -= iOffset;
		ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.y += iOffset;
		
		// below
		posCur.y += iOffset;
		ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.y -= iOffset;
	}
	ImGui::GetBackgroundDrawList()->AddText(posCur, col, text_begin, text_end);
}

void CGUI::Render2DColouredText(ImVec2& vecOrigPos, bool bOutline, splittedText *pSplittedText) {
	if(pSplittedText) {
		ImVec2 vecTempPos = vecOrigPos;
		for(int i = pSplittedText->uiCount - 1; i >= 0; --i) {
			ImU32 uiTempColor = 0xFFFFFFFF;
			if(pSplittedText->splittedData[i].szColor.size()) {
				std::istringstream convertStringToU32(pSplittedText->splittedData[i].szColor);
				convertStringToU32 >> std::hex >> uiTempColor;
				uiTempColor = __builtin_bswap32(uiTempColor);
			}

			int iNewLinePos = pSplittedText->splittedData[i].szText.find("\n");
			if (iNewLinePos == std::string::npos) {
				int iLength = pSplittedText->splittedData[i].szText.size();
				if (iLength) {
					const char *szText = pSplittedText->splittedData[i].szText.c_str();
					RenderText(
						vecTempPos,
						uiTempColor,
						bOutline,
						szText,
						szText + iLength
					);

					vecTempPos.x += ImGui::CalcTextSize(szText).x;
				}
			} else {
				std::string szOldLine = pSplittedText->splittedData[i].szText.substr(0, iNewLinePos);

				int iLength = szOldLine.size();
				if (iLength) {
					const char *szText = szOldLine.c_str();
					RenderText(
						vecTempPos,
						uiTempColor,
						bOutline,
						szText,
						szText + iLength
					);
				}

				vecTempPos.x = vecOrigPos.x;
				vecTempPos.y += GetFontSize();

				std::string szNewLine = pSplittedText->splittedData[i].szText.substr(iNewLinePos + 1);
				if (szNewLine.size()) {
					std::vector<std::string> vecStringSplitted = splitLineByDelimiter(szNewLine, '\n');
					for (int j = 0; j < vecStringSplitted.size(); ++j) {
						if(j) {
							vecTempPos.x = vecOrigPos.x;
							vecTempPos.y += GetFontSize();
						}

						std::string szSplittedText = vecStringSplitted.at(j);
						int iLength = szSplittedText.size();
						if(iLength) {
							const char *szText = szSplittedText.c_str();
							RenderText(
								vecTempPos,
								uiTempColor,
								bOutline,
								szText,
								szText + iLength
							);

							vecTempPos.x = vecOrigPos.x + ImGui::CalcTextSize(szText).x;
						}
					}
				}
			}
		}
	}
	return;
}

void CGUI::Render2DColouredText(ImVec2& vecOrigPos, ImU32 uiColor, bool bOutline, std::string szText) {
	ImVec2 vecTempPos = vecOrigPos;

	std::stringstream convertU32ToHex;
	convertU32ToHex << std::hex << uiColor;
	std::string szColor = convertU32ToHex.str();

	splittedText *pSplittedText = splitText(szColor, szText);
	if(pSplittedText) {
		ImVec2 vecTempPos = vecOrigPos;
		for(int i = pSplittedText->uiCount - 1; i >= 0; --i) {
			ImU32 uiTempColor = 0xFFFFFFFF;
			if(pSplittedText->splittedData[i].szColor.size()) {
				std::istringstream convertStringToU32(pSplittedText->splittedData[i].szColor);
				convertStringToU32 >> std::hex >> uiTempColor;
				uiTempColor = __builtin_bswap32(uiTempColor);
			}

			int iNewLinePos = pSplittedText->splittedData[i].szText.find("\n");
			if (iNewLinePos == std::string::npos) {
				int iLength = pSplittedText->splittedData[i].szText.size();
				if (iLength) {
					const char *szText = pSplittedText->splittedData[i].szText.c_str();
					RenderText(
						vecTempPos,
						uiTempColor,
						bOutline,
						szText,
						szText + iLength
					);

					vecTempPos.x += ImGui::CalcTextSize(szText).x;
				}
			} else {
				std::string szOldLine = pSplittedText->splittedData[i].szText.substr(0, iNewLinePos);

				int iLength = szOldLine.size();
				if (iLength) {
					const char *szText = szOldLine.c_str();
					RenderText(
						vecTempPos,
						uiTempColor,
						bOutline,
						szText,
						szText + iLength
					);
				}

				vecTempPos.x = vecOrigPos.x;
				vecTempPos.y += GetFontSize();

				std::string szNewLine = pSplittedText->splittedData[i].szText.substr(iNewLinePos + 1);
				if (szNewLine.size()) {
					std::vector<std::string> vecStringSplitted = splitLineByDelimiter(szNewLine, '\n');
					for (int j = 0; j < vecStringSplitted.size(); ++j) {
						if(j) {
							vecTempPos.x = vecOrigPos.x;
							vecTempPos.y += GetFontSize();
						}

						std::string szSplittedText = vecStringSplitted.at(j);
						int iLength = szSplittedText.size();
						if(iLength) {
							const char *szText = szSplittedText.c_str();
							RenderText(
								vecTempPos,
								uiTempColor,
								bOutline,
								szText,
								szText + iLength
							);

							vecTempPos.x = vecOrigPos.x + ImGui::CalcTextSize(szText).x;
						}
					}
				}
			}
		}
	}
	return;
}
