#include "main.h"
#include "game/game.h"
#include "netgame.h"
#include "../playerslist.h"
#include "chatwindow.h"
#include "dialog.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
extern CDialogWindow *pDialogWindow;
extern CPlayersList *pPlayersList;

int iNetModeNormalOnfootSendRate	= NETMODE_ONFOOT_SENDRATE;
int iNetModeNormalInCarSendRate		= NETMODE_INCAR_SENDRATE;
int iNetModeFiringSendRate			= NETMODE_FIRING_SENDRATE;
int iNetModeSendMultiplier 			= NETMODE_SEND_MULTIPLIER;

int g_iLagCompensation = 0;

void ProcessIncommingEvent(uint16_t bytePlayerID, int iEventType, uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3);

void InitGame(RPCParameters *rpcParams)
{
	Log("RPC: InitGame");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
	
	bsData.ReadCompressed(pNetGame->m_bZoneNames);
	bsData.ReadCompressed(pNetGame->m_bUseCJWalk);
	bsData.ReadCompressed(pNetGame->m_bAllowWeapons);
	bsData.ReadCompressed(pNetGame->m_bLimitGlobalChatRadius);
	bsData.Read(pNetGame->m_fGlobalChatRadius);
	
	bool bStuntBonus;
	bsData.ReadCompressed(bStuntBonus);
	
	bsData.Read(pNetGame->m_fNameTagDrawDistance);
	bsData.ReadCompressed(pNetGame->m_bDisableEnterExits);
	bsData.ReadCompressed(pNetGame->m_bNameTagLOS);
	bsData.ReadCompressed(pNetGame->m_bManualVehicleEngineAndLight);
	bsData.Read(pNetGame->m_iSpawnsAvailable);
	
	uint16_t playerId;
	bsData.Read(playerId);
	
	bsData.ReadCompressed(pNetGame->m_bShowPlayerTags);
	bsData.Read(pNetGame->m_iShowPlayerMarkers);
	bsData.Read(pNetGame->m_byteWorldTime);
	bsData.Read(pNetGame->m_byteWeather);
	bsData.Read(pNetGame->m_fGravity);
	
	bool bLanMode;
	bsData.ReadCompressed(bLanMode);
	
	bsData.Read(pNetGame->m_iDeathDropMoney);
	bsData.ReadCompressed(pNetGame->m_bInstagib);

	bsData.Read(iNetModeNormalOnfootSendRate);
	bsData.Read(iNetModeNormalInCarSendRate);
	bsData.Read(iNetModeFiringSendRate);
	bsData.Read(iNetModeSendMultiplier);

	bsData.Read(pNetGame->m_iLagCompensation);
	g_iLagCompensation = pNetGame->m_iLagCompensation;

	uint8_t byteStrLen;
	bsData.Read(byteStrLen);
	
	if(byteStrLen) {
		bsData.Read(pNetGame->m_szHostName, byteStrLen);
	}
	pNetGame->m_szHostName[byteStrLen] = '\0';

	uint8_t byteVehicleModels[212];
	bsData.Read((char *)&byteVehicleModels[0], sizeof(byteVehicleModels));
	
	bsData.Read(pNetGame->m_iVehicleFriendlyFire);

	pGame->SetGravity(pNetGame->m_fGravity);

	if(pNetGame->m_bDisableEnterExits) {
		pGame->DisableInteriorEnterExits();
	}
	
	pNetGame->SetGameState(GAMESTATE_CONNECTED);
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if(pLocalPlayer) {
			if(!pLocalPlayer->IsSpectating()) {
				pLocalPlayer->HandleClassSelection();
			}
		}
	}
	
	pGame->SetWorldWeather(pNetGame->m_byteWeather);
	pGame->ToggleCJWalk(pNetGame->m_bUseCJWalk);

	if(pChatWindow) {
		pChatWindow->AddDebugMessage("Connected to {B9C9BF}%.64s", pNetGame->m_szHostName);
	}
}

void ServerJoin(RPCParameters *rpcParams)
{
	Log("RPC: ServerJoin");

	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	char szPlayerName[MAX_PLAYER_NAME+1];
	uint16_t playerId;
	unsigned char byteNameLen = 0;
	uint8_t bIsNPC = 0;
	int pading;

	bsData.Read(playerId);
	bsData.Read(pading);
	bsData.Read(bIsNPC);
	
	bsData.Read(byteNameLen);
	bsData.Read(szPlayerName, byteNameLen);
	szPlayerName[byteNameLen] = '\0';

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		pPlayerPool->New(playerId, szPlayerName, bIsNPC);
	}

	pPlayersList->playersCount++;
}

void ServerQuit(RPCParameters *rpcParams)
{
	Log("RPC: ServerQuit");

	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t playerId;
	uint8_t byteReason;
	
	bsData.Read(playerId);
	bsData.Read(byteReason);
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		pPlayerPool->Delete(playerId, byteReason);
	}

	pPlayersList->playersCount--;
}

void ClientMessage(RPCParameters *rpcParams)
{
	Log("RPC: ClientMessage");

	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint32_t uiColor;
	int32_t iLength;
	char szMessage[144];

	bsData.Read(uiColor);
	
	bsData.Read(iLength);
	if(iLength <= 0 || iLength >= sizeof(szMessage)) {
		return;
	}

	bsData.Read(szMessage, iLength);
	szMessage[iLength] = 0;

	if(pChatWindow) {
		pChatWindow->AddClientMessage(uiColor, szMessage);
	}
}

void Chat(RPCParameters *rpcParams)
{
	Log("RPC: Chat");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t playerId;
	uint8_t byteTextLen;
	char szMessage[144];

	bsData.Read(playerId);
	
	bsData.Read(byteTextLen);
	if(byteTextLen >= sizeof(szMessage)) {
		return;
	}
	
	bsData.Read(szMessage, byteTextLen);
	szMessage[byteTextLen] = '\0';

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		if (playerId == pPlayerPool->GetLocalPlayerID()) {
			CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
			if (pLocalPlayer) {
				pChatWindow->AddChatMessage(pPlayerPool->GetLocalPlayerName(), pLocalPlayer->GetPlayerColor(), szMessage);
			}
		} else {
			CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
			if(pRemotePlayer) {
				pRemotePlayer->Say(reinterpret_cast<unsigned char *>(szMessage));
			}
		}
	}
}

void RequestClass(RPCParameters *rpcParams)
{
	Log("RPC: RequestClass");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteRequestOutcome = 0;
	PLAYER_SPAWN_INFO SpawnInfo;

	bsData.Read(byteRequestOutcome);
	bsData.Read((char *)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if(pLocalPlayer) {
			if(byteRequestOutcome) {
				pLocalPlayer->SetSpawnInfo(&SpawnInfo);
				pLocalPlayer->HandleClassSelectionOutcome();
			}
		}
	}
}

void Weather(RPCParameters *rpcParams)
{
	Log("RPC: Weather");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteWeather;
	
	bsData.Read(byteWeather);
	
	pNetGame->m_byteWeather = byteWeather;
	pGame->SetWorldWeather(byteWeather);
}

void RequestSpawn(RPCParameters *rpcParams)
{
	Log("RPC: RequestSpawn");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteRequestOutcome = 0;
	
	bsData.Read(byteRequestOutcome);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if(pLocalPlayer) {
			if(byteRequestOutcome == 2 || (byteRequestOutcome && pLocalPlayer->m_bWaitingForSpawnRequestReply)) {
				pLocalPlayer->Spawn();
			} else {
				pLocalPlayer->m_bWaitingForSpawnRequestReply = false;
			}
		}
	}
}

void WorldTime(RPCParameters *rpcParams)
{
	Log("RPC: WorldTime");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteWorldTime;
	
	bsData.Read(byteWorldTime);
	
	pNetGame->m_byteWorldTime = byteWorldTime;
}

void SetTimeEx(RPCParameters *rpcParams)
{
	Log("RPC: SetTimeEx");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteHour;
	uint8_t byteMinute;
	
	bsData.Read(byteHour);
	bsData.Read(byteMinute);

	pGame->SetWorldTime(byteHour, byteMinute);
	pNetGame->m_byteWorldTime = byteHour;
	pNetGame->m_byteWorldMinute = byteMinute;
}

void WorldPlayerAdd(RPCParameters *rpcParams)
{
	Log("RPC: WorldPlayerAdd");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t playerId;
	uint8_t byteFightingStyle = 4;
	uint8_t byteTeam = 0;
	unsigned int iSkin = 0;
	VECTOR vecPos;
	float fRotation = 0;
	uint32_t dwColor = 0;

	bsData.Read(playerId);
	bsData.Read(byteTeam);
	bsData.Read(iSkin);
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	bsData.Read(fRotation);
	bsData.Read(dwColor);
	bsData.Read(byteFightingStyle);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CRemotePlayer* pRemotePlayer = pPlayerPool->GetAt(playerId);
		if(pRemotePlayer) {
			pRemotePlayer->Spawn(byteTeam, iSkin, &vecPos, fRotation, dwColor, byteFightingStyle);
		}
	}
}

void WorldPlayerRemove(RPCParameters *rpcParams)
{
	Log("RPC: WorldPlayerRemove");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t playerId;

	bsData.Read(playerId);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CRemotePlayer* pRemotePlayer = pPlayerPool->GetAt(playerId);
		if(pRemotePlayer) {
			pRemotePlayer->Remove();
		}
	}
}

void SetCheckpoint(RPCParameters *rpcParams)
{
	Log("RPC: SetCheckpoint");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	VECTOR vecPosition;
	float fSize;

	bsData.Read(vecPosition.X);
	bsData.Read(vecPosition.Y);
	bsData.Read(vecPosition.Z);
	bsData.Read(fSize);
	
	VECTOR vecExtent;
	vecExtent.X = fSize;
	vecExtent.Y = fSize;
	vecExtent.Z = fSize;

	pGame->SetCheckpointInformation(&vecPosition, &vecExtent);
	pGame->ToggleCheckpoints(true);
}

void DisableCheckpoint(RPCParameters *rpcParams)
{
	Log("RPC: DisableCheckpoint");

	pGame->ToggleCheckpoints(false);
}

void SetRaceCheckpoint(RPCParameters *rpcParams)
{
	Log("RPC: SetRaceCheckpoint");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	float fX,fY,fZ;
	uint8_t byteType;
	VECTOR pos, next;

	bsData.Read(byteType);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	pos.X = fX;
	pos.Y = fY;
	pos.Z = fZ;

	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	next.X = fX;
	next.Y = fY;
	next.Z = fZ;

	bsData.Read(fX);

	pGame->SetRaceCheckpointInformation(byteType, &pos, &next, fX);
	pGame->ToggleCheckpoints(true);
}

void DisableRaceCheckpoint(RPCParameters *rpcParams)
{
	Log("RPC: DisableRaceCheckpoint");

	pGame->ToggleRaceCheckpoints(false);
}

void WorldVehicleAdd(RPCParameters *rpcParams)
{
	Log("RPC: WorldVehicleAdd");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	NEW_VEHICLE NewVehicle;
	bsData.Read((char *)&NewVehicle,sizeof(NEW_VEHICLE));

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool) {
		if(NewVehicle.iVehicleType >= 400 && NewVehicle.iVehicleType <= 611) {
			pVehiclePool->New(&NewVehicle);
		}
	}
}

void WorldVehicleRemove(RPCParameters *rpcParams)
{
	Log("RPC: WorldVehicleRemove");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t vehicleId;

	bsData.Read(vehicleId);
	
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool) {
		pVehiclePool->Delete(vehicleId);
	}
}

void EnterVehicle(RPCParameters *rpcParams)
{
	Log("RPC: EnterVehicle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t playerId;
	uint16_t vehicleId = 0;
	uint8_t bytePassenger = 0;

	bsData.Read(playerId);
	bsData.Read(vehicleId);
	bsData.Read(bytePassenger);
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(playerId);
		if(pPlayer) {
			pPlayer->EnterVehicle(vehicleId, (bytePassenger != 0) ? true : false);
		}
	}	
}

void ExitVehicle(RPCParameters *rpcParams)
{
	Log("RPC: ExitVehicle");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t playerId;
	uint16_t vehicleId;

	bsData.Read(playerId);
	bsData.Read(vehicleId);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(playerId);
		if(pPlayer) {
			pPlayer->ExitVehicle();
		}
	}	
}

void DialogBox(RPCParameters *rpcParams)
{
	Log("RPC: DialogBox");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t usDialogId;
	
	uint8_t ucDialogStyle;
	
	uint8_t ucTitleLen;
	char szTitle[64];
	
	uint8_t ucLeftButtonLen;
	char szLeftButton[32];
	
	uint8_t ucRightButtonLen;
	char szRightButton[32];
	
	uint8_t ucTextBoxLen;
	char szTextBox[4096];

	pDialogWindow->Clear();

	bsData.Read(usDialogId);
	
	bsData.Read(ucDialogStyle);
	
	bsData.Read(ucTitleLen);
	bsData.Read(szTitle, ucTitleLen);
	szTitle[ucTitleLen] = '\0';

	bsData.Read(ucLeftButtonLen);
	bsData.Read(szLeftButton, ucLeftButtonLen);
	szLeftButton[ucLeftButtonLen] = '\0';

	bsData.Read(ucRightButtonLen);
	bsData.Read(szRightButton, ucRightButtonLen);
	szRightButton[ucRightButtonLen] = '\0';

	stringCompressor->DecodeString(szTextBox, 4096, &bsData);

	dialogData *incomingDialogue = new dialogData;
	incomingDialogue->usDialogId = usDialogId;
	incomingDialogue->ucDialogType = (dialogStyles)ucDialogStyle;

	char szTemp[4096];
		
	cp1251_to_utf8(szTemp, szTitle);
	incomingDialogue->szTitle = std::string(szTemp);
		
	cp1251_to_utf8(szTemp, szLeftButton);
	incomingDialogue->szLeftButton = std::string(szTemp);
		
	cp1251_to_utf8(szTemp, szRightButton);
	incomingDialogue->szRightButton = std::string(szTemp);

	cp1251_to_utf8(szTemp, szTextBox);
	incomingDialogue->szMainText = std::string(szTemp);
	
	if(pDialogWindow)
	{
		pDialogWindow->Show(incomingDialogue);
	}
	return;
}

void GameModeRestart(RPCParameters *rpcParams)
{
	Log("RPC: GameModeRestart");
	
	pChatWindow->AddInfoMessage("The server is restarting..");
	pNetGame->ShutDownForGameRestart();
}

#define REJECT_REASON_BAD_VERSION   1
#define REJECT_REASON_BAD_NICKNAME  2
#define REJECT_REASON_BAD_MOD		3
#define REJECT_REASON_BAD_PLAYERID	4
void ConnectionRejected(RPCParameters *rpcParams)
{
	Log("RPC: ConnectionRejected");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint8_t byteRejectReason;

	bsData.Read(byteRejectReason);

	switch(byteRejectReason) {
		case REJECT_REASON_BAD_VERSION: {
			pChatWindow->AddInfoMessage("CONNECTION REJECTED: Incorrect Version.");
			break;
		}
		case REJECT_REASON_BAD_NICKNAME: {
			pChatWindow->AddInfoMessage("CONNECTION REJECTED: Unacceptable NickName");
			pChatWindow->AddInfoMessage("Please choose another nick between and 3-20 characters");
			pChatWindow->AddInfoMessage("Please use only a-z, A-Z, 0-9");
			pChatWindow->AddInfoMessage("Use /quit to exit or press ESC and select Quit Game");
			break;
		}
		case REJECT_REASON_BAD_MOD: {
			pChatWindow->AddInfoMessage("CONNECTION REJECTED: Bad mod version.");
			break;
		}
		case REJECT_REASON_BAD_PLAYERID: {
			pChatWindow->AddInfoMessage("CONNECTION REJECTED: Unable to allocate a player slot.");
			break;
		}
	}

	pNetGame->GetRakClient()->Disconnect(500);
}

void Pickup(RPCParameters *rpcParams)
{
	Log("RPC: Pickup");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	PICKUP Pickup;
	int iIndex;

	bsData.Read(iIndex);
	bsData.Read((char*)&Pickup, sizeof (PICKUP));

	CPickupPool *pPickupPool = pNetGame->GetPickupPool();
	if (pPickupPool) {
		pPickupPool->New(&Pickup, iIndex);
	}
}

void DestroyPickup(RPCParameters *rpcParams)
{
	Log("RPC: DestroyPickup");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	int iIndex;
	bsData.Read(iIndex);

	CPickupPool *pPickupPool = pNetGame->GetPickupPool();
	if (pPickupPool) {
		pPickupPool->Destroy(iIndex);
	}
}

void Create3DTextLabel(RPCParameters *rpcParams)
{
	Log("RPC: Create3DTextLabel");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	int16_t iLabelId;
	uint32_t uiColor;
	VECTOR vecPos;
	float fDrawDistance;
	uint8_t cLineOfSight;
	int16_t sPlayerId;
	int16_t sVehicleId;
	char szText[4096];

	bsData.Read(iLabelId);
	bsData.Read(uiColor);
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	bsData.Read(fDrawDistance);
	bsData.Read(cLineOfSight);
	bsData.Read(sPlayerId);
	bsData.Read(sVehicleId);

	stringCompressor->DecodeString(szText, 4096, &bsData);

	CLabelPool *pLabelsPool = pNetGame->GetLabelPool();
	if(pLabelsPool) {
		pLabelsPool->CreateTextLabel(iLabelId, szText, uiColor, vecPos, fDrawDistance, cLineOfSight, sPlayerId, sVehicleId);
	}
}

void Destroy3DTextLabel(RPCParameters *rpcParams) 
{
	Log("RPC: Destroy3DTextLabel");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t iLabelId;
	
	bsData.Read(iLabelId);

	CLabelPool *pLabelsPool = pNetGame->GetLabelPool();
	if(pLabelsPool) {
		pLabelsPool->Delete(iLabelId);
	}
}

void WorldPlayerDeath(RPCParameters *rpcParams)
{
	Log("RPC: WorldPlayerDeath");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t playerId;
	
	bsData.Read(playerId);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if(pRemotePlayer) {
			pRemotePlayer->HandleDeath();
		}
	}
}

void ScmEvent(RPCParameters *rpcParams)
{
	Log("RPC: ScmEvent");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t usPlayerId;
	int iEventId;
	uint32_t uiParam1;
	uint32_t uiParam2;
	uint32_t uiParam3;

	bsData.Read(usPlayerId);
	bsData.Read(iEventId);
	bsData.Read(uiParam1);
	bsData.Read(uiParam2);
	bsData.Read(uiParam3);

	ProcessIncommingEvent(usPlayerId, iEventId, uiParam1, uiParam2, uiParam3);
}

void WorldActorAdd(RPCParameters* rpcParams)
{
	Log("RPC: WorldActorAdd");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t actorId;
	int iSkinId;
	VECTOR vecPos;
	float fRotation;
	float fHealth;
	bool bInvulnerable;

	bsData.Read(actorId);
	bsData.Read(iSkinId);
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	bsData.Read(fRotation);
	bsData.Read(fHealth);
	bsData.Read(bInvulnerable);

	CActorPool *pActorPool = pNetGame->GetActorPool();
	if(pActorPool) {
		pActorPool->Spawn(actorId, iSkinId, vecPos, fRotation, fHealth, bInvulnerable);
	}
}

void WorldActorRemove(RPCParameters* rpcParams)
{
	Log("RPC: WorldActorRemove");
	
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t actorId;

	bsData.Read(actorId);
	
	CActorPool *pActorPool = pNetGame->GetActorPool();
	if(pActorPool) {
		pActorPool->Delete(actorId);
	}
}

void DamageVehicle(RPCParameters *rpcParams)
{
	Log("RPC: DamageVehicle");
	
	PlayerID sender = rpcParams->sender;
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);
   
	uint16_t  VehicleID;
	uint32_t  dwPanels;
	uint32_t  dwDoors;
	uint8_t   byteLights;
	uint8_t   byteTire;

	bsData.Read(VehicleID);
	bsData.Read(dwPanels);
	bsData.Read(dwDoors);
	bsData.Read(byteLights);
	bsData.Read(byteTire);

	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);
	if(pVehicle) {
		pVehicle->UpdateDamageStatus(dwPanels, dwDoors, byteLights, byteTire);
	}
}

void UpdateScoresPingsIPs(RPCParameters *rpcParams)
{
	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	uint16_t playerId;
	int  iPlayerScore;
	uint32_t dwPlayerPing;

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	for (int8_t i = 0; i < (rpcParams->numberOfBitsOfData /8) /9; i++)
	{
		bsData.Read(playerId);
		bsData.Read(iPlayerScore);
		bsData.Read(dwPlayerPing);

		pPlayerPool->UpdateScore(playerId, iPlayerScore);
		pPlayerPool->UpdatePing(playerId, dwPlayerPing);
	}
}

void RegisterRPCs(RakClientInterface* pRakClient)
{
	Log("Registering RPC's..");

	pRakClient->RegisterAsRemoteProcedureCall(RPC_InitGame, InitGame);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ServerJoin, ServerJoin);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ServerQuit, ServerQuit);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ClientMessage, ClientMessage);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_Chat, Chat);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_RequestClass, RequestClass);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_RequestSpawn, RequestSpawn);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_Weather, Weather);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_WorldTime, WorldTime);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_SetTimeEx, SetTimeEx);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_WorldPlayerAdd, WorldPlayerAdd);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_WorldPlayerRemove, WorldPlayerRemove);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_WorldPlayerDeath, WorldPlayerDeath);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_SetCheckpoint, SetCheckpoint);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_DisableCheckpoint, DisableCheckpoint);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_SetRaceCheckpoint, SetRaceCheckpoint);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_DisableRaceCheckpoint, DisableRaceCheckpoint);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_WorldVehicleAdd, WorldVehicleAdd);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_WorldVehicleRemove, WorldVehicleRemove);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_EnterVehicle, EnterVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ExitVehicle, ExitVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ShowDialog, DialogBox);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_GameModeRestart, GameModeRestart);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ConnectionRejected, ConnectionRejected);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_Pickup, Pickup);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_DestroyPickup, DestroyPickup);

	pRakClient->RegisterAsRemoteProcedureCall(RPC_Show3DText, Create3DTextLabel);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_Destroy3DText, Destroy3DTextLabel);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_ScmEvent, ScmEvent);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_WorldActorAdd, WorldActorAdd);
	pRakClient->RegisterAsRemoteProcedureCall(RPC_WorldActorRemove, WorldActorRemove);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_DamageVehicle, DamageVehicle);
	
	pRakClient->RegisterAsRemoteProcedureCall(RPC_UpdateScoresPingsIPs, UpdateScoresPingsIPs);
}

void UnRegisterRPCs(RakClientInterface* pRakClient)
{
	Log("UnRegistering RPC's..");

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_InitGame);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ServerJoin);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ServerQuit);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ClientMessage);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_Chat);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_RequestClass);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_RequestSpawn);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_Weather);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_WorldTime);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_SetTimeEx);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_WorldPlayerAdd);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_WorldPlayerRemove);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_WorldPlayerDeath);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_SetCheckpoint);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_DisableCheckpoint);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_SetRaceCheckpoint);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_DisableRaceCheckpoint);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_WorldVehicleAdd);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_WorldVehicleRemove);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_EnterVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ExitVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ShowDialog);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_GameModeRestart);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ConnectionRejected);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_Pickup);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_DestroyPickup);

	pRakClient->UnregisterAsRemoteProcedureCall(RPC_Show3DText);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_Destroy3DText);
	
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_ScmEvent);
	
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_WorldActorAdd);
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_WorldActorRemove);
	
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_DamageVehicle);
	
	pRakClient->UnregisterAsRemoteProcedureCall(RPC_UpdateScoresPingsIPs);
}