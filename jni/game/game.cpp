#include "../main.h"
#include "game.h"
#include "../util/armhook.h"
#include "../net/netgame.h"

extern CNetGame *pNetGame;

void ApplyPatches();
void ApplyInGamePatches();
void InstallHooks();
void LoadSplashTexture();
void InitScripting();

uint16_t *wszGameTextGXT = nullptr;
uint16_t *wszTextDrawGXT = nullptr;

bool bUsedPlayerSlots[PLAYER_PED_SLOTS];

extern char* PLAYERS_REALLOC;

CGame::CGame() {
	m_pGameCamera = new CCamera();
	m_pGamePlayer = nullptr;

	m_bClockEnabled = true;
	m_bCheckpointsEnabled = false;
	m_dwCheckpointMarker = 0;

	m_bRaceCheckpointsEnabled = 0;
	m_dwRaceCheckpointHandle = 0;
	m_dwRaceCheckpointMarker = 0;

	memset(&bUsedPlayerSlots[0], 0, PLAYER_PED_SLOTS);
}

bool CGame::IsMenuActive()
{
	return (*(uint8_t *)(g_libGTASA + 0x008C9BA3) == 1);	 
}

uint8_t CGame::FindFirstFreePlayerPedSlot() {
	for(uint8_t x = 2; x < PLAYER_PED_SLOTS; ++x) {
		if(!bUsedPlayerSlots[x]) {
			return x;
		}
	}

	return 0;
}

CPlayerPed* CGame::NewPlayer(int iSkin, VECTOR vecPosition, float fRotation, uint8_t byteCreateMarker)
{
	uint8_t ucPlayerNum = FindFirstFreePlayerPedSlot();
	if(!ucPlayerNum) {
		return nullptr;
	}

	CPlayerPed *pPlayerNew = new CPlayerPed(ucPlayerNum, iSkin, vecPosition.X, vecPosition.Y, vecPosition.Z, fRotation);
	if(pPlayerNew) {
		bUsedPlayerSlots[ucPlayerNum] = true;
	}
	return pPlayerNew;
}

CVehicle* CGame::NewVehicle(int iType, VECTOR vecPosition, float fRotation, bool bAddSiren)
{
	CVehicle *pVehicleNew = new	CVehicle(iType, vecPosition, fRotation, bAddSiren);
	return pVehicleNew;
}

CActorPed *CGame::NewActor(int iSkin, VECTOR vecPosition, float fRotation, float fHealth, bool bInvulnerable)
{
	CActorPed *pActorNew = new CActorPed(iSkin, vecPosition, fRotation, fHealth, bInvulnerable);
	return pActorNew;
}

CObject *CGame::NewObject(int iModel, VECTOR vecPosition, VECTOR vecRotation, float fDrawDistance)
{
	CObject *pObjectNew = new CObject(iModel, vecPosition, vecRotation, fDrawDistance);
	return pObjectNew;
}

uint32_t CGame::CreatePickup(uint16_t usModel, int iType, float fX, float fY, float fZ, int* unk)
{
	if(!HasModelLoaded(usModel)) {
		RequestModel(usModel, GAME_REQUIRED);
		LoadRequestedModels(false);
	}

	uint32_t pickupGTAId;
	ScriptCommand(&create_pickup, usModel, iType, fX, fY, fZ, &pickupGTAId);

	int lol = 32 * (uint16_t)pickupGTAId;
	if(lol) {
		lol /= 32;
	}
	if(unk) {
		*unk = lol;
	}

	return pickupGTAId;
}

void CGame::InitInMenu()
{
	Log("CGame: InitInMenu");
	
	ApplyPatches();
	InstallHooks();
	LoadSplashTexture();

	wszGameTextGXT = new uint16_t[0xFF];
	wszTextDrawGXT = new uint16_t[MAX_TEXT_DRAW_LINE];
}

void CGame::InitInGame()
{
	Log("CGame: InitInGame");

	ApplyInGamePatches();
	InitScripting();
	
	GameAimSyncInit();
	GameResetRadarColors();
}

float CGame::FindGroundZForCoord(float x, float y, float z)
{
	float fGroundZ;
	ScriptCommand(&get_ground_z, x, y, z, &fGroundZ);
	return fGroundZ;
}

void CGame::SetCheckpointInformation(VECTOR *pos, VECTOR *extent)
{
	memcpy(&m_vecCheckpointPos, pos, sizeof(VECTOR));
	memcpy(&m_vecCheckpointExtent, extent, sizeof(VECTOR));

	if(m_dwCheckpointMarker) {
		DisableMarker(m_dwCheckpointMarker);
		m_dwCheckpointMarker = CreateRadarMarkerIcon(0, m_vecCheckpointPos.X, m_vecCheckpointPos.Y, m_vecCheckpointPos.Z, 1005, 0);
	}
}

void CGame::SetRaceCheckpointInformation(uint8_t byteType, VECTOR *pos, VECTOR *next, float fSize)
{
	memcpy(&m_vecRaceCheckpointPos, pos, sizeof(VECTOR));
	memcpy(&m_vecRaceCheckpointNext, next, sizeof(VECTOR));

	m_fRaceCheckpointSize = fSize;
	m_byteRaceType = byteType;

	if(m_dwRaceCheckpointMarker) {
		DisableMarker(m_dwRaceCheckpointMarker);
		m_dwRaceCheckpointMarker = CreateRadarMarkerIcon(0, m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z, 1005, 0);
	}

	MakeRaceCheckpoint();
}

void CGame::MakeRaceCheckpoint()
{
	DisableRaceCheckpoint();
	ScriptCommand(&create_racing_checkpoint, (int)m_byteRaceType, m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z, m_vecRaceCheckpointNext.X, m_vecRaceCheckpointNext.Y, m_vecRaceCheckpointNext.Z, m_fRaceCheckpointSize, &m_dwRaceCheckpointHandle);
	m_bRaceCheckpointsEnabled = true;
}

void CGame::DisableRaceCheckpoint()
{
	if (m_dwRaceCheckpointHandle) {
		ScriptCommand(&destroy_racing_checkpoint, m_dwRaceCheckpointHandle);
		m_dwRaceCheckpointHandle = NULL;
	}
	m_bRaceCheckpointsEnabled = false;
}

void CGame::UpdateCheckpoints()
{
	if(m_bCheckpointsEnabled) {
		CPlayerPed *pPlayerPed = this->FindPlayerPed();
		if(pPlayerPed) {
			ScriptCommand(&is_actor_near_point_3d, pPlayerPed->m_dwGTAId, m_vecCheckpointPos.X, m_vecCheckpointPos.Y, m_vecCheckpointPos.Z, m_vecCheckpointExtent.X, m_vecCheckpointExtent.Y, m_vecCheckpointExtent.Z, 1);

			if (!m_dwCheckpointMarker) {
				m_dwCheckpointMarker = CreateRadarMarkerIcon(0, m_vecCheckpointPos.X, m_vecCheckpointPos.Y, m_vecCheckpointPos.Z, 1005, 0);
			}
		}
	} else if(m_dwCheckpointMarker) {
		DisableMarker(m_dwCheckpointMarker);
		m_dwCheckpointMarker = 0;
	}

	if(m_bRaceCheckpointsEnabled) {
		CPlayerPed *pPlayerPed = this->FindPlayerPed();
		if(pPlayerPed) {
			if (!m_dwRaceCheckpointMarker) {
				m_dwRaceCheckpointMarker = CreateRadarMarkerIcon(0, m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z, 1005, 0);
			}
		}
	} else if(m_dwRaceCheckpointMarker) {
		DisableMarker(m_dwRaceCheckpointMarker);
		DisableRaceCheckpoint();
		m_dwRaceCheckpointMarker = 0;
	}
}

uint32_t CGame::CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor, int iStyle)
{
	uint32_t dwMarkerID = 0;

	if(iStyle == 1) {
		ScriptCommand(&create_marker_icon, fX, fY, fZ, iMarkerType, &dwMarkerID);
	} else if(iStyle == 2) {
		ScriptCommand(&create_radar_marker_icon, fX, fY, fZ, iMarkerType, &dwMarkerID);
	} else if(iStyle == 3) {
		ScriptCommand(&create_icon_marker_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
	} else {
		ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
	}

	if(iMarkerType == 0) {
		if(iColor >= 1004) {
			ScriptCommand(&set_marker_color, dwMarkerID, iColor);
			ScriptCommand(&show_on_radar, dwMarkerID, 3);
		} else {
			ScriptCommand(&set_marker_color, dwMarkerID, iColor);
			ScriptCommand(&show_on_radar, dwMarkerID, 2);
		}
	}

	return dwMarkerID;
}

uint8_t CGame::GetActiveInterior()
{
	uint32_t dwRet;
	ScriptCommand(&get_active_interior, &dwRet);
	return (uint8_t)dwRet;
}

void CGame::SetWorldTime(int iHour, int iMinute)
{
	*(uint8_t *)(g_libGTASA + 0x008B18A4) = (uint8_t)iMinute;
	*(uint8_t *)(g_libGTASA + 0x008B18A5) = (uint8_t)iHour;
	ScriptCommand(&set_current_time, iHour, iMinute);
}

void CGame::SetWorldWeather(unsigned char byteWeatherID)
{
	*(uint8_t *)(g_libGTASA + 0x009DB98E) = byteWeatherID;

	if(!m_bClockEnabled) {
		*(uint16_t *)(g_libGTASA + 0x009DB990) = byteWeatherID;
		*(uint16_t *)(g_libGTASA + 0x009DB992) = byteWeatherID;
	}
}

void CGame::ToggleThePassingOfTime(bool bOnOff)
{
	if(bOnOff) {
		WriteMemory(g_libGTASA + 0x0038C154, (uintptr_t)"\x2D\xE9", 2);
	} else {
		WriteMemory(g_libGTASA + 0x0038C154, (uintptr_t)"\xF7\x46", 2);
	}
}

void CGame::EnableClock(bool bEnable)
{
	char byteClockData[] = { '%', '0', '2', 'd', ':', '%', '0', '2', 'd', 0 };
	UnFuck(g_libGTASA + 0x00599504);

	if(bEnable) {
		ToggleThePassingOfTime(true);
		m_bClockEnabled = true;
		memcpy((void *)(g_libGTASA + 0x00599504), byteClockData, 10);
	} else {
		ToggleThePassingOfTime(false);
		m_bClockEnabled = false;
		memset((void *)(g_libGTASA + 0x00599504), 0, 10);
	}
}

void CGame::EnableZoneNames(bool bEnable)
{
	ScriptCommand(&enable_zone_names, bEnable);
}

void CGame::DisplayWidgets(bool bDisp)
{
	*(uint16_t *)(g_libGTASA + 0x008B82A0 + 0x10C) = (bDisp ? 0 : 1);
}

// допилить
void CGame::PlaySound(int iSound, float fX, float fY, float fZ)
{
	ScriptCommand(&play_sound, fX, fY, fZ, iSound);
}

void CGame::ToggleRadar(bool iToggle)
{
	*(uint8_t *)(g_libGTASA + 0x008EF36B) = (uint8_t)!iToggle;
}

void CGame::DisplayHUD(bool bDisp)
{
	//CTheScripts::bDisplayHud
	if(bDisp) {	
		*(uint8_t *)(g_libGTASA + 0x007165E8) = 1;
		ToggleRadar(1);
	} else {
		*(uint8_t *)(g_libGTASA + 0x007165E8) = 0;
		ToggleRadar(0);
	}
}

void CGame::RequestModel(uint16_t usModelId, uint8_t ucStreamingFlag) {
	if(usModelId < 0 || usModelId > 20000) {
		return;
	}

	//CStreaming::RequestModel(int,int) С 0x0028EB10
	((void (*)(int32_t, int32_t))(g_libGTASA + 0x0028EB10 + 1))(usModelId, ucStreamingFlag);
}

void CGame::RequestVehicleUpgrade(uint16_t usModelId, uint8_t ucStreamingFlag) {
	if(usModelId < 0 || usModelId > 20000) {
		return;
	}
	
	//CStreaming::RequestVehicleUpgrade(int,int) С 0x0028F2F8
	((void (*)(int32_t, int32_t))(g_libGTASA + 0x0028F2F8 + 1))(usModelId, ucStreamingFlag);
}

void CGame::LoadRequestedModels(bool bOnlyPriorityRequests) {
	//CStreaming::LoadAllRequestedModels(bool) С 0x00294CB4
	((void (*)(bool))(g_libGTASA + 0x00294CB4 + 1))(bOnlyPriorityRequests);
}

bool CGame::HasModelLoaded(uint16_t usModelId) {
	if(usModelId < 0 || usModelId > 20000) {
		return false;
	}
	
	if(IS_VEHICLE_MOD(usModelId)) {
		//CStreaming::HasVehicleUpgradeLoaded(int) С 0x0028F328
		return ((bool (*)(int32_t))(g_libGTASA + 0x0028F328 + 1))(usModelId);
	} else {
		/*uintptr_t *pModelInfo = (uintptr_t *)g_libGTASA + 0x0087BF48;
		if(pModelInfo) {
			if(pModelInfo[dwModelID] != 0) {
				if(*(uintptr_t *)(pModelInfo + 0x34) != 0) {
					return true;
				}
			}
		}
		return false;*/
		return ScriptCommand(&is_model_available, (uint32_t)usModelId) ? true : false;
	}
}

void CGame::RefreshStreamingAt(float x, float y)
{
	ScriptCommand(&refresh_streaming_at, x, y);
}

void CGame::DisableTrainTraffic()
{
	ScriptCommand(&enable_train_traffic, 0);
}

void CGame::SetMaxStats()
{
	// CCheat::VehicleSkillsCheat
	((int (*)())(g_libGTASA + 0x002BAED0 + 1))();

	// CCheat::WeaponSkillsCheat
	((int (*)())(g_libGTASA + 0x002BAE68 + 1))();

	// CStats::SetStatValue nop
	WriteMemory(g_libGTASA + 0x003B9074, (uintptr_t)"\xF7\x46", 2);
}

void CGame::SetWantedLevel(uint8_t byteLevel)
{
	WriteMemory(g_libGTASA + 0x0027D8D2, (uintptr_t)&byteLevel, 1);
}

bool CGame::IsAnimationLoaded(char *szAnimFile)
{
	return ScriptCommand(&is_animation_loaded, szAnimFile);
}

void CGame::RequestAnimation(char *szAnimFile)
{
	ScriptCommand(&request_animation, szAnimFile);
}

void CGame::DisplayGameText(char *szStr, int iTime, int iType)
{
	ScriptCommand(&text_clear_all);

	memset(wszGameTextGXT, 0, sizeof(wszGameTextGXT));
	CFont::AsciiToGxtChar(szStr, wszGameTextGXT);

	// CMessages::AddBigMesssage
	((void (*)(uint16_t *, int, int))(g_libGTASA + 0x004D18C0 + 1))(wszGameTextGXT, iTime, iType);
}

void CGame::SetGravity(float fGravity)
{
	UnFuck(g_libGTASA + 0x003A0B64);
	*(float *)(g_libGTASA + 0x003A0B64) = fGravity;
}

void CGame::ToggleCJWalk(bool bUseCJWalk)
{
	if(bUseCJWalk) {
		WriteMemory(g_libGTASA + 0x0045477E, (uintptr_t)"\xC4\xF8\xDC\x64", 4);
	} else {
		NOP(g_libGTASA + 0x0045477E, 2);
	}
}

void CGame::DisableMarker(uint32_t dwMarkerID)
{
	ScriptCommand(&disable_marker, dwMarkerID);
}

// 0.3.7
int CGame::GetLocalMoney()
{
	return *(int*)(PLAYERS_REALLOC+0xB8);
}

// 0.3.7
void CGame::AddToLocalMoney(int iAmmount)
{
	ScriptCommand(&add_to_player_money, 0, iAmmount);
}

// 0.3.7
void CGame::ResetLocalMoney()
{
	int iMoney = GetLocalMoney();
	if(!iMoney) return;

	if(iMoney < 0)
		AddToLocalMoney(abs(iMoney));
	else
		AddToLocalMoney(-(iMoney));
}

void CGame::DisableInteriorEnterExits()
{
	uintptr_t addr = *(uintptr_t *)(g_libGTASA + 0x00700120);
	uint32_t count = *(uint32_t *)(addr + 0x8);
	addr = *(uintptr_t *)addr;
	for(int i = 0; i < count; i++) {
		*(uint16_t *)(addr + 0x30) = 0;
		addr += 0x3C;
	}
}

extern uint8_t bGZ;
void CGame::DrawGangZone(float fPos[], uint32_t dwColor)
{
	((void (*)(float *, uint32_t *, uint8_t))(g_libGTASA + 0x003DE7F8 + 1))(fPos, &dwColor, bGZ);
}

int CGame::GetScreenWidth() {
	int retn_RwEngineGetCurrentVideoMode = ((int (*)(void))(g_libGTASA + 0x001AE184 + 1))();

	RwVideoMode mode;
	((void (*)(RwVideoMode *, int))(g_libGTASA + 0x001AE154 + 1))(&mode, retn_RwEngineGetCurrentVideoMode);

	return mode.width;
}

int CGame::GetScreenHeight() {
	int retn_RwEngineGetCurrentVideoMode = ((int (*)(void))(g_libGTASA + 0x001AE184 + 1))();

	RwVideoMode mode;
	((void (*)(RwVideoMode *, int))(g_libGTASA + 0x001AE154 + 1))(&mode, retn_RwEngineGetCurrentVideoMode);

	return mode.height;
}

void CGame::RemoveBuilding(uint32_t dwModel, VECTOR vecPosition, float fRange)
{
	OBJECT_REMOVE objectToRemove;
	if(dwModel != -1)
	{
		objectToRemove.dwModel = dwModel;
		if(vecPosition.X != -1 || vecPosition.Y != -1 || vecPosition.Z != -1)
		{
			objectToRemove.vecPosition = vecPosition;
			if(fRange != -1)
			{
				objectToRemove.fRange = fRange;

				m_vecObjectToRemove.push_back(objectToRemove);
				
				ScriptCommand(&create_static_object, vecPosition.X, vecPosition.Y, vecPosition.Z, fRange, (uint16_t)dwModel, 0);
			}
		}
	}
}

void CGame::SetVehicleNumberPlate(uint16_t vehicleId, char *szPlate)
{
	if(vehicleId > 0 || vehicleId <= MAX_VEHICLES)
	{
		if(strlen(szPlate) != -1)
		{
			ScriptCommand(&set_car_numberplate, vehicleId, szPlate);
		}
	}
}

void CGame::CreateExplosion(VECTOR vecPosition, int16_t iType, float fRadius)
{
	if(vecPosition.X != -1 || vecPosition.Y != -1 || vecPosition.Z != -1)
	{
		if(iType != -1)
		{
			if(fRadius != -1)
			{
				ScriptCommand(&create_explosion_with_radius, vecPosition.X, vecPosition.Y, vecPosition.Z, iType, fRadius);
			}
		}
	}		
}

bool CGame::IsLineOfSightClear(VECTOR const& origin, VECTOR const& target, bool buildings, bool vehicles, bool peds, bool objects, bool dummies, bool doSeeThroughCheck, bool doCameraIgnoreCheck) {
	if(!IsValidPosition(origin)) {
		return false;
	}
	
	if(!IsValidPosition(target)) {
		return false;
	}
	
	// CWorld::GetIsLineOfSightClear(CVector const&, CVector const&, bool, bool, bool, bool, bool, bool, bool) — 0x003C7EEC
	return ((bool (*)(VECTOR const&, VECTOR const&, bool, bool, bool, bool, bool, bool, bool))(g_libGTASA + 0x003C7EEC + 1))(
		origin,
		target,
		buildings,
		vehicles,
		peds,
		objects,
		dummies,
		doSeeThroughCheck,
		doCameraIgnoreCheck
	);
}