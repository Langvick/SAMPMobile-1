#include "main.h"
#include "chatwindow.h"
#include "dialog.h"
#include "game/game.h"
#include "gui/gui.h"
#include "netgame.h"

extern CNetGame* pNetGame;
extern CChatWindow* pChatWindow;
extern CDialogWindow *pDialogWindow;

char g_szMenuItems[MAX_MENU_ITEMS][MAX_COLUMNS][MAX_MENU_LINE] = 
{
	{"SAMP000", "SAMP100"},
	{"SAMP001", "SAMP101"},
	{"SAMP002", "SAMP102"},
	{"SAMP003", "SAMP103"},
	{"SAMP004", "SAMP104"},
	{"SAMP005", "SAMP105"},
	{"SAMP006", "SAMP106"},
	{"SAMP007", "SAMP107"},
	{"SAMP008", "SAMP108"},
	{"SAMP009", "SAMP109"},
	{"SAMP010", "SAMP110"},
	{"SAMP011", "SAMP111"},
};

CMenuPool::CMenuPool()
{
	// loop through and initialize all net players to null and slot states to false
	for (uint8_t byteMenuID = 0; byteMenuID < MAX_MENUS; byteMenuID++)
	{
		m_bMenuSlotState[byteMenuID] = false;
		m_pMenus[byteMenuID] = nullptr;
	}
	m_byteCurrentMenu = MAX_MENUS;
}

CMenuPool::~CMenuPool()
{	
	if (m_byteCurrentMenu != MAX_MENUS) 
		m_pMenus[m_byteCurrentMenu]->Hide();
	
	for (uint8_t byteMenuID = 0; byteMenuID < MAX_MENUS; byteMenuID++)
		if (m_pMenus[byteMenuID])
			delete m_pMenus[byteMenuID];
}

CMenu* CMenuPool::New(uint8_t byteMenuID, char* pTitle, float fX, float fY, uint8_t byteColumns, float fCol1Width, float fCol2Width, MENU_INT* MenuInteraction)
{
	if (m_pMenus[byteMenuID])
		delete m_pMenus[byteMenuID];
	
	m_bMenuSlotState[byteMenuID] = false;
	
	CMenu* pMenu = new CMenu(pTitle, fX, fY, byteColumns, fCol1Width, fCol2Width, MenuInteraction);
	
	if (pMenu)
	{
		m_bMenuSlotState[byteMenuID] = true;
		m_pMenus[byteMenuID] = pMenu;
		return pMenu;
	}
	return nullptr;
}

bool CMenuPool::Delete(uint8_t byteMenuID)
{
	if (m_bMenuSlotState[byteMenuID] == false || !m_pMenus[byteMenuID])
		return false;

	m_bMenuSlotState[byteMenuID] = false;
	delete m_pMenus[byteMenuID];

	return true;
}

CMenu* CMenuPool::GetCurrentMenu()
{
	if (m_byteCurrentMenu == MAX_MENUS) 
		return 0;
	
	return m_pMenus[m_byteCurrentMenu];
}

void CMenuPool::ShowMenu(uint8_t byteMenuID)
{
	if (byteMenuID >= MAX_MENUS) 
		return;
	
	if (m_bMenuSlotState[byteMenuID] == false || !m_pMenus[byteMenuID]) 
		return;

	if (m_byteCurrentMenu != MAX_MENUS) 
		m_pMenus[m_byteCurrentMenu]->Hide();
	
	m_pMenus[byteMenuID]->Show();
	m_byteCurrentMenu = byteMenuID;

	//m_byteSelected = 0;
	m_byteExited = 0;
}

void CMenuPool::HideMenu(uint8_t byteMenuID)
{
	if (byteMenuID >= MAX_MENUS || m_byteCurrentMenu == MAX_MENUS) 
		return;
	
	if (m_bMenuSlotState[byteMenuID] == false || !m_pMenus[byteMenuID]) 
		return;
	
	m_pMenus[byteMenuID]->Hide();
	m_byteCurrentMenu = MAX_MENUS;
}

char* CMenuPool::GetTextPointer(char* szName, uint8_t* bRowId, uint8_t* bColumnIndex)
{
	if (m_byteCurrentMenu == MAX_MENUS) 
		return 0;
	
	CMenu* pMenu = m_pMenus[m_byteCurrentMenu];
	
	if (!pMenu) 
		return 0;

	if (!strcmp(szName, "HED")) 
		return pMenu->GetMenuTitle();
	
	if (!strcmp(szName, "RW1")) 
		return pMenu->GetMenuHeader(0);
	
	if (!strcmp(szName, "RW2")) 
		return pMenu->GetMenuHeader(1);
	
	for (uint8_t i = 0; i < MAX_MENU_ITEMS; i++)
	{
		if (!strcmp(szName, g_szMenuItems[i][0] + 4))
		{
			if(bRowId && bColumnIndex)
			{
				*bRowId = i;
				*bColumnIndex = 0;
			}
			
			return pMenu->GetMenuItem(0, i);
		}
		
		if (!strcmp(szName, g_szMenuItems[i][1] + 4))
		{
			if(bRowId && bColumnIndex)
			{
				*bRowId = i;
				*bColumnIndex = 1;
			}
			
			return pMenu->GetMenuItem(1, i);
		}
	}
	return 0;
}

uint8_t CMenuPool::FindClosestRow(int x, int y) {
	uint8_t result = INVALID_MENU_ROW_ID;
	
	if (m_byteCurrentMenu == MAX_MENUS) 
		return result;
	
 	CMenu* pMenu = m_pMenus[m_byteCurrentMenu];
	
	if (!pMenu) 
		return result;
	
	for (uint8_t i = 0; i < MAX_MENU_ITEMS; i++)
	{
		char * menuItem = pMenu->GetMenuItem(0, i);
		if (menuItem && strlen(menuItem))
		{
			RECT rect = pMenu->m_MenuItemRect[0][i];
			if ((x >= (int)rect.X1 && x <= (int)rect.X2) && (y >= (int)rect.Y1 && y <= (int)rect.Y2)) 
			{
				result = i;
				break;
			}
		}
	}
	
	return result;
}

bool CMenuPool::OnTouchEvent(int type, bool multi, int x, int y)
{
	if (m_byteCurrentMenu == MAX_MENUS) 
		return true;
	
 	CMenu* pMenu = m_pMenus[m_byteCurrentMenu];
	
	if (!pMenu) 
		return true;
	
	uint8_t rowId = FindClosestRow(x, y);
	if(rowId == INVALID_MENU_ROW_ID) 
		return true;
	
	if(pDialogWindow && pDialogWindow->GetState()) {
		return true;
	}
	
	if(type == TOUCH_POP)
	{
		if(rowId != INVALID_MENU_ROW_ID) 
		{
			uint8_t selectedRowId = pMenu->GetSelectedRow();
			if (rowId == selectedRowId)
			{
				// PlaySelectSound
				(( void (*)())(g_libGTASA + 0x000258EB0 + 1))();
				
				pNetGame->SendMenuSelect(rowId);
				HideMenu(m_byteCurrentMenu);
			}
			else
			{ 
				// PlayNavSound
				(( void (*)())(g_libGTASA + 0x000258E88 + 1))();
				
				pMenu->SetSelectedRow(rowId);
			}
		}
	}
	return false;	
}

void CMenuPool::RenderDebug()
{
	if (m_byteCurrentMenu == MAX_MENUS) 
		return;
	
 	CMenu* pMenu = m_pMenus[m_byteCurrentMenu];
	
	if (!pMenu) 
		return;
	
	for (uint8_t i = 0; i < MAX_MENU_ITEMS; i++)
	{
		char * menuItem = pMenu->GetMenuItem(0, i);
		if (menuItem && strlen(menuItem))
		{
			RECT rect = pMenu->m_MenuItemRect[0][i];
			ImGui::GetOverlayDrawList()->AddRect(ImVec2(rect.X1, rect.Y1), ImVec2(rect.X2, rect.Y2), IM_COL32(0, 255, 0, 255));
		}
		
		menuItem = pMenu->GetMenuItem(1, i);
		if (menuItem && strlen(menuItem))
		{
			RECT rect = pMenu->m_MenuItemRect[1][i];
			ImGui::GetOverlayDrawList()->AddRect(ImVec2(rect.X1, rect.Y1), ImVec2(rect.X2, rect.Y2), IM_COL32(255, 255, 0, 255));
		}
	}
}