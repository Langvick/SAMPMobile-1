#include "main.h"
#include "game/game.h"
#include "netgame.h"
#include "../playerslist.h"


extern CPlayersList *pPlayersList;

CPlayerPool::CPlayerPool() {
	m_pLocalPlayer = new CLocalPlayer();

	for(uint16_t playerId = 0; playerId < MAX_PLAYERS; playerId++)
	{
		m_bPlayerSlotState[playerId] = false;
		m_pPlayers[playerId] = nullptr;
	}
}

CPlayerPool::~CPlayerPool() {
	if(m_pLocalPlayer) {
		delete m_pLocalPlayer;
		m_pLocalPlayer = nullptr;
	}

	for(uint16_t playerId = 0; playerId < MAX_PLAYERS; playerId++) {
		Delete(playerId, 0);
	}
}

bool CPlayerPool::Process() {
	for(uint16_t playerId = 0; playerId < MAX_PLAYERS; playerId++) {
		if(m_bPlayerSlotState[playerId]) {
			m_pPlayers[playerId]->Process();
		}
	}

	m_pLocalPlayer->Process();	

	return true;
}

bool CPlayerPool::New(uint16_t playerId, char *szPlayerName, bool IsNPC)
{
	if(!IsValidPlayerId(playerId)) {
		return false;
	}
	
	m_pPlayers[playerId] = new CRemotePlayer();

	if(m_pPlayers[playerId]) {
		strcpy(m_szPlayerNames[playerId], szPlayerName);
		m_pPlayers[playerId]->SetID(playerId);
		m_pPlayers[playerId]->SetNPC(IsNPC);
		m_bPlayerSlotState[playerId] = true;

		return true;
	}

	return false;
}

bool CPlayerPool::Delete(uint16_t playerId, uint8_t byteReason)
{
	if(!IsValidPlayerId(playerId)) {
		return false;
	}
	
	if(GetLocalPlayer()->IsSpectating()) {
		if(GetLocalPlayer()->m_SpectateID == playerId) {
			GetLocalPlayer()->ToggleSpectating(false);
		}
	}

	m_bPlayerSlotState[playerId] = false;
	
	if(m_pPlayers[playerId]) {
		delete m_pPlayers[playerId];
		m_pPlayers[playerId] = nullptr;
	}

	return true;
}

uint16_t CPlayerPool::FindRemotePlayerIDFromGtaPtr(PED_TYPE *pActor)
{
	for(uint16_t playerId = 0; playerId < MAX_PLAYERS; playerId++) {
		if(m_bPlayerSlotState[playerId]) {
			CPlayerPed *pPlayerPed = m_pPlayers[playerId]->GetPlayerPed();
			if(pPlayerPed) {
				PED_TYPE *pGtaActor = pPlayerPed->GetGtaActor();
				if(pGtaActor == pActor) {
					return m_pPlayers[playerId]->GetID();
				}
			}
		}
	}

	return INVALID_PLAYER_ID;	
}

CPlayerPed *CPlayerPool::FindRemotePlayerPedFromGtaPtr(PED_TYPE *pActor)
{
	for(uint16_t playerId = 0; playerId < MAX_PLAYERS; playerId++) {
		if(m_bPlayerSlotState[playerId]) {
			CPlayerPed *pPlayerPed = m_pPlayers[playerId]->GetPlayerPed();
			if(pPlayerPed) {
				PED_TYPE *pGtaActor = pPlayerPed->GetGtaActor();
				if(pGtaActor == pActor) {
					return pPlayerPed;
				}
			}
		}
	}

	return nullptr;	
}

void CPlayerPool::ApplyCollisionChecking()
{
	for(int i = 0; i < MAX_PLAYERS; i++) {
		CRemotePlayer *pPlayer = GetAt(i);
		if(pPlayer) {
			CPlayerPed *pPlayerPed = pPlayer->GetPlayerPed();
			if(pPlayerPed) {
				if(!pPlayerPed->IsInVehicle()) {
					m_bCollisionChecking[i] = pPlayerPed->GetCollisionChecking();
					pPlayerPed->SetCollisionChecking(true);
				}
			}
		}
	}
}

void CPlayerPool::ResetCollisionChecking()
{
	for(int i = 0; i < MAX_PLAYERS; i++) {
		CRemotePlayer *pPlayer = GetAt(i);
		if(pPlayer) {
			CPlayerPed *pPlayerPed = pPlayer->GetPlayerPed();
			if(pPlayerPed) {
				if(!pPlayerPed->IsInVehicle()) {
					pPlayerPed->SetCollisionChecking(m_bCollisionChecking[i]);
				}
			}
		}
	}
}