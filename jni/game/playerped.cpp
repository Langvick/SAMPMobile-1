#include "../main.h"
#include "game.h"
#include "net/netgame.h"
#include "util/armhook.h"

extern CGame* pGame;
extern CNetGame *pNetGame;

CPlayerPed::CPlayerPed()
{
	m_dwGTAId = 1;
	m_pPed = (PED_TYPE*)GamePool_FindPlayerPed();
	m_pEntity = (ENTITY_TYPE*)GamePool_FindPlayerPed();

	m_bytePlayerNumber = 0;
	SetPlayerPedPtrRecord(m_bytePlayerNumber,(uintptr_t)m_pPed);
	ScriptCommand(&set_actor_weapon_droppable, m_dwGTAId, 1);
	ScriptCommand(&set_actor_can_be_decapitated, m_dwGTAId, 0);

	m_dwArrow = 0;
	m_bHaveBulletData = false;
	memset(&m_bulletData, 0, sizeof(m_bulletData));
}

CPlayerPed::CPlayerPed(uint8_t bytePlayerNumber, int iSkin, float fX, float fY, float fZ, float fRotation)
{
	uint32_t dwPlayerActorID = 0;
	int iPlayerNum = bytePlayerNumber;

	m_pPed = nullptr;
	m_dwGTAId = 0;

	ScriptCommand(&create_player, &iPlayerNum, fX, fY, fZ, &dwPlayerActorID);
	ScriptCommand(&create_actor_from_player, &iPlayerNum, &dwPlayerActorID);

	m_dwGTAId = dwPlayerActorID;
	m_pPed = GamePool_Ped_GetAt(m_dwGTAId);
	m_pEntity = (ENTITY_TYPE*)GamePool_Ped_GetAt(m_dwGTAId);

	m_bytePlayerNumber = bytePlayerNumber;
	SetPlayerPedPtrRecord(m_bytePlayerNumber, (uintptr_t)m_pPed);
	ScriptCommand(&set_actor_weapon_droppable, m_dwGTAId, 1);
	ScriptCommand(&set_actor_immunities, m_dwGTAId, 0, 0, 1, 0, 0);
	ScriptCommand(&set_actor_can_be_decapitated, m_dwGTAId, 0);

	if(pNetGame)
		SetMoney(pNetGame->m_iDeathDropMoney);

	SetModelIndex(iSkin);
	ForceTargetRotation(fRotation);

	MATRIX4X4 mat;
	GetMatrix(&mat);
	mat.pos.X = fX;
	mat.pos.Y = fY;
	mat.pos.Z = fZ + 0.15f;
	SetMatrix(mat);

	m_bHaveBulletData = false;
	memset(&m_bulletData, 0, sizeof(BULLET_DATA));
	memset(&RemotePlayerKeys[m_bytePlayerNumber], 0, sizeof(PAD_KEYS));
}

CPlayerPed::~CPlayerPed()
{
	Destroy();
}

void CPlayerPed::Destroy()
{
	memset(&RemotePlayerKeys[m_bytePlayerNumber], 0, sizeof(PAD_KEYS));
	SetPlayerPedPtrRecord(m_bytePlayerNumber, 0);

	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId) || m_pPed->entity.vtable == 0x5C7358)
	{
		Log("CPlayerPed::Destroy: invalid pointer/vtable");
		m_pPed = nullptr;
		m_pEntity = nullptr;
		m_dwGTAId = 0;
		return;
	}

	/*
		if(m_dwParachute) ... (допилить)
	*/

	Log("Removing from vehicle..");
	if(IN_VEHICLE(m_pPed))
		RemoveFromVehicleAndPutAt(100.0f, 100.0f, 10.0f);

	Log("Setting flag state..");
	uintptr_t dwPedPtr = (uintptr_t)m_pPed;
	*(uint32_t*)(*(uintptr_t*)(dwPedPtr + 1088) + 76) = 0;
	// CPlayerPed::Destructor
	Log("Calling destructor..");
	(( void (*)(PED_TYPE*))(*(void**)(m_pPed->entity.vtable+0x4)))(m_pPed);

	m_pPed = nullptr;
	m_pEntity = nullptr;
}

// 0.3.7
bool CPlayerPed::IsInVehicle()
{
	if(!m_pPed) return false;

	if(IN_VEHICLE(m_pPed))
		return true;

	return false;
}

// 0.3.7
bool CPlayerPed::IsAPassenger()
{
	if(m_pPed->pVehicle && IN_VEHICLE(m_pPed))
	{
		VEHICLE_TYPE *pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle;

		if(	pVehicle->m_pDriver != m_pPed ||
			pVehicle->entity.nModelIndex == TRAIN_PASSENGER ||
			pVehicle->entity.nModelIndex == TRAIN_FREIGHT )
			return true;
	}

	return false;
}

// 0.3.7
VEHICLE_TYPE* CPlayerPed::GetGtaVehicle()
{
	return (VEHICLE_TYPE*)m_pPed->pVehicle;
}

// 0.3.7
void CPlayerPed::RemoveFromVehicleAndPutAt(float fX, float fY, float fZ)
{
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(m_pPed && IN_VEHICLE(m_pPed))
		ScriptCommand(&remove_actor_from_car_and_put_at, m_dwGTAId, fX, fY, fZ);
}

// 0.3.7
void CPlayerPed::SetInitialState()
{
	(( void (*)(PED_TYPE*))(g_libGTASA+0x458D1C+1))(m_pPed);
}

// 0.3.7
void CPlayerPed::SetHealth(float fHealth)
{
	if(!m_pPed) return;
	m_pPed->fHealth = fHealth;
}

// 0.3.7
float CPlayerPed::GetHealth()
{
	if(!m_pPed) return 0.0f;
	return m_pPed->fHealth;
}

// 0.3.7
void CPlayerPed::SetArmour(float fArmour)
{
	if(!m_pPed) return;
	m_pPed->fArmour = fArmour;
}

float CPlayerPed::GetArmour()
{
	if(!m_pPed) return 0.0f;
	return m_pPed->fArmour;
}

void CPlayerPed::SetInterior(uint8_t byteID)
{
	if(!m_pPed) return;

	ScriptCommand(&select_interior, byteID);
	ScriptCommand(&link_actor_to_interior, m_dwGTAId, byteID);

	MATRIX4X4 mat;
	GetMatrix(&mat);
	ScriptCommand(&refresh_streaming_at, mat.pos.X, mat.pos.Y);
}

void CPlayerPed::PutDirectlyInVehicle(int iVehicleID, int iSeat)
{
	if(!m_pPed) return;
	if(!GamePool_Vehicle_GetAt(iVehicleID)) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	VEHICLE_TYPE *pVehicle = GamePool_Vehicle_GetAt(iVehicleID);

	if(pVehicle->m_fHealth == 0.0f) return;
	// check is cplaceable
	if (pVehicle->entity.vtable == g_libGTASA+0x5C7358) return;
	// check seatid (допилить)

	if(iSeat == 0)
	{
		if(pVehicle->m_pDriver && IN_VEHICLE(pVehicle->m_pDriver)) return;
		ScriptCommand(&put_actor_in_car, m_dwGTAId, iVehicleID);
	}
	else
	{
		iSeat--;
		ScriptCommand(&put_actor_in_car2, m_dwGTAId, iVehicleID, iSeat);
	}

	if(m_pPed == GamePool_FindPlayerPed() && IN_VEHICLE(m_pPed))
		pGame->GetCamera()->SetBehindPlayer();

	if(pNetGame)
	{
		// допилить (трейлеры)
	}
}

void CPlayerPed::EnterVehicle(int iVehicleID, bool bPassenger)
{
	if(!m_pPed) return;
	VEHICLE_TYPE* ThisVehicleType;
	if((ThisVehicleType = GamePool_Vehicle_GetAt(iVehicleID)) == 0) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(bPassenger)
	{
		if(ThisVehicleType->entity.nModelIndex == TRAIN_PASSENGER &&
			(m_pPed == GamePool_FindPlayerPed()))
		{
			ScriptCommand(&put_actor_in_car2, m_dwGTAId, iVehicleID, -1);
		}
		else
		{
			ScriptCommand(&send_actor_to_car_passenger,m_dwGTAId,iVehicleID, 3000, -1);
		}
	}
	else
		ScriptCommand(&send_actor_to_car_driverseat, m_dwGTAId, iVehicleID, 3000);
}

// 0.3.7
void CPlayerPed::ExitCurrentVehicle()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	VEHICLE_TYPE* ThisVehicleType = 0;

	if(IN_VEHICLE(m_pPed))
	{
		if(GamePool_Vehicle_GetIndex((VEHICLE_TYPE*)m_pPed->pVehicle))
		{
			int index = GamePool_Vehicle_GetIndex((VEHICLE_TYPE*)m_pPed->pVehicle);
			ThisVehicleType = GamePool_Vehicle_GetAt(index);
			if(ThisVehicleType)
			{
				if(	ThisVehicleType->entity.nModelIndex != TRAIN_PASSENGER &&
					ThisVehicleType->entity.nModelIndex != TRAIN_PASSENGER_LOCO)
				{
					ScriptCommand(&make_actor_leave_car, m_dwGTAId, GetCurrentVehicleID());
				}
			}
		}
	}
}

// 0.3.7
int CPlayerPed::GetCurrentVehicleID()
{
	if(!m_pPed) return 0;

	VEHICLE_TYPE *pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle;
	return GamePool_Vehicle_GetIndex(pVehicle);
}

int CPlayerPed::GetVehicleSeatID()
{
	VEHICLE_TYPE *pVehicle;

	if( GetActionTrigger() == ACTION_INCAR && (pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle) != 0 ) 
	{
		if(pVehicle->m_pDriver == m_pPed) return 0;
		if(pVehicle->m_apPassengers[0] == m_pPed) return 1;
		if(pVehicle->m_apPassengers[1] == m_pPed) return 2;
		if(pVehicle->m_apPassengers[2] == m_pPed) return 3;
		if(pVehicle->m_apPassengers[3] == m_pPed) return 4;
		if(pVehicle->m_apPassengers[4] == m_pPed) return 5;
		if(pVehicle->m_apPassengers[5] == m_pPed) return 6;
		if(pVehicle->m_apPassengers[6] == m_pPed) return 7;
	}

	return (-1);
}

// 0.3.7
void CPlayerPed::TogglePlayerControllable(bool bToggle)
{
	MATRIX4X4 mat;

	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(!bToggle)
	{
		ScriptCommand(&toggle_player_controllable, m_bytePlayerNumber, 0);
		ScriptCommand(&lock_actor, m_dwGTAId, 1);
	}
	else
	{
		ScriptCommand(&toggle_player_controllable, m_bytePlayerNumber, 1);
		ScriptCommand(&lock_actor, m_dwGTAId, 0);
		if(!IsInVehicle()) 
		{
			GetMatrix(&mat);
			TeleportTo(mat.pos);
		}
	}
}

// 0.3.7
void CPlayerPed::SetModelIndex(unsigned int uiModel)
{
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(!IsPedModel(uiModel))
		uiModel = 0;

	if(m_pPed)
	{
		// CClothes::RebuildPlayer nulled
		WriteMemory(g_libGTASA+0x3F1030, (uintptr_t)"\x70\x47", 2);
		DestroyFollowPedTask();
		CEntity::SetModelIndex(uiModel);

		// reset the Ped Audio Attributes
		(( void (*)(uintptr_t, uintptr_t))(g_libGTASA+0x34B2A8+1))(((uintptr_t)m_pPed+660), (uintptr_t)m_pPed);
	}
}

// допилить
void CPlayerPed::DestroyFollowPedTask()
{

}

void CPlayerPed::ClearAllWeapons() {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	//CPed::ClearWeapons(void) — 0x004345AC
	((void (*)(uintptr_t))(g_libGTASA + 0x004345AC + 1))((uintptr_t)m_pPed);
}

// допилить
void CPlayerPed::ResetDamageEntity()
{

}

// 0.3.7
void CPlayerPed::RestartIfWastedAt(VECTOR *vecRestart, float fRotation)
{	
	ScriptCommand(&restart_if_wasted_at, vecRestart->X, vecRestart->Y, vecRestart->Z, fRotation, 0);
}

// 0.3.7
void CPlayerPed::ForceTargetRotation(float fRotation)
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	m_pPed->fRotation1 = DegToRad(fRotation);
	m_pPed->fRotation2 = DegToRad(fRotation);

	ScriptCommand(&set_actor_z_angle,m_dwGTAId,fRotation);
}

void CPlayerPed::SetRotation(float fRotation)
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	m_pPed->fRotation1 = DegToRad(fRotation);
	m_pPed->fRotation2 = DegToRad(fRotation);
}

// 0.3.7
uint8_t CPlayerPed::GetActionTrigger()
{
	return (uint8_t)m_pPed->dwAction;
}

// 0.3.7
bool CPlayerPed::IsDead()
{
	if(!m_pPed) return true;
	if(m_pPed->fHealth > 0.0f) return false;
	return true;
}

void CPlayerPed::SetMoney(int iAmount)
{
	ScriptCommand(&set_actor_money, m_dwGTAId, 0);
	ScriptCommand(&set_actor_money, m_dwGTAId, iAmount);
}

// 0.3.7
void CPlayerPed::ShowMarker(uint32_t iMarkerColorID)
{
	if(m_dwArrow) HideMarker();
	ScriptCommand(&create_arrow_above_actor, m_dwGTAId, &m_dwArrow);
	ScriptCommand(&set_marker_color, m_dwArrow, iMarkerColorID);
	ScriptCommand(&show_on_radar2, m_dwArrow, 2);
}

// 0.3.7
void CPlayerPed::HideMarker()
{
	if(m_dwArrow) ScriptCommand(&disable_marker, m_dwArrow);
	m_dwArrow = 0;
}

// 0.3.7
void CPlayerPed::SetFightingStyle(int iStyle)
{
	if(!m_pPed) return;
	ScriptCommand( &set_fighting_style, m_dwGTAId, iStyle, 6 );
}

// 0.3.7
void CPlayerPed::ApplyAnimation( char *szAnimName, char *szAnimFile, float fT,
								 int opt1, int opt2, int opt3, int opt4, int iUnk )
{
	int iWaitAnimLoad = 0;

	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(!strcasecmp(szAnimFile,"SEX")) return;

	if(!pGame->IsAnimationLoaded(szAnimFile))
	{
		pGame->RequestAnimation(szAnimFile);
		while(!pGame->IsAnimationLoaded(szAnimFile))
		{
			usleep(1000);
			iWaitAnimLoad++;
			if(iWaitAnimLoad > 15) return;
		}
	}

	ScriptCommand(&apply_animation, m_dwGTAId, szAnimName, szAnimFile, fT, opt1, opt2, opt3, opt4, iUnk);
}

void CPlayerPed::FindDeathReasonAndResponsiblePlayer(uint16_t *nPlayer)
{
	CPlayerPool *pPlayerPool;
	CVehiclePool *pVehiclePool;
	uint16_t PlayerIDWhoKilled 	 = INVALID_PLAYER_ID;
	
	if(pNetGame) 
	{
		pVehiclePool = pNetGame->GetVehiclePool();
		pPlayerPool = pNetGame->GetPlayerPool();
	}
	else 
	{ // just leave if there's no netgame.
		*nPlayer = INVALID_PLAYER_ID;
		return;
	}

	if(m_pPed)
	{
		if(m_pPed->pdwDamageEntity)
		{
			PlayerIDWhoKilled = pPlayerPool->FindRemotePlayerIDFromGtaPtr((PED_TYPE *)m_pPed->pdwDamageEntity);
			if(PlayerIDWhoKilled != INVALID_PLAYER_ID) 
			{
					// killed by another player with a weapon, this is all easy.
					*nPlayer = PlayerIDWhoKilled;
					return;
			}
			else
			{
				if(pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)m_pPed->pdwDamageEntity) != INVALID_VEHICLE_ID) 
				{
					VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *)m_pPed->pdwDamageEntity;
					PlayerIDWhoKilled = pPlayerPool->FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pGtaVehicle->m_pDriver);
												
					if(PlayerIDWhoKilled != INVALID_PLAYER_ID) 
					{
						*nPlayer = PlayerIDWhoKilled;
						return;
					}
				}
			}
		}
	}

	*nPlayer = INVALID_PLAYER_ID;
}

// 0.3.7
void CPlayerPed::GetBonePosition(int iBoneID, VECTOR* vecOut)
{
	if(!m_pPed) return;
	if(m_pEntity->vtable == g_libGTASA+0x5C7358) return;

	(( void (*)(PED_TYPE*, VECTOR*, int, int))(g_libGTASA+0x436590+1))(m_pPed, vecOut, iBoneID, 0);
}

void CPlayerPed::GetBoneMatrix(MATRIX4X4 *matOut, int iBoneId) {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	uintptr_t animHierarchy = ((uintptr_t (*)(RpClump *))(g_libGTASA + 0x00559338 + 1))(m_pEntity->m_pRpClump);

	int iAnimIndex = ((uintptr_t (*)(uintptr_t, uintptr_t))(g_libGTASA + 0x0019A448 + 1))(animHierarchy, iBoneId) << 6;

	MATRIX4X4 *tempMatOut = (MATRIX4X4 *)(iAnimIndex + *(uintptr_t *)(animHierarchy + 8));
	if(tempMatOut) {
		memcpy(matOut, tempMatOut, sizeof(MATRIX4X4));
	}
}

// допилить
uint16_t CPlayerPed::GetKeys(uint16_t *lrAnalog, uint16_t *udAnalog, uint8_t *additionalKey) {
	*lrAnalog = LocalPlayerKeys.wKeyLR;
	*udAnalog = LocalPlayerKeys.wKeyUD;

	uint16_t wRet = 0;

	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_YES]) {
		*additionalKey = 0b01;
	}

	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_NO]) {
		*additionalKey = 0b10;
	}
	
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK]) {
		*additionalKey = 0b11;
	}

	// KEY_ANALOG_RIGHT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_RIGHT]) wRet |= 1;
	wRet <<= 1;
	// KEY_ANALOG_LEFT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_LEFT]) wRet |= 1;
	wRet <<= 1;
	// KEY_ANALOG_DOWN
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_DOWN]) wRet |= 1;
	wRet <<= 1;
	// KEY_ANALOG_UP
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_UP]) wRet |= 1;
	wRet <<= 1;
	// KEY_WALK
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK]) wRet |= 1;
	wRet <<= 1;
	// KEY_SUBMISSION
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION]) wRet |= 1;
	wRet <<= 1;
	// KEY_WALK
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK]) wRet |= 1;
	wRet <<= 1;
	// KEY_SUBMISSION
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION]) wRet |= 1;
	wRet <<= 1;
	// KEY_LOOK_LEFT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_LOOK_LEFT]) wRet |= 1;
	wRet <<= 1;
	// KEY_HANDBRAKE
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE]) wRet |= 1;
	wRet <<= 1;
	// KEY_LOOK_RIGHT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_LOOK_RIGHT]) wRet |= 1;
	wRet <<= 1;
	// KEY_JUMP
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP]) wRet |= 1;
	wRet <<= 1;
	// KEY_SECONDARY_ATTACK
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK]) wRet |= 1;
	wRet <<= 1;
	// KEY_SPRINT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT]) wRet |= 1;
	wRet <<= 1;
	// KEY_FIRE
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE]) wRet |= 1;
	wRet <<= 1;
	// KEY_CROUCH
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH]) wRet |= 1;
	wRet <<= 1;
	// KEY_ACTION
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ACTION]) wRet |= 1;

	memset(LocalPlayerKeys.bKeys, 0, ePadKeys::SIZE);

	return wRet;
}

void CPlayerPed::SetKeys(uint16_t wKeys, uint16_t lrAnalog, uint16_t udAnalog)
{
	PAD_KEYS *pad = &RemotePlayerKeys[m_bytePlayerNumber];

	// LEFT/RIGHT
	pad->wKeyLR = lrAnalog;
	// UP/DOWN
	pad->wKeyUD = udAnalog;

	// KEY_ACTION
	pad->bKeys[ePadKeys::KEY_ACTION] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_CROUCH
	pad->bKeys[ePadKeys::KEY_CROUCH] = (wKeys & 1);
	if(!pad->bKeys[ePadKeys::KEY_CROUCH]) {
		pad->bIgnoreCrouch = false;
	}
	wKeys >>= 1;
	// KEY_FIRE
	pad->bKeys[ePadKeys::KEY_FIRE] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_SPRINT
	pad->bKeys[ePadKeys::KEY_SPRINT] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_SECONDARY_ATTACK
	pad->bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_JUMP
	pad->bKeys[ePadKeys::KEY_JUMP] = (wKeys & 1);
	if(!pad->bKeys[ePadKeys::KEY_JUMP]) pad->bIgnoreJump = false;
	wKeys >>= 1;
	// KEY_LOOK_RIGHT
	pad->bKeys[ePadKeys::KEY_LOOK_RIGHT] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_HANDBRAKE
	pad->bKeys[ePadKeys::KEY_HANDBRAKE] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_LOOK_LEFT
	pad->bKeys[ePadKeys::KEY_LOOK_LEFT] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_SUBMISSION
	pad->bKeys[ePadKeys::KEY_SUBMISSION] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_WALK
	pad->bKeys[ePadKeys::KEY_WALK] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_ANALOG_UP
	pad->bKeys[ePadKeys::KEY_ANALOG_UP] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_ANALOG_DOWN
	pad->bKeys[ePadKeys::KEY_ANALOG_DOWN] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_ANALOG_LEFT
	pad->bKeys[ePadKeys::KEY_ANALOG_LEFT] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_ANALOG_RIGHT
	pad->bKeys[ePadKeys::KEY_ANALOG_RIGHT] = (wKeys & 1);

	return;
}

void CPlayerPed::SetAmmo(int iWeapon, int iAmmo) {
	WEAPON_SLOT_TYPE *pWeaponSlot = FindWeaponSlot(iWeapon);
	if(pWeaponSlot) {
		pWeaponSlot->dwAmmo = iAmmo;
	}
}

WEAPON_SLOT_TYPE *CPlayerPed::FindWeaponSlot(int iWeapon) {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return nullptr;
	}

	for(int i = 0; i < 13; i++) {
		if(m_pPed->WeaponSlots[i].dwType == iWeapon) {
			return &m_pPed->WeaponSlots[i];
		}
	}
	return nullptr;
}

void CPlayerPed::GiveWeapon(int iWeapon, int iAmmo) {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}

	uint32_t uiWeaponModel = GameGetWeaponModelIDFromWeaponID(iWeapon);
	if(uiWeaponModel != -1) {
		if(!pGame->HasModelLoaded(uiWeaponModel)) {
			pGame->RequestModel(uiWeaponModel, GAME_REQUIRED);
			pGame->LoadRequestedModels(false);
		}

		// CPed::GiveWeapon
		((void (*)(PED_TYPE *, int, int, bool))(g_libGTASA + 0x0043429C + 1))(m_pPed, iWeapon, iAmmo, true);

		SetArmedWeapon(iWeapon);
	}
}

void CPlayerPed::SetArmedWeapon(int iWeapon) {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}

	ScriptCommand(&set_actor_armed_weapon, m_dwGTAId, iWeapon);
}

uint8_t CPlayerPed::GetCurrentWeapon() {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return 0;
	}

	int dwRetVal = 0;
	ScriptCommand(&get_actor_armed_weapon, m_dwGTAId, &dwRetVal);

	return dwRetVal;
}

CAMERA_AIM *CPlayerPed::GetCurrentAim() {
	return GameGetInternalAim();
}

uint8_t CPlayerPed::GetCameraMode() {
	if(m_bytePlayerNumber == 0) {
		return GameGetLocalPlayerCameraMode();
	}
	else {
		return GameGetPlayerCameraMode(m_bytePlayerNumber);
	}
}

WEAPON_SLOT_TYPE *CPlayerPed::GetCurrentWeaponSlot() {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return nullptr;
	}
	
	return &m_pPed->WeaponSlots[m_pPed->byteCurWeaponSlot];
}

float CPlayerPed::GetAimZ()
{
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return 0.0f;
	}
	
	return *(float *)(m_pPed->dwPlayerInfoOffset + 0x54);
}

void CPlayerPed::SetCameraMode(uint8_t byteCamMode) {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	GameSetPlayerCameraMode(byteCamMode, m_bytePlayerNumber);
}

void CPlayerPed::SetCurrentAim(CAMERA_AIM *pAim) {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	GameStoreRemotePlayerAim(m_bytePlayerNumber, pAim);
}

void CPlayerPed::SetAimZ(float fAimZ)
{
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	if(!isnan(fAimZ)) {
		if(fAimZ <= 100.0 && fAimZ >= -100.0) {
			*(float*)(m_pPed->dwPlayerInfoOffset + 0x54) = fAimZ;
		}
	}
}

void CPlayerPed::SetCameraExtendedZoom(float fExtZoom, float fAspect) {
	GameSetPlayerCameraExtZoom(m_bytePlayerNumber, fExtZoom, fAspect);
}

CPlayerPed *g_pCurrentFiredPed = nullptr;
BULLET_DATA *g_pCurrentBulletData = nullptr;
extern uint32_t (*CWeapon__FireInstantHit)(WEAPON_SLOT_TYPE* thiz, PED_TYPE* pFiringEntity, VECTOR* vecOrigin, VECTOR* muzzlePosn, ENTITY_TYPE* targetEntity, VECTOR *target, VECTOR* originForDriveBy, int arg6, int muzzle);
void CPlayerPed::FireInstant() {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	uint8_t byteCameraMode;
	if(m_bytePlayerNumber != 0) {
		byteCameraMode = *pbyteCameraMode;
		*pbyteCameraMode = GameGetPlayerCameraMode(m_bytePlayerNumber);

		// wCameraMode2
		GameStoreLocalPlayerCameraExtZoom();
		GameSetRemotePlayerCameraExtZoom(m_bytePlayerNumber);

		GameStoreLocalPlayerAim();
		GameSetRemotePlayerAim(m_bytePlayerNumber);
	}

	g_pCurrentFiredPed = this;

	if(m_bHaveBulletData) {
		g_pCurrentBulletData = &m_bulletData;
	} else {
		g_pCurrentBulletData = nullptr;
	}

	WEAPON_SLOT_TYPE *pSlot = GetCurrentWeaponSlot();
	if(pSlot) {
		if(GetCurrentWeapon() == WEAPON_SNIPER) {
			if(pSlot) {
				Weapon_FireSniper(pSlot, m_pPed);
			} else {
				Weapon_FireSniper(nullptr, nullptr);
			}
		} else {
			VECTOR vecBonePos;
			VECTOR vecOut;

			GetWeaponInfoForFire(false, &vecBonePos, &vecOut);
			CWeapon__FireInstantHit(pSlot, m_pPed, &vecBonePos, &vecOut, nullptr, nullptr, nullptr, 0, 1);
		}
	}

	g_pCurrentFiredPed = nullptr;
	g_pCurrentBulletData = nullptr;

	if(m_bytePlayerNumber != 0) {
		*pbyteCameraMode = byteCameraMode;

		// wCamera2
		GameSetLocalPlayerCameraExtZoom();
		GameSetLocalPlayerAim();
	}
}

void CPlayerPed::GetWeaponInfoForFire(int bLeft, VECTOR *vecBone, VECTOR *vecOut)
{
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	if(IsGameEntityArePlaceable(&m_pPed->entity)) {
		return;
	}
	
	VECTOR *pFireOffset = GetCurrentWeaponFireOffset();
	if(pFireOffset && vecBone && vecOut) {
		vecOut->X = pFireOffset->X;
		vecOut->Y = pFireOffset->Y;
		vecOut->Z = pFireOffset->Z;

		int bone_id = 24;
		if(bLeft) {
			bone_id = 34;
		}
	
		// CPed::GetBonePosition
		((void (*)(PED_TYPE *, VECTOR *, int, bool))(g_libGTASA + 0x436590 + 1))(m_pPed, vecBone, bone_id, false);

		vecBone->Z += pFireOffset->Z + 0.15f;

		// CPed::GetTransformedBonePosition
		((void (*)(PED_TYPE *, VECTOR *, int, bool))(g_libGTASA + 0x4383C0 + 1))(m_pPed, vecOut, bone_id, false);
	}
}

VECTOR* CPlayerPed::GetCurrentWeaponFireOffset()
{
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return nullptr;
	}

	WEAPON_SLOT_TYPE *pSlot = GetCurrentWeaponSlot();
	if(pSlot) {
		return (VECTOR *)(GetWeaponInfo(pSlot->dwType, 1) + 0x24);
	}
	return nullptr;
}

void CPlayerPed::ProcessBulletData(BULLET_DATA *btData)
{
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	BULLET_SYNC_DATA bulletSyncData;

	if(btData) {
		m_bHaveBulletData = true;
		m_bulletData.pEntity = btData->pEntity;
		m_bulletData.vecOrigin.X = btData->vecOrigin.X;
		m_bulletData.vecOrigin.Y = btData->vecOrigin.Y;
		m_bulletData.vecOrigin.Z = btData->vecOrigin.Z;

		m_bulletData.vecPos.X = btData->vecPos.X;
		m_bulletData.vecPos.Y = btData->vecPos.Y;
		m_bulletData.vecPos.Z = btData->vecPos.Z;

		m_bulletData.vecOffset.X = btData->vecOffset.X;
		m_bulletData.vecOffset.Y = btData->vecOffset.Y;
		m_bulletData.vecOffset.Z = btData->vecOffset.Z;

		uint8_t byteHitType = 0;
		unsigned short InstanceID = 0xFFFF;

		if(m_bytePlayerNumber == 0)
		{
			if(pNetGame)
			{
				CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
				if(pPlayerPool)
				{
					CPlayerPed *pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
					if(pPlayerPed)
					{
						memset(&bulletSyncData, 0, sizeof(BULLET_SYNC_DATA));
						if(pPlayerPed->GetCurrentWeapon() != WEAPON_SNIPER || btData->pEntity)
						{
							if(btData->pEntity)
							{
								CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
								CObjectPool *pObjectPool = pNetGame->GetObjectPool();

								uint16_t PlayerID;
								uint16_t VehicleID;
								uint16_t ObjectID;

								if(pVehiclePool && pObjectPool)
								{
									PlayerID = pPlayerPool->FindRemotePlayerIDFromGtaPtr((PED_TYPE*)btData->pEntity);
									if(PlayerID == INVALID_PLAYER_ID)
									{
										VehicleID = pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE*)btData->pEntity);
										if(VehicleID == INVALID_VEHICLE_ID)
										{
											ObjectID = pObjectPool->FindIDFromGtaPtr(btData->pEntity);
											if(ObjectID == INVALID_OBJECT_ID)
											{
												VECTOR vecOut;
												vecOut.X = 0.0f;
												vecOut.Y = 0.0f;
												vecOut.Z = 0.0f;
												
												if(btData->pEntity->mat)
												{
													ProjectMatrix(&vecOut, btData->pEntity->mat, &btData->vecOffset);
													btData->vecOffset.X = vecOut.X;
													btData->vecOffset.Y = vecOut.Y;
													btData->vecOffset.Z = vecOut.Z;
												}
												else
												{
													btData->vecOffset.X = btData->pEntity->mat->pos.X + btData->vecOffset.X;
													btData->vecOffset.Y = btData->pEntity->mat->pos.Y + btData->vecOffset.Y;
													btData->vecOffset.Z = btData->pEntity->mat->pos.Z + btData->vecOffset.Z;
												}
											}
											else
											{
												// object
												byteHitType = 3;
												InstanceID = ObjectID;
											}
										}
										else
										{
											// vehicle
											byteHitType = 2;
											InstanceID = VehicleID;
										}
									}
									else
									{
										// player
										byteHitType = 1;
										InstanceID = PlayerID;
									}
								}
							}

							bulletSyncData.vecOrigin.X = btData->vecOrigin.X;
							bulletSyncData.vecOrigin.Y = btData->vecOrigin.Y;
							bulletSyncData.vecOrigin.Z = btData->vecOrigin.Z;

							bulletSyncData.vecPos.X = btData->vecPos.X;
							bulletSyncData.vecPos.Y = btData->vecPos.Y;
							bulletSyncData.vecPos.Z = btData->vecPos.Z;

							bulletSyncData.vecOffset.X = btData->vecOffset.X;
							bulletSyncData.vecOffset.Y = btData->vecOffset.Y;
							bulletSyncData.vecOffset.Z = btData->vecOffset.Z;

							bulletSyncData.byteHitType = byteHitType;
							bulletSyncData.PlayerID = InstanceID;
							bulletSyncData.byteWeaponID = pPlayerPed->GetCurrentWeapon();

							RakNet::BitStream bsBullet;
							bsBullet.Write((char)ID_BULLET_SYNC);
							bsBullet.Write((char*)&bulletSyncData, sizeof(BULLET_SYNC_DATA));
							pNetGame->GetRakClient()->Send(&bsBullet, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
						}
					}
				}
			}
		}
	}
	else
	{
		m_bHaveBulletData = false;
		memset(&m_bulletData, 0, sizeof(BULLET_DATA));
	}
}

void CPlayerPed::SetDead() {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	MATRIX4X4 mat;
	GetMatrix(&mat);
	TeleportTo(mat.pos);
	
	SetHealth(0.0f);
	
	*pbyteCurrentPlayer = m_bytePlayerNumber;
	
	ScriptCommand(&kill_actor, m_dwGTAId);
	
	*pbyteCurrentPlayer = 0;
}

void CPlayerPed::CreateAccessory(int iSlot, PED_ACCESSORY acessoryData) {
	if(iSlot < 0 || iSlot >= MAX_PED_ACCESSORIES) {
		return;
	}
		
	DeleteAccessory(iSlot);
	
	MATRIX4X4 matrix;
	GetMatrix(&matrix);
	m_Accessories.m_info[iSlot] = acessoryData;
	m_Accessories.m_pObject[iSlot] = new CObject(m_Accessories.m_info[iSlot].iModel, matrix.pos, VECTOR(0.0f, 0.0f, 0.0f), 150.0f);
}

void CPlayerPed::DeleteAccessory(int iSlot) {
	if(iSlot < 0 || iSlot >= MAX_PED_ACCESSORIES) {
		return;
	}
	
	if(m_Accessories.m_pObject[iSlot]) {
		delete m_Accessories.m_pObject[iSlot];
	}
	m_Accessories.m_pObject[iSlot] = nullptr;
}

void CPlayerPed::ClearAccessories() {
	for(int i = 0; i < MAX_PED_ACCESSORIES; i++) {
		DeleteAccessory(i);
	}
}

void CPlayerPed::UpdateAccessories() {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	VECTOR vecProject;
	MATRIX4X4 boneMat;

	((void (*)(ENTITY_TYPE *))(g_libGTASA + 0x00391968 + 1))(m_pEntity);

	for(int i = 0; i < MAX_PED_ACCESSORIES; i++) {
		if(!m_Accessories.m_pObject[i]) {
			continue;
		}
	
		if(!m_Accessories.m_pObject[i]->m_pEntity) {
			continue;
		}
		
		if(!m_pPed->pPedBones[m_Accessories.m_info[i].iBone]) {
			continue;
		}

		((void (*)(ENTITY_TYPE *))(*(void **)(m_pEntity->vtable + 0x10)))(m_Accessories.m_pObject[i]->m_pEntity);

		GetBoneMatrix(&boneMat, m_pPed->pPedBones[m_Accessories.m_info[i].iBone]->iNodeId);
		ProjectMatrix(&vecProject, &boneMat, &m_Accessories.m_info[i].vecOffset);

		boneMat.pos = vecProject;

		VECTOR *vecRot = &m_Accessories.m_info[i].vecRotation;
		if(!vecRot) {
			continue;
		}

		if(vecRot->X != 0.0f) {
			RwMatrixRotate(&boneMat, 0, vecRot->X);
		}
		
		if(vecRot->Y != 0.0f) {
			RwMatrixRotate(&boneMat, 1, vecRot->Y);
		}
		
		if(vecRot->Z != 0.0f) {
			RwMatrixRotate(&boneMat, 2, vecRot->Z);
		}

		VECTOR *vecScale = &m_Accessories.m_info[i].vecScale;
		if(!vecScale) {
			continue;
		}

		RwMatrixScale(&boneMat, vecScale);
		
		m_Accessories.m_pObject[i]->SetAndUpdateMatrix(boneMat);
		
		m_Accessories.m_pObject[i]->m_pEntity->nEntityFlags.m_bUsesCollision = 0;
		m_Accessories.m_pObject[i]->m_pEntity->nEntityFlags.m_bCollisionProcessed = 0;
	}
}