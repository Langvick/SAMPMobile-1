#pragma once

#include "common.h"
#include "aimstuff.h"
#include "pad.h"
#include "quaternion.h"
#include "entity.h"
#include "actorped.h"
#include "playerped.h"
#include "vehicle.h"
#include "object.h"
#include "camera.h"
#include "font.h"
#include "scripting.h"
#include "radarcolors.h"
#include "util.h"
#include "textdraw.h"
#include "menu.h"
#include "modelpreview.h"

#define MAX_REMOVEBUILDING_COUNT (1024)

class CGame
{
public:
	CGame();
	~CGame() {};
	
	CPlayerPed *NewPlayer(int iSkin, VECTOR vecPosition, float fRotation, uint8_t byteCreateMarker = 0);
	CVehicle *NewVehicle(int iType, VECTOR vecPosition, float fRotation, bool bAddSiren);
	CActorPed *NewActor(int iSkin, VECTOR vecPosition, float fRotation, float fHealth, bool bInvulnerable);
	CObject *NewObject(int iModel, VECTOR vecPosition, VECTOR vecRotation, float fDrawDistance);

	void InitInMenu();
	void InitInGame();

	CCamera* GetCamera() {
		return m_pGameCamera;
	}

	CPlayerPed* FindPlayerPed() {
		if(!m_pGamePlayer) {
			m_pGamePlayer = new CPlayerPed();
		}
		return m_pGamePlayer;
	}
	
	bool IsMenuActive();

	uint8_t FindFirstFreePlayerPedSlot();
	
	uint32_t CreatePickup(uint16_t usModel, int iType, float fX, float fY, float fZ, int *unk);

	float FindGroundZForCoord(float x, float y, float z);

	uint8_t GetActiveInterior();

	void SetWorldTime(int iHour, int iMinute);

	void SetWorldWeather(unsigned char byteWeatherID);

	void EnableClock(bool bEnable);
	void ToggleThePassingOfTime(bool bOnOff);

	void EnableZoneNames(bool bEnable);
	void DisplayWidgets(bool bDisp);
	void PlaySound(int iSound, float fX, float fY, float fZ);

	void ToggleRadar(bool iToggle);
	void DisplayHUD(bool bDisp);

	void ToggleCheckpoints(bool bEnabled) { m_bCheckpointsEnabled = bEnabled; }
	void ToggleRaceCheckpoints(bool bEnabled) { m_bRaceCheckpointsEnabled = bEnabled; }
	void SetCheckpointInformation(VECTOR *pos, VECTOR *extent);
	void SetRaceCheckpointInformation(uint8_t byteType, VECTOR *pos, VECTOR *next, float fSize);
	void UpdateCheckpoints();
	void MakeRaceCheckpoint();
	void DisableRaceCheckpoint();

	uint32_t CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor, int iStyle);
	void DisableMarker(uint32_t dwMarkerID);

	void RequestModel(uint16_t usModelId, uint8_t ucStreamingFlag = GAME_REQUIRED);
	void RequestVehicleUpgrade(uint16_t usModelId, uint8_t ucStreamingFlag = 12);
	void LoadRequestedModels(bool bOnlyPriorityRequests);
	bool HasModelLoaded(uint16_t usModelId);

	void RefreshStreamingAt(float x, float y);
	void DisableTrainTraffic();
	void SetMaxStats();

	void SetWantedLevel(uint8_t byteLevel);

	bool IsAnimationLoaded(char* szAnimFile);
	void RequestAnimation(char* szAnimFile);

	void DisplayGameText(char *szStr, int iTime, int iType);

	void SetGravity(float fGravity);
	void ToggleCJWalk(bool bUseCJWalk);
	void DisableInteriorEnterExits();

	int GetLocalMoney();
	void AddToLocalMoney(int iAmmount);
	void ResetLocalMoney();

	void DrawGangZone(float fPos[], uint32_t dwColor);

	int GetScreenWidth();
	int GetScreenHeight();
	
	std::vector<OBJECT_REMOVE> GetObjectsToRemoveList() {
		return m_vecObjectToRemove;
	};
	void RemoveBuilding(uint32_t dwModel, VECTOR vecPosition, float fRange);
	
	void SetVehicleNumberPlate(uint16_t vehicleId, char *szPlate);
	
	void CreateExplosion(VECTOR vecPosition, int16_t iType, float fRadius);
	
	bool IsLineOfSightClear(VECTOR const& origin, VECTOR const& target, bool buildings, bool vehicles, bool peds, bool objects, bool dummies, bool doSeeThroughCheck, bool doCameraIgnoreCheck);
	
private:
	CCamera* 					m_pGameCamera;
	CPlayerPed*					m_pGamePlayer;

	// checkpoint
	bool						m_bCheckpointsEnabled;
	uint32_t					m_dwCheckpointMarker;
	VECTOR						m_vecCheckpointPos;
	VECTOR						m_vecCheckpointExtent;
	// race checkpoint
	bool						m_bRaceCheckpointsEnabled;
	uint32_t					m_dwRaceCheckpointHandle;
	uint32_t					m_dwRaceCheckpointMarker;
	float						m_fRaceCheckpointSize;
	uint8_t						m_byteRaceType;
	VECTOR 						m_vecRaceCheckpointPos;
	VECTOR						m_vecRaceCheckpointNext;

	bool 						m_bClockEnabled;
	
	std::vector<OBJECT_REMOVE>	m_vecObjectToRemove;
};