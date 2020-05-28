#include "main.h"
#include "game/game.h"
#include "netgame.h"

extern CGame *pGame;

CObjectPool::CObjectPool()
{
	for(uint16_t objectId = 0; objectId < MAX_OBJECTS; objectId++) {
		m_bObjectSlotState[objectId] = false;
		m_pObjects[objectId] = nullptr;

		attachToObjectData[objectId] = new stAttachToObjectData();
	}
}

CObjectPool::~CObjectPool()
{
	for(uint16_t objectId = 0; objectId < MAX_OBJECTS; objectId++) {
		Delete(objectId);
	}
}

bool CObjectPool::Delete(uint16_t objectId)
{
	if(!IsValidObjectId(objectId)) {
		return false;
	}

	m_bObjectSlotState[objectId] = false;
	
	if(m_pObjects[objectId] != nullptr) {
		delete m_pObjects[objectId];
		m_pObjects[objectId] = nullptr;
	}
	
	if(attachToObjectData[objectId] != nullptr) {
		delete attachToObjectData[objectId];
		attachToObjectData[objectId] = nullptr;
	}
	return true;
}

bool CObjectPool::New(uint16_t objectId, int iModel, VECTOR vecPos, VECTOR vecRot, float fDrawDistance)
{
	if(!IsValidObjectId(objectId)) {
		return false;
	}
	
	Delete(objectId);
	
	if(pGame) {
		m_pObjects[objectId] = pGame->NewObject(iModel, vecPos, vecRot, fDrawDistance);
	}

	if(m_pObjects[objectId]) {
		m_bObjectSlotState[objectId] = true;

		uint16_t attachedObjectId = IsObjectHasAttachedObject(objectId);
		if(attachedObjectId != INVALID_OBJECT_ID && attachToObjectData[attachedObjectId]->bReady) {
			m_pObjects[attachedObjectId]->AttachToObject(
				m_pObjects[objectId],
				attachToObjectData[attachedObjectId]->vecOffset,
				attachToObjectData[attachedObjectId]->vecRot,
				attachToObjectData[attachedObjectId]->bSyncRot
			);
			attachToObjectData[attachedObjectId]->SetReady(false);
		}
	}
	return false;
}

CObject *CObjectPool::GetObjectFromGtaPtr(ENTITY_TYPE *pGtaObject)
{
	for(uint16_t objectId = 0; objectId < MAX_OBJECTS; objectId++) {
		CObject *pObject = GetAt(objectId);
		if(pObject) {
			if(pGtaObject == pObject->m_pEntity) {
				return pObject;
			}
		}
	}
	return nullptr;
}

uint16_t CObjectPool::FindIDFromGtaPtr(ENTITY_TYPE *pGtaObject)
{
	for(uint16_t objectId = 0; objectId < MAX_OBJECTS; objectId++) {
		CObject *pObject = GetAt(objectId);
		if(pObject) {
			if(pGtaObject == pObject->m_pEntity) {
				return objectId;
			}
		}
	}
	
	return INVALID_OBJECT_ID;
}

void CObjectPool::AttachObjectToObject(uint16_t objectId, uint16_t attachToId, VECTOR offset, VECTOR rotation, bool syncRot) 
{
	if(!IsValidObjectId(objectId) || !IsValidObjectId(attachToId)) {
		return;
	}
	
	CObject *object, *attachToObject;
	object = m_pObjects[objectId];
	attachToObject = m_pObjects[attachToId];

	attachToObjectData[objectId]->SetData(attachToId, offset, rotation, syncRot);

	if(attachToObject) {
		object->AttachToObject(attachToObject, offset, rotation, syncRot);
	} else {
		attachToObjectData[objectId]->SetReady(true);
	}
}

uint16_t CObjectPool::IsObjectHasAttachedObject(uint16_t Id)
{
	for(uint16_t objectId = 0; objectId < MAX_OBJECTS; objectId++) {
		if(attachToObjectData[objectId]) {
			if(attachToObjectData[objectId]->usObjectId == Id) {
				return objectId;
			}
		}
	}

	return INVALID_OBJECT_ID;
}

void CObjectPool::Process()
{
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if (m_bObjectSlotState[i])
		{
			m_pObjects[i]->Process();
		}
	}
}
