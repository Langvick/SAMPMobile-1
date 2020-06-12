#include "../main.h"
#include "game.h"
#include "net/netgame.h"
#include "util/armhook.h"

extern CGame *pGame;
extern CNetGame *pNetGame;

CActorPed::CActorPed(int iSkin, VECTOR vecPosition, float fRotation, float fHealth, bool bInvulnerable = true) {
	Destroy();

	if(!pGame->HasModelLoaded(iSkin)) {
		pGame->RequestModel(iSkin, GAME_REQUIRED);
		pGame->LoadRequestedModels(false);
	}

	if(!IsPedModel(iSkin)) {
		iSkin = 0;
	}

	uint32_t actorGTAId = 0;
	ScriptCommand(&create_actor, 22, iSkin, vecPosition.X, vecPosition.Y, vecPosition.Z, &actorGTAId);

	m_dwGTAId = actorGTAId;
	m_pPed = GamePool_Ped_GetAt(m_dwGTAId);
	m_pEntity = (ENTITY_TYPE *)m_pPed;

	ForceTargetRotation(fRotation);
	TeleportTo(vecPosition);

	if(fHealth < 1.0f) {
		SetDead();
	} else {
		SetHealth(fHealth);
	}

	ScriptCommand(&lock_actor, m_dwGTAId, 1);
	m_pEntity->nEntityFlags.m_bUsesCollision = 1;

	if(bInvulnerable) {
		ScriptCommand(&set_actor_immunities, m_dwGTAId, 1, 1, 1, 1, 1);
	} else {
		ScriptCommand(&set_actor_immunities, m_dwGTAId, 0, 0, 0, 0, 0);
	}
	return;
}

CActorPed::~CActorPed() {
	Destroy();
	return;
}

void CActorPed::Destroy() {
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId) || m_pPed->entity.vtable == 0x005C7358)
	{
		Log("CActorPed::Destroy: invalid pointer/vtable");
		m_pPed = nullptr;
		m_pEntity = nullptr;
		m_dwGTAId = 0;
		return;
	}

	Log("Removing from vehicle..");
	if(IN_VEHICLE(m_pPed))
		RemoveFromVehicleAndPutAt(100.0f, 100.0f, 10.0f);
	
	// CActorPed::Destructor
	Log("Calling destructor..");
	(( void (*)(PED_TYPE*))(*(void**)(m_pPed->entity.vtable + 0x4)))(m_pPed);
	
	m_pPed = nullptr;
	m_pEntity = nullptr;
	return;
}

void CActorPed::SetHealth(float fHealth) {
	if(!m_pPed) {
		return;
	}

	m_pPed->fHealth = fHealth;
	return;
}

void CActorPed::ForceTargetRotation(float fRotation) {
	if(!m_pPed) {
		return;
	}

	m_pPed->fRotation1 = DegToRad(fRotation);
	m_pPed->fRotation2 = DegToRad(fRotation);

	ScriptCommand(&set_actor_z_angle, m_dwGTAId, fRotation);
	return;
}

void CActorPed::ApplyAnimation(char *szAnimName, char *szAnimFile, float fDelta, int bLoop, int bLockX, int bLockY, int bFreeze, int uiTime) {
	if(!m_pPed) {
		return;
	}
	
	if(!strcasecmp(szAnimFile,"SEX")) return;
	
	int iWaitAnimLoad = 0;
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

	ScriptCommand(&apply_animation, m_dwGTAId, szAnimName, szAnimFile, fDelta, bLoop, bLockX, bLockY, bFreeze, uiTime);
	return;
}

void CActorPed::SetDead() {
	if(!m_pPed) {
		return;
	}
	
	MATRIX4X4 matEntity;
	GetMatrix(&matEntity);
	TeleportTo(matEntity.pos);
	
	SetHealth(0.0f);
	ScriptCommand(&kill_actor, m_dwGTAId);
}

void CActorPed::RemoveFromVehicleAndPutAt(float fX, float fY, float fZ)
{
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId)) {
		return;
	}
	
	if(IN_VEHICLE(m_pPed)) {
		ScriptCommand(&remove_actor_from_car_and_put_at, m_dwGTAId, fX, fY, fZ);
	}
}
