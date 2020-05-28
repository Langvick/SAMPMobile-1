#include "main.h"
#include "game.h"
#include "net/netgame.h"
#include "../playerslist.h"
#include "../timer.hpp"
#include "util/armhook.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CPlayersList *pPlayersList;

PAD_KEYS LocalPlayerKeys;
PAD_KEYS RemotePlayerKeys[PLAYER_PED_SLOTS];

uintptr_t dwCurPlayerActor = 0;
uint8_t byteCurPlayer = 0;
uint8_t byteCurDriver = 0;

uint16_t (*CPad__GetPedWalkLeftRight)(uintptr_t thiz);
uint16_t CPad__GetPedWalkLeftRight_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		// Remote player
		uint16_t dwResult = RemotePlayerKeys[byteCurPlayer].wKeyLR;
		if((dwResult == 0xFF80 || dwResult == 0x80) && RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_WALK]) {
			dwResult = 0x40;
		}
		return dwResult;
	} else {
		// Local player
		LocalPlayerKeys.wKeyLR = CPad__GetPedWalkLeftRight(thiz);
		return LocalPlayerKeys.wKeyLR;
	}
}

uint16_t (*CPad__GetPedWalkUpDown)(uintptr_t thiz);
uint16_t CPad__GetPedWalkUpDown_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		// Remote player
		uint16_t dwResult = RemotePlayerKeys[byteCurPlayer].wKeyUD;
		if((dwResult == 0xFF80 || dwResult == 0x80) && RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_WALK]) {
			dwResult = 0x40;
		}
		return dwResult;
	} else {
		// Local player
		LocalPlayerKeys.wKeyUD = CPad__GetPedWalkUpDown(thiz);
		return LocalPlayerKeys.wKeyUD;
	}
}

uint32_t (*CPad__GetSprint)(uintptr_t thiz, uint32_t unk);
uint32_t CPad__GetSprint_hook(uintptr_t thiz, uint32_t unk)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_SPRINT];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT] = CPad__GetSprint(thiz, unk);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT];
	}
}

uint32_t (*CPad__JumpJustDown)(uintptr_t thiz);
uint32_t CPad__JumpJustDown_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0))
	{
		if(!RemotePlayerKeys[byteCurPlayer].bIgnoreJump && RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_JUMP] && !RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE]) {
			RemotePlayerKeys[byteCurPlayer].bIgnoreJump = true;
			return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_JUMP];
		}

		return 0;
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = CPad__JumpJustDown(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP];
	}
}

uint32_t (*CPad__GetJump)(uintptr_t thiz);
uint32_t CPad__GetJump_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		if(RemotePlayerKeys[byteCurPlayer].bIgnoreJump) return 0;
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_JUMP];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = CPad__JumpJustDown(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP];
	}
}

uint32_t (*CPad__GetAutoClimb)(uintptr_t thiz);
uint32_t CPad__GetAutoClimb_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_JUMP];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = CPad__GetAutoClimb(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP];
	}
}

uint32_t (*CPad__GetAbortClimb)(uintptr_t thiz);
uint32_t CPad__GetAbortClimb_hook(uintptr_t thiz) {
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_SECONDARY_ATTACK];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = CPad__GetAutoClimb(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK];
	}
}

uint32_t (*CPad__DiveJustDown)();
uint32_t CPad__DiveJustDown_hook()
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		// remote player
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_FIRE];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = CPad__DiveJustDown();
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE];
	}
}

uint32_t (*CPad__SwimJumpJustDown)(uintptr_t thiz);
uint32_t CPad__SwimJumpJustDown_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_JUMP];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = CPad__SwimJumpJustDown(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP];
	}
}

uint32_t (*CPad__DuckJustDown)(uintptr_t thiz, int unk);
uint32_t CPad__DuckJustDown_hook(uintptr_t thiz, int unk)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0))
	{
		if(!RemotePlayerKeys[byteCurPlayer].bIgnoreCrouch && RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_CROUCH]) {
			RemotePlayerKeys[byteCurPlayer].bIgnoreCrouch = true;
			return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_CROUCH];
		}

		return 0;
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH] = CPad__DuckJustDown(thiz, unk);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH];
	}
}

uint32_t (*CPad__GetDuck)(uintptr_t thiz);
uint32_t CPad__GetDuck_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		if(RemotePlayerKeys[byteCurPlayer].bIgnoreCrouch) return 0;
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_CROUCH];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH] = CPad__GetDuck(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH];
	}
}

uint32_t (*CPad__MeleeAttackJustDown)(uintptr_t thiz);
uint32_t CPad__MeleeAttackJustDown_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		if(RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE] && RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_SECONDARY_ATTACK]) {
			return 2;
		}

		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_FIRE];
	} else {
		uint32_t dwResult = CPad__MeleeAttackJustDown(thiz);
		LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = dwResult;
		return dwResult;
	}
}

// WEAPONS
bool (*CPad__GetEnterTargeting)(uintptr_t thiz);
bool CPad__GetEnterTargeting_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE] = CPad__GetEnterTargeting(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE];
	}
}

bool (*CPad__GetWeapon)(uintptr_t thiz, PED_TYPE* pPed);
bool CPad__GetWeapon_hook(uintptr_t thiz, PED_TYPE *pPed)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_FIRE];
	} else {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = CPad__GetWeapon(thiz, pPed);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE];
	}
}

uint32_t (*CCamera_IsTargetingActive)(uintptr_t thiz);
uint32_t CCamera_IsTargetingActive_hook(uintptr_t thiz)
{
	uintptr_t dwRetAddr = 0;
 	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
 	dwRetAddr -= g_libGTASA;

 	if(dwRetAddr == 0x003ADAD7) {
 		return CCamera_IsTargetingActive(thiz);
 	}

 	if(dwRetAddr == 0x00387455) {
 		return CCamera_IsTargetingActive(thiz);
 	}

	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		return RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE] ? 1 : 0;
	} else {
		*(uint8_t*)(g_libGTASA + 0x008E864C) = 0;
		LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE] = CCamera_IsTargetingActive(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE];
	}
}

uint32_t (*CPad__GetBlock)(uintptr_t thiz);
uint32_t CPad__GetBlock_hook(uintptr_t thiz)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		if(RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_JUMP] && RemotePlayerKeys[byteCurPlayer].bKeys[ePadKeys::KEY_HANDBRAKE]) {
			return 1;
		}

		return 0;
	} else {
		return CPad__GetBlock(thiz);
	}
}

int16_t (*CPad__GetSteeringLeftRight)(uintptr_t thiz);
int16_t CPad__GetSteeringLeftRight_hook(uintptr_t thiz)
{
	if(byteCurDriver != 0) {
		// remote player
		return (int16_t)RemotePlayerKeys[byteCurDriver].wKeyLR;
	} else {
		// local player
		LocalPlayerKeys.wKeyLR = CPad__GetSteeringLeftRight(thiz);
		return LocalPlayerKeys.wKeyLR;
	}
}

uint16_t (*CPad__GetSteeringUpDown)(uintptr_t thiz);
uint16_t CPad__GetSteeringUpDown_hook(uintptr_t thiz)
{
	if(byteCurDriver != 0) {
		// remote player
		return RemotePlayerKeys[byteCurDriver].wKeyUD;
	} else {
		// local player
		LocalPlayerKeys.wKeyUD = CPad__GetSteeringUpDown(thiz);
		return LocalPlayerKeys.wKeyUD;
	}
}

uint16_t (*CPad__GetAccelerate)(uintptr_t thiz);
uint16_t CPad__GetAccelerate_hook(uintptr_t thiz)
{
	if(byteCurDriver != 0) {
		// remote player
		return RemotePlayerKeys[byteCurDriver].bKeys[ePadKeys::KEY_SPRINT] ? 0xFF : 0x00;
	} else {
		// local player
		uint16_t wAccelerate = CPad__GetAccelerate(thiz);
		LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT] = wAccelerate;
		return wAccelerate;
	}
}

uint16_t (*CPad__GetBrake)(uintptr_t thiz);
uint16_t CPad__GetBrake_hook(uintptr_t thiz)
{
	if(byteCurDriver != 0) {
		// remote player
		return RemotePlayerKeys[byteCurDriver].bKeys[ePadKeys::KEY_JUMP] ? 0xFF : 0x00;
	} else {
		// local player
		uint16_t wBrake = CPad__GetBrake(thiz);
		LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP] = wBrake;
		return wBrake;
	}
}

uint32_t (*CPad__GetHandBrake)(uintptr_t thiz);
uint32_t CPad__GetHandBrake_hook(uintptr_t thiz)
{
	if(byteCurDriver != 0) {
		// remote player
		return RemotePlayerKeys[byteCurDriver].bKeys[ePadKeys::KEY_HANDBRAKE] ? 0xFF : 0x00;
	} else {
		// local player
		uint32_t handBrake = CPad__GetHandBrake(thiz);
		LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE] = handBrake;
		return handBrake;
	}
}

uint32_t (*CPad__GetHorn)(uintptr_t thiz);
uint32_t CPad__GetHorn_hook(uintptr_t thiz)
{
	if(byteCurDriver != 0) {
		// remote player
		return RemotePlayerKeys[byteCurDriver].bKeys[ePadKeys::KEY_CROUCH];
	} else {
		// local player
		uint32_t horn = CPad__GetHorn(thiz);
		//Log("horn: %d", horn);
		LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH] = CPad__GetHorn(thiz);
		return LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH];
	}
}

uint32_t (*CPad__ExitVehicleJustDown)(uintptr_t thiz, int a2, uintptr_t vehicle, int a4, uintptr_t vec);
uint32_t CPad__ExitVehicleJustDown_hook(uintptr_t thiz, int a2, uintptr_t vehicle, int a4, uintptr_t vec)
{
	CPlayerPool *pPlayerPool;
	CLocalPlayer *pLocalPlayer;

	if(pNetGame)
	{
		pPlayerPool = pNetGame->GetPlayerPool();
		if(pPlayerPool)
		{
			pLocalPlayer = pPlayerPool->GetLocalPlayer();
			if(pLocalPlayer)
			{
				if( pLocalPlayer->HandlePassengerEntry() )
					return 0;
			}
		}
	}

	return CPad__ExitVehicleJustDown(thiz, a2, vehicle, a4, vec);
}

void (*CPed__ProcessControl)(uintptr_t thiz);
void CPed__ProcessControl_hook(uintptr_t thiz)
{
	dwCurPlayerActor = thiz;
	byteCurPlayer = FindPlayerNumFromPedPtr(thiz);

	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		// CPed::UpdatePosition nulled from CPed::ProcessControl
		NOP(g_libGTASA + 0x00439B7A, 2);
		
		// call original
		CPed__ProcessControl(thiz);
		
		// restore
		WriteMemory(g_libGTASA + 0x00439B7A, (uintptr_t)"\xFA\xF7\x1D\xF8", 4);
	} else {
		// LOCAL PLAYER

		// Apply the original code to set ped rot from Cam
		WriteMemory(g_libGTASA + 0x004BED92, (uintptr_t)"\x10\x60", 2);

		(*CPed__ProcessControl)(thiz);

		// Reapply the no ped rots from Cam patch
		WriteMemory(g_libGTASA + 0x004BED92, (uintptr_t)"\x00\x46", 2);
	}

	return;
}

void AllVehicles__ProcessControl_hook(uintptr_t thiz)
{
	VEHICLE_TYPE *pVehicle = (VEHICLE_TYPE*)thiz;
	uintptr_t this_vtable = pVehicle->entity.vtable;
	this_vtable -= g_libGTASA;

	uintptr_t call_addr = 0;

	switch(this_vtable)
	{
		// CAutomobile
		case 0x005CC9F0:
		call_addr = 0x004E314C;
		break;

		// CBoat
		case 0x005CCD48:
		call_addr = 0x004F7408;
		break;

		// CBike
		case 0x005CCB18:
		call_addr = 0x004EE790;
		break;

		// CPlane
		case 0x005CD0B0:
		call_addr = 0x005031E8;
		break;

		// CHeli
		case 0x005CCE60:
		call_addr = 0x004FE62C;
		break;

		// CBmx
		case 0x005CCC30:
		call_addr = 0x004F3CE8;
		break;

		// CMonsterTruck
		case 0x005CCF88:
		call_addr = 0x00500A34;
		break;

		// CQuadBike
		case 0x005CD1D8:
		call_addr = 0x00505840;
		break;

		// CTrain
		case 0x005CD428:
		call_addr = 0x0050AB24;
		break;
	}

	if(pVehicle && pVehicle->m_pDriver)
	{
		byteCurDriver = FindPlayerNumFromPedPtr((uintptr_t)pVehicle->m_pDriver);
	}

	if(pVehicle->m_pDriver && pVehicle->m_pDriver->dwPedType == 0 &&
		pVehicle->m_pDriver != GamePool_FindPlayerPed() && 
		*(uint8_t*)(g_libGTASA + 0x008E864C) == 0) // CWorld::PlayerInFocus
	{
		*(uint8_t*)(g_libGTASA + 0x008E864C) = 0;

		pVehicle->m_pDriver->dwPedType = 4;
		//CAEVehicleAudioEntity::Service
		(( void (*)(uintptr_t))(g_libGTASA + 0x00364B64+1))(thiz+0x138);
		pVehicle->m_pDriver->dwPedType = 0;
	}
	else
	{
		(( void (*)(uintptr_t))(g_libGTASA + 0x00364B64+1))(thiz+0x138);
	}

	// VEHTYPE::ProcessControl()
    (( void (*)(VEHICLE_TYPE*))(g_libGTASA+call_addr+1))(pVehicle);
}

// TaskUseGun_Hook
void CTaskSimpleUseGun__SetPedPosition_hook(uintptr_t thiz, PED_TYPE* pPed)
{
	if(dwCurPlayerActor && (byteCurPlayer != 0)) {
		// remote player
		
		// byteCameraMode
		uint8_t byteSavedCameraMode = *pbyteCameraMode;
		*pbyteCameraMode = GameGetPlayerCameraMode(byteCurPlayer);

		GameStoreLocalPlayerCameraExtZoom();
		GameSetRemotePlayerCameraExtZoom(byteCurPlayer);

		GameStoreLocalPlayerAim();
		GameSetRemotePlayerAim(byteCurPlayer);

		// CTaskSimpleUseGun::SetPedPosition()
		((void (*)(uintptr_t, PED_TYPE *))(g_libGTASA + 0x0046D6AC + 1))(thiz, pPed);

		*pbyteCameraMode = byteSavedCameraMode;
		GameSetLocalPlayerCameraExtZoom();
		
		GameSetLocalPlayerAim();
	} else {
		// local player

		// CTaskSimpleUseGun::SetPedPosition()
		(( void (*)(uintptr_t, PED_TYPE*))(g_libGTASA + 0x0046D6AC + 1))(thiz, pPed);
	}
}

void HookCPad()
{
	memset(&LocalPlayerKeys, 0, sizeof(PAD_KEYS));

	
	// CPed::ProcessControl
	SetUpHook(g_libGTASA + 0x0045A280, (uintptr_t)CPed__ProcessControl_hook, (uintptr_t*)&CPed__ProcessControl);
	
	// all vehicles ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CCA1C, (uintptr_t)AllVehicles__ProcessControl_hook); // CAutomobile::ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CCD74, (uintptr_t)AllVehicles__ProcessControl_hook); // CBoat::ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CCB44, (uintptr_t)AllVehicles__ProcessControl_hook); // CBike::ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CD0DC, (uintptr_t)AllVehicles__ProcessControl_hook); // CPlane::ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CCE8C, (uintptr_t)AllVehicles__ProcessControl_hook); // CHeli::ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CCC5C, (uintptr_t)AllVehicles__ProcessControl_hook); // CBmx::ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CCFB4, (uintptr_t)AllVehicles__ProcessControl_hook); // CMonsterTruck::ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CD204, (uintptr_t)AllVehicles__ProcessControl_hook); // CQuadBike::ProcessControl
	InstallMethodHook(g_libGTASA + 0x005CD454, (uintptr_t)AllVehicles__ProcessControl_hook); // CTrain::ProcessControl
	
	// TaskUseGun
	InstallMethodHook(g_libGTASA + 0x005C8610, (uintptr_t)CTaskSimpleUseGun__SetPedPosition_hook);

	// lr/ud (onfoot)
	SetUpHook(g_libGTASA + 0x0039D08C, (uintptr_t)CPad__GetPedWalkLeftRight_hook, (uintptr_t*)&CPad__GetPedWalkLeftRight);
	SetUpHook(g_libGTASA + 0x0039D110, (uintptr_t)CPad__GetPedWalkUpDown_hook, (uintptr_t*)&CPad__GetPedWalkUpDown);

	// sprint/jump stuff
	SetUpHook(g_libGTASA + 0x0039EAA4, (uintptr_t)CPad__GetSprint_hook, (uintptr_t*)&CPad__GetSprint);
	SetUpHook(g_libGTASA + 0x0039E9B8, (uintptr_t)CPad__JumpJustDown_hook, (uintptr_t*)&CPad__JumpJustDown);
	SetUpHook(g_libGTASA + 0x0039E96C, (uintptr_t)CPad__GetJump_hook, (uintptr_t*)&CPad__GetJump);
	SetUpHook(g_libGTASA + 0x0039E7B0, (uintptr_t)CPad__DuckJustDown_hook, (uintptr_t*)&CPad__DuckJustDown);
	SetUpHook(g_libGTASA + 0x0039E74C, (uintptr_t)CPad__GetDuck_hook, (uintptr_t*)&CPad__GetDuck);
	SetUpHook(g_libGTASA + 0x0039E824, (uintptr_t)CPad__GetAutoClimb_hook, (uintptr_t*)&CPad__GetAutoClimb);
	SetUpHook(g_libGTASA + 0x0039E8C0, (uintptr_t)CPad__GetAbortClimb_hook, (uintptr_t*)&CPad__GetAbortClimb);

	// swimm
	SetUpHook(g_libGTASA + 0x0039EA0C, (uintptr_t)CPad__DiveJustDown_hook, (uintptr_t*)&CPad__DiveJustDown);
	SetUpHook(g_libGTASA + 0x0039EA4C, (uintptr_t)CPad__SwimJumpJustDown_hook, (uintptr_t*)&CPad__SwimJumpJustDown);

	// WEAPON
	SetUpHook(g_libGTASA + 0x0039DD9C, (uintptr_t)CPad__MeleeAttackJustDown_hook, (uintptr_t*)&CPad__MeleeAttackJustDown);
	SetUpHook(g_libGTASA + 0x0039E498, (uintptr_t)CPad__GetEnterTargeting_hook, (uintptr_t*)&CPad__GetEnterTargeting);
	SetUpHook(g_libGTASA + 0x0039E038, (uintptr_t)CPad__GetWeapon_hook, (uintptr_t*)&CPad__GetWeapon);
	
	SetUpHook(g_libGTASA + 0x0037440C, (uintptr_t)CCamera_IsTargetingActive_hook, (uintptr_t*)&CCamera_IsTargetingActive);
	
	SetUpHook(g_libGTASA + 0x0039DB50, (uintptr_t)CPad__GetBlock_hook, (uintptr_t*)&CPad__GetBlock);

	// steering lr/ud (incar)
	SetUpHook(g_libGTASA + 0x0039C9E4, (uintptr_t)CPad__GetSteeringLeftRight_hook, (uintptr_t*)&CPad__GetSteeringLeftRight);
	SetUpHook(g_libGTASA + 0x0039CBF0, (uintptr_t)CPad__GetSteeringUpDown_hook, (uintptr_t*)&CPad__GetSteeringUpDown);

	SetUpHook(g_libGTASA + 0x0039DB7C, (uintptr_t)CPad__GetAccelerate_hook, (uintptr_t*)&CPad__GetAccelerate);
	SetUpHook(g_libGTASA + 0x0039D938, (uintptr_t)CPad__GetBrake_hook, (uintptr_t*)&CPad__GetBrake);
	SetUpHook(g_libGTASA + 0x0039D754, (uintptr_t)CPad__GetHandBrake_hook, (uintptr_t*)&CPad__GetHandBrake);
	SetUpHook(g_libGTASA + 0x0039D4C8, (uintptr_t)CPad__GetHorn_hook, (uintptr_t*)&CPad__GetHorn);
	
	SetUpHook(g_libGTASA + 0x0039DA1C, (uintptr_t)CPad__ExitVehicleJustDown_hook, (uintptr_t*)&CPad__ExitVehicleJustDown);
}