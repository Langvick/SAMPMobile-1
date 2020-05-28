#pragma once

#define MAX_MATERIALS_PER_MODEL 16

class CObject : public CEntity
{
public:
	float m_fMoveSpeed;

	bool		m_bIsPlayerSurfing;
	
	struct {
		bool isUsed = false;
		struct {
			bool isStored = false;
			struct {
				RwTexture *pTexture = nullptr;
				RwRGBA cColor;
			} materialData[MAX_MATERIALS_PER_MODEL];
		} origData;
		struct {
			bool isUsed = false;
			RwTexture *pTexture = nullptr;
			RwRGBA cColor;
		} materialData[MAX_MATERIALS_PER_MODEL];
	} m_objectMaterial;

	CObject(int iModel, VECTOR vecPosition, VECTOR vecRotation, float fDrawDistance);
	~CObject();

	void Process();
	
	void MoveObject(VECTOR vecPos, VECTOR newVecPos, VECTOR vecRot, float fMoveSpeed);
	void StopMovingObject(VECTOR vecPos, VECTOR vecRot, float fMoveSpeed);

	void SetPos(float x, float y, float z);
	void InstantRotate(float x, float y, float z);
	void AttachToObject(CObject * object, VECTOR offset, VECTOR rotation, bool syncRot);
	void SetObjectMaterial(uint8_t iMaterialId, char const *szTxdName, char const *szTexName, uint32_t uiMaterialColor);
};