#pragma once

#define INVALID_MENU_ROW_ID 255

class CMenuPool
{
private:
	CMenu *m_pMenus[MAX_MENUS];
	bool m_bMenuSlotState[MAX_MENUS];
	uint8_t m_byteCurrentMenu;
	uint8_t m_byteExited;

public:
	CMenuPool();
	~CMenuPool();

	CMenu* New(uint8_t byteMenuID, char* pTitle, float fX, float fY, uint8_t byteColumns, float fCol1Width, float fCol2Width, MENU_INT* MenuInteraction);
	
	bool Delete(uint8_t byteMenuID);
	
	// Retrieve a menu by id
	CMenu* GetAt(uint8_t byteMenuID)
	{
		if(byteMenuID > MAX_MENUS) 
			return nullptr;
		
		return m_pMenus[byteMenuID];
	};
	
	// Find out if the slot is inuse.
	bool GetSlotState(uint8_t byteMenuID)
	{
		if(byteMenuID > MAX_MENUS) 
			return false;
		
		return m_bMenuSlotState[byteMenuID];
	};
	
	CMenu* GetCurrentMenu();
	void ShowMenu(uint8_t byteMenuID);
	void HideMenu(uint8_t byteMenuID);

	char* GetTextPointer(char* szName, uint8_t* bRowId = 0, uint8_t* bColumnIndex = 0);
	
	uint8_t FindClosestRow(int x, int y);
	bool OnTouchEvent(int type, bool multi, int x, int y);
	void RenderDebug();
};