#pragma once

#include "CSprite2D.h"
#include "RW/RenderWare.h"

typedef struct _TEXT_DRAW_TRANSMIT
{
	union
	{
		uint8_t byteFlags;
		struct
		{
			uint8_t byteBox : 1;
			uint8_t byteLeft : 1;
			uint8_t byteRight : 1;
			uint8_t byteCenter : 1;
			uint8_t byteProportional : 1;
			uint8_t bytePadding : 3;
		};
	};
	float fLetterWidth;
	float fLetterHeight;
	uint32_t dwLetterColor;
	float fLineWidth;
	float fLineHeight;
	uint32_t dwBoxColor;
	uint8_t byteShadow;
	uint8_t byteOutline;
	uint32_t dwBackgroundColor;
	uint8_t byteStyle;
	uint8_t byteSelectable;
	float fX;
	float fY;
	uint16_t wModelID;
	VECTOR vecRot;
	float fZoom;
	uint16_t wColor1;
	uint16_t wColor2;
} TEXT_DRAW_TRANSMIT;

enum eFontAlignment : unsigned char {
    ALIGN_CENTER,
    ALIGN_LEFT,
    ALIGN_RIGHT
};

enum eTextDrawStyle : unsigned char {
    TEXTDRAW_FONT_GOTHIC,
    TEXTDRAW_FONT_SUBTITLES,
    TEXTDRAW_FONT_MENU,
    TEXTDRAW_FONT_PRICEDOWN,
	TEXTDRAW_TXD_SPRITE,
	TEXTDRAW_MODEL_PREVIEW
};

class CTextDraw
{
public:
	char				m_szText[MAX_TEXT_DRAW_LINE];
	TEXT_DRAW_TRANSMIT	m_TextDrawTransmit;
	RECT				m_TextDrawRect;
	bool				m_bHovered;
	uint32_t			m_dwSelectionColor;

	CTextDraw(TEXT_DRAW_TRANSMIT *pTextDrawTransmit, char *szText);
	~CTextDraw() { pSprite2D->Delete(); };

	void SetText(char *szText);
	void SetTexture(char *txd_name, char *tex_name);
	void Draw();
	
private:
	CSprite2D *pSprite2D;
};