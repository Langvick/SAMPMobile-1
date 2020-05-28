#include "main.h"
#include "game.h"
#include "net/netgame.h"

extern CGame *pGame;
extern CNetGame *pNetGame;

CVehicle::CVehicle(uint16_t usModel, VECTOR vecPosition, float fRotation, bool bSiren)
{
	uint32_t dwRetID = 0;

	m_pVehicle = nullptr;
	m_dwGTAId = 0;
	m_pTrailer = nullptr;
	
	switch(usModel) {
		case TRAIN_PASSENGER_LOCO:
		case TRAIN_FREIGHT_LOCO:
		case TRAIN_TRAM: {
			break;
		}
		
		case TRAIN_PASSENGER:
		case TRAIN_FREIGHT: {
			break;
		}
		
		default: {
			if(!pGame->HasModelLoaded(usModel)) {
				pGame->RequestModel(usModel, GAME_REQUIRED);
				pGame->LoadRequestedModels(false);
			}

			ScriptCommand(&create_car, usModel, vecPosition.X, vecPosition.Y, vecPosition.Z, &dwRetID);
			ScriptCommand(&set_car_z_angle, dwRetID, fRotation);
			ScriptCommand(&car_gas_tank_explosion, dwRetID, 0);
			ScriptCommand(&set_car_hydraulics, dwRetID, 0);
			ScriptCommand(&toggle_car_tires_vulnerable, dwRetID, 0);

			m_pVehicle = (VEHICLE_TYPE *)GamePool_Vehicle_GetAt(dwRetID);
			m_pEntity = (ENTITY_TYPE *)m_pVehicle;
			m_dwGTAId = dwRetID;

			if(m_pVehicle) {
				m_pVehicle->m_nDoorLock = 0;
				m_pVehicle->nFlags.bIsLocked = 0;
				m_bIsLocked = false;
				
				m_pVehicle->nFlags.bEngineOn = 0;
				
				m_pVehicle->nFlags.bLightsOn = 0;
				
				m_pVehicle->nFlags.bSirenOrAlarm = 0;
				
				m_pVehicle->nFlags.bIsDamaged = 1;

				MATRIX4X4 mat;
				GetMatrix(&mat);
				
				mat.pos = vecPosition;
				switch(GetVehicleSubtype()) {
					case VEHICLE_SUBTYPE_BIKE:
					case VEHICLE_SUBTYPE_PUSHBIKE: {
						break;
					}
					
					default: {
						mat.pos.Z += 0.25f;
						break;
					}
				}

				SetMatrix(mat);
				
				if(bSiren) {
					SetSirenOn(1);
				} else {
					SetSirenOn(0);
				}
			}
			break;
		}
	}

	m_byteObjectiveVehicle = 0;
	m_bSpecialMarkerEnabled = false;
	m_dwMarkerID = 0;
	m_bIsInvulnerable = false;
}

CVehicle::~CVehicle()
{
	m_pVehicle = GamePool_Vehicle_GetAt(m_dwGTAId);
	if(m_pVehicle) {
		if(m_dwMarkerID) {
			ScriptCommand(&disable_marker, m_dwMarkerID);
			m_dwMarkerID = 0;
		}

		RemoveEveryoneFromVehicle();

		if(m_pTrailer) {
			
		}

		if(m_pVehicle->entity.nModelIndex == TRAIN_PASSENGER_LOCO || m_pVehicle->entity.nModelIndex == TRAIN_FREIGHT_LOCO) {
			ScriptCommand(&destroy_train, m_dwGTAId);
		} else {
			int nModelIndex = m_pVehicle->entity.nModelIndex;
			ScriptCommand(&destroy_car, m_dwGTAId);

			if(!GetModelReferenceCount(nModelIndex)) {
				if(pGame->HasModelLoaded(nModelIndex)) {
					((void (*)(int))(g_libGTASA + 0x00290C4C + 1))(nModelIndex);
				}
			}
		}
	}
}

void CVehicle::LinkToInterior(int iInterior)
{
	if(GamePool_Vehicle_GetAt(m_dwGTAId)) {
		ScriptCommand(&link_vehicle_to_interior, m_dwGTAId, iInterior);
	}
}

void CVehicle::SetZAngle(float fZAngle)
{
	if(fZAngle != -1)
	{
		ScriptCommand(&set_car_z_angle, m_dwGTAId, fZAngle);
	}
}

void CVehicle::SetColor(int iColor1, int iColor2)
{
	if(m_pVehicle) {
		m_pVehicle->m_nPrimaryColor = (uint8_t)iColor1;
		m_pVehicle->m_nSecondaryColor = (uint8_t)iColor2;
	}

	m_byteColor1 = (uint8_t)iColor1;
	m_byteColor2 = (uint8_t)iColor2;
	m_bColorChanged = true;
}

void CVehicle::SetHealth(float fHealth)
{
	if(m_pVehicle) {
		m_pVehicle->m_fHealth = fHealth;
	}
}

float CVehicle::GetHealth()
{
	if(m_pVehicle) {
		return m_pVehicle->m_fHealth;
	} else {
		return 0.0f;
	}
}

void CVehicle::SetInvulnerable(bool bInv)
{
	if(!m_pVehicle) {
		return;
	}
	
	if(IsGameEntityArePlaceable(&m_pVehicle->entity)) {
		return;
	}
	
	if(!GamePool_Vehicle_GetAt(m_dwGTAId)) {
		return;
	}
	
	if(bInv) {
		ScriptCommand(&set_car_immunities, m_dwGTAId, 1, 1, 1, 1, 1);
		ScriptCommand(&toggle_car_tires_vulnerable, m_dwGTAId, 0);
	} else { 
		ScriptCommand(&set_car_immunities, m_dwGTAId, 0, 0, 0, 0, 0);
		ScriptCommand(&toggle_car_tires_vulnerable, m_dwGTAId, 1);
	}
	m_bIsInvulnerable = bInv;
}

bool CVehicle::IsDriverLocalPlayer()
{
	if(m_pVehicle) {
		PED_TYPE *pLocalPed = pGame->FindPlayerPed()->GetGtaActor();
		if(pLocalPed) {
			if((PED_TYPE *)m_pVehicle->m_pDriver == pLocalPed) {
				return true;
			}
		}
	}

	return false;
}

bool CVehicle::HasSunk()
{
	if(!m_pVehicle) {
		return false;
	}
	
	return ScriptCommand(&has_car_sunk, m_dwGTAId);
}

void CVehicle::RemoveEveryoneFromVehicle()
{
	if(!m_pVehicle) {
		return;
	}
	
	if(!GamePool_Vehicle_GetAt(m_dwGTAId)) {
		return;
	}
	
	float fPosX = m_pVehicle->entity.mat->pos.X;
	float fPosY = m_pVehicle->entity.mat->pos.Y;
	float fPosZ = m_pVehicle->entity.mat->pos.Z;

	int iPlayerID = 0;
	if(m_pVehicle->m_pDriver) {
		iPlayerID = GamePool_Ped_GetIndex( m_pVehicle->m_pDriver );
		ScriptCommand(&remove_actor_from_car_and_put_at, iPlayerID, fPosX, fPosY, fPosZ + 2.0f);
	}

	for(int i = 0; i < m_pVehicle->m_nNumPassengers; i++) {
		if(m_pVehicle->m_apPassengers[i] != nullptr) {
			iPlayerID = GamePool_Ped_GetIndex(m_pVehicle->m_apPassengers[i]);
			ScriptCommand(&remove_actor_from_car_and_put_at, iPlayerID, fPosX, fPosY, fPosZ + 2.0f);
		}
	}
}

bool CVehicle::IsOccupied()
{
	if(m_pVehicle) {
		if(m_pVehicle->m_pDriver != nullptr) {
			return true;
		}
		for(int i = 0; i < m_pVehicle->m_nNumPassengers; ++i) {
			if(m_pVehicle->m_apPassengers[i] != nullptr) {
				return true;
			}
		}
	}

	return false;
}

void CVehicle::ProcessMarkers()
{
	if(!m_pVehicle) {
		return;
	}
	
	if(m_byteObjectiveVehicle) {
		if(!m_bSpecialMarkerEnabled) {
			if(m_dwMarkerID) {
				ScriptCommand(&disable_marker, m_dwMarkerID);
				m_dwMarkerID = 0;
			}

			ScriptCommand(&tie_marker_to_car, m_dwGTAId, 1, 3, &m_dwMarkerID);
			ScriptCommand(&set_marker_color, m_dwMarkerID, 1006);
			ScriptCommand(&show_on_radar, m_dwMarkerID, 3);
			m_bSpecialMarkerEnabled = true;
		}
		return;
	}

	if(m_byteObjectiveVehicle && m_bSpecialMarkerEnabled) {
		if(m_dwMarkerID) {
			ScriptCommand(&disable_marker, m_dwMarkerID);
			m_bSpecialMarkerEnabled = false;
			m_dwMarkerID = 0;
		}
	}

	if(GetDistanceFromLocalPlayerPed() < 200.0f && !IsOccupied()) {
		if(!m_dwMarkerID) {
			ScriptCommand(&tie_marker_to_car, m_dwGTAId, 1, 2, &m_dwMarkerID);
			ScriptCommand(&set_marker_color, m_dwMarkerID, 1004);
		}
	} else if(IsOccupied() || GetDistanceFromLocalPlayerPed() >= 200.0f) {
		if(m_dwMarkerID) {
			ScriptCommand(&disable_marker, m_dwMarkerID);
			m_dwMarkerID = 0;
		}
	}
}

void CVehicle::SetWheelPopped(uint8_t bytePopped)
{

}

void CVehicle::SetDoorLockState(uint8_t byteState) {
	if(!m_pVehicle) return;
	
	if(byteState == 1) {
		m_pVehicle->m_nDoorLock = 2;
		m_pVehicle->nFlags.bIsLocked = 1;
		m_bIsLocked = true;
	}else{
		m_pVehicle->m_nDoorLock = 0;
		m_pVehicle->nFlags.bIsLocked = 0;
		m_bIsLocked = false;
	}
}

void CVehicle::SetEngineState(uint8_t byteState) {
	if(!m_pVehicle) return;
	
	if(byteState == 1) {
		m_pVehicle->nFlags.bEngineOn = 1;
	}else{
		m_pVehicle->nFlags.bEngineOn = 0;
	}
}

void CVehicle::SetLightsState(uint8_t byteState) {
	if(!m_pVehicle) return;
	
	if(byteState == 1){
		m_pVehicle->nFlags.bLightsOn = 1;
	}else{
		m_pVehicle->nFlags.bLightsOn = 0;
	}
}

void CVehicle::SetAlarmState(uint8_t byteState) {
	if(!m_pVehicle) return;
	
	if(byteState == 1){
		m_pVehicle->nFlags.bSirenOrAlarm = 1;
	}else{
		m_pVehicle->nFlags.bSirenOrAlarm = 0;
	}
}

void CVehicle::SetObjState(uint8_t byteState) {
	if(!m_pVehicle) return;
	
	if(byteState == 1) {
		m_byteObjectiveVehicle = 1;
		m_bSpecialMarkerEnabled = false;
	}else{
		m_byteObjectiveVehicle = 0;
		m_bSpecialMarkerEnabled = false;
	}
}

void CVehicle::SetDoorOpenFlag(uint8_t byteState, int iCurrentDoorId) {
	if(!m_pVehicle) return;
	
	if(byteState == 1) {
		ScriptCommand(&open_car_component, m_dwGTAId, (int)iCurrentDoorId);
	} else {
		ScriptCommand(&set_car_component_rotate, m_dwGTAId, (int)iCurrentDoorId, 0.0f);
	}
}

void CVehicle::SetSirenOn(uint8_t byteState)
{
	if(!m_pVehicle) return;
	
	if(byteState == 1){
		m_pVehicle->nFlags.bSirenOrAlarm = 1;
	}else{
		m_pVehicle->nFlags.bSirenOrAlarm = 0;
	}
}

uint8_t CVehicle::IsSirenOn()
{
	return (m_pVehicle->nFlags.bSirenOrAlarm == 1);
}

void CVehicle::UpdateDamageStatus(uint32_t dwPanelDamage, uint32_t dwDoorDamage, uint8_t byteLightDamage, uint8_t byteTire)
{
	
}

unsigned int CVehicle::GetVehicleSubtype()
{
	if(m_pVehicle) {
		if(m_pVehicle->entity.vtable == g_libGTASA + 0x005CC9F0) {
			return VEHICLE_SUBTYPE_CAR;
		}
		else if(m_pVehicle->entity.vtable == g_libGTASA + 0x005CCD48) {
			return VEHICLE_SUBTYPE_BOAT;
		}
		else if(m_pVehicle->entity.vtable == g_libGTASA + 0x005CCB18) {
			return VEHICLE_SUBTYPE_BIKE;
		}
		else if(m_pVehicle->entity.vtable == g_libGTASA + 0x005CD0B0) {
			return VEHICLE_SUBTYPE_PLANE;
		}
		else if(m_pVehicle->entity.vtable == g_libGTASA + 0x005CCE60) {
			return VEHICLE_SUBTYPE_HELI;
		}
		else if(m_pVehicle->entity.vtable == g_libGTASA + 0x005CCC30) {
			return VEHICLE_SUBTYPE_PUSHBIKE;
		}
		else if(m_pVehicle->entity.vtable == g_libGTASA + 0x005CD428) {
			return VEHICLE_SUBTYPE_TRAIN;
		}
	}
	return 0;
}