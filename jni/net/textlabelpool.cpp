#include "main.h"
#include "gui/gui.h"
#include "game/game.h"
#include "chatwindow.h"
#include "netgame.h"

#include <sstream>
#include <iomanip>

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
extern CGUI *pGUI;

CLabelPool::CLabelPool()
{
	for(int x = 0; x < MAX_TEXT_LABELS + MAX_PLAYER_TEXT_LABELS + 2; x++) {
		m_pTextLabels[x] = nullptr;
		m_bSlotState[x] = false;
	}
}

CLabelPool::~CLabelPool()
{
	for (int x = 0; x < MAX_TEXT_LABELS + MAX_PLAYER_TEXT_LABELS + 2; x++) {
		Delete(x);
	}
}

void CLabelPool::CreateTextLabel(int labelId, char *szText, uint32_t uiColor, VECTOR vecPosition, float fDrawDistance, bool bDrawLOS, uint16_t usAttachedPlayerId, uint16_t usAttachedVehicleId)
{
	Delete(labelId);

	textLabelData* pTextLabel = new textLabelData;
	if (pTextLabel) {
		cp1251_to_utf8(pTextLabel->szText, szText);
		pTextLabel->uiColor = uiColor;
		
		std::stringstream convertU32ToHex;
		convertU32ToHex << std::hex << uiColor;
		std::string szColor = convertU32ToHex.str();
		pTextLabel->splittedMainText = splitText(szColor, pTextLabel->szText);

		pTextLabel->vecPosition = vecPosition;
		pTextLabel->fDrawDistance = fDrawDistance;
		pTextLabel->bDrawLOS = bDrawLOS;
		pTextLabel->usAttachedPlayerId = usAttachedPlayerId;
		pTextLabel->usAttachedVehicleId = usAttachedVehicleId;

		if (usAttachedPlayerId >= 0 && usAttachedPlayerId < MAX_PLAYERS) {
			pTextLabel->usAttachedPlayerId = usAttachedPlayerId;
		} else if(usAttachedVehicleId >= 0 && usAttachedVehicleId < MAX_VEHICLES) {
			pTextLabel->usAttachedVehicleId = usAttachedVehicleId;
		}

		m_pTextLabels[labelId] = pTextLabel;
		m_bSlotState[labelId] = true;
	}
}

void CLabelPool::Delete(int labelId)
{
	m_bSlotState[labelId] = false;
	
	if (m_pTextLabels[labelId]) {
		if(m_pTextLabels[labelId]->splittedMainText) {
			delete m_pTextLabels[labelId]->splittedMainText;
			m_pTextLabels[labelId]->splittedMainText = nullptr;
		}
		
		delete m_pTextLabels[labelId];
		m_pTextLabels[labelId] = nullptr;
	}
}

void CLabelPool::Draw()
{
	bool bLOS = false;

	for (int x = 0; x < MAX_TEXT_LABELS + MAX_PLAYER_TEXT_LABELS + 2; x++) {
		if (x == INVALID_TEXT_LABEL) {
			continue;
		}
		
		if (m_bSlotState[x]) {
			VECTOR vecPosition = m_pTextLabels[x]->vecPosition;
			if(m_pTextLabels[x]->usAttachedPlayerId >= 0 && m_pTextLabels[x]->usAttachedPlayerId < MAX_PLAYERS) {
				CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
				if(pPlayerPool) {
					if(m_pTextLabels[x]->usAttachedPlayerId != pPlayerPool->GetLocalPlayerID()) {
						CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(m_pTextLabels[x]->usAttachedPlayerId);
						if(pRemotePlayer) {
							CPlayerPed *pPlayerPed = pRemotePlayer->GetPlayerPed();
							if(pPlayerPed) {
								VECTOR vecBonePosition;
								pPlayerPed->GetBonePosition(2, &vecBonePosition);

								vecPosition.X += vecBonePosition.X;
								vecPosition.Y += vecBonePosition.Y;
								vecPosition.Z += vecBonePosition.Z;
							}
						}
					}
				}
			} else if(m_pTextLabels[x]->usAttachedVehicleId > 0 && m_pTextLabels[x]->usAttachedVehicleId < MAX_VEHICLES) {
				CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
				if(pVehiclePool) {
					CVehicle *pVehicle = pVehiclePool->GetAt(m_pTextLabels[x]->usAttachedVehicleId);
					if (pVehicle) {
						MATRIX4X4 matVehicle;
						pVehicle->GetMatrix(&matVehicle);

						vecPosition.X += matVehicle.pos.X;
						vecPosition.Y += matVehicle.pos.Y;
						vecPosition.Z += matVehicle.pos.Z;
					}
				}
			}

			bLOS = false;

			if (m_pTextLabels[x]->bDrawLOS) {
				CAMERA_AIM *pAimCamera = GameGetInternalAim();
				if(pAimCamera) {
					VECTOR vecCamera;
					vecCamera.X = pAimCamera->pos1x;
					vecCamera.Y = pAimCamera->pos1y;
					vecCamera.Z = pAimCamera->pos1z;
					
					bLOS = pGame->IsLineOfSightClear(vecPosition, vecCamera, true, false, false, true, false, false, false);
				}
			}

			if (!m_pTextLabels[x]->bDrawLOS || bLOS) {
				CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
				if(pPlayerPed) {
					if(pPlayerPed->GetDistanceFromPoint(vecPosition.X, vecPosition.Y, vecPosition.Z) <= m_pTextLabels[x]->fDrawDistance) {
						RwV3d rwPosition;
						rwPosition.x = vecPosition.X;
						rwPosition.y = vecPosition.Y;
						rwPosition.z = vecPosition.Z;
						
						RwV3d rwOutResult;
						
						// CSprite::CalcScreenCoors(RwV3d const&, RwV3d *, float *, float *, bool, bool) — 0x0054EEC0
						((void (*)(RwV3d const&, RwV3d *, float *, float *, bool, bool))(g_libGTASA + 0x0054EEC0 + 1))(rwPosition, &rwOutResult, 0, 0, 0, 0);
						if(rwOutResult.z < 1.0f) {
							break;
						}

						ImVec2 vecRealPos = ImVec2(rwOutResult.x, rwOutResult.y);
						pGUI->Render2DColouredText(vecRealPos, true, m_pTextLabels[x]->splittedMainText);
					}	
				}
			}
		}
	}
	return;
}