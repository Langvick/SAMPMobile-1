#include "main.h"
#include "gui/gui.h"
#include "game/game.h"
#include "net/netgame.h"
#include "settings.h"
#include "dialog.h"
#include "spawnscreen.h"
#include "extrakeyboard.h"
#include "playerslist.h"

extern CGUI *pGUI;
extern CGame *pGame;
extern CNetGame *pNetGame;
extern CSettings *pSettings;
extern CPlayersList *pPlayersList;
extern CDialogWindow *pDialogWindow;
extern CSpawnScreen *pSpawnScreen;

CExtraKeyBoard::CExtraKeyBoard() {
	m_bIsActive = false;
	m_bIsItemShow = false;
	m_tabactive = false;
}

CExtraKeyBoard::~CExtraKeyBoard() { }

void CExtraKeyBoard::Show(bool bShow) {
	m_bIsActive = bShow;
}

void CExtraKeyBoard::Render() {
	if(!m_bIsActive || pSpawnScreen->GetState() || pDialogWindow->GetState()) {
		return;
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
	
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		ImGuiIO &io = ImGui::GetIO();

		ImGui::GetStyle().ButtonTextAlign = ImVec2(0.5f, 0.5f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

		ImGui::Begin("Neckeys", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

		m_fButWidth = ImGui::CalcTextSize("QWERTYU").x;
		m_fButHeight = ImGui::CalcTextSize("QWERT").x;
			
		if(m_bIsItemShow) {
	        if (ImGui::Button("TAB", ImVec2(m_fButWidth, m_fButHeight)))
	        {
	        	if(!m_tabactive)
	        	{
	        		pPlayersList->Show(true);
	        		m_tabactive = true;
	        	}
	        	else
	        	{
	        		pPlayersList->Show(false);
	        		m_tabactive = false;	
	        	}	
	            
	        }
	        ImGui::SameLine(0, 3);

			if (ImGui::Button("F", ImVec2(m_fButWidth, m_fButHeight))) {
				LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = true;
			}
			ImGui::SameLine(0, 3);
			
			if (ImGui::Button("H", ImVec2(m_fButWidth, m_fButHeight))) {
				LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;
			}
			ImGui::SameLine(0, 3);
		
			if (ImGui::Button("Y", ImVec2(m_fButWidth, m_fButHeight))) {
				LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = true;
			}
			ImGui::SameLine(0, 3);
		
			if (ImGui::Button("N", ImVec2(m_fButWidth, m_fButHeight))) {
				LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;
			}
			ImGui::SameLine(0, 3);

			if(pPlayerPed->IsInVehicle()) {
				if (ImGui::Button("ALT", ImVec2(m_fButWidth, m_fButHeight))) {
					LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = true;
				}
				ImGui::SameLine(0, 3);
				
				if(!pPlayerPed->IsAPassenger()) {
					if (ImGui::Button("2", ImVec2(m_fButWidth, m_fButHeight))) {
						LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION] = true;
					}
				}
			} else {
				if (ImGui::Button("ALT", ImVec2(m_fButWidth, m_fButHeight))) {
					LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
				}
				ImGui::SameLine(0, 3);
		
				if (ImGui::Button("ENTER", ImVec2(m_fButWidth, m_fButHeight))) {
					LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = true;
				}
				ImGui::SameLine(0, 3);
				
				CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
				if(pVehiclePool) {
					uint16_t sNearestVehicleID = pVehiclePool->FindNearestToLocalPlayerPed();

					CVehicle *pVehicle = pVehiclePool->GetAt(sNearestVehicleID);
					if(pVehicle) {
						if(pVehicle->GetDistanceFromLocalPlayerPed() < 4.0f) {
							CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
							if(pPlayerPool) {
								CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
								if(pLocalPlayer) {
									if(!pLocalPlayer->IsSpectating()) {
										if(ImGui::Button("G", ImVec2(m_fButWidth, m_fButHeight))) {
											pPlayerPed->EnterVehicle(pVehicle->m_dwGTAId, true);
											pLocalPlayer->SendEnterVehicleNotification(sNearestVehicleID, true);
										}
									}
								}
							}
						}
					}
				}
			}
			ImGui::SameLine(0, 3);
			
			if (ImGui::Button("<<", ImVec2(m_fButWidth, m_fButHeight))) {
				m_bIsItemShow = false;
			}
		}else{
			if (ImGui::Button(">>", ImVec2(m_fButWidth, m_fButHeight))) {
				m_bIsItemShow = true;
			}
		}
	
		ImGui::SetWindowSize(ImVec2(-1, -1));

		ImVec2 size = ImGui::GetWindowSize();
		ImGui::SetWindowPos(ImVec2(pGUI->ScaleX(10), pGUI->ScaleY(345)));
		ImGui::End();
	}
	return;
}