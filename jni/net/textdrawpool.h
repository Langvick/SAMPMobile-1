#pragma once

#define MAX_GLOBAL_TEXT_DRAWS 2048
#define MAX_PLAYER_TEXT_DRAWS 256
#define MAX_TEXT_DRAWS MAX_GLOBAL_TEXT_DRAWS + MAX_PLAYER_TEXT_DRAWS

#define INVALID_TEXT_DRAW_ID 65535

class CTextDrawPool
{	
public:
	CTextDrawPool();
	~CTextDrawPool();

	CTextDraw * New(int iTextId, TEXT_DRAW_TRANSMIT *TextDrawTransmit, char *szText);
	void Delete(int iTextId);
	void Draw();
	void RenderDebug();
	
	void ResetHover();
	void UpdateHover(int x, int y);
	uint16_t FindClosestTextDraw(int x, int y);
	void SetSelection(bool enable, uint32_t color);
	
	bool OnTouchEvent(int type, bool multi, int x, int y);
	
	bool IsValidTextDrawId(uint16_t textDrawId) {
		if(textDrawId >= 0 && textDrawId < MAX_TEXT_DRAWS) {
			return true;
		}
		return false;
	};
	
	bool GetSlotState(uint16_t textDrawId) {
		if(IsValidTextDrawId(textDrawId)) {
			return m_bSlotState[textDrawId];
		}
		return false;
	};
	
	CTextDraw * GetAt(int iTextId) {
		if (!GetSlotState(iTextId)) {
			return nullptr;
		}
		return m_pTextDraw[iTextId];
	};
	
	bool GetSelectionActive() {
		return m_bSelection;
	};
	
private:
	CTextDraw	*m_pTextDraw[MAX_TEXT_DRAWS];
	bool		m_bSlotState[MAX_TEXT_DRAWS];
	bool		m_bSelection;
	uint32_t	m_dwSelectionColor;
};