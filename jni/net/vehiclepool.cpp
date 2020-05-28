#include "main.h"
#include "game/game.h"
#include "netgame.h"
#include "chatwindow.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;

CVehiclePool::CVehiclePool()
{
	for(uint16_t VehicleID = 0; VehicleID < MAX_VEHICLES; VehicleID++) {
		m_bVehicleSlotState[VehicleID] = false;
		m_pVehicles[VehicleID] = nullptr;
		m_pGTAVehicles[VehicleID] = nullptr;
	}
}

CVehiclePool::~CVehiclePool()
{
	for(uint16_t VehicleID = 0; VehicleID < MAX_VEHICLES; VehicleID++) {
		Delete(VehicleID);
	}
}

void CVehiclePool::Process()
{
	CVehicle *pVehicle;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	CLocalPlayer* pLocalPlayer = pPlayerPool->GetLocalPlayer();

	for(uint16_t x = 0; x < MAX_VEHICLES; x++) {
		if(GetSlotState(x)) {
			pVehicle = m_pVehicles[x];

			if(m_bIsActive[x]) {
				if(pVehicle->IsDriverLocalPlayer()) {
					pVehicle->SetInvulnerable(false);
				} else {
					pVehicle->SetInvulnerable(true);
				}
				
				if(pVehicle->GetHealth() == 0.0f) {
					NotifyVehicleDeath(x);
					continue;
				}

				if(pVehicle->GetVehicleSubtype() != VEHICLE_SUBTYPE_BOAT &&
					pVehicle->GetDistanceFromLocalPlayerPed() < 200.0f &&
					pVehicle->HasSunk()) {
					NotifyVehicleDeath(x);
					continue;
				}

				if(pVehicle->m_pVehicle != m_pGTAVehicles[x]) {
					m_pGTAVehicles[x] = pVehicle->m_pVehicle;
				}

				pVehicle->ProcessMarkers();
			}
		}
	}
}

bool CVehiclePool::New(NEW_VEHICLE *pNewVehicle)
{
	if(!IsValidVehicleId(pNewVehicle->VehicleID)) {
		return false;
	}
	
	if(m_pVehicles[pNewVehicle->VehicleID]) {
		pChatWindow->AddDebugMessage("Warning: vehicle %u was not deleted", pNewVehicle->VehicleID);
		Delete(pNewVehicle->VehicleID);
	}

	m_pVehicles[pNewVehicle->VehicleID] = pGame->NewVehicle(pNewVehicle->iVehicleType, pNewVehicle->vecPos, pNewVehicle->fRotation, pNewVehicle->byteAddSiren);

	if(m_pVehicles[pNewVehicle->VehicleID])
	{
		// colors
		if(pNewVehicle->aColor1 != -1 || pNewVehicle->aColor2 != -1) {
			m_pVehicles[pNewVehicle->VehicleID]->SetColor(pNewVehicle->aColor1, pNewVehicle->aColor2);
		}

		// health
		m_pVehicles[pNewVehicle->VehicleID]->SetHealth(pNewVehicle->fHealth);

		// gta handle
		m_pGTAVehicles[pNewVehicle->VehicleID] = m_pVehicles[pNewVehicle->VehicleID]->m_pVehicle;
		m_bVehicleSlotState[pNewVehicle->VehicleID] = true;

		// interior
		if(pNewVehicle->byteInterior > 0)
			LinkToInterior(pNewVehicle->VehicleID, pNewVehicle->byteInterior);

		// damage status
		if(pNewVehicle->dwPanelDamageStatus || 
			pNewVehicle->dwDoorDamageStatus || 
			pNewVehicle->byteLightDamageStatus ||
			pNewVehicle->byteTireDamageStatus)
		{
			m_pVehicles[pNewVehicle->VehicleID]->UpdateDamageStatus(
				pNewVehicle->dwPanelDamageStatus, 
				pNewVehicle->dwDoorDamageStatus,
				pNewVehicle->byteLightDamageStatus,
				pNewVehicle->byteTireDamageStatus);
		}

		m_pVehicles[pNewVehicle->VehicleID]->SetWheelPopped(0);
		
		m_vecSpawnPos[pNewVehicle->VehicleID].X = pNewVehicle->vecPos.X;
		m_vecSpawnPos[pNewVehicle->VehicleID].Y = pNewVehicle->vecPos.Y;
		m_vecSpawnPos[pNewVehicle->VehicleID].Z = pNewVehicle->vecPos.Z;

		m_bIsActive[pNewVehicle->VehicleID] = true;
		m_bIsWasted[pNewVehicle->VehicleID] = false;
		return true;
	}

	return false;
}

bool CVehiclePool::Delete(uint16_t VehicleID)
{
	if(!IsValidVehicleId(VehicleID)) {
		return false;
	}
	
	m_bVehicleSlotState[VehicleID] = false;
	
	if(m_pVehicles[VehicleID]) {
		delete m_pVehicles[VehicleID];
		m_pVehicles[VehicleID] = nullptr;
	}
	
	m_pGTAVehicles[VehicleID] = nullptr;

	return true;
}

uint16_t CVehiclePool::FindIDFromGtaPtr(VEHICLE_TYPE *pGtaVehicle)
{
	for(int x = 1; x < MAX_VEHICLES; ++x) {
		if(pGtaVehicle == m_pGTAVehicles[x]) {
			return x;
		}
	}

	return INVALID_VEHICLE_ID;
}

int CVehiclePool::FindGtaIDFromID(int iID)
{
	if(!IsValidVehicleId(iID)) {
		return INVALID_VEHICLE_ID;
	}

	if(m_pGTAVehicles[iID]) {
		return GamePool_Vehicle_GetIndex(m_pGTAVehicles[iID]);
	}
	return INVALID_VEHICLE_ID;
}

VEHICLE_TYPE *CVehiclePool::FindGtaVehicleFromId(int iID)
{
	if(!IsValidVehicleId(iID)) {
		return nullptr;
	}

	return m_pGTAVehicles[iID];
}

int CVehiclePool::FindNearestToLocalPlayerPed()
{
	float fLeastDistance = 10000.0f;
	float fThisDistance = 0.0f;
	
	uint16_t usNearestId = INVALID_VEHICLE_ID;
	for(uint16_t x = 0; x < MAX_VEHICLES; ++x) {
		if(GetSlotState(x) && m_bIsActive[x]) {
			fThisDistance = m_pVehicles[x]->GetDistanceFromLocalPlayerPed();
			if(fThisDistance < fLeastDistance) {
				fLeastDistance = fThisDistance;
				usNearestId = x;
			}
		}
	}

	return usNearestId;
}

void CVehiclePool::LinkToInterior(uint16_t VehicleID, int iInterior)
{
	if(!IsValidVehicleId(VehicleID)) {
		return;
	}
	
	if(m_bVehicleSlotState[VehicleID]) {
		m_pVehicles[VehicleID]->LinkToInterior(iInterior);
	}
}

void CVehiclePool::NotifyVehicleDeath(uint16_t VehicleID)
{
	if(!IsValidVehicleId(VehicleID)) {
		return;
	}
	
	if(pNetGame->GetPlayerPool()->GetLocalPlayer()->m_LastVehicle != VehicleID) {
		return;
	}

	RakNet::BitStream bsDeath;
	bsDeath.Write(VehicleID);
	pNetGame->GetRakClient()->RPC(RPC_VehicleDestroyed, &bsDeath, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	pNetGame->GetPlayerPool()->GetLocalPlayer()->m_LastVehicle = INVALID_VEHICLE_ID;
}

void CVehiclePool::AssignSpecialParamsToVehicle(uint16_t VehicleID, uint8_t byteObjective, uint8_t byteDoorsLocked)
{
	if(!IsValidVehicleId(VehicleID)) {
		return;
	}
	
	if(!GetSlotState(VehicleID)) {
		return;
	}
	
	CVehicle *pVehicle = m_pVehicles[VehicleID];
	if(pVehicle && m_bIsActive[VehicleID]) {
		pVehicle->SetObjState(byteObjective);

		pVehicle->SetDoorLockState(byteDoorsLocked);
	}
}