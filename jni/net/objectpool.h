#pragma once

#define INVALID_OBJECT_ID 65535
#define MAX_OBJECTS 1024

struct stAttachToObjectData {
	uint16_t usObjectId = INVALID_OBJECT_ID;
	bool bReady = false;
	VECTOR vecOffset;
	VECTOR vecRot;
	bool bSyncRot = false;

	void SetData(uint16_t toId, VECTOR offset, VECTOR rot, bool syncRot)
	{
		usObjectId = toId;
		vecOffset = offset;
		vecRot = rot;
		bSyncRot = syncRot;
	}

	void SetReady(bool rdy)
	{
		bReady = rdy;
	}
};

class CObjectPool
{
private:
	bool m_bObjectSlotState[MAX_OBJECTS];
	CObject *m_pObjects[MAX_OBJECTS];

	stAttachToObjectData *attachToObjectData[MAX_OBJECTS];

public:
	CObjectPool();
	~CObjectPool();

	bool New(uint16_t objectId, int iModel, VECTOR vecPos, VECTOR vecRot, float fDrawDistance = 0);
	bool Delete(uint16_t objectId);

	void AttachObjectToObject(uint16_t objectId, uint16_t attachToId, VECTOR offset, VECTOR rotation, bool syncRot);

	bool IsValidObjectId(uint16_t objectId) {
		if(objectId > 0 && objectId < MAX_OBJECTS) {
			return true;
		}
		return false;
	};

	bool GetSlotState(uint16_t objectId) {
		if(IsValidObjectId(objectId)) {
			return m_bObjectSlotState[objectId];
		}
		return false;
	};

	CObject *GetAt(uint16_t objectId) {
		if(!GetSlotState(objectId)) {
			return nullptr;
		}
		return m_pObjects[objectId];
	};

	uint16_t FindIDFromGtaPtr(ENTITY_TYPE *pGtaObject);
	uint16_t IsObjectHasAttachedObject(uint16_t Id);

	CObject *GetObjectFromGtaPtr(ENTITY_TYPE *pGtaObject);

	void Process();
};