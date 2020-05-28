#pragma once

#define MAX_MENU_ITEMS 12
#define MAX_MENU_LINE 32
#define MAX_COLUMNS 2

struct MENU_INT
{
	int iMenu;
	int iRow[MAX_MENU_ITEMS];
	int iPadding[8 - ((MAX_MENU_ITEMS + 1) % 8)]; 
};

class CMenu
{
private:
	uint8_t m_byteMenuID;
	
	char m_charTitle[MAX_MENU_LINE];
	char m_charItems[MAX_MENU_ITEMS][MAX_COLUMNS][MAX_MENU_LINE];
	char m_charHeader[MAX_COLUMNS][MAX_MENU_LINE];
 
	float m_fXPos;
	float m_fYPos;
	float m_fCol1Width;
	float m_fCol2Width;
	uint8_t m_byteColumns;
	MENU_INT m_MenuInteraction;
	//float fHeight
	
	uint8_t m_byteColCount[MAX_COLUMNS];
	
	uint32_t m_dwPanel;
	
public:
	CMenu(char* pTitle, float fX, float fY, uint8_t byteColumns, float fCol1Width, float fCol2Width, MENU_INT* pMenuInteraction);
	~CMenu(){};
	
	void AddMenuItem(uint8_t byteColumn, uint8_t byteRow, char* pText);
	void SetColumnTitle(uint8_t byteColumn, char* pText);
	void Show();
	void Hide();
	char* GetMenuItem(uint8_t byteColumn, uint8_t byteRow);
	char* GetMenuTitle();
	char* GetMenuHeader(uint8_t byteColumn);
	char* MS(uint8_t byteColumn, uint8_t byteRow);
	uint8_t GetSelectedRow();
	void SetSelectedRow(uint8_t rowId);
	
	RECT m_MenuItemRect[MAX_MENU_ITEMS][MAX_COLUMNS];
};