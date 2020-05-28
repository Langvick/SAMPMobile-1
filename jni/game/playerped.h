#pragma once

#include "object.h"

struct stAccessories
{
	PED_ACCESSORY		m_info[MAX_PED_ACCESSORIES];
	CObject				*m_pObject[MAX_PED_ACCESSORIES];
	
	stAccessories() {		
		for(int i = 0; i < MAX_PED_ACCESSORIES; i++) {
			m_pObject[i] = nullptr;
			memset(&m_info[i], 0, sizeof(m_info[i]));
		}
	}
};

class CPlayerPed : public CEntity
{
public:
	CPlayerPed();	// local
	CPlayerPed(uint8_t bytePlayerNumber, int iSkin, float fX, float fY, float fZ, float fRotation); // remote
	~CPlayerPed();

	void Destroy();

	// 0.3.7
	bool IsInVehicle();
	// 0.3.7
	bool IsAPassenger();
	// 0.3.7
	VEHICLE_TYPE* GetGtaVehicle();
	// 0.3.7
	void RemoveFromVehicleAndPutAt(float fX, float fY, float fZ);
	// 0.3.7
	void SetInitialState();
	// 0.3.7
	void SetHealth(float fHealth);
	void SetArmour(float fArmour);
	// 0.3.7
	float GetHealth();
	float GetArmour();
	// 0.3.7
	void TogglePlayerControllable(bool bToggle);
	// 0.3.7
	void SetModelIndex(unsigned int uiModel);

	void SetInterior(uint8_t byteID);

	void PutDirectlyInVehicle(int iVehicleID, int iSeat);
	void EnterVehicle(int iVehicleID, bool bPassenger);
	// 0.3.7
	void ExitCurrentVehicle();
	// 0.3.7
	int GetCurrentVehicleID();
	int GetVehicleSeatID();

	ENTITY_TYPE* GetEntityUnderPlayer();

	// допилить
	void ClearAllWeapons();
	// допилить
	void DestroyFollowPedTask();
	// допилить
	void ResetDamageEntity();

	// 0.3.7
	void RestartIfWastedAt(VECTOR *vecRestart, float fRotation);
	// 0.3.7
	void ForceTargetRotation(float fRotation);
	// 0.3.7
	uint8_t GetActionTrigger();
	// 0.3.7
	bool IsDead();
	uint16_t GetKeys(uint16_t *lrAnalog, uint16_t *udAnalog, uint8_t *additionalKey);
	void SetKeys(uint16_t wKeys, uint16_t lrAnalog, uint16_t udAnalog);
	// 0.3.7
	void SetMoney(int iAmount);
	// 0.3.7
	void ShowMarker(uint32_t iMarkerColorID);
	// 0.3.7
	void HideMarker();
	// 0.3.7
	void SetFightingStyle(int iStyle);
	// 0.3.7
	void SetRotation(float fRotation);
	// 0.3.7
	void ApplyAnimation( char *szAnimName, char *szAnimFile, float fT, int opt1, int opt2, int opt3, int opt4, int iUnk );
	// 0.3.7
	void GetBonePosition(int iBoneID, VECTOR* vecOut);
	void GetBoneMatrix(MATRIX4X4 *matOut, int iBoneId);
	// roflan
	void FindDeathReasonAndResponsiblePlayer(uint16_t *nPlayer);

	PED_TYPE * GetGtaActor() { return m_pPed; };
	
	void SetAmmo(int iWeapon, int iAmmo);
	WEAPON_SLOT_TYPE* FindWeaponSlot(int iWeapon);
	void GiveWeapon(int iWeapon, int iAmmo);
	void SetArmedWeapon(int iWeapon);
	uint8_t GetCurrentWeapon();
	
	CAMERA_AIM* GetCurrentAim();
	uint8_t GetCameraMode();
	WEAPON_SLOT_TYPE* GetCurrentWeaponSlot();
	float GetAimZ();
	float GetCameraExtendedZoom() { return GameGetLocalPlayerCameraExtZoom(); }
	void SetCameraMode(uint8_t byteCameraMode);
	void SetCurrentAim(CAMERA_AIM *pAim);
	void SetAimZ(float fAim);
	void SetCameraExtendedZoom(float fExtZoom, float fAspect);

	void FireInstant();
	void GetWeaponInfoForFire(int bLeft, VECTOR *vecBone, VECTOR *vecOut);
	VECTOR* GetCurrentWeaponFireOffset();
	void ProcessBulletData(BULLET_DATA *btData);

	void SetDead();
	
	void CreateAccessory(int iSlot, PED_ACCESSORY acessoryData);
	void DeleteAccessory(int iSlot);
	void ClearAccessories();
	void UpdateAccessories();

public:
	PED_TYPE*	m_pPed;
	uint8_t		m_bytePlayerNumber;
	uint32_t	m_dwArrow;
	
	bool			m_bHaveBulletData;
	BULLET_DATA 	m_bulletData;
	stAccessories	m_Accessories;
};