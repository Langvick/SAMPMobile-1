#include "main.h"
#include "game/game.h"
#include "netgame.h"
#include "chatwindow.h"
#include "gui/gui.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
extern CGUI *pGUI;

void ScrDisplayGameText(RPCParameters *rpcParams)
{
	Log("RPC: DisplayGameText");

	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	char szMessage[512];
	int iType;
	int iTime;
	int iLength;

	bsData.Read(iType);
	bsData.Read(iTime);
	
	bsData.Read(iLength);
	if(iLength >= sizeof(szMessage)) {
		return;
	}
	
	bsData.Read(szMessage, iLength);
	szMessage[iLength] = '\0';
	
	pGame->DisplayGameText(szMessage, iTime, iType);
}

void ScrSetGravity(RPCParameters *rpcParams)
{
	Log("RPC: SetGravity");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	float fGravity;

	bsData.Read(fGravity);

	pGame->SetGravity(fGravity);
}

void ScrForceSpawnSelection(RPCParameters *rpcParams)
{
	Log("RPC: ForceSpawnSelection");
	
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	if(pLocalPlayer) {
		pLocalPlayer->ReturnToClassSelection();
	}
}

void ScrSetPlayerPos(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerPos");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	VECTOR vecPos;
	
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->TeleportTo(vecPos);
	}
}

void ScrSetCameraPos(RPCParameters *rpcParams)
{
	Log("RPC: SetCameraPos");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	VECTOR vecPos;
	
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	
	CCamera *pCamera = pGame->GetCamera();
	if(pCamera) {
		pCamera->SetPosition(vecPos.X, vecPos.Y, vecPos.Z, 0.0f, 0.0f, 0.0f);
	}
}

void ScrSetCameraLookAt(RPCParameters *rpcParams)
{
	Log("RPC: SetCameraLookAt");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	VECTOR vecPos;
	
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	
	CCamera *pCamera = pGame->GetCamera();
	if(pCamera) {
		pCamera->LookAtPoint(vecPos.X, vecPos.Y, vecPos.Z, 2);
	}
}

void ScrSetPlayerFacingAngle(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerFacingAngle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	float fAngle;
	bsData.Read(fAngle);
	
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->ForceTargetRotation(fAngle);
	}
}

void ScrSetFightingStyle(RPCParameters *rpcParams)
{
	Log("RPC: SetFightingStyle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t playerId;
	uint8_t byteFightingStyle = 0;
	
	bsData.Read(playerId);
	bsData.Read(byteFightingStyle);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CPlayerPed *pPlayerPed = nullptr;
		if(playerId == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pGame->FindPlayerPed();
		} else if(pPlayerPool->GetSlotState(playerId)) {
			CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
			if (pRemotePlayer) {
				pPlayerPed = pRemotePlayer->GetPlayerPed();
			}
		}
		
		if(pPlayerPed) {
			pPlayerPed->SetFightingStyle(byteFightingStyle);
		}
	}
}

void ScrSetPlayerSkin(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerSkin");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int32_t playerId;
	uint32_t uiSkin;
	
	bsData.Read(playerId);
	bsData.Read(uiSkin);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CPlayerPed *pPlayerPed = nullptr;
		if(playerId == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pGame->FindPlayerPed();
		} else if(pPlayerPool->GetSlotState(playerId)) {
			CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
			if (pRemotePlayer) {
				pPlayerPed = pRemotePlayer->GetPlayerPed();
			}
		}
		
		if(pPlayerPed) {
			pPlayerPed->SetModelIndex(uiSkin);
		}
	}
}

void ScrApplyPlayerAnimation(RPCParameters *rpcParams)
{
	Log("RPC: ApplyPlayerAnimation");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t playerId;
	uint8_t uiAnimLibLen;
	char szAnimLib[64];
	uint8_t uiAnimNameLen;
	char szAnimName[64];
	float fDelta;
	bool bLoop;
	bool bLockX;
	bool bLockY;
	bool bFreeze;
	uint32_t uiTime;

	bsData.Read(playerId);
	
	bsData.Read(uiAnimLibLen);
	if(uiAnimLibLen < 0 || uiAnimLibLen >= sizeof(szAnimLib)) {
		return;
	}
	bsData.Read(szAnimLib, uiAnimLibLen);
	szAnimLib[uiAnimLibLen] = '\0';
	
	bsData.Read(uiAnimNameLen);
	if(uiAnimNameLen < 0 || uiAnimNameLen >= sizeof(szAnimName)) {
		return;
	}
	bsData.Read(szAnimName, uiAnimNameLen);
	szAnimName[uiAnimNameLen] = '\0';
	
	bsData.Read(fDelta);
	bsData.Read(bLoop);
	bsData.Read(bLockX);
	bsData.Read(bLockY);
	bsData.Read(bFreeze);
	bsData.Read(uiTime);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CPlayerPed *pPlayerPed = nullptr;
		if(playerId == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pGame->FindPlayerPed();
		} else if(pPlayerPool->GetSlotState(playerId)) {
			CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
			if (pRemotePlayer) {
				pPlayerPed = pRemotePlayer->GetPlayerPed();
			}
		}
		
		if(pPlayerPed) {
			pPlayerPed->ApplyAnimation(szAnimName, szAnimLib, fDelta, (int)bLoop, (int)bLockX, (int)bLockY, (int)bFreeze, (int)uiTime);
		}
	}
}

void ScrClearPlayerAnimations(RPCParameters *rpcParams)
{
	Log("RPC: ClearPlayerAnimations");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t playerId;
	
	bsData.Read(playerId);
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CPlayerPed *pPlayerPed = nullptr;
		if(playerId == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pGame->FindPlayerPed();
		} else if(pPlayerPool->GetSlotState(playerId)) {
			CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
			if (pRemotePlayer) {
				pPlayerPed = pRemotePlayer->GetPlayerPed();
			}
		}
		
		if(pPlayerPed) {
			MATRIX4X4 mat;
			pPlayerPed->GetMatrix(&mat);
			pPlayerPed->TeleportTo(mat.pos);
		}
	}
}

void ScrSetSpawnInfo(RPCParameters *rpcParams)
{
	Log("RPC: SetSpawnInfo");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	PLAYER_SPAWN_INFO SpawnInfo;

	bsData.Read((char*)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if(pLocalPlayer) {
			pLocalPlayer->SetSpawnInfo(&SpawnInfo);
		}
	}
}

void ScrCreateExplosion(RPCParameters *rpcParams)
{
	Log("RPC: CreateExplosion");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	VECTOR vecPosition;
	int iType;
	float fRadius;

	bsData.Read(vecPosition.X);
	bsData.Read(vecPosition.Y);
	bsData.Read(vecPosition.Z);
	bsData.Read(iType);
	bsData.Read(fRadius);
	
	pGame->CreateExplosion(vecPosition, iType, fRadius);
}

void ScrSetPlayerHealth(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerHealth");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	float fHealth;
	bsData.Read(fHealth);

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->SetHealth(fHealth);
	}
}

void ScrSetPlayerArmour(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerArmour");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	float fHealth;
	bsData.Read(fHealth);

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->SetArmour(fHealth);
	}
}

void ScrSetPlayerColor(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerColor");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t playerId;
	uint32_t dwColor;
	
	bsData.Read(playerId);
	bsData.Read(dwColor);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		if(playerId == pPlayerPool->GetLocalPlayerID()) {
			CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
			if(pLocalPlayer) {
				pLocalPlayer->SetPlayerColor(dwColor);
			}
		} else {
			CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
			if(pRemotePlayer) {
				pRemotePlayer->SetPlayerColor(dwColor);
			}
		}
	}
}

void ScrSetPlayerName(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerName");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t playerId;
	uint8_t byteNickLen;
	char szNewName[MAX_PLAYER_NAME];
	uint8_t byteSuccess;

	bsData.Read(playerId);
	
	bsData.Read(byteNickLen);
	bsData.Read(szNewName, byteNickLen);
	szNewName[byteNickLen] = '\0';
	
	bsData.Read(byteSuccess);

	if(byteNickLen < MAX_PLAYER_NAME) {
		CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
		if(pPlayerPool) {
			if (byteSuccess == 1) {
				pPlayerPool->SetPlayerName(playerId, szNewName);
			}
			
			if(playerId == pPlayerPool->GetLocalPlayerID()) {
				pPlayerPool->SetLocalPlayerName(szNewName);
			}
		}
	}
}

void ScrSetPlayerPosFindZ(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerPosFindZ");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	VECTOR vecPos;

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	vecPos.Z = pGame->FindGroundZForCoord(vecPos.X, vecPos.Y, vecPos.Z);
	vecPos.Z += 1.5f;

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->TeleportTo(vecPos);
	}
}

void ScrSetPlayerInterior(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerInterior");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t byteInterior;
	bsData.Read(byteInterior);
	
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->SetInterior(byteInterior);	
	}
}

void ScrSetPlayerVelocity(RPCParameters* rpcParams)
{
	Log("RPC: SetPlayerVelocity");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	VECTOR vecMoveSpeed;

	bsData.Read(vecMoveSpeed.X);
	bsData.Read(vecMoveSpeed.Y);
	bsData.Read(vecMoveSpeed.Z);

	CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
	
	if(pPlayerPed)
	{
		if(pPlayerPed->m_pPed->dwStateFlags & 3)
		{
			uint32_t dwStateFlags = pPlayerPed->m_pPed->dwStateFlags;
			dwStateFlags ^= 3;
			pPlayerPed->m_pPed->dwStateFlags = dwStateFlags;
		}
		pPlayerPed->SetMoveSpeedVector(vecMoveSpeed);
	}
}

void ScrSetMapIcon(RPCParameters *rpcParams)
{
	Log("RPC: SetMapIcon");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t ucIndex;
	uint8_t ucIcon;
	uint32_t uiColor;
	VECTOR vecPosition;
	uint8_t ucStyle;

	bsData.Read(ucIndex);
	bsData.Read(vecPosition.X);
	bsData.Read(vecPosition.Y);
	bsData.Read(vecPosition.Z);
	bsData.Read(ucIcon);
	bsData.Read(uiColor);
	bsData.Read(ucStyle);

	pNetGame->SetMapIcon(ucIndex, vecPosition.X, vecPosition.Y, vecPosition.Z, ucIcon, uiColor, ucStyle);
}

void ScrDisableMapIcon(RPCParameters *rpcParams)
{
	Log("RPC: DisableMapIcon");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t byteIndex;

	bsData.Read(byteIndex);

	pNetGame->DisableMapIcon(byteIndex);
}

void ScrSetCameraBehindPlayer(RPCParameters *rpcParams)
{
	Log("RPC: SetCameraBehindPlayer");
	
	CCamera *pCamera = pGame->GetCamera();
	if(pCamera) {
		pCamera->SetBehindPlayer();
	}
}

void ScrSetPlayerSpecialAction(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerSpecialAction");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t byteSpecialAction;
	
	bsData.Read(byteSpecialAction);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if(pLocalPlayer) {
			pLocalPlayer->ApplySpecialAction(byteSpecialAction);
		}
	}
}

void ScrTogglePlayerSpectating(RPCParameters *rpcParams)
{
	Log("RPC: TogglePlayerSpectating");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t bToggle;
	
	bsData.Read(bToggle);
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if(pLocalPlayer) {
			pPlayerPool->GetLocalPlayer()->ToggleSpectating(bToggle);
		}
	}
}

void ScrSetPlayerSpectating(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerSpectating");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t playerId;

	bsData.Read(playerId);
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) {
			pRemotePlayer->SetState(PLAYER_STATE_SPECTATING);
		}
	}
}

#define SPECTATE_TYPE_NORMAL	1
#define SPECTATE_TYPE_FIXED		2
#define SPECTATE_TYPE_SIDE		3

void ScrPlayerSpectatePlayer(RPCParameters *rpcParams)
{
	Log("RPC: PlayerSpectatePlayer");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t playerId;
	uint8_t byteMode;

	bsData.Read(playerId);
	bsData.Read(byteMode);

	switch (byteMode) {
		case SPECTATE_TYPE_FIXED: {
			byteMode = 15;
			break;
		}
		case SPECTATE_TYPE_SIDE: {
			byteMode = 14;
			break;
		}
		default: {
			byteMode = 4;
			break;
		}
	}
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if(pPlayerPool) {
			pLocalPlayer->m_byteSpectateMode = byteMode;
			pLocalPlayer->SpectatePlayer(playerId);
		}
	}
}

void ScrPlayerSpectateVehicle(RPCParameters *rpcParams)
{
	Log("RPC: PlayerSpectateVehicle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t vehicleId;
	uint8_t byteMode;

	bsData.Read(vehicleId);
	bsData.Read(byteMode);

	switch (byteMode) {
		case SPECTATE_TYPE_FIXED: {
			byteMode = 15;
			break;
		}
		case SPECTATE_TYPE_SIDE: {
			byteMode = 14;
			break;
		}
		default: {
			byteMode = 3;
			break;
		}
	}

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if(pPlayerPool) {
			pLocalPlayer->m_byteSpectateMode = byteMode;
			pLocalPlayer->SpectateVehicle(vehicleId);
		}
	}
}

void ScrPutPlayerInVehicle(RPCParameters *rpcParams)
{
	Log("RPC: PutPlayerInVehicle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t vehicleId;
	uint8_t seatId;
	
	bsData.Read(vehicleId);
	bsData.Read(seatId);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool) {
		CVehicle *pVehicle = pVehiclePool->GetAt(vehicleId);
		if(pVehicle) {
			CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
			if(pPlayerPed) {
				pPlayerPed->PutDirectlyInVehicle(pVehiclePool->FindGtaIDFromID(vehicleId), seatId);
			}
		}
	}
}

void ScrVehicleParams(RPCParameters *rpcParams)
{
	Log("RPC: SetVehicleParams");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t VehicleID;
	
	bsData.Read(VehicleID);
	
	if(VehicleID > 0 && VehicleID <= MAX_VEHICLES)
	{
		uint8_t byteObjectiveVehicle;
		uint8_t byteDoorsLocked;
	
		bsData.Read(byteObjectiveVehicle);
		bsData.Read(byteDoorsLocked);

		CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
		if(pVehiclePool) {
			pVehiclePool->AssignSpecialParamsToVehicle(VehicleID, byteObjectiveVehicle, byteDoorsLocked);
		}
	}
}

void ScrVehicleParamsEx(RPCParameters *rpcParams)
{
	Log("RPC: VehicleParamsEx");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t vehicleId;
	
	bsData.Read(vehicleId);
	
	if(vehicleId > 0 && vehicleId <= MAX_VEHICLES)
	{
		uint8_t engine, lights, alarm, doors, bonnet, boot, objective;
		
		bsData.Read(engine);
		bsData.Read(lights);
		bsData.Read(alarm);
		bsData.Read(doors);
		bsData.Read(bonnet);
		bsData.Read(boot);
		bsData.Read(objective);

		CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
		if(pVehiclePool) {
			CVehicle *pVehicle = pVehiclePool->GetAt(vehicleId);
			if(pVehicle) {
				pVehicle->SetEngineState(engine);
				pVehicle->SetLightsState(lights);
				pVehicle->SetAlarmState(alarm);
				pVehicle->SetDoorLockState(doors);
				pVehicle->SetDoorOpenFlag(bonnet, 0);
				pVehicle->SetDoorOpenFlag(boot, 1);
				pVehicle->SetObjState(objective);
			}
		}
	}
}

void ScrHaveSomeMoney(RPCParameters *rpcParams)
{
	Log("RPC: HaveSomeMoney");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int iAmmount;
	bsData.Read(iAmmount);
	
	pGame->AddToLocalMoney(iAmmount);
}

void ScrResetMoney(RPCParameters *rpcParams)
{
	Log("RPC: ResetMoney");

	pGame->ResetLocalMoney();
}

void ScrLinkVehicle(RPCParameters *rpcParams)
{
	Log("RPC: LinkVehicle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t VehicleID;
	uint8_t byteInterior;

	bsData.Read(VehicleID);
	bsData.Read(byteInterior);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool) {
		pVehiclePool->LinkToInterior(VehicleID, (int)byteInterior);
	}
}

void ScrRemovePlayerFromVehicle(RPCParameters *rpcParams)
{
	Log("RPC: RemovePlayerFromVehicle");
	
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->ExitCurrentVehicle();
	}
}

void ScrSetVehicleHealth(RPCParameters *rpcParams)
{
	Log("RPC: SetVehicleHealth");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	float fHealth;
	uint16_t vehicleId;

	bsData.Read(vehicleId);
	bsData.Read(fHealth);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool) {
		CVehicle *pVehicle = pVehiclePool->GetAt(vehicleId);
		if(pVehicle) {
			pVehicle->SetHealth(fHealth);
		}
	}
}

void ScrSetVehiclePos(RPCParameters *rpcParams)
{
	Log("RPC: SetVehiclePos");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t vehicleId;
	float fX, fY, fZ;
	
	bsData.Read(vehicleId);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool) {
		CVehicle *pVehicle = pVehiclePool->GetAt(vehicleId);
		if(pVehicle) {
			pVehicle->TeleportTo(VECTOR(fX, fY, fZ));
		}
	}
}

void ScrSetVehicleVelocity(RPCParameters *rpcParams)
{
	Log("RPC: SetVehicleVelocity");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t turn = false;
	VECTOR vecMoveSpeed;
	
	bsData.Read(turn);
	bsData.Read(vecMoveSpeed.X);
	bsData.Read(vecMoveSpeed.Y);
	bsData.Read(vecMoveSpeed.Z);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool) {
		CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
		if(pPlayerPed) {
			CVehicle *pVehicle = pVehiclePool->GetAt(pVehiclePool->FindIDFromGtaPtr(pPlayerPed->GetGtaVehicle()));
			if(pVehicle) {
				pVehicle->SetMoveSpeedVector(vecMoveSpeed);
			}
		}
	}
}

void ScrNumberPlate(RPCParameters *rpcParams)
{
	Log("RPC: NumberPlate");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t vehicleId;
	uint8_t byteLen;
	char szNumberPlate[32+1];

	bsData.Read(vehicleId);
	
	if(vehicleId > 0 || vehicleId <= MAX_VEHICLES)
	{
		bsData.Read(byteLen);
		bsData.Read(szNumberPlate, byteLen);
		szNumberPlate[byteLen] = '\0';
		
		if(strlen(szNumberPlate) > 0)
		{
			pGame->SetVehicleNumberPlate(vehicleId, szNumberPlate);
		}
		else
		{
			pGame->SetVehicleNumberPlate(vehicleId, "SGBTEAM.ID");
		}
	}
}

void ScrInterpolateCamera(RPCParameters *rpcParams)
{
	Log("RPC: InterpolateCamera");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	bool bSetPos = true;
	VECTOR vecFrom, vecDest;
	int time;
	uint8_t mode;

	bsData.Read(bSetPos);
	bsData.Read(vecFrom.X);
	bsData.Read(vecFrom.Y);
	bsData.Read(vecFrom.Z);
	bsData.Read(vecDest.X);
	bsData.Read(vecDest.Y);
	bsData.Read(vecDest.Z);
	bsData.Read(time);
	bsData.Read(mode);

	if(mode < 1 || mode > 2) {
		mode = 2;
	}
	
	if(time > 0) {
		CCamera *pCamera = pGame->GetCamera();
		if(pCamera) {
			if(bSetPos) {
				pCamera->InterpolateCameraPos(&vecFrom, &vecDest, time, mode);
			} else {
				pCamera->InterpolateCameraLookAt(&vecFrom, &vecDest, time, mode);
			}
		}
	}
}

void ScrAddGangZone(RPCParameters *rpcParams)
{
	Log("RPC: AddGangZone");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t wZoneID;
	float minx, miny, maxx, maxy;
	uint32_t dwColor;
	
	bsData.Read(wZoneID);
	bsData.Read(minx);
	bsData.Read(miny);
	bsData.Read(maxx);
	bsData.Read(maxy);
	bsData.Read(dwColor);
	
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if(pGangZonePool) {
		pGangZonePool->New(wZoneID, minx, miny, maxx, maxy, dwColor);
	}
}

void ScrRemoveGangZone(RPCParameters *rpcParams)
{
	Log("RPC: RemoveGangZone");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t wZoneID;
	
	bsData.Read(wZoneID);
	
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if(pGangZonePool) {
		pGangZonePool->Delete(wZoneID);
	}
}

void ScrFlashGangZone(RPCParameters *rpcParams)
{
	Log("RPC: FlashGangZone");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t wZoneID;
	uint32_t dwColor;
	
	bsData.Read(wZoneID);
	bsData.Read(dwColor);
	
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if(pGangZonePool) {
		pGangZonePool->Flash(wZoneID, dwColor);
	}
}

void ScrStopFlashGangZone(RPCParameters *rpcParams)
{
	Log("RPC: StopFlashGangZone");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t wZoneID;
	
	bsData.Read(wZoneID);
	
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if(pGangZonePool) {
		pGangZonePool->StopFlash(wZoneID);
	}
}

void ScrCreateObject(RPCParameters *rpcParams)
{
	Log("RPC: CreateObject");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t usObjectId;
	int iModel;
	VECTOR vecPos;
	VECTOR vecRot;
	float fDrawDistance;
	int8_t cNoCameraCol;
	
	uint16_t usAttachVehicleId;
	uint16_t usAttachObjectId;

	bsData.Read(usObjectId);
	
	if(usObjectId < 0 || usObjectId >= MAX_OBJECTS) {
		return;
	}
	
	bsData.Read(iModel);

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	bsData.Read(vecRot.X);
	bsData.Read(vecRot.Y);
	bsData.Read(vecRot.Z);

	bsData.Read(fDrawDistance);
	bsData.Read(cNoCameraCol);

	bsData.Read(usAttachVehicleId);
	bsData.Read(usAttachObjectId);
	
	CObjectPool *pObjectPool = pNetGame->GetObjectPool();
	if(pObjectPool)
	{
		if(!pObjectPool->GetSlotState(usObjectId))
		{
			pObjectPool->New(usObjectId, iModel, vecPos, vecRot, fDrawDistance);
		}
		
		if(usAttachVehicleId > 0 && usAttachVehicleId <= MAX_VEHICLES)
		{
			float X, Y, Z;
			float rX, rY, rZ;
			int8_t cAttachSyncRot;
			
			bsData.Read(X);
			bsData.Read(Y);
			bsData.Read(Z);
				
			bsData.Read(rX);
			bsData.Read(rY);
			bsData.Read(rZ);
				
			bsData.Read(cAttachSyncRot);
			
			CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
			if(pVehiclePool)
			{
				CObject* pObject = pObjectPool->GetAt(usObjectId);
				if(pObject)
				{
					CVehicle *pVehicle = pVehiclePool->GetAt(usAttachVehicleId);
					if(pVehicle)
					{
						ScriptCommand(&attach_object_to_vehicle, pObject->m_dwGTAId, pVehicle->m_dwGTAId, X, Y, Z, rX, rY, rZ);
					}
				}
			}
		}
		if(usAttachObjectId > 0 && usAttachObjectId <= MAX_OBJECTS)
		{
			float X, Y, Z;
			float rX, rY, rZ;
			int8_t cAttachSyncRot;
			
			bsData.Read(X);
			bsData.Read(Y);
			bsData.Read(Z);
				
			bsData.Read(rX);
			bsData.Read(rY);
			bsData.Read(rZ);
				
			bsData.Read(cAttachSyncRot);
			
			pObjectPool->AttachObjectToObject(usObjectId, usAttachObjectId, VECTOR(X, Y, Z), VECTOR(rX, rY, rZ), cAttachSyncRot);
		}
	}
}

void ScrDestroyObject(RPCParameters *rpcParams)
{
	Log("RPC: DestroyObject");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int16_t sObjectId;
	
	bsData.Read(sObjectId);
	
	if(sObjectId < 0 || sObjectId >= MAX_OBJECTS) {
		return;
	}
	
	CObjectPool* pObjectPool = pNetGame->GetObjectPool();
	if(pObjectPool) {
		if(pObjectPool->GetSlotState(sObjectId)) {
			pObjectPool->Delete(sObjectId);
		}
	}
}

void ScrSetObjectPos(RPCParameters *rpcParams)
{
	Log("RPC: SetObjectPos");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int16_t sObjectId;
	VECTOR vecPos;
	
	bsData.Read(sObjectId);
	
	if(sObjectId < 0 || sObjectId >= MAX_OBJECTS) {
		return;
	}
	
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	CObjectPool* pObjectPool = pNetGame->GetObjectPool();
	if(pObjectPool)  {
		if(pObjectPool->GetSlotState(sObjectId)) {
			CObject* pObject = pObjectPool->GetAt(sObjectId);
			if(pObject) {
				pObject->SetPos(vecPos.X, vecPos.Y, vecPos.Z);
			}
		}
	}
}

void ScrSetObjectRotation(RPCParameters *rpcParams)
{
	Log("RPC: SetObjectRotation");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t sObjectId;
	VECTOR vecRot;

	bsData.Read(sObjectId);
	
	if(sObjectId < 0 || sObjectId >= MAX_OBJECTS) {
		return;
	}
	
	bsData.Read(vecRot.X);
	bsData.Read(vecRot.Y);
	bsData.Read(vecRot.Z);

	CObjectPool* pObjectPool = pNetGame->GetObjectPool();
	if(pObjectPool) {
		if(pObjectPool->GetSlotState(sObjectId)) {
			CObject* pObject = pObjectPool->GetAt(sObjectId);
			if(pObject) {
				pObject->InstantRotate(vecRot.X, vecRot.Y, vecRot.Z);
			}
		}
	}
}

void ScrAttachObjectToPlayer(RPCParameters *rpcParams)
{
	Log("RPC: AttachObjectToPlayer");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int16_t sObjectId;
	int16_t sPlayerID;
	float OffsetX;
	float OffsetY;
	float OffsetZ;
	float rX;
	float rY;
	float rZ;

	bsData.Read(sObjectId);
	
	if(sObjectId < 0 || sObjectId >= MAX_OBJECTS) {
		return;
	}
	
	bsData.Read(sPlayerID);

	bsData.Read(OffsetX);
	bsData.Read(OffsetY);
	bsData.Read(OffsetZ);

	bsData.Read(rX);
	bsData.Read(rY);
	bsData.Read(rZ);

	CObjectPool* pObjectPool = pNetGame->GetObjectPool();
	if(pObjectPool) {
		CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
		if(pPlayerPool) {
			CObject* pObject = pObjectPool->GetAt(sObjectId);
			if(pObject) {
				if(sPlayerID == pPlayerPool->GetLocalPlayerID()) {
					CLocalPlayer *pPlayer = pPlayerPool->GetLocalPlayer();
					if(pPlayer) {
						ScriptCommand(&attach_object_to_actor, pObject->m_dwGTAId, pPlayer->GetPlayerPed()->m_dwGTAId, OffsetX, OffsetY, OffsetZ, rX, rY, rZ);
					}
				} else {
					CRemotePlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt(sPlayerID);
					if(pPlayer) {
						ScriptCommand(&attach_object_to_actor, pObject->m_dwGTAId, pPlayer->GetPlayerPed()->m_dwGTAId, OffsetX, OffsetY, OffsetZ, rX, rY, rZ);
					}
				}
			}
		}
	}
}

void ScrPlaySound(RPCParameters *rpcParams)
{
	Log("RPC: PlaySound");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int iSound;
	float fX, fY, fZ;
	
	bsData.Read(iSound);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	
	pGame->PlaySound(iSound, fX, fY, fZ);
}

void ScrSetPlayerWantedLevel(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerWantedLevel");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t byteLevel;
	
	bsData.Read(byteLevel);
	
	pGame->SetWantedLevel(byteLevel);
}

void ScrTogglePlayerControllable(RPCParameters *rpcParams)
{
	Log("RPC: TogglePlayerControllable");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t byteControllable;
	
	bsData.Read(byteControllable);
	
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->TogglePlayerControllable(byteControllable);
	}
}

void ScrSetPlayerAmmo(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerAmmo");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteWeapon;
	uint16_t wAmmo;
	
	bsData.Read(byteWeapon);
	bsData.Read(wAmmo);
	
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->SetAmmo(byteWeapon, wAmmo);
	}
}

void ScrResetPlayerWeapons(RPCParameters *rpcParams)
{
	Log("RPC: ResetPlayerWeapons");

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->ClearAllWeapons();
	}
}

void ScrGivePlayerWeapon(RPCParameters *rpcParams)
{
	Log("RPC: GivePlayerWeapon");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int iWeapon;
	int iAmmo;
	
	bsData.Read(iWeapon);
	bsData.Read(iAmmo);
	
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		pPlayerPed->GiveWeapon(iWeapon, iAmmo);
	}
}

void ScrSetArmedWeapon(RPCParameters *rpcParams)
{
	Log("RPC: SetArmedWeapon");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int iWeapon;

	bsData.Read(iWeapon);

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed) {
		if(iWeapon >= 0 && iWeapon <= 46) {
			pPlayerPed->SetArmedWeapon(iWeapon);
		}
	}
}

void ScrShowTextDraw(RPCParameters *rpcParams)
{
	Log("RPC: ShowTextDraw");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t wTextID;
	TEXT_DRAW_TRANSMIT TextDrawTransmit;
	char szText[MAX_TEXT_DRAW_LINE];
	uint16_t sTextLen = 0;

	bsData.Read(wTextID);
	
	bsData.Read((char *)&TextDrawTransmit, sizeof(TEXT_DRAW_TRANSMIT));
	
	bsData.Read(sTextLen);
	bsData.Read(szText, sTextLen);
	szText[sTextLen] = '\0';

	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool) {
		if(wTextID >= 0) {
			if(wTextID < MAX_TEXT_DRAWS) {
				pTextDrawPool->New(wTextID, &TextDrawTransmit, szText);
			} /* else if(wTextID < MAX_TEXT_DRAWS + MAX_PLAYER_TEXT_DRAWS) {
				//PlayerText
			} else {
				return;
			}*/
		}
	}
}

void ScrHideTextDraw(RPCParameters *rpcParams)
{
	Log("RPC: HideTextDraw");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t wTextID;

	bsData.Read(wTextID);
		
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool) {
		if(wTextID >= 0) {
			if(wTextID < MAX_TEXT_DRAWS) {
				pTextDrawPool->Delete(wTextID);
			} /* else if(wTextID < MAX_TEXT_DRAWS + MAX_PLAYER_TEXT_DRAWS) {
				//PlayerText
			} else {
				return;
			}*/
		}
	}
}

void ScrEditTextDraw(RPCParameters *rpcParams)
{
	Log("RPC: EditTextDraw");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
		
	uint16_t wTextID;
	char szText[MAX_TEXT_DRAW_LINE];
	uint16_t sTextLen = 0;

	bsData.Read(wTextID);
	
	bsData.Read(sTextLen);
	bsData.Read(szText, sTextLen);
	szText[sTextLen] = '\0';
	
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool) {
		if(wTextID >= 0) {
			if(wTextID < MAX_TEXT_DRAWS) {
				CTextDraw* pText = pTextDrawPool->GetAt(wTextID);
				if (pText) {
					pText->SetText(szText);
				}
			} /* else if(wTextID < MAX_TEXT_DRAWS + MAX_PLAYER_TEXT_DRAWS) {
				//PlayerText
			} else {
				return;
			}*/
		}
	}
}

void ScrSetPlayerAttachedObject(RPCParameters *rpcParams)
{
	Log("RPC: SetPlayerAttachedObject");

	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t iPlayerId;
	uint32_t iSlotIndex;
	bool bCreate;
	PED_ACCESSORY accessotyData;

	bsData.Read(iPlayerId);
	bsData.Read(iSlotIndex);
	bsData.Read(bCreate);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CPlayerPed *pPlayerPed = nullptr;
	
	if(pPlayerPool)
	{
		if(iPlayerId == pPlayerPool->GetLocalPlayerID())
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		else if(pPlayerPool->GetSlotState(iPlayerId)) 
			pPlayerPed = pPlayerPool->GetAt(iPlayerId)->GetPlayerPed();
		
		if(pPlayerPed)
		{
			if(bCreate)
			{
				bsData.Read((char *)&accessotyData, sizeof(PED_ACCESSORY));
				pPlayerPed->CreateAccessory(iSlotIndex, accessotyData);
			}
			else
				pPlayerPed->DeleteAccessory(iSlotIndex);
		}
	}
}

int iRemoveBuildingCount = 0;
void ScrRemoveBuildingForPlayer(RPCParameters *rpcParams) {
	Log("RPC: RemoveBuildingForPlayer");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint32_t dwModel;
	VECTOR vecPosition;
	float fRange;

	if(iRemoveBuildingCount > 0 || iRemoveBuildingCount <= MAX_REMOVEBUILDING_COUNT)
	{
		bsData.Read(dwModel);
		
		if(dwModel >= 0)
		{
			Log("Building Model: %d", dwModel);
			bsData.Read(vecPosition.X);
			bsData.Read(vecPosition.Y);
			bsData.Read(vecPosition.Z);
			bsData.Read(fRange);
		
			pGame->RemoveBuilding(dwModel, vecPosition, fRange);
		}
	}
	iRemoveBuildingCount++;
}

void ScrMoveObject(RPCParameters *rpcParams) {
	Log("RPC: MoveObject");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int16_t objectId;
	VECTOR vecPosition;
	VECTOR vecNewPosition;
	float fMoveSpeed;
	VECTOR vecRotation;

	bsData.Read(objectId);
	
	if(objectId < 0 || objectId >= MAX_OBJECTS) {
		return;
	}
	
	bsData.Read(vecPosition.X);
	bsData.Read(vecPosition.Y);
	bsData.Read(vecPosition.Z);
	bsData.Read(vecNewPosition.X);
	bsData.Read(vecNewPosition.Y);
	bsData.Read(vecNewPosition.Z);
	bsData.Read(fMoveSpeed);
	bsData.Read(vecRotation.X);
	bsData.Read(vecRotation.Y);
	bsData.Read(vecRotation.Z);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(objectId);
	if (pObject) {
		pObject->MoveObject(vecPosition, vecNewPosition, vecRotation, fMoveSpeed);
	}
}

void ScrStopObject(RPCParameters *rpcParams) {
	Log("RPC: StopObject");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	int16_t sObjectId;
	VECTOR vecPosition;
	VECTOR vecRotation;

	bsData.Read(sObjectId);

	if(sObjectId < 0 || sObjectId >= MAX_OBJECTS) {
		return;
	}

	bsData.Read(vecPosition.X);
	bsData.Read(vecPosition.Y);
	bsData.Read(vecPosition.Z);
	
	bsData.Read(vecRotation.X);
	bsData.Read(vecRotation.Y);
	bsData.Read(vecRotation.Z);
	
	CObject* pObject = pNetGame->GetObjectPool()->GetAt(sObjectId);
	if (pObject) {
		pObject->StopMovingObject(vecPosition, vecRotation, pObject->m_fMoveSpeed);
	}
}

void ScrSetObjectMaterial(RPCParameters *rpcParams) {
	Log("RPC: SetObjectMaterial");

	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t sObjectId;
	uint8_t ucMaterialType;
	uint8_t ucMaterialId;

	bsData.Read(sObjectId);

	if(sObjectId < 0 || sObjectId >= MAX_OBJECTS) {
		return;
	}

	bsData.Read(ucMaterialType);
	bsData.Read(ucMaterialId);

	CObject *pObject = pNetGame->GetObjectPool()->GetAt(sObjectId);
	if(pObject) {
		switch(ucMaterialType) {
			case 1: {
				uint16_t usModelId;
				uint8_t ucTxdNameLen;
				char szTxdName[32];
				uint8_t ucTexNameLen;
				char szTexName[32];
				uint32_t uiColor;

				bsData.Read(usModelId); //wtf?
				
				bsData.Read(ucTxdNameLen);
				if(ucTxdNameLen >= sizeof(szTxdName)) {
					break;
				}
				
				bsData.Read(szTxdName, ucTxdNameLen);
				szTxdName[ucTxdNameLen] = 0;
				
				bsData.Read(ucTexNameLen);
				if(ucTexNameLen >= sizeof(szTexName)) {
					break;
				}
				
				bsData.Read(szTexName, ucTexNameLen);
				szTexName[ucTexNameLen] = 0;
				
				bsData.Read(uiColor);
				
				pObject->SetObjectMaterial(ucMaterialId, szTxdName, szTexName, uiColor);
				break;
			}
			/*case 2: {
				uint8_t ucModelSize;
				uint8_t ucFontNameLen;
				char szFontName[32];
				uint8_t ucFontSize;
				uint8_t ucBold;
				uint32_t uiFontColor;
				uint32_t uiBackgroundColor;
				uint8_t ucAlign;
				char szText[1024];
				
				bsData.Read(ucModelSize);
				
				bsData.Read(ucFontNameLen);
				bsData.Read(szFontName);
				szFontName[ucFontNameLen] = '\0';
				
				bsData.Read(ucFontSize);
				bsData.Read(ucBold);
				
				bsData.Read(uiFontColor);
				bsData.Read(uiBackgroundColor);
				
				bsData.Read(ucAlign);
				
				stringCompressor->DecodeString(szText, sizeof(szText), &bsData);
				break;
			}*/
		}
	}
}

void ScrTextDrawSelection(RPCParameters *rpcParams) 
{
	Log("RPC: TextDrawSelection");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	bool bSelection;
	uint32_t dwHoverColor;

	bsData.Read(bSelection);
 	bsData.Read(dwHoverColor);
	
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool) {
		pTextDrawPool->SetSelection(bSelection, dwHoverColor);
	}
}

void ScrSetVehicleZAngle(RPCParameters *rpcParams) 
{
	Log("RPC: SetVehicleZAngle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t vehicleId;
	float fZAngle;
	
	bsData.Read(vehicleId);
	bsData.Read(fZAngle);

	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(vehicleId);
	if(pVehicle) {
		pVehicle->SetZAngle(fZAngle);
	}
}

void ScrApplyActorAnimation(RPCParameters *rpcParams)
{
	Log("RPC: ApplyActorAnimation");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t actorId;
	uint8_t uiAnimLibLen;
	char szAnimLib[64];
	uint8_t uiAnimNameLen;
	char szAnimName[64];
	float fDelta;
	bool bLoop;
	bool bLockX;
	bool bLockY;
	bool bFreeze;
	uint32_t uiTime;

	bsData.Read(actorId);
	
	bsData.Read(uiAnimLibLen);
	if(uiAnimLibLen < 0 || uiAnimLibLen >= sizeof(szAnimLib)) {
		return;
	}
	bsData.Read(szAnimLib, uiAnimLibLen);
	szAnimLib[uiAnimLibLen] = '\0';
	
	bsData.Read(uiAnimNameLen);
	if(uiAnimNameLen < 0 || uiAnimNameLen >= sizeof(szAnimName)) {
		return;
	}
	bsData.Read(szAnimName, uiAnimNameLen);
	szAnimName[uiAnimNameLen] = '\0';
	
	bsData.Read(fDelta);
	bsData.Read(bLoop);
	bsData.Read(bLockX);
	bsData.Read(bLockY);
	bsData.Read(bFreeze);
	bsData.Read(uiTime);

	CActorPool *pActorPool = pNetGame->GetActorPool();
	if(pActorPool) {
		CRemoteActor *pRemoteActor = pActorPool->GetAt(actorId);
		if(pRemoteActor) {
			CActorPed *pActorPed = pRemoteActor->GetActorPed();
			if(pActorPed) {
				pActorPed->ApplyAnimation(szAnimName, szAnimLib, fDelta, bLoop, bLockX, bLockY, bFreeze, uiTime);
			}
		}
	}
}

void ScrClearActorAnimations(RPCParameters *rpcParams)
{
	Log("RPC: ClearActorAnimation");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t actorId;
	
	bsData.Read(actorId);
	
	CActorPool *pActorPool = pNetGame->GetActorPool();
	if(pActorPool) {
		CRemoteActor *pRemoteActor = pActorPool->GetAt(actorId);
		if(pRemoteActor) {
			CActorPed *pActorPed = pRemoteActor->GetActorPed();
			if(pActorPed) {
				MATRIX4X4 mat;
				pActorPed->GetMatrix(&mat);
				pActorPed->TeleportTo(mat.pos);
			}
		}
	}
}

void ScrSetActorFacingAngle(RPCParameters *rpcParams)
{
	Log("RPC: SetActorFacingAngle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t actorId;
	float fRotation;
	
	bsData.Read(actorId);
	bsData.Read(fRotation);
	
	CActorPool *pActorPool = pNetGame->GetActorPool();
	if(pActorPool) {
		CRemoteActor *pRemoteActor = pActorPool->GetAt(actorId);
		if(pRemoteActor) {
			CActorPed *pActorPed = pRemoteActor->GetActorPed();
			if(pActorPed) {
				pActorPed->ForceTargetRotation(fRotation);
			}
		}
	}
}

void ScrSetActorPos(RPCParameters *rpcParams)
{
	Log("RPC: SetActorPos");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t actorId;
	VECTOR vecPos;

	bsData.Read(actorId);
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	CActorPool *pActorPool = pNetGame->GetActorPool();
	if(pActorPool) {
		CRemoteActor *pRemoteActor = pActorPool->GetAt(actorId);
		if(pRemoteActor) {
			CActorPed *pActorPed = pRemoteActor->GetActorPed();
			if(pActorPed) {
				pActorPed->TeleportTo(vecPos);
			}
		}
	}
}

void ScrSetActorHealth(RPCParameters *rpcParams)
{
	Log("RPC: SetActorHealth");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t actorId;
	float fHealth;

	bsData.Read(actorId);
	bsData.Read(fHealth);

	CActorPool *pActorPool = pNetGame->GetActorPool();
	if(pActorPool) {
		CRemoteActor *pRemoteActor = pActorPool->GetAt(actorId);
		if(pRemoteActor) {
			CActorPed *pActorPed = pRemoteActor->GetActorPed();
			if(pActorPed) {
				pActorPed->SetHealth(fHealth);
			}
		}
	}
}

void ScrShowMenu(RPCParameters *rpcParams)
{
	Log("RPC: ShowMenu");

	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteMenuID;

	bsData.Read(byteMenuID);

	CMenuPool *pMenuPool = pNetGame->GetMenuPool();
	if(pMenuPool) {
		pMenuPool->ShowMenu(byteMenuID);
	}
}

void ScrHideMenu(RPCParameters *rpcParams)
{
	Log("RPC: HideMenu");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t byteMenuID;
	
	bsData.Read(byteMenuID);
	
	CMenuPool *pMenuPool = pNetGame->GetMenuPool();
	if(pMenuPool) {
		pMenuPool->HideMenu(byteMenuID);
	}
}
 
void ScrInitMenu(RPCParameters *rpcParams)
{
	Log("RPC: InitMenu");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint8_t byteMenuID;
	int iColumns; // 0 = 1, 1 = 2
	char cText[MAX_MENU_LINE];
	float fX;
	float fY;
	float fCol1;
	float fCol2 = 0.0;
	MENU_INT MenuInteraction;
	
	bsData.Read(byteMenuID);
	bsData.Read(iColumns);
	bsData.Read(cText, MAX_MENU_LINE);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fCol1);
	
	if (iColumns) 
		bsData.Read(fCol2);
	
	bsData.Read(MenuInteraction.iMenu);
	
	for (uint8_t i = 0; i < MAX_MENU_ITEMS; i++)
		bsData.Read(MenuInteraction.iRow[i]);

	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if(!pMenuPool)
		return;
	
	if (pMenuPool->GetSlotState(byteMenuID))
		pMenuPool->Delete(byteMenuID);
	
	CMenu* pMenu = pMenuPool->New(byteMenuID, cText, fX, fY, ((uint8_t)iColumns) + 1, fCol1, fCol2, &MenuInteraction);
	if (!pMenu) 
		return;
	
	uint8_t byteColCount;
	bsData.Read(cText, MAX_MENU_LINE);
	pMenu->SetColumnTitle(0, cText);
	
	bsData.Read(byteColCount);
	for (uint8_t i = 0; i < byteColCount; i++)
	{
		bsData.Read(cText, MAX_MENU_LINE);
		pMenu->AddMenuItem(0, i, cText);
	}
	
	if (iColumns)
	{
		bsData.Read(cText, MAX_MENU_LINE);
		pMenu->SetColumnTitle(1, cText);
		
		bsData.Read(byteColCount);
		for (uint8_t i = 0; i < byteColCount; i++)
		{
			bsData.Read(cText, MAX_MENU_LINE);
			pMenu->AddMenuItem(1, i, cText);
		}
	}
}

void ScrSetPlayerDrunkVisuals(RPCParameters* rpcParams)
{
	Log("RPC: SetPlayerDrunkVisuals");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	int32_t iVisuals;

	bsData.Read(iVisuals);

	ScriptCommand(&set_player_drunk_visuals, 0, iVisuals);
}

void ScrSetPlayerDrunkHandling(RPCParameters* rpcParams)
{
	Log("RPC: SetPlayerDrunkHandling");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	int32_t iVisuals;

	bsData.Read(iVisuals);

	ScriptCommand(&handling_responsiveness, 0, iVisuals);
}

void ScrChatBubble(RPCParameters *rpcParams)
{
	Log("RPC: ChatBubble");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	uint16_t playerId;
	
	uint8_t byteTextLen;
	
	bsData.Read(playerId);
	
	uint32_t dwColor;
	float fDist;
	int iTime;
	unsigned char szText[144];
	memset(szText, 0, 144);
		
	bsData.Read(byteTextLen);
	bsData.Read((char*)szText, byteTextLen);
		
	szText[byteTextLen] = '\0';
}

void RegisterScriptRPCs(RakClientInterface* pRakClient)
{
	Log("Registering ScriptRPC's..");

	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetWeaponAmmo, ScrSetPlayerAmmo);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrResetPlayerWeapons, ScrResetPlayerWeapons);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrGivePlayerWeapon, ScrGivePlayerWeapon);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetArmedWeapon, ScrSetArmedWeapon);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrDisplayGameText, ScrDisplayGameText);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetGravity, ScrSetGravity);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrForceSpawnSelection, ScrForceSpawnSelection);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerPos, ScrSetPlayerPos);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetCameraPos, ScrSetCameraPos);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetCameraLookAt, ScrSetCameraLookAt);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerFacingAngle, ScrSetPlayerFacingAngle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetFightingStyle, ScrSetFightingStyle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerSkin, ScrSetPlayerSkin);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrApplyAnimation, ScrApplyPlayerAnimation);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrClearAnimations, ScrClearPlayerAnimations);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetSpawnInfo, ScrSetSpawnInfo);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrCreateExplosion, ScrCreateExplosion);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerHealth, ScrSetPlayerHealth);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerArmour, ScrSetPlayerArmour);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerColor, ScrSetPlayerColor);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerName, ScrSetPlayerName);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerPosFindZ, ScrSetPlayerPosFindZ);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetInterior, ScrSetPlayerInterior);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerVelocity, ScrSetPlayerVelocity);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetMapIcon, ScrSetMapIcon);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrDisableMapIcon, ScrDisableMapIcon);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetCameraBehindPlayer, ScrSetCameraBehindPlayer);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetSpecialAction, ScrSetPlayerSpecialAction);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrTogglePlayerSpectating, ScrTogglePlayerSpectating);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerSpectating, ScrSetPlayerSpectating);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrPlayerSpectatePlayer, ScrPlayerSpectatePlayer);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrPlayerSpectateVehicle, ScrPlayerSpectateVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrPutPlayerInVehicle, ScrPutPlayerInVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrVehicleParams, ScrVehicleParams);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrVehicleParamsEx, ScrVehicleParamsEx);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrHaveSomeMoney, ScrHaveSomeMoney);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrResetMoney, ScrResetMoney);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrLinkVehicle, ScrLinkVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrRemovePlayerFromVehicle, ScrRemovePlayerFromVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetVehicleHealth, ScrSetVehicleHealth);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetVehiclePos, ScrSetVehiclePos);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetVehicleVelocity, ScrSetVehicleVelocity);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrNumberPlate, ScrNumberPlate);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrInterpolateCamera, ScrInterpolateCamera);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrAddGangZone, ScrAddGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrRemoveGangZone, ScrRemoveGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrFlashGangZone, ScrFlashGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrStopFlashGangZone, ScrStopFlashGangZone);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrCreateObject, ScrCreateObject);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetObjectPos, ScrSetObjectPos);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetObjectRotation, ScrSetObjectRotation);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrDestroyObject, ScrDestroyObject);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrPlaySound, ScrPlaySound);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerWantedLevel, ScrSetPlayerWantedLevel);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrTogglePlayerControllable, ScrTogglePlayerControllable);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrAttachObjectToPlayer, ScrAttachObjectToPlayer);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrShowTextDraw, ScrShowTextDraw);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrHideTextDraw, ScrHideTextDraw);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrEditTextDraw, ScrEditTextDraw);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrMoveObject, ScrMoveObject);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrStopObject, ScrStopObject);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetPlayerAttachedObject, ScrSetPlayerAttachedObject);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrRemoveBuildingForPlayer, ScrRemoveBuildingForPlayer);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetObjectMaterial, ScrSetObjectMaterial);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrTextDrawSelection, ScrTextDrawSelection);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetVehicleZAngle, ScrSetVehicleZAngle);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrApplyActorAnimation, ScrApplyActorAnimation);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrClearActorAnimations, ScrClearActorAnimations);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetActorFacingAngle, ScrSetActorFacingAngle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetActorPos, ScrSetActorPos);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrSetActorHealth, ScrSetActorHealth);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrInitMenu, ScrInitMenu);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrShowMenu, ScrShowMenu);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrHideMenu, ScrHideMenu);
	
	// pRakClient->RegisterAsRemoteProcedureCall(RPC_ScrChatBubble, ScrChatBubble);
}

void UnRegisterScriptRPCs(RakClientInterface* pRakClient)
{
	Log("Unregistering ScriptRPC's..");

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetWeaponAmmo);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrResetPlayerWeapons);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrGivePlayerWeapon);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetArmedWeapon);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrDisplayGameText);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetGravity);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrForceSpawnSelection);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerPos);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetCameraPos);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetCameraLookAt);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerFacingAngle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetFightingStyle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerSkin);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrApplyAnimation);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrClearAnimations);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetSpawnInfo);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrCreateExplosion);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerHealth);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerArmour);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerColor);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerName);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerPosFindZ);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetInterior);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerVelocity);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetMapIcon);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrDisableMapIcon);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetCameraBehindPlayer);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetSpecialAction);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrTogglePlayerSpectating);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerSpectating);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrPlayerSpectatePlayer);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrPlayerSpectateVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrPutPlayerInVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrVehicleParams);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrVehicleParamsEx);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrHaveSomeMoney);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrResetMoney);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrLinkVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrRemovePlayerFromVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetVehicleHealth);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetVehiclePos);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetVehicleVelocity);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrNumberPlate);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrInterpolateCamera);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrAddGangZone);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrRemoveGangZone);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrFlashGangZone);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrStopFlashGangZone);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrCreateObject);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetObjectPos);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetObjectRotation);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrDestroyObject);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrPlaySound);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerWantedLevel);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrTogglePlayerControllable);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrAttachObjectToPlayer);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrShowTextDraw);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrHideTextDraw);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrEditTextDraw);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetPlayerAttachedObject);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrRemoveBuildingForPlayer);
	
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetObjectMaterial);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrTextDrawSelection);
	
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetVehicleZAngle);
	
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrApplyActorAnimation);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrClearActorAnimations);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetActorFacingAngle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetActorPos);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrSetActorHealth);
	
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrInitMenu);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrShowMenu);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrHideMenu);
	
	// pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScrChatBubble);
}
