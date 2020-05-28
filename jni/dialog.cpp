#include "main.h"
#include "dialog.h"

extern CGUI *pGUI;
extern CGame *pGame;
extern CNetGame *pNetGame;
extern CKeyBoard *pKeyBoard;
extern CChatWindow *pChatWindow;

char szDialogInputBuffer[2][128];

void CDialogWindow::Show(dialogData *inputDialog) {
	Clear();

	if(inputDialog) {
		currentDialogData = inputDialog;

		splittedTitle = splitText(std::string("FFFFFFFF"), inputDialog->szTitle);
		splittedMainText = splitText((inputDialog->ucDialogType == DIALOG_STYLE_MSGBOX) ? std::string("A9C4E4FF") : std::string("FFFFFFFF"), inputDialog->szMainText);
		mainTextLines = std::count(inputDialog->szMainText.begin(), inputDialog->szMainText.end(), '\n');

		if(pGame)
		{
			pGame->FindPlayerPed()->TogglePlayerControllable(false);
		}

		m_bIsActive = true;
	}
}

void CDialogWindow::Clear() {
	m_bIsActive = false;
	mainTextLines = 0;
	
	// Input Text, Without Password
	memset(szDialogInputBuffer[0], '\0', sizeof(char) * sizeof(szDialogInputBuffer[0]));
	memset(szDialogInputBuffer[1], '\0', sizeof(char) * sizeof(szDialogInputBuffer[1]));
	
	// Input Text, With Password
	memset(m_szPasswordInputText, '\0', sizeof(m_szPasswordInputText));
	
	if(pGame)
	{
		pGame->FindPlayerPed()->TogglePlayerControllable(true);
	}
	
	if(pNetGame) {
		if(pNetGame->GetTextDrawPool()->GetSelectionActive()) {
			if(pGame)
			{
				pGame->FindPlayerPed()->TogglePlayerControllable(false);
			}
		}
	}

	if(splittedTitle) {
		delete splittedTitle;
	}
	splittedTitle = nullptr;

	if(splittedMainText) {
		delete splittedMainText;
	}
	splittedMainText = nullptr;

	if(currentDialogData) {
		delete currentDialogData;
	}
	currentDialogData = nullptr;
	
	if(m_byteInputState) {
		m_byteInputState = 0;
	}
	m_byteInputState = 0;
}

void dialogueInputHandler(const char *szText) {
	if(!szText || *szText == '\0') {
		return;
	}
	
	strcpy(szDialogInputBuffer[0], szText);
	cp1251_to_utf8(szDialogInputBuffer[1], szText);
}

ImVec4 CDialogWindow::CovertToImGuiColor(ImU32 uiColor, bool bUseAlpha) {
	ImVec4 retnColor;

	retnColor.x = static_cast<float>((uiColor & 0xFF000000) >> 24) / 255.0f;
	retnColor.y = static_cast<float>((uiColor & 0x00FF0000) >> 16) / 255.0f;
	retnColor.z = static_cast<float>((uiColor & 0x0000FF00) >> 8) / 255.0f;
	
	if(bUseAlpha) {
		retnColor.w = static_cast<float>((uiColor & 0x000000FF)) / 255.0f;
	} else {
		retnColor.w = 1.0f;
	}
	
	return retnColor;
}

void CDialogWindow::PushTextWithColour(std::string szText, ImU32 uiColor) {
	int iLength = szText.size();
	if (iLength) {
		const char *pszText = szText.c_str();
		ImGui::PushStyleColor(ImGuiCol_Text, CovertToImGuiColor(uiColor, false));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextUnformatted(pszText, pszText + iLength);
		ImGui::PopStyleColor();
	}
	return;
}

void CDialogWindow::PushColouredText(splittedText *pSplittedText) {
	if(pSplittedText) {
		for(int i = pSplittedText->uiCount - 1; i >= 0; --i) {
			ImU32 uiTempColor = 0xFFFFFFFF;
			if(pSplittedText->splittedData[i].szColor.size()) {
				std::istringstream convertStringToU32(pSplittedText->splittedData[i].szColor);
				convertStringToU32 >> std::hex >> uiTempColor;
			}

			int iNewLinePos = pSplittedText->splittedData[i].szText.find("\n");
			if (iNewLinePos == std::string::npos) {
				PushTextWithColour(pSplittedText->splittedData[i].szText, uiTempColor);
			} else {
				PushTextWithColour(pSplittedText->splittedData[i].szText.substr(0, iNewLinePos), uiTempColor);
				ImGui::NewLine();

				std::vector<std::string> vecStringSplitted = splitLineByDelimiter(pSplittedText->splittedData[i].szText.substr(iNewLinePos + 1), '\n');
				for (int j = 0; j < vecStringSplitted.size(); ++j) {
					PushTextWithColour(vecStringSplitted.at(j), uiTempColor);
					if(j < vecStringSplitted.size() - 1) {
						ImGui::NewLine();
					}
				}
			}
		}
	}
}

void CDialogWindow::SwitchContextId(uint16_t usListCount) {
	if(lastPushedId != usListCount) {
		ImGui::PopID();
		ImGui::PushID(usListCount);
		lastPushedId = usListCount;
	}
}

void CDialogWindow::PushListItemButton(uint16_t usListCount) {
	SwitchContextId(usListCount);
	
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
	
	const char *szItem = std::string(std::to_string(usListCount)).c_str();
	if(ImGui::Selectable(szItem, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(pGUI->ScaleX(0), pGUI->ScaleY(30)))) {
		if(pNetGame) {
			pNetGame->SendDialogResponse(currentDialogData->usDialogId, 1, usListCount, "none");
			m_bIsActive = false;
		}
	}
	ImGui::PopStyleVar(1);
}

void CDialogWindow::PushColouredList(splittedText *pSplittedText) {
	if(pSplittedText) {
		uint16_t usListCount = 0;

		lastPushedId = 0;
		ImGui::PushID(lastPushedId);
		PushListItemButton(0);

		for(int i = pSplittedText->uiCount - 1; i >= 0; --i) {
			ImU32 uiTempColor = 0xFFFFFFFF;
			if(pSplittedText->splittedData[i].szColor.size()) {
				std::istringstream convertStringToU32(pSplittedText->splittedData[i].szColor);
				convertStringToU32 >> std::hex >> uiTempColor;
			}

			int iNewLinePos = pSplittedText->splittedData[i].szText.find("\n");
			if (iNewLinePos == std::string::npos) {
				PushTextWithColour(pSplittedText->splittedData[i].szText, uiTempColor);
			} else {
				PushTextWithColour(pSplittedText->splittedData[i].szText.substr(0, iNewLinePos), uiTempColor);
				ImGui::NewLine();
				PushListItemButton(++usListCount);
				
				uiTempColor = 0xFFFFFFFF;
				
				std::vector<std::string> vecStringSplitted = splitLineByDelimiter(pSplittedText->splittedData[i].szText.substr(iNewLinePos + 1), '\n');
				for (int j = 0; j < vecStringSplitted.size(); ++j) {
					PushTextWithColour(vecStringSplitted.at(j), uiTempColor);
					if(j < vecStringSplitted.size() - 1) {
						ImGui::NewLine();
						PushListItemButton(++usListCount);
					}
				}
			}
		}

		ImGui::PopID();
	}
}

void CDialogWindow::SetPasswordText(char* szText) {
	strcpy(m_szPasswordInputText, szText);
	cp1251_to_utf8(m_szPasswordInputText, szText);
}

void CDialogWindow::Render() {
	if(!m_bIsActive) {
		return;
	}
	
	if(!currentDialogData) {
		m_bIsActive = false;
		return;
	}

	ImGuiIO &imGuiIO = ImGui::GetIO();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(pGUI->ScaleX(8 * 2), pGUI->ScaleY(8 * 2)));
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

	// PushDialogueColors();

	ImGui::Begin("dialogWindow", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	PushColouredText(splittedTitle);
	if(currentDialogData->ucDialogType != DIALOG_STYLE_LIST) {
		ImGui::NewLine();
	}
	
	if(currentDialogData->ucDialogType == DIALOG_STYLE_INPUT) {
		m_byteInputState = 1;
	}
	
	if(currentDialogData->ucDialogType == DIALOG_STYLE_PASSWORD) {
		m_byteInputState = 2;
	}

	ImGui::ItemSize(ImVec2(0, (pGUI->GetFontSize() / 2) + 2.75f));

	switch(currentDialogData->ucDialogType) {
		case DIALOG_STYLE_MSGBOX: {
			if (mainTextLines <= SCROLL_BAR_ELEMENTS_MAX) {
				PushColouredText(splittedMainText);
				
				ImGui::ItemSize(ImVec2(0, (pGUI->GetFontSize() / 2) + pGUI->ScaleY(50 * 2)));
				ImGui::ItemSize(ImVec2(pGUI->ScaleX(200 * 2), 0));
			} else {
				ImGui::BeginChild("scrollArea1", ImVec2(pGUI->ScaleX(500 * 2), (ImGui::GetWindowHeight() / 2) + pGUI->ScaleY(80 * 2)), false, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_HorizontalScrollbar);
				
				PushColouredText(splittedMainText);
				
				ImGui::ItemSize(ImVec2(0, pGUI->GetFontSize() / 2 + pGUI->ScaleY(50 * 2)));
				ImGui::EndChild();
			}
			break;
		}

		case DIALOG_STYLE_INPUT: {
			if (mainTextLines <= SCROLL_BAR_ELEMENTS_MAX) {
				PushColouredText(splittedMainText);
				
				ImGui::ItemSize(ImVec2(0, (pGUI->GetFontSize() / 2) + pGUI->ScaleY(50 * 2)));
				ImGui::ItemSize(ImVec2(pGUI->ScaleX(200 * 2), 0));
			} else {
				ImGui::BeginChild("scrollArea2", ImVec2(pGUI->ScaleX(500 * 2), (ImGui::GetWindowHeight() / 2) + pGUI->ScaleY(11 * 2)), false, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_HorizontalScrollbar);
				
				PushColouredText(splittedMainText);
				
				ImGui::ItemSize(ImVec2(0, pGUI->GetFontSize() / 2 + pGUI->ScaleY(50 * 2)));
				ImGui::EndChild();
			}
			
			ImGui::ItemSize(ImVec2(0, (pGUI->GetFontSize() / 2) + pGUI->ScaleY(10 * 2)));
			if (ImGui::Button(szDialogInputBuffer[1], ImVec2(pGUI->ScaleX(500 * 2), pGUI->ScaleY(45 * 2)))) {
				if (!pKeyBoard->IsOpen()) {
					pKeyBoard->Open(&dialogueInputHandler);
				}
			}
			ImGui::ItemSize(ImVec2(0, pGUI->GetFontSize() / 2 + pGUI->ScaleY(10 * 2)));
			break;
		}
		
		case DIALOG_STYLE_PASSWORD: {
			if (mainTextLines <= SCROLL_BAR_ELEMENTS_MAX) {
				PushColouredText(splittedMainText);
				
				ImGui::ItemSize(ImVec2(0, (pGUI->GetFontSize() / 2) + pGUI->ScaleY(50 * 2)));
				ImGui::ItemSize(ImVec2(pGUI->ScaleX(200 * 2), 0));
			} else {
				ImGui::BeginChild("scrollArea3", ImVec2(pGUI->ScaleX(500 * 2), (ImGui::GetWindowHeight() / 2) + pGUI->ScaleY(11 * 2)), false, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_HorizontalScrollbar);
				
				PushColouredText(splittedMainText);
				
				ImGui::ItemSize(ImVec2(0, pGUI->GetFontSize() / 2 + pGUI->ScaleY(50 * 2)));
				ImGui::EndChild();
			}
			
			ImGui::ItemSize(ImVec2(0, (pGUI->GetFontSize() / 2) + pGUI->ScaleY(10 * 2)));
			if (ImGui::Button((char*)m_szPasswordInputText, ImVec2(pGUI->ScaleX(500 * 2), pGUI->ScaleY(45 * 2)))) {
				if (!pKeyBoard->IsOpen()) {
					pKeyBoard->Open(&dialogueInputHandler);
				}
			}
			ImGui::ItemSize(ImVec2(0, pGUI->GetFontSize() / 2 + pGUI->ScaleY(10 * 2)));
			break;
		}

		case DIALOG_STYLE_TABLIST_HEADERS:
		case DIALOG_STYLE_TABLIST:
		case DIALOG_STYLE_LIST: {
			ImGui::BeginChild("scrollArea4", ImVec2(pGUI->ScaleX(500 * 2), ImGui::GetWindowHeight() / 2 + pGUI->ScaleY(66 * 2)), false, ImGuiWindowFlags_NoSavedSettings);
			PushColouredList(splittedMainText);
			ImGui::EndChild();
			break;
		}
	}

	ImGui::Dummy(ImVec2(0.0f, pGUI->ScaleY(5 * 2)));

	if (currentDialogData->szRightButton.size() > 0) {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - pGUI->ScaleX(270 * 2 + 1) + ImGui::GetStyle().ItemSpacing.x) / 2);
	} else {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - pGUI->ScaleX(125 * 2 + 1) / 2);
	}

	ImGui::PopStyleVar(2);
	
	if(currentDialogData->szLeftButton.size() > 0) {
		if(ImGui::Button(currentDialogData->szLeftButton.c_str(), ImVec2(pGUI->ScaleX(125 * 2), pGUI->ScaleY(50 * 2)))) {
			if(pNetGame) {
				pNetGame->SendDialogResponse(currentDialogData->usDialogId, 1, -1, szDialogInputBuffer[0]);
				m_bIsActive = false;
			}
		}
	}

	ImGui::SameLine(0, pGUI->GetFontSize());

	if(currentDialogData->szRightButton.size() > 0) {
		if(ImGui::Button(currentDialogData->szRightButton.c_str(), ImVec2(pGUI->ScaleX(125 * 2), pGUI->ScaleY(50 * 2)))) {
			if(pNetGame) {
				pNetGame->SendDialogResponse(currentDialogData->usDialogId, 0, -1, szDialogInputBuffer[0]);
				m_bIsActive = false;
			}
		}
	}

	ImGui::SetWindowSize(ImVec2(-1, -1));

	ImVec2 imGuiWindowSize = ImGui::GetWindowSize();
	ImGui::SetWindowPos(ImVec2(((imGuiIO.DisplaySize.x - imGuiWindowSize.x) / 2), ((imGuiIO.DisplaySize.y - imGuiWindowSize.y) / 2)));

	ImGui::End();

	// PopDialogueColors();
	
	if(!m_bIsActive) {
		Clear();
	}
};