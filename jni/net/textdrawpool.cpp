#include "../main.h"
#include "../chatwindow.h"
#include "../dialog.h"
#include "../gui/gui.h"
#include "../game/game.h"
#include "../game/textdraw.h"
#include "textdrawpool.h"
#include "netgame.h"

extern CGame* pGame;
extern CChatWindow *pChatWindow;
extern CDialogWindow *pDialogWindow;
extern CNetGame *pNetGame;

CTextDrawPool::CTextDrawPool()
{
	for(int iTextId = 0; iTextId < MAX_TEXT_DRAWS; ++iTextId) {
		m_pTextDraw[iTextId] = nullptr;
		m_bSlotState[iTextId] = false;
	}
	
	m_bSelection = false;
	m_dwSelectionColor = -1;
}

CTextDrawPool::~CTextDrawPool()
{
	for(int iTextId = 0; iTextId < MAX_TEXT_DRAWS; ++iTextId) {
		Delete(iTextId);
	}
}

CTextDraw* CTextDrawPool::New(int iTextId, TEXT_DRAW_TRANSMIT *TextDrawTransmit, char *szText) {
	if(!IsValidTextDrawId(iTextId)) {
		return nullptr;
	}
	
	Delete(iTextId);

	m_pTextDraw[iTextId] = new CTextDraw(TextDrawTransmit, szText);
	if(m_pTextDraw[iTextId]) {
		if(TextDrawTransmit->byteStyle == TEXTDRAW_TXD_SPRITE) {
			char *szTxdName, *szTexName;
			szTxdName = strtok(szText, ":");
			if(szTxdName) {
				szTexName = szTxdName;
				szTexName = strtok(0, ":");
				if(szTexName) {
					m_pTextDraw[iTextId]->SetTexture(szTxdName, szTexName);
				}
			}
				 
		}

		m_bSlotState[iTextId] = true;
		return m_pTextDraw[iTextId];
	}

	Log("Failed to create TextDraw (%i)", iTextId);
	return nullptr;
}

void CTextDrawPool::Delete(int iTextId) {
	if(!IsValidTextDrawId(iTextId)) {
		return;
	}
	
	m_bSlotState[iTextId] = false;
	
	if(m_pTextDraw[iTextId] != nullptr) {
		delete m_pTextDraw[iTextId];
		m_pTextDraw[iTextId] = nullptr;
	}
}

void CTextDrawPool::Draw() {
	for(int iTextId = 0; iTextId < MAX_TEXT_DRAWS; ++iTextId) {
		if(m_bSlotState[iTextId]) {
			if(m_pTextDraw[iTextId]) {
				m_pTextDraw[iTextId]->Draw();
			} else {
				Log("Failed to draw TextDraw (%i)", iTextId);
				Delete(iTextId);
			}
		}
	}
}

void CTextDrawPool::SetSelection(bool enable, uint32_t color)
{
	m_bSelection = enable;
	m_dwSelectionColor = color;
	
	if(pGame) {
		pGame->FindPlayerPed()->TogglePlayerControllable(!enable);
	}
}

void CTextDrawPool::UpdateHover(int x, int y)
{
	for(int iTextId = 0; iTextId < MAX_TEXT_DRAWS; ++iTextId)  {
		if(!m_bSlotState[iTextId]) {
			continue;
		}
		
		if(!m_pTextDraw[iTextId]) {
			continue;
		}
		
		if(!m_pTextDraw[iTextId]->m_TextDrawTransmit.byteSelectable) {
			continue;
		}
		
		if ((x >= (int)m_pTextDraw[iTextId]->m_TextDrawRect.X1 && x <= (int)m_pTextDraw[iTextId]->m_TextDrawRect.X2) && (y >= (int)m_pTextDraw[iTextId]->m_TextDrawRect.Y1 && y <= (int)m_pTextDraw[iTextId]->m_TextDrawRect.Y2)) {
			m_pTextDraw[iTextId]->m_bHovered = true;
			m_pTextDraw[iTextId]->m_dwSelectionColor = m_dwSelectionColor;
		} else {
			m_pTextDraw[iTextId]->m_bHovered = false;
		}
	}	
}

uint16_t CTextDrawPool::FindClosestTextDraw(int x, int y) {
	uint16_t result = INVALID_TEXT_DRAW_ID;
	
	for(int iTextId = 0; iTextId < MAX_TEXT_DRAWS; ++iTextId) {
		if(!m_bSlotState[iTextId]) {
			continue;
		}
		
		if(!m_pTextDraw[iTextId]) {
			continue;
		}
		
		if(!m_pTextDraw[iTextId]->m_bHovered) {
			continue;
		}
		
		result = iTextId;
		break;
	}	
	
	return result;
}

void CTextDrawPool::ResetHover()
{
	for(int iTextId = 0; iTextId < MAX_TEXT_DRAWS; ++iTextId) {
		if(!m_bSlotState[iTextId]) {
			continue;
		}
		
		if(!m_pTextDraw[iTextId]) {
			continue;
		}
		
		m_pTextDraw[iTextId]->m_bHovered = false;
	}	
}

bool CTextDrawPool::OnTouchEvent(int type, bool multi, int x, int y)
{
	if(!m_bSelection) {
		return true;
	}
	
	if(pDialogWindow && pDialogWindow->GetState()) {
		return true;
	}
	
	switch(type)
	{
		case TOUCH_PUSH:
		case TOUCH_MOVE: {
			UpdateHover(x, y);
			break;
		}

		case TOUCH_POP: {
			uint16_t tdId = FindClosestTextDraw(x, y);
			if(tdId != INVALID_TEXT_DRAW_ID)
			{
				pNetGame->SendClickTextDraw(tdId);
				ResetHover();
				return true;
			}
			ResetHover();
			break;
		}
		
		default: break;
	}

	return true;	
}

void CTextDrawPool::RenderDebug()
{
	if(!m_bSelection) {
		return;
	}

	for(int iTextId = 0; iTextId < MAX_TEXT_DRAWS; ++iTextId) {
		if(!m_bSlotState[iTextId]) {
			continue;
		}

		if(!m_pTextDraw[iTextId]) {
			continue;
		}

		if(!m_pTextDraw[iTextId]->m_TextDrawTransmit.byteSelectable) {
			continue;
		}

		ImGui::GetOverlayDrawList()->AddRect(
			ImVec2(m_pTextDraw[iTextId]->m_TextDrawRect.X1, m_pTextDraw[iTextId]->m_TextDrawRect.Y1),
			ImVec2(m_pTextDraw[iTextId]->m_TextDrawRect.X2, m_pTextDraw[iTextId]->m_TextDrawRect.Y2),
			IM_COL32(0, 255, 0, 255)
		);
	}	
}
