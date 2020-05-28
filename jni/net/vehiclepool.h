#pragma once

#define INVALID_VEHICLE_ID 65535

typedef struct _NEW_VEHICLE
{
	uint16_t 	VehicleID;
	int 		iVehicleType;
	VECTOR		vecPos;
	float 		fRotation;
	uint8_t		aColor1;
	uint8_t		aColor2;
	float		fHealth;
	uint8_t		byteInterior;
	uint32_t	dwDoorDamageStatus;
	uint32_t 	dwPanelDamageStatus;
	uint8_t		byteLightDamageStatus;
	uint8_t		byteTireDamageStatus;
	uint8_t		byteAddSiren;
	uint8_t		byteModSlots[14];
	uint8_t	  	bytePaintjob;
	uint32_t	cColor1;
	uint32_t	cColor2;
	uint8_t		byteUnk;
} NEW_VEHICLE;

class CVehiclePool
{
public:
	CVehiclePool();
	~CVehiclePool();

	void Process();

	bool New(NEW_VEHICLE* pNewVehicle);
	bool Delete(uint16_t VehicleID);
	
	bool IsValidVehicleId(uint16_t vehicleId) {
		if(vehicleId > 0 && vehicleId < MAX_VEHICLES) {
			return true;
		}
		return false;
	};

	bool GetSlotState(uint16_t vehicleId) {
		if(IsValidVehicleId(vehicleId)) {
			return m_bVehicleSlotState[vehicleId];
		}
		return false;
	};
	
	CVehicle* GetAt(uint16_t vehicleId) {
		if(!GetSlotState(vehicleId)) {
			return nullptr;
		}
		return m_pVehicles[vehicleId];
	};

	uint16_t FindIDFromGtaPtr(VEHICLE_TYPE * pGtaVehicle);
	int FindGtaIDFromID(int ID);
	VEHICLE_TYPE *FindGtaVehicleFromId(int iID);

	void AssignSpecialParamsToVehicle(uint16_t VehicleID, uint8_t byteObjective, uint8_t byteDoorsLocked);

	int FindNearestToLocalPlayerPed();

	void LinkToInterior(uint16_t VehicleID, int iInterior);

	void NotifyVehicleDeath(uint16_t VehicleID);

	VECTOR			m_vecSpawnPos[MAX_VEHICLES];

private:
	CVehicle*		m_pVehicles[MAX_VEHICLES];
	VEHICLE_TYPE*	m_pGTAVehicles[MAX_VEHICLES];
	bool			m_bVehicleSlotState[MAX_VEHICLES];

	bool			m_bIsActive[MAX_VEHICLES];
	bool			m_bIsWasted[MAX_VEHICLES];
};