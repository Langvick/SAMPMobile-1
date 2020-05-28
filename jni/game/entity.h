#pragma once

class CEntity
{
public:
	CEntity() {};
	virtual ~CEntity() {};

	void GetMatrix(PMATRIX4X4 Matrix);
	void SetMatrix(MATRIX4X4 Matrix);	
	void SetAndUpdateMatrix(MATRIX4X4 Matrix);
 
	void GetMoveSpeedVector(PVECTOR Vector);
	void SetMoveSpeedVector(VECTOR Vector);

	void GetTurnSpeedVector(PVECTOR Vector);
	void SetTurnSpeedVector(VECTOR Vector);

	uint16_t GetModelIndex();

	void TeleportTo(VECTOR vecPosition);

	bool SetModelIndex(uint16_t usModel);

	bool IsAdded();

	float GetDistanceFromCamera();
	float GetDistanceFromLocalPlayerPed();
	float GetDistanceFromPoint(float x, float y, float z);

	bool GetCollisionChecking();
	void SetCollisionChecking(bool bCheck);

public:
	ENTITY_TYPE		*m_pEntity;
	uint32_t		m_dwGTAId;
};