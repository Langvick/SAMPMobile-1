#include <iostream>
#include <algorithm>
#include <vector>
#include <string.h>
#include "../main.h"
#include "game.h"
#include "textdraw.h"
#include "font.h"

extern CGame *pGame;

CTextDraw::CTextDraw(TEXT_DRAW_TRANSMIT *pTextDrawTransmit, char *szText)
{
	if(!strlen(szText))
		return;
	
	SetText(szText);
	m_bHovered = false;
	pSprite2D = new CSprite2D();
	m_TextDrawTransmit = *pTextDrawTransmit;
}

void CTextDraw::SetText(char *szText)
{
	memset(m_szText, 0, sizeof(m_szText));
	strncpy(m_szText, szText, sizeof(m_szText));
	m_szText[MAX_TEXT_DRAW_LINE - 1] = 0;
}

void CTextDraw::SetTexture(char *txd_name, char *tex_name)
{
	for (int i = 0; i < strlen(tex_name); i++) {
		tex_name[i] = tolower(tex_name[i]);
	}
	
	uintptr_t pTexture = FindRwTexture(txd_name, tex_name);
	if(pTexture) {
		pSprite2D->m_pTexture = (RwTexture *)pTexture;
	}
}

void CTextDraw::Draw()
{
	int iScreenWidth = pGame->GetScreenWidth();
	int iScreenHeight = pGame->GetScreenHeight();
	float fHorizHudScale = 1.0f / 640.0f;
	float fVertHudScale = 1.0f / 448.0f;
	float fScaleX = (float)iScreenWidth * fHorizHudScale * m_TextDrawTransmit.fLetterWidth;
	float fScaleY = (float)iScreenHeight * fVertHudScale * m_TextDrawTransmit.fLetterHeight * 0.5f;
	float fUseX = iScreenWidth - ((640.0f - m_TextDrawTransmit.fX) * (iScreenWidth * fHorizHudScale));
	float fUseY = iScreenHeight - ((448.0f - m_TextDrawTransmit.fY) * (iScreenHeight * fVertHudScale));

	RECT rectClickableArea;
	CRGBA tdColor;

	if(m_bHovered) {
		tdColor.Set(m_dwSelectionColor);
	} else {
		tdColor.FromABGR(m_TextDrawTransmit.dwLetterColor);
	}
	
	switch(m_TextDrawTransmit.byteStyle) {
		case TEXTDRAW_FONT_GOTHIC:
		case TEXTDRAW_FONT_SUBTITLES:
		case TEXTDRAW_FONT_MENU:
		case TEXTDRAW_FONT_PRICEDOWN: {
			if(!m_szText) {
				return;
			}
			
			if(!strlen(m_szText)) {
				return;
			}
			
			const float fExtraScale[4] = { 0.94f, 1.2f, 1.0f, 1.05f };
			fScaleX *= fExtraScale[m_TextDrawTransmit.byteStyle];
		
			CFont::SetJustify(false);
			CFont::SetScaleXY(fScaleX, fScaleY);
			CFont::SetColor(tdColor); 
			
			if(m_TextDrawTransmit.byteRight) {
				CFont::SetOrientation(ALIGN_RIGHT);
			} else if(m_TextDrawTransmit.byteCenter) { 
				CFont::SetOrientation(ALIGN_CENTER);
			} else {
				CFont::SetOrientation(ALIGN_LEFT);
			}
			
			CFont::SetWrapx(iScreenWidth * fHorizHudScale * m_TextDrawTransmit.fLineWidth);

			CFont::SetCentreSize(iScreenWidth * fHorizHudScale * m_TextDrawTransmit.fLineHeight);
			
			CFont::SetBackground(m_TextDrawTransmit.byteBox, 0);
			
			CRGBA backGroungColor;
			backGroungColor.FromABGR(m_TextDrawTransmit.dwBoxColor);
			CFont::SetBackgroundColor(backGroungColor);
			
			if(m_TextDrawTransmit.byteOutline) {
				CFont::SetEdge(true);
			} else {
				CFont::SetDropShadowPosition(true);
			}
			
			CRGBA dropColor;
			dropColor.FromABGR(m_TextDrawTransmit.dwBackgroundColor);
			CFont::SetDropColor(dropColor);
			
			CFont::SetProportional(m_TextDrawTransmit.byteProportional);
			CFont::SetFontStyle(m_TextDrawTransmit.byteStyle);
			
			CFont::PrintString(fUseX,fUseY,m_szText);
			
			CFont::GetTextRect(&rectClickableArea, fUseX, fUseY);
			m_TextDrawRect = rectClickableArea;
			break;
		}
		case TEXTDRAW_TXD_SPRITE: {
			if(pSprite2D) {
				if(pSprite2D->m_pTexture) {
					// dont touch thiz code dude
					fScaleY = (float)iScreenHeight * fVertHudScale * m_TextDrawTransmit.fLineHeight;
					fScaleX = (float)iScreenWidth * fHorizHudScale * m_TextDrawTransmit.fLineWidth;
					
					pSprite2D->Draw(fUseX, fUseY, fScaleX, fScaleY, tdColor);
					rectClickableArea.X1 = fUseX;
					rectClickableArea.Y1 = fUseY;
					rectClickableArea.X2 = fUseX + fScaleX;
					rectClickableArea.Y2 = fUseY + fScaleY;
					m_TextDrawRect = rectClickableArea;
				}
			}
			break;
		}
		/* case TEXTDRAW_MODEL_PREVIEW:
		{
			if(pSprite2D) 
			{
				if(!pSprite2D->m_pTexture) 
				{
					pSprite2D->m_pTexture = CreateModelPreviewForVehicle(m_TextDrawTransmit.wModelID, CRGBA(255,255,255,255), m_TextDrawTransmit.vecRot, m_TextDrawTransmit.fZoom, m_TextDrawTransmit.wColor1, m_TextDrawTransmit.wColor2);
				}
				// dont touch thiz code dude
				fScaleY = (float)iScreenHeight * fVertHudScale * m_TextDrawTransmit.fLineHeight;
				fScaleX = (float)iScreenWidth * fHorizHudScale * m_TextDrawTransmit.fLineWidth;
				
				pSprite2D->Draw(fUseX, fUseY, fScaleX, fScaleY, tdColor);
				rectClickableArea.X1 = fUseX;
				rectClickableArea.Y1 = fUseY;
				rectClickableArea.X2 = fUseX + fScaleX;
				rectClickableArea.Y2 = fUseY + fScaleY;
				m_TextDrawRect = rectClickableArea;
			}
			break;
		} */
	}
}