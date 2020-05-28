#pragma once

enum eCarNodes : int {
	CAR_NODE_NONE = 0,
	CAR_WHEEL_RF = 1,
	CAR_WHEEL_RM = 2,
	CAR_WHEEL_RB = 3,
	CAR_WHEEL_LF = 4,
	CAR_WHEEL_LM = 5,
	CAR_WHEEL_LB = 6,
	CAR_BUMP_FRONT = 7,
	CAR_BUMP_REAR = 8,
	CAR_WING_RF = 9,
	CAR_WING_RR = 10,
	CAR_DOOR_RF = 11,
	CAR_DOOR_RR = 12,
	CAR_WING_LF = 13,
	CAR_WING_LR = 14,
	CAR_DOOR_LF = 15,
	CAR_DOOR_LR = 16,
	CAR_BONNET = 17,
	CAR_BOOT = 18,
	CAR_WINDSCREEN = 19,
	CAR_NUM_NODES
};

enum eDoors : int {
	BONNET = 0,
	BOOT = 1,
	DOOR_FRONT_LEFT = 2,
	DOOR_FRONT_RIGHT = 3,
	DOOR_REAR_LEFT = 4,
	DOOR_REAR_RIGHT = 5
};

enum eLandingGearState 
{
	LGS_CHANGING = 0,
	LGS_UP = 1,
	LGS_DOWN = 2,
};

class CVehicle : public CEntity
{
public:
	CVehicle(uint16_t usModel, VECTOR vecPosition, float fRotation = 0.0f, bool bSiren = false);
	~CVehicle();

	void LinkToInterior(int);
	void SetZAngle(float);
	
	void SetColor(int, int);
	void SetHealth(float);
	float GetHealth();
	bool IsOccupied();
	void SetInvulnerable(bool);
	bool IsDriverLocalPlayer();
	bool HasSunk();
	void ProcessMarkers();
	void RemoveEveryoneFromVehicle();
	unsigned int GetVehicleSubtype();
	
	void SetWheelPopped(uint8_t);
	void SetDoorLockState(uint8_t);
	void SetEngineState(uint8_t);
	void SetLightsState(uint8_t);
	void SetAlarmState(uint8_t);
	void SetObjState(uint8_t);
	void SetDoorOpenFlag(uint8_t, int);
	
	void SetSirenOn(uint8_t byteState);
	uint8_t IsSirenOn();
	
	void UpdateDamageStatus(uint32_t, uint32_t, uint8_t, uint8_t);

public:
	VEHICLE_TYPE	*m_pVehicle;

	bool 			m_bIsLocked;

	CVehicle		*m_pTrailer;
	uint32_t		m_dwMarkerID;
	bool 			m_bIsInvulnerable;
	uint8_t			m_byteObjectiveVehicle;
	uint8_t			m_bSpecialMarkerEnabled;

	uint8_t			m_byteColor1;
	uint8_t			m_byteColor2;
	bool 			m_bColorChanged;
	
	uint32_t 		m_dwPanelDamage;
	uint32_t 		m_dwDoorDamage;
	uint8_t 		m_byteLightDamage;
	uint8_t 		m_byteTireDamage;
};