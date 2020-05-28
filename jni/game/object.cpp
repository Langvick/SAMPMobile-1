#include "main.h"
#include "game.h"
#include "net/netgame.h"
#include <cmath>

extern CGame *pGame;
extern CNetGame *pNetGame;

CObject::CObject(int iModel, VECTOR vecPosition, VECTOR vecRotation, float fDrawDistance)
{
	m_pEntity = 0;
	m_dwGTAId = 0;

	uint32_t dwRetID = 0;
	
	ScriptCommand(&create_object, iModel, vecPosition.X, vecPosition.Y, vecPosition.Z, &dwRetID);
	ScriptCommand(&put_object_at, dwRetID, vecPosition.X, vecPosition.Y, vecPosition.Z);

	m_pEntity = GamePool_Object_GetAt(dwRetID);
	m_dwGTAId = dwRetID;
	
	m_fMoveSpeed = 0.0f;
	
	m_bIsPlayerSurfing = false;

	InstantRotate(vecRotation.X, vecRotation.Y, vecRotation.Z);
}

CObject::~CObject()
{
	m_pEntity = GamePool_Object_GetAt(m_dwGTAId);
	if(m_pEntity) {
		ScriptCommand(&destroy_object, m_dwGTAId);
	}
}

void CObject::Process()
{
	
}

void CObject::MoveObject(VECTOR vecPos, VECTOR newVecPos, VECTOR vecRot, float fMoveSpeed)
{
	ScriptCommand(&make_object_moveable, m_dwGTAId, 1);
	if(vecPos.X != -1 || vecPos.Y != -1 || vecPos.Z != -1) {
		TeleportTo(vecPos);
	}
	
	if(newVecPos.X != -1 || newVecPos.Y != -1 || newVecPos.Z != -1) {
		ScriptCommand(&move_object, m_dwGTAId, newVecPos.X, newVecPos.Y, newVecPos.Z, fMoveSpeed, fMoveSpeed, fMoveSpeed, 1);
	}
	
	if(vecRot.X != -1 || vecRot.Y != -1 || vecRot.Z != -1) {
		ScriptCommand(&set_object_rotation, m_dwGTAId, vecRot.X, vecRot.Y, vecRot.Z);
	}
	
	if(fMoveSpeed != -1)
	{
		m_fMoveSpeed = fMoveSpeed;
	}
}

void CObject::StopMovingObject(VECTOR vecPos, VECTOR vecRot, float fMoveSpeed)
{
	if(vecPos.X != -1 || vecPos.Y != -1 || vecPos.Z != -1) {
		ScriptCommand(&put_object_at, m_dwGTAId, vecPos.X, vecPos.Y, vecPos.Z);
	}
	
	if(vecRot.X != -1 || vecRot.Y != -1 || vecRot.Z != -1) {
		ScriptCommand(&set_object_rotation, m_dwGTAId, vecRot.X, vecRot.Y, vecRot.Z);
	}
	
	if(fMoveSpeed != -1)
	{
		m_fMoveSpeed = fMoveSpeed;
	}
	ScriptCommand(&make_object_moveable, m_dwGTAId, 0);
}

void CObject::SetPos(float x, float y, float z)
{
	ScriptCommand(&put_object_at, m_dwGTAId, x, y, z);
}

void CObject::InstantRotate(float x, float y, float z)
{
	ScriptCommand(&set_object_rotation, m_dwGTAId, x, y, z);
}

void CObject::AttachToObject(CObject * object, VECTOR offset, VECTOR rotation, bool syncRot)
{
	ScriptCommand(&attach_object_to_object, m_dwGTAId, object->m_dwGTAId, offset.X, offset.Y, offset.Z, rotation.X, rotation.Y, rotation.Z);
}

void CObject::SetObjectMaterial(uint8_t iMaterialId, char const *szTxdName, char const *szTexName, uint32_t uiMaterialColor) {
	if(iMaterialId < 0 || iMaterialId > MAX_MATERIALS_PER_MODEL) {
		return;
	}
	
	if(strcmp(szTxdName, "none") != 0 && strcmp(szTxdName, "none") != 0) {
		m_objectMaterial.materialData[iMaterialId].pTexture = (RwTexture *)FindRwTexture(szTxdName, szTexName);
		if(!m_objectMaterial.materialData[iMaterialId].pTexture) {
			return;
		}
	}
	
	//m_objectMaterial.materialData[iMaterialId].cColor = cMaterialColor;
	m_objectMaterial.materialData[iMaterialId].isUsed = true;
	m_objectMaterial.isUsed = true;
	
	return;
}