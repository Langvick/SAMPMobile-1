#include "../main.h"
#include "game.h"
#include "net/netgame.h"
#include "chatwindow.h"

#include <cmath>

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;

void CEntity::GetMatrix(PMATRIX4X4 Matrix)
{
	if (!m_pEntity || !m_pEntity->mat) return;

	Matrix->right.X = m_pEntity->mat->right.X;
	Matrix->right.Y = m_pEntity->mat->right.Y;
	Matrix->right.Z = m_pEntity->mat->right.Z;

	Matrix->up.X = m_pEntity->mat->up.X;
	Matrix->up.Y = m_pEntity->mat->up.Y;
	Matrix->up.Z = m_pEntity->mat->up.Z;

	Matrix->at.X = m_pEntity->mat->at.X;
	Matrix->at.Y = m_pEntity->mat->at.Y;
	Matrix->at.Z = m_pEntity->mat->at.Z;

	Matrix->pos.X = m_pEntity->mat->pos.X;
	Matrix->pos.Y = m_pEntity->mat->pos.Y;
	Matrix->pos.Z = m_pEntity->mat->pos.Z;
}

void CEntity::SetMatrix(MATRIX4X4 Matrix)
{
	if (!m_pEntity || !m_pEntity->mat) return;

	m_pEntity->mat->right.X = Matrix.right.X;
	m_pEntity->mat->right.Y = Matrix.right.Y;
	m_pEntity->mat->right.Z = Matrix.right.Z;

	m_pEntity->mat->up.X = Matrix.up.X;
	m_pEntity->mat->up.Y = Matrix.up.Y;
	m_pEntity->mat->up.Z = Matrix.up.Z;

	m_pEntity->mat->at.X = Matrix.at.X;
	m_pEntity->mat->at.Y = Matrix.at.Y;
	m_pEntity->mat->at.Z = Matrix.at.Z;

	m_pEntity->mat->pos.X = Matrix.pos.X;
	m_pEntity->mat->pos.Y = Matrix.pos.Y;
	m_pEntity->mat->pos.Z = Matrix.pos.Z;
}

void CEntity::SetAndUpdateMatrix(MATRIX4X4 Matrix)
{
	if(!m_pEntity) {
		return;
	}
	
	if(IsGameEntityArePlaceable(m_pEntity)) {
		return;
	}
	
	SetMatrix(Matrix);
	TeleportTo(Matrix.pos);
}

// 0.3.7
void CEntity::GetMoveSpeedVector(PVECTOR Vector)
{
	Vector->X = m_pEntity->vecMoveSpeed.X;
	Vector->Y = m_pEntity->vecMoveSpeed.Y;
	Vector->Z = m_pEntity->vecMoveSpeed.Z;
}

// 0.3.7
void CEntity::SetMoveSpeedVector(VECTOR Vector)
{
	m_pEntity->vecMoveSpeed.X = Vector.X;
	m_pEntity->vecMoveSpeed.Y = Vector.Y;
	m_pEntity->vecMoveSpeed.Z = Vector.Z;
}

void CEntity::GetTurnSpeedVector(PVECTOR Vector)
{
	Vector->X = m_pEntity->vecTurnSpeed.X;
	Vector->Y = m_pEntity->vecTurnSpeed.Y;
	Vector->Z = m_pEntity->vecTurnSpeed.Z;
}

void CEntity::SetTurnSpeedVector(VECTOR Vector)
{
	m_pEntity->vecTurnSpeed.X = Vector.X;
	m_pEntity->vecTurnSpeed.Y = Vector.Y;
	m_pEntity->vecTurnSpeed.Z = Vector.Z;
}

// 0.3.7
uint16_t CEntity::GetModelIndex()
{
	return m_pEntity->nModelIndex;
}

bool CEntity::IsAdded()
{
	if(m_pEntity)
	{
		if(IsGameEntityArePlaceable(m_pEntity))
			return false;
		
		if(m_pEntity->dwUnkModelRel)
			return true;
	}
	return false;
}

bool CEntity::SetModelIndex(uint16_t usModel)
{
	if(!m_pEntity) {
		return false;
	}
	
	if(IsGameEntityArePlaceable(m_pEntity)) {
		return false;
	}

	if(!pGame->HasModelLoaded(usModel)) {
		pGame->RequestModel(usModel, GAME_REQUIRED);
		pGame->LoadRequestedModels(false);
	}

	// CEntity::DeleteRWObject(CEntity *)
	((void (*)(ENTITY_TYPE *))(*(void **)(m_pEntity->vtable + 0x24)))(m_pEntity);

	m_pEntity->nModelIndex = usModel;

	// CEntity::SetModelIndex(CEntity *, uint)
	((void (*)(ENTITY_TYPE *, uint32_t))(*(void **)(m_pEntity->vtable + 0x18)))(m_pEntity, usModel);
	return true;
}

void CEntity::TeleportTo(VECTOR vecPosition)
{
	if(!m_pEntity) {
		return;
	}
	
	if(IsGameEntityArePlaceable(m_pEntity)) {
		return;
	}
	
	uint16_t modelIndex = m_pEntity->nModelIndex;
	if(modelIndex != TRAIN_PASSENGER_LOCO && modelIndex != TRAIN_FREIGHT_LOCO && modelIndex != TRAIN_TRAM) {
		((void (*)(ENTITY_TYPE *, VECTOR, uint8_t))(*(void **)(m_pEntity->vtable + 0x3C)))(m_pEntity, vecPosition, 0);
	} else {
		ScriptCommand(&put_train_at, m_dwGTAId, vecPosition.X, vecPosition.Y, vecPosition.Z);
	}
}

float CEntity::GetDistanceFromCamera()
{
	if(!m_pEntity) {
		return 100000.0f;
	}
	
	if(IsGameEntityArePlaceable(m_pEntity)) {
		return 100000.0f;
	}

	MATRIX4X4 matEnt;
	GetMatrix(&matEnt);

	float tmpX = (matEnt.pos.X - *(float *)(g_libGTASA + 0x008B1134));
	float tmpY = (matEnt.pos.Y - *(float *)(g_libGTASA + 0x008B1138));
	float tmpZ = (matEnt.pos.Z - *(float *)(g_libGTASA + 0x008B113C));

	return sqrt( tmpX*tmpX + tmpY*tmpY + tmpZ*tmpZ );
}

float CEntity::GetDistanceFromLocalPlayerPed()
{
	MATRIX4X4 matFromPlayer;
	MATRIX4X4 matThis;
	float fSX, fSY, fSZ;

	CPlayerPed *pLocalPlayerPed = pGame->FindPlayerPed();
	CLocalPlayer *pLocalPlayer  = nullptr;

	if(!pLocalPlayerPed) {
		return 10000.0f;
	}

	GetMatrix(&matThis);

	if(pNetGame) {
		pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		if(pLocalPlayer && (pLocalPlayer->IsSpectating() || pLocalPlayer->IsInRCMode())) {
			CCamera *pCamera = pGame->GetCamera();
			if(pCamera) {
				pCamera->GetMatrix(&matFromPlayer);
			}
		} else {
			pLocalPlayerPed->GetMatrix(&matFromPlayer);
		}
	} else {
		pLocalPlayerPed->GetMatrix(&matFromPlayer);
	}

	fSX = (matThis.pos.X - matFromPlayer.pos.X) * (matThis.pos.X - matFromPlayer.pos.X);
	fSY = (matThis.pos.Y - matFromPlayer.pos.Y) * (matThis.pos.Y - matFromPlayer.pos.Y);
	fSZ = (matThis.pos.Z - matFromPlayer.pos.Z) * (matThis.pos.Z - matFromPlayer.pos.Z);

	return (float)sqrt(fSX + fSY + fSZ);
}

float CEntity::GetDistanceFromPoint(float X, float Y, float Z)
{
	MATRIX4X4 matThis;
	float fSX, fSY, fSZ;

	GetMatrix(&matThis);
	fSX = (matThis.pos.X - X) * (matThis.pos.X - X);
	fSY = (matThis.pos.Y - Y) * (matThis.pos.Y - Y);
	fSZ = (matThis.pos.Z - Z) * (matThis.pos.Z - Z);

	return (float)sqrt(fSX + fSY + fSZ);
}

bool CEntity::GetCollisionChecking()
{
	if(!m_pEntity) {
		return true;
	}
	
	if(IsGameEntityArePlaceable(m_pEntity)) {
		return true;
	}

	return m_pEntity->nEntityFlags.m_bCollisionProcessed;
}

void CEntity::SetCollisionChecking(bool bCheck)
{
	if(!m_pEntity) {
		return;
	}
	
	if(IsGameEntityArePlaceable(m_pEntity)) {
		return;
	}

	m_pEntity->nEntityFlags.m_bCollisionProcessed = bCheck;
}