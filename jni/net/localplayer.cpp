#include "main.h"
#include "game/game.h"
#include "netgame.h"
#include "spawnscreen.h"
#include "extrakeyboard.h"

#include "util/armhook.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CSpawnScreen *pSpawnScreen;
extern CExtraKeyBoard *pExtraKeyBoard;

bool bFirstSpawn = true;

extern int iNetModeNormalOnfootSendRate;
extern int iNetModeNormalInCarSendRate;
extern int iNetModeFiringSendRate;
extern int iNetModeSendMultiplier;

extern bool bUsedPlayerSlots[];

CLocalPlayer::CLocalPlayer()
{
	m_pPlayerPed = pGame->FindPlayerPed();
	m_bIsActive = false;
	m_bIsWasted = false;

	m_iSelectedClass = 0;
	m_bHasSpawnInfo = false;
	m_bWaitingForSpawnRequestReply = false;
	m_bWantsAnotherClass = false;
	m_bInRCMode = false;

	memset(&m_OnFootData, 0, sizeof(ONFOOT_SYNC_DATA));

	m_dwLastSendTick = GetTickCount();
	
	m_dwLastSendAimTick = m_dwLastSendTick;
	m_dwLastStatsUpdateTick = m_dwLastSendTick;
	m_dwLastSendSpecTick = m_dwLastSendTick;
	m_dwLastUpdateOnFootData = m_dwLastSendTick;
	m_dwLastUpdateInCarData = m_dwLastSendTick;
	m_dwLastUpdatePassengerData = m_dwLastSendTick;
	m_dwPassengerEnterExit = m_dwLastSendTick;
	m_dwLastSendSyncTick = m_dwLastSendTick;

	m_CurrentVehicle = 0;
	ResetAllSyncAttributes();

	m_bIsSpectating = false;
	m_byteSpectateType = SPECTATE_TYPE_NONE;
	m_SpectateID = 0xFFFFFFFF;
	
	for (uint8_t i = 0; i < 13; i++) {
		m_byteLastWeapon[i] = 0;
		m_dwLastAmmo[i] = 0;
	}
}

CLocalPlayer::~CLocalPlayer()
{

}

void CLocalPlayer::ResetAllSyncAttributes()
{
	m_byteCurInterior = 0;
	m_LastVehicle = INVALID_VEHICLE_ID;
	m_bInRCMode = false;
	m_DamageVehicleUpdating = INVALID_VEHICLE_ID;
}

bool CLocalPlayer::Process()
{
	uint32_t dwThisTick = GetTickCount();

	if(m_bIsActive && m_pPlayerPed)
	{
		// handle dead
		if(!m_bIsWasted && m_pPlayerPed->GetActionTrigger() == ACTION_DEATH || m_pPlayerPed->IsDead()) {
			ToggleSpectating(false);

			// reset tasks/anims
			m_pPlayerPed->TogglePlayerControllable(true);

			if(m_pPlayerPed->IsInVehicle() && !m_pPlayerPed->IsAPassenger()) {
				SendInCarFullSyncData();
				m_LastVehicle = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());
			}

			SendWastedNotification();

			m_bIsActive = false;
			m_bIsWasted = true;

			return true;
		}

		// server checkpoints update
		pGame->UpdateCheckpoints();
		
		// STATS UPDATES
		if((dwThisTick - m_dwLastStatsUpdateTick) > STATS_UPDATE_TICKS) {
			SendStatsUpdate();
			m_dwLastStatsUpdateTick = dwThisTick;
		}
		
		CheckWeapons();

		// handle interior changing
		uint8_t byteInterior = pGame->GetActiveInterior();
		if(byteInterior != m_byteCurInterior) {
			UpdateRemoteInterior(byteInterior);
		}

		// SPECTATING
		if(m_bIsSpectating) {
			ProcessSpectating();
		}
		
		// DRIVER
		else if(m_pPlayerPed->IsInVehicle() && !m_pPlayerPed->IsAPassenger())
		{
			CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
			
			if(pVehiclePool) {
				m_CurrentVehicle = pVehiclePool->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());
			}

			if((dwThisTick - m_dwLastSendTick) > (unsigned int)GetOptimumInCarSendRate()) {
				m_dwLastSendTick = GetTickCount();
				SendInCarFullSyncData();
			}
			ProcessVehicleDamageUpdates(m_CurrentVehicle);
		}
		// ONFOOT
		else if(m_pPlayerPed->GetActionTrigger() == ACTION_NORMAL || m_pPlayerPed->GetActionTrigger() == ACTION_SCOPE)
		{
			UpdateSurfing();

			if(m_CurrentVehicle != INVALID_VEHICLE_ID)
			{
				m_LastVehicle = m_CurrentVehicle;
				m_CurrentVehicle = INVALID_VEHICLE_ID;
			}

			if((dwThisTick - m_dwLastSendTick) > (unsigned int)GetOptimumOnFootSendRate())
			{
				m_dwLastSendTick = GetTickCount();
				SendOnFootFullSyncData();
			}

			bool bNeedAimSync = false;
			if(LocalPlayerKeys.bKeys[KEY_HANDBRAKE])
			{
				bNeedAimSync = (dwThisTick - m_dwLastSendAimTick) > iNetModeFiringSendRate;
			}
			else
			{
				bNeedAimSync = (dwThisTick - m_dwLastSendAimTick) > 1000;
			}

			if(bNeedAimSync) {
				m_dwLastSendAimTick = dwThisTick;
				SendAimSyncData();
			}
		}
		// PASSENGER
		else if(m_pPlayerPed->IsInVehicle() && m_pPlayerPed->IsAPassenger())
		{
			if((dwThisTick - m_dwLastSendTick) > (unsigned int)GetOptimumInCarSendRate())
			{
				m_dwLastSendTick = GetTickCount();
				SendPassengerFullSyncData();
			}
		}
	}

	// handle !IsActive spectating
	if(m_bIsSpectating && !m_bIsActive) {
		ProcessSpectating();
		return true;
	}

	// handle needs to respawn
	if(m_bIsWasted && (m_pPlayerPed->GetActionTrigger() != ACTION_WASTED) && (m_pPlayerPed->GetActionTrigger() != ACTION_DEATH)) {
		if(m_bClearedToSpawn && !m_bWantsAnotherClass && pNetGame->GetGameState() == GAMESTATE_CONNECTED) {
			if(m_pPlayerPed->GetHealth() > 0.0f) {
				Spawn();
			}
		} else {
			m_bIsWasted = false;
			HandleClassSelection();
			m_bWantsAnotherClass = false;
		}
	}

	return true;
}

void CLocalPlayer::SendWastedNotification()
{
	RakNet::BitStream bsPlayerDeath;
	uint8_t byteDeathReason = 0;
	uint16_t WhoWasResponsible = INVALID_PLAYER_ID;

	//m_pPlayerPed->FindDeathReasonAndResponsiblePlayer(&WhoWasResponsible);

	bsPlayerDeath.Write(byteDeathReason);
	bsPlayerDeath.Write(WhoWasResponsible);
	pNetGame->GetRakClient()->RPC(RPC_Death, &bsPlayerDeath, HIGH_PRIORITY, RELIABLE_ORDERED, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
}

void CLocalPlayer::SendStatsUpdate() {
	RakNet::BitStream bsStats;
	int32_t iMoney = pGame->GetLocalMoney();
	int32_t iDrunkLevel = 0 /* where can i find it? */;

	bsStats.Write((uint8_t)ID_STATS_UPDATE);
	bsStats.Write(iMoney);
	bsStats.Write(iDrunkLevel);
	pNetGame->GetRakClient()->Send(&bsStats, HIGH_PRIORITY, UNRELIABLE, 0);
}

void CLocalPlayer::CheckWeapons() {
	if (m_pPlayerPed->IsInVehicle()) {
		return;
	}
	
	bool bSend = false;
	for (uint8_t i = 0; i <= 12; i++) {
		if(m_byteLastWeapon[i] != m_pPlayerPed->m_pPed->WeaponSlots[i].dwType) {
			m_byteLastWeapon[i] = m_pPlayerPed->m_pPed->WeaponSlots[i].dwType;
			bSend = true;
		}

		if(m_dwLastAmmo[i] != m_pPlayerPed->m_pPed->WeaponSlots[i].dwAmmo) {
			m_dwLastAmmo[i] = m_pPlayerPed->m_pPed->WeaponSlots[i].dwAmmo;
			bSend = true;
		}
	}
	
	if(bSend) {
		RakNet::BitStream bsWeapons;
		bsWeapons.Write((uint8_t)ID_WEAPONS_UPDATE);
		for(uint8_t i = 0; i <= 12; ++i) {
			bsWeapons.Write((uint8_t)i);
			bsWeapons.Write((uint8_t)m_byteLastWeapon[i]);
			bsWeapons.Write((uint16_t)m_dwLastAmmo[i]);
		}
		pNetGame->GetRakClient()->Send(&bsWeapons, HIGH_PRIORITY, UNRELIABLE, 0);
	}
}

void CLocalPlayer::HandleClassSelection()
{
	m_bClearedToSpawn = false;

	if(m_pPlayerPed)
	{
		m_pPlayerPed->SetInitialState();
		m_pPlayerPed->SetHealth(100.0f);
		m_pPlayerPed->TogglePlayerControllable(0);
	}

	RequestClass(m_iSelectedClass);
	pSpawnScreen->Show(true);

	return;
}

void CLocalPlayer::HandleClassSelectionOutcome()
{
	if(m_pPlayerPed)
	{
		m_pPlayerPed->ClearAllWeapons();
		m_pPlayerPed->SetModelIndex(m_SpawnInfo.iSkin);
	}

	m_bClearedToSpawn = true;
}

void CLocalPlayer::SendNextClass()
{
	MATRIX4X4 matPlayer;
	m_pPlayerPed->GetMatrix(&matPlayer);

	if(m_iSelectedClass == (pNetGame->m_iSpawnsAvailable - 1)) m_iSelectedClass = 0;
	else m_iSelectedClass++;

	pGame->PlaySound(1052, matPlayer.pos.X, matPlayer.pos.Y, matPlayer.pos.Z);
	RequestClass(m_iSelectedClass);
}

void CLocalPlayer::SendPrevClass()
{
	MATRIX4X4 matPlayer;
	m_pPlayerPed->GetMatrix(&matPlayer);

	if(m_iSelectedClass == 0) m_iSelectedClass = (pNetGame->m_iSpawnsAvailable - 1);
	else m_iSelectedClass--;		

	pGame->PlaySound(1053,matPlayer.pos.X,matPlayer.pos.Y,matPlayer.pos.Z);
	RequestClass(m_iSelectedClass);
}

void CLocalPlayer::SendSpawn()
{
	RequestSpawn();
	m_bWaitingForSpawnRequestReply = true;
}

void CLocalPlayer::RequestClass(int iClass)
{
	RakNet::BitStream bsSpawnRequest;
	bsSpawnRequest.Write(iClass);
	pNetGame->GetRakClient()->RPC(RPC_RequestClass, &bsSpawnRequest, HIGH_PRIORITY, RELIABLE, 0, false);
}

void CLocalPlayer::RequestSpawn()
{
	// anti player freeze
	m_pPlayerPed->TogglePlayerControllable(true);
	
	RakNet::BitStream bsSpawnRequest;
	pNetGame->GetRakClient()->RPC(RPC_RequestSpawn, &bsSpawnRequest, HIGH_PRIORITY, RELIABLE, 0, false);
}

bool CLocalPlayer::HandlePassengerEntry()
{
	if(GetTickCount() - m_dwPassengerEnterExit < 1000 )
		return true;

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	// CTouchInterface::IsHoldDown
    int isHoldDown = (( int (*)(int, int, int))(g_libGTASA + 0x00270818+1))(0, 1, 1);

	if(isHoldDown)
	{
		uint16_t ClosetVehicleID = pVehiclePool->FindNearestToLocalPlayerPed();
		if(ClosetVehicleID < MAX_VEHICLES && pVehiclePool->GetSlotState(ClosetVehicleID))
		{
			CVehicle* pVehicle = pVehiclePool->GetAt(ClosetVehicleID);
			if(pVehicle->GetDistanceFromLocalPlayerPed() < 4.0f)
			{
				m_pPlayerPed->EnterVehicle(pVehicle->m_dwGTAId, true);
				SendEnterVehicleNotification(ClosetVehicleID, true);
				m_dwPassengerEnterExit = GetTickCount();
				return true;
			}
		}
	}

	return false;
}

void CLocalPlayer::UpdateSurfing() {};

void CLocalPlayer::SendEnterVehicleNotification(uint16_t VehicleID, bool bPassenger)
{
	RakNet::BitStream bsSend;
	bsSend.Write(VehicleID);
	bsSend.Write((uint8_t)(bPassenger ? 1 : 0));

	pNetGame->GetRakClient()->RPC(RPC_EnterVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0,false, UNASSIGNED_NETWORK_ID, nullptr);
}

void CLocalPlayer::SendExitVehicleNotification(uint16_t VehicleID)
{
	RakNet::BitStream bsSend;

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = pVehiclePool->GetAt(VehicleID);

	if(pVehicle)
	{ 
		if (!m_pPlayerPed->IsAPassenger()) 
			m_LastVehicle = VehicleID;

		bsSend.Write(VehicleID);
		pNetGame->GetRakClient()->RPC(RPC_ExitVehicle,&bsSend,HIGH_PRIORITY,RELIABLE_SEQUENCED,0,false, UNASSIGNED_NETWORK_ID, NULL);
	}
}

void CLocalPlayer::UpdateRemoteInterior(uint8_t byteInterior)
{
	m_byteCurInterior = byteInterior;
	RakNet::BitStream bsUpdateInterior;
	bsUpdateInterior.Write(byteInterior);
	pNetGame->GetRakClient()->RPC(RPC_SetInteriorId, &bsUpdateInterior, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CLocalPlayer::SetSpawnInfo(PLAYER_SPAWN_INFO *pSpawn)
{
	memcpy(&m_SpawnInfo, pSpawn, sizeof(PLAYER_SPAWN_INFO));
	m_bHasSpawnInfo = true;
}

bool CLocalPlayer::Spawn()
{
	if(!m_bHasSpawnInfo) {
		return false;
	}

	pSpawnScreen->Show(false);
	pExtraKeyBoard->Show(true);

	CCamera *pCamera = pGame->GetCamera();
	if(pCamera) {
		pCamera->SetBehindPlayer();
	}

	pGame->DisplayWidgets(true);
	pGame->DisplayHUD(true);

	m_pPlayerPed->TogglePlayerControllable(true);

	if(!bFirstSpawn) {
		m_pPlayerPed->SetInitialState();
	} else {
		bFirstSpawn = false;
	}

	pGame->RefreshStreamingAt(m_SpawnInfo.vecPos.X, m_SpawnInfo.vecPos.Y);

	m_pPlayerPed->RestartIfWastedAt(&m_SpawnInfo.vecPos, m_SpawnInfo.fRotation);
	m_pPlayerPed->SetModelIndex(m_SpawnInfo.iSkin);
	m_pPlayerPed->ClearAllWeapons();
	m_pPlayerPed->ResetDamageEntity();

	pGame->DisableTrainTraffic();

	// CCamera::Fade
	WriteMemory(g_libGTASA + 0x0036EA2C, (uintptr_t)"\x70\x47", 2); // bx lr

	m_pPlayerPed->TeleportTo(VECTOR(m_SpawnInfo.vecPos.X, m_SpawnInfo.vecPos.Y, m_SpawnInfo.vecPos.Z + 0.5f));
	m_pPlayerPed->ForceTargetRotation(m_SpawnInfo.fRotation);

	m_bIsWasted = false;
	m_bIsActive = true;
	m_bWaitingForSpawnRequestReply = false;

	RakNet::BitStream bsSendSpawn;
	pNetGame->GetRakClient()->RPC(RPC_Spawn, &bsSendSpawn, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	return true;
}

uint32_t CLocalPlayer::GetPlayerColor()
{
	return TranslateColorCodeToRGBA(pNetGame->GetPlayerPool()->GetLocalPlayerID());
}

void CLocalPlayer::SetPlayerColor(uint32_t dwColor)
{
	SetRadarColor(pNetGame->GetPlayerPool()->GetLocalPlayerID(), dwColor);
}

void CLocalPlayer::ApplySpecialAction(uint8_t byteSpecialAction)
{
	switch(byteSpecialAction)
	{
		case SPECIAL_ACTION_NONE:
		break;

		case SPECIAL_ACTION_USEJETPACK:
		break;
	}
}

int CLocalPlayer::GetOptimumOnFootSendRate()
{
	if(!m_pPlayerPed) return 1000;

	return (iNetModeNormalOnfootSendRate + DetermineNumberOfPlayersInLocalRange() * iNetModeSendMultiplier);
}

int CLocalPlayer::GetOptimumInCarSendRate()
{
	if(!m_pPlayerPed) return 1000;

	return (iNetModeNormalInCarSendRate + DetermineNumberOfPlayersInLocalRange() * iNetModeSendMultiplier);
}

uint8_t CLocalPlayer::DetermineNumberOfPlayersInLocalRange()
{
	uint8_t iNumPlayersInRange = 0;
	for(uint8_t i = 2; i < PLAYER_PED_SLOTS; i++) {
		if(bUsedPlayerSlots[i]) {
			iNumPlayersInRange++;
		}
	}
	return iNumPlayersInRange;
}

void CLocalPlayer::SendOnFootFullSyncData()
{
	RakNet::BitStream bsPlayerSync;
	MATRIX4X4 matPlayer;
	VECTOR vecMoveSpeed;
	uint16_t lrAnalog, udAnalog;
	uint8_t additionalKey = 0;
	uint16_t wKeys = m_pPlayerPed->GetKeys(&lrAnalog, &udAnalog, &additionalKey);

	ONFOOT_SYNC_DATA ofSync;

	m_pPlayerPed->GetMatrix(&matPlayer);
	m_pPlayerPed->GetMoveSpeedVector(&vecMoveSpeed);

	ofSync.lrAnalog = lrAnalog;
	ofSync.udAnalog = udAnalog;
	ofSync.wKeys = wKeys;
	ofSync.vecPos.X = matPlayer.pos.X;
	ofSync.vecPos.Y = matPlayer.pos.Y;
	ofSync.vecPos.Z = matPlayer.pos.Z;

	ofSync.quat.SetFromMatrix(matPlayer);
	ofSync.quat.Normalize();

	if( FloatOffset(ofSync.quat.w, m_OnFootData.quat.w) < 0.00001 &&
		FloatOffset(ofSync.quat.x, m_OnFootData.quat.x) < 0.00001 &&
		FloatOffset(ofSync.quat.y, m_OnFootData.quat.y) < 0.00001 &&
		FloatOffset(ofSync.quat.z, m_OnFootData.quat.z) < 0.00001)
	{
		ofSync.quat.Set(m_OnFootData.quat);
	}

	ofSync.byteHealth = (uint8_t)m_pPlayerPed->GetHealth();
	ofSync.byteArmour = (uint8_t)m_pPlayerPed->GetArmour();

	ofSync.byteCurrentKeyAndWeapon = (additionalKey << 6) | (m_pPlayerPed->GetCurrentWeapon() & 0x3F);

	ofSync.byteSpecialAction = 0;

	ofSync.vecMoveSpeed.X = vecMoveSpeed.X;
	ofSync.vecMoveSpeed.Y = vecMoveSpeed.Y;
	ofSync.vecMoveSpeed.Z = vecMoveSpeed.Z;

	ofSync.vecSurfOffsets.X = 0.0f;
	ofSync.vecSurfOffsets.Y = 0.0f;
	ofSync.vecSurfOffsets.Z = 0.0f;
	ofSync.wSurfInfo = 0;

	ofSync.dwAnimation = 0;

	if( (GetTickCount() - m_dwLastUpdateOnFootData) > 500 || memcmp(&m_OnFootData, &ofSync, sizeof(ONFOOT_SYNC_DATA)))
	{
		m_dwLastUpdateOnFootData = GetTickCount();

		bsPlayerSync.Write((uint8_t)ID_PLAYER_SYNC);
		bsPlayerSync.Write((char*)&ofSync, sizeof(ONFOOT_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsPlayerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		memcpy(&m_OnFootData, &ofSync, sizeof(ONFOOT_SYNC_DATA));
	}
}

void CLocalPlayer::SendInCarFullSyncData()
{
	RakNet::BitStream bsVehicleSync;
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(!pVehiclePool) return;

	MATRIX4X4 matPlayer;
	VECTOR vecMoveSpeed;

	uint16_t lrAnalog, udAnalog;
	uint8_t additionalKey = 0;
	uint16_t wKeys = m_pPlayerPed->GetKeys(&lrAnalog, &udAnalog, &additionalKey);

	CVehicle *pVehicle;

	INCAR_SYNC_DATA icSync;
	memset(&icSync, 0, sizeof(INCAR_SYNC_DATA));

	if(m_pPlayerPed)
	{
		icSync.VehicleID = pVehiclePool->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());

		if(icSync.VehicleID == INVALID_VEHICLE_ID) return;

		icSync.lrAnalog = lrAnalog;
		icSync.udAnalog = udAnalog;
		icSync.wKeys = wKeys;

		pVehicle = pVehiclePool->GetAt(icSync.VehicleID);
		if(!pVehicle) return;

		pVehicle->GetMatrix(&matPlayer);
		pVehicle->GetMoveSpeedVector(&vecMoveSpeed);

		icSync.quat.SetFromMatrix(matPlayer);
		icSync.quat.Normalize();

		if(	FloatOffset(icSync.quat.w, m_InCarData.quat.w) < 0.00001 &&
			FloatOffset(icSync.quat.x, m_InCarData.quat.x) < 0.00001 &&
			FloatOffset(icSync.quat.y, m_InCarData.quat.y) < 0.00001 &&
			FloatOffset(icSync.quat.z, m_InCarData.quat.z) < 0.00001)
		{
			icSync.quat.Set(m_InCarData.quat);
		}

		// pos
		icSync.vecPos.X = matPlayer.pos.X;
		icSync.vecPos.Y = matPlayer.pos.Y;
		icSync.vecPos.Z = matPlayer.pos.Z;

		// move speed
		icSync.vecMoveSpeed.X = vecMoveSpeed.X;
		icSync.vecMoveSpeed.Y = vecMoveSpeed.Y;
		icSync.vecMoveSpeed.Z = vecMoveSpeed.Z;

		icSync.byteCurrentKeyAndWeapon = (additionalKey << 6) | (m_pPlayerPed->GetCurrentWeapon() & 0x3F);

		icSync.fCarHealth = pVehicle->GetHealth();
		icSync.bytePlayerHealth = (uint8_t)m_pPlayerPed->GetHealth();
		icSync.bytePlayerArmour = (uint8_t)m_pPlayerPed->GetArmour();
		
		if(pVehicle->IsSirenOn()) icSync.byteSirenOn = 1;
		else icSync.byteSirenOn = 0;

		// send
		if( (GetTickCount() - m_dwLastUpdateInCarData) > 500 || memcmp(&m_InCarData, &icSync, sizeof(INCAR_SYNC_DATA)))
		{
			m_dwLastUpdateInCarData = GetTickCount();

			bsVehicleSync.Write((uint8_t)ID_VEHICLE_SYNC);
			bsVehicleSync.Write((char*)&icSync, sizeof(INCAR_SYNC_DATA));
			pNetGame->GetRakClient()->Send(&bsVehicleSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

			memcpy(&m_InCarData, &icSync, sizeof(INCAR_SYNC_DATA));
		}
	}
}

void CLocalPlayer::SendPassengerFullSyncData()
{
	RakNet::BitStream bsPassengerSync;
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

	uint16_t lrAnalog, udAnalog;
	uint8_t additionalKey = 0;
	uint16_t wKeys = m_pPlayerPed->GetKeys(&lrAnalog, &udAnalog, &additionalKey);

	PASSENGER_SYNC_DATA psSync;
	MATRIX4X4 mat;

	psSync.VehicleID = pVehiclePool->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());

	if(psSync.VehicleID == INVALID_VEHICLE_ID) return;

	psSync.lrAnalog = lrAnalog;
	psSync.udAnalog = udAnalog;
	psSync.wKeys = wKeys;
	psSync.bytePlayerHealth = (uint8_t)m_pPlayerPed->GetHealth();
	psSync.bytePlayerArmour = (uint8_t)m_pPlayerPed->GetArmour();

	psSync.byteSeatFlags = m_pPlayerPed->GetVehicleSeatID();
	psSync.byteDriveBy = 0;//m_bPassengerDriveByMode;
	
	psSync.byteCurrentKeyAndWeapon = (additionalKey << 6) | (m_pPlayerPed->GetCurrentWeapon() & 0x3F);

	m_pPlayerPed->GetMatrix(&mat);
	psSync.vecPos.X = mat.pos.X;
	psSync.vecPos.Y = mat.pos.Y;
	psSync.vecPos.Z = mat.pos.Z;

	// send
	if((GetTickCount() - m_dwLastUpdatePassengerData) > 500 || memcmp(&m_PassengerData, &psSync, sizeof(PASSENGER_SYNC_DATA)))
	{
		m_dwLastUpdatePassengerData = GetTickCount();

		bsPassengerSync.Write((uint8_t)ID_PASSENGER_SYNC);
		bsPassengerSync.Write((char*)&psSync, sizeof(PASSENGER_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsPassengerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		memcpy(&m_PassengerData, &psSync, sizeof(PASSENGER_SYNC_DATA));
	}
}

void CLocalPlayer::SendAimSyncData()
{
	AIM_SYNC_DATA aimSync;

	CAMERA_AIM* caAim = m_pPlayerPed->GetCurrentAim();

	aimSync.byteCamMode = m_pPlayerPed->GetCameraMode();
	aimSync.vecAimf.X = caAim->f1x;
	aimSync.vecAimf.Y = caAim->f1y;
	aimSync.vecAimf.Z = caAim->f1z;
	aimSync.vecAimPos.X = caAim->pos1x;
	aimSync.vecAimPos.Y = caAim->pos1y;
	aimSync.vecAimPos.Z = caAim->pos1z;
	aimSync.fAimZ = m_pPlayerPed->GetAimZ();
	aimSync.aspect_ratio = GameGetAspectRatio() * 255.0;
	aimSync.byteCamExtZoom = (uint8_t)(m_pPlayerPed->GetCameraExtendedZoom() * 63.0f);

	WEAPON_SLOT_TYPE* pwstWeapon = m_pPlayerPed->GetCurrentWeaponSlot();
	if (pwstWeapon->dwState == 2) {
		aimSync.byteWeaponState = WS_RELOADING;
	} else {
		aimSync.byteWeaponState = (pwstWeapon->dwAmmoInClip > 1) ? WS_MORE_BULLETS : pwstWeapon->dwAmmoInClip;
	}

	if ((GetTickCount() - m_dwLastSendSyncTick) > 500 || memcmp(&m_aimSync, &aimSync, sizeof(AIM_SYNC_DATA)))
	{
		m_dwLastSendSyncTick = GetTickCount();
		RakNet::BitStream bsAimSync;
		bsAimSync.Write((char)ID_AIM_SYNC);
		bsAimSync.Write((char*)&aimSync, sizeof(AIM_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsAimSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 1);
		memcpy(&m_aimSync, &aimSync, sizeof(AIM_SYNC_DATA));
	}
}

void CLocalPlayer::ProcessSpectating()
{
	CPlayerPed *pLocalPed = pGame->FindPlayerPed();
	if(pLocalPed) {
		CCamera *pCamera = pGame->GetCamera();
		if(pCamera) {
			MATRIX4X4 matPos;
			pCamera->GetMatrix(&matPos);

			SPECTATOR_SYNC_DATA spSync;
			spSync.vecPos = matPos.pos;

			uint16_t lrAnalog, udAnalog;
			uint8_t additionalKey = 0;
			uint16_t wKeys = pLocalPed->GetKeys(&lrAnalog, &udAnalog, &additionalKey);

			spSync.lrAnalog = lrAnalog;
			spSync.udAnalog = udAnalog;
			spSync.wKeys = wKeys;

			if((GetTickCount() - m_dwLastSendSpecTick) > GetOptimumOnFootSendRate()) {
				RakNet::BitStream bsSpectatorSync;
				bsSpectatorSync.Write((uint8_t)ID_SPECTATOR_SYNC);
				bsSpectatorSync.Write((char *)&spSync, sizeof(SPECTATOR_SYNC_DATA));
				pNetGame->GetRakClient()->Send(&bsSpectatorSync, HIGH_PRIORITY, UNRELIABLE, 0);

				if((GetTickCount() - m_dwLastSendAimTick) > (GetOptimumOnFootSendRate() * 2)) {
					SendAimSyncData();
					m_dwLastSendAimTick = GetTickCount();
				}
				
				m_dwLastSendSpecTick = GetTickCount();
			}

			pLocalPed->SetHealth(100.0f);
			pLocalPed->TeleportTo(VECTOR(spSync.vecPos.X, spSync.vecPos.Y, spSync.vecPos.Z + 20.0f));
			pGame->DisplayHUD(false);

			if(m_byteSpectateType == SPECTATE_TYPE_PLAYER) {
				CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
				if(pPlayerPool) {
					CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(m_SpectateID);
					if(pRemotePlayer) {
						if(!pRemotePlayer->IsActive() || pRemotePlayer->GetState() == PLAYER_STATE_WASTED) {
							m_byteSpectateType = SPECTATE_TYPE_NONE;
							m_bSpectateProcessed = false;
							return;
						}
					} else {
						m_byteSpectateType = SPECTATE_TYPE_NONE;
						m_bSpectateProcessed = false;
						return;
					}
				}
			}

			if(m_bSpectateProcessed) {
				return;
			}

			switch(m_byteSpectateType) {
				case SPECTATE_TYPE_PLAYER: {
					CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
					if(pPlayerPool) {
						CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(m_SpectateID);
						if(pRemotePlayer) {
							CPlayerPed *pPlayerPed = pRemotePlayer->GetPlayerPed();
							if(pPlayerPed) {
								ScriptCommand(&camera_on_actor, pPlayerPed->m_dwGTAId, m_byteSpectateMode, 2);
								m_bSpectateProcessed = true;
							}
						}
					}
					break;
				}
				
				case SPECTATE_TYPE_VEHICLE: {
					CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
					if(pVehiclePool) {
						CVehicle *pVehicle = pVehiclePool->GetAt(m_SpectateID);
						if(pVehicle) {
							ScriptCommand(&camera_on_vehicle, pVehicle->m_dwGTAId, m_byteSpectateMode, 2);
							m_bSpectateProcessed = true;
						}
					}
					break;
				}
				
				default: {
					pLocalPed->RemoveFromVehicleAndPutAt(0.0f, 0.0f, 10.0f);
					pCamera->SetPosition(50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f);
					pCamera->LookAtPoint(60.0f, 60.0f, 50.0f, 2);
					m_bSpectateProcessed = true;
					break;
				}
			}
		}
	}
	return;
}

void CLocalPlayer::ToggleSpectating(bool bToggle)
{
	if(!bToggle) {
		ScriptCommand(&restore_camera);
		Spawn();
	}

	m_bIsSpectating = bToggle;
	m_byteSpectateType = SPECTATE_TYPE_NONE;
	m_SpectateID = INVALID_PLAYER_ID;
	m_bSpectateProcessed = false;
}

void CLocalPlayer::SpectatePlayer(uint16_t playerId)
{
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(pPlayerPool) {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if(pRemotePlayer) {
			if(pRemotePlayer->GetState() != PLAYER_STATE_NONE && pRemotePlayer->GetState() != PLAYER_STATE_WASTED) {
				m_byteSpectateType = SPECTATE_TYPE_PLAYER;
				m_SpectateID = playerId;
				m_bSpectateProcessed = false;
			}
		}
	}
}

void CLocalPlayer::SpectateVehicle(uint16_t VehicleID)
{
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool) {
		if(pVehiclePool->GetSlotState(VehicleID)) {
			m_byteSpectateType = SPECTATE_TYPE_VEHICLE;
			m_SpectateID = VehicleID;
			m_bSpectateProcessed = false;
		}
	}
}

void CLocalPlayer::ProcessVehicleDamageUpdates(uint16_t CurrentVehicle)
{
	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(CurrentVehicle);
	if(!pVehicle) return;
	
	if(CurrentVehicle != m_DamageVehicleUpdating) {
        m_dwLastPanelDamageStatus = pVehicle->m_dwPanelDamage;
		m_dwLastDoorDamageStatus = pVehicle->m_dwDoorDamage;
		m_byteLastLightsDamageStatus = pVehicle->m_byteLightDamage;
		m_DamageVehicleUpdating = CurrentVehicle;
		return;
	}

	if( m_dwLastPanelDamageStatus != pVehicle->m_dwPanelDamage ||
		m_dwLastDoorDamageStatus != pVehicle->m_dwDoorDamage ||
		m_byteLastLightsDamageStatus != pVehicle->m_byteLightDamage ||
		m_byteLastLightsDamageStatus != pVehicle->m_byteLightDamage ) 
	{			
			m_dwLastPanelDamageStatus = pVehicle->m_dwPanelDamage;
			m_dwLastDoorDamageStatus = pVehicle->m_dwDoorDamage;
			m_byteLastLightsDamageStatus = pVehicle->m_byteLightDamage;
			m_byteLastTireDamageStatus = pVehicle->m_byteTireDamage;

			RakNet::BitStream bsData;
			bsData.Write(m_DamageVehicleUpdating);
			bsData.Write(m_dwLastPanelDamageStatus);
			bsData.Write(m_dwLastDoorDamageStatus);
            bsData.Write(m_byteLastLightsDamageStatus);
            bsData.Write(m_byteLastTireDamageStatus);
			pNetGame->GetRakClient()->RPC(RPC_DamageVehicle, &bsData, HIGH_PRIORITY, RELIABLE_ORDERED, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	}    
}