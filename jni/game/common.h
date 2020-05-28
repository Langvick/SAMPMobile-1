#pragma once

#include "RW\RenderWare.h"

#define MAX_TEXT_DRAW_LINE 1024

#define PLAYER_PED_SLOTS 192

#define MAX_PED_ACCESSORIES 10

#define MAX_MENUS 128

#define PADDING(x,y) uint8_t x[y]
#define IN_VEHICLE(x) ((x->dwStateFlags & 0x100) >> 8)
#define IS_VEHICLE_MOD(x) (x >= 1000 && x <= 1193)

enum eStreamingFlags {
    GAME_REQUIRED = 0x2,
    MISSION_REQUIRED = 0x4,
    KEEP_IN_MEMORY = 0x8,
    PRIORITY_REQUEST = 0x10
};

enum eStreamingLoadState {
    LOADSTATE_NOT_LOADED = 0,
    LOADSTATE_LOADED = 1,
    LOADSTATE_REQUESTED = 2,
    LOADSTATE_CHANNELED = 3,
    LOADSTATE_FINISHING = 4
};

#pragma pack(push, 1)
typedef struct _VECTOR {
	float X, Y, Z;
	
	_VECTOR()
	{
		X = 0; Y = 0; Z = 0;
	}
	
	_VECTOR(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}
} VECTOR, *PVECTOR;

typedef struct _RECT {
	float X1, Y1, X2, Y2;
} RECT, *PRECT;

typedef struct _MATRIX4X4 {
	VECTOR right;
	uint32_t flags;
	VECTOR up;
	float pad_u;
	VECTOR at;
	float pad_a;
	VECTOR pos;
	float pad_p;
} MATRIX4X4, *PMATRIX4X4;

typedef struct _ENTITY_TYPE {
	uint32_t vtable;
	
	PADDING(_pad0, 12);
	
	float fRotZBeforeMat;
	
	MATRIX4X4 *mat;
	
	union {
		RwObject *m_pRwObject;
		RpClump *m_pRpClump;
		RpAtomic *m_pRpAtomic;
	};
	
	union {
		uint32_t m_nEntityFlags;
		struct {
			uint32_t m_bUsesCollision : 1;
			uint32_t m_bCollisionProcessed : 1;
			uint32_t m_bIsStatic : 1;
			uint32_t m_bHasContacted : 1;
			uint32_t m_bIsStuck : 1;
			uint32_t m_bIsInSafePosition : 1;
			uint32_t m_bWasPostponed : 1;
			uint32_t m_bIsVisible : 1;

			uint32_t m_bIsBIGBuilding : 1;
			uint32_t m_bRenderDamaged : 1;
			uint32_t m_bStreamingDontDelete : 1;
			uint32_t m_bRemoveFromWorld : 1;
			uint32_t m_bHasHitWall : 1;
			uint32_t m_bImBeingRendered : 1;
			uint32_t m_bDrawLast :1;
			uint32_t m_bDistanceFade : 1;
		 
			uint32_t m_bDontCastShadowsOn : 1;
			uint32_t m_bOffscreen : 1;
			uint32_t m_bIsStaticWaitingForCollision : 1;
			uint32_t m_bDontStream : 1;
			uint32_t m_bUnderwater : 1;
			uint32_t m_bHasPreRenderEffects : 1;
			uint32_t m_bIsTempBuilding : 1;
			uint32_t m_bDontUpdateHierarchy : 1;
		 
			uint32_t m_bHasRoadsignText : 1;
			uint32_t m_bDisplayedSuperLowLOD : 1;
			uint32_t m_bIsProcObject : 1;
			uint32_t m_bBackfaceCulled : 1;
			uint32_t m_bLightObject : 1;
			uint32_t m_bUnimportantStream : 1;
			uint32_t m_bTunnel : 1;
			uint32_t m_bTunnelTransition : 1;
		} nEntityFlags;
	};
	
	PADDING(_pad1, 2);
	
	uint16_t nModelIndex;
	
	PADDING(_pad2, 32);

	VECTOR vecMoveSpeed;
	VECTOR vecTurnSpeed;
	
	PADDING(_pad3, 88);
	
	uint32_t dwUnkModelRel;
} ENTITY_TYPE;

typedef struct _WEAPON_SLOT_TYPE {
	uint32_t dwType;
	uint32_t dwState;
	uint32_t dwAmmoInClip;
	uint32_t dwAmmo;
	
	PADDING(_pwep1,12);
} WEAPON_SLOT_TYPE;

typedef struct
{
	char unk[0x14];
	int iNodeId;
} AnimBlendFrameData;

typedef struct _PED_TYPE {
	ENTITY_TYPE entity;
	
	PADDING(_pad102, 904);
	
	uintptr_t dwPlayerInfoOffset;
	
	PADDING(_pad103, 4);
	
	uint32_t dwAction;
	
	PADDING(_pad104, 52);
	
	uint32_t dwStateFlags;
	
	PADDING(_pad105, 12);
	
	AnimBlendFrameData* pPedBones[19];
	
	PADDING(_pad106, 100);
	
	float fHealth;
	float fMaxHealth;
	float fArmour;
	
	PADDING(_pad107, 12);
	
	float fRotation1;
	float fRotation2;
	
	PADDING(_pad108, 44);
	
	uint32_t pVehicle;
	
	PADDING(_pad109, 8);
	
	uint32_t dwPedType;
	
	PADDING(_pad110, 4);
	
	WEAPON_SLOT_TYPE WeaponSlots[13];
	
	PADDING(_pad111, 12);
	
	uint8_t byteCurWeaponSlot;
	
	PADDING(_pad112, 75);
	
	uint32_t* pdwDamageEntity;
} PED_TYPE;

typedef struct _VEHICLE_TYPE {
	ENTITY_TYPE entity;

	PADDING(_pad201, 880);

	struct {
		unsigned char bIsLawEnforcer : 1;
		unsigned char bIsAmbulanceOnDuty : 1;
		unsigned char bIsFireTruckOnDuty : 1;
		unsigned char bIsLocked : 1;
		unsigned char bEngineOn : 1;
		unsigned char bIsHandbrakeOn : 1;
		unsigned char bLightsOn : 1;
		unsigned char bFreebies : 1;

		unsigned char bIsVan : 1;
		unsigned char bIsBus : 1;
		unsigned char bIsBig : 1;
		unsigned char bLowVehicle : 1;
		unsigned char bComedyControls : 1;
		unsigned char bWarnedPeds : 1;
		unsigned char bCraneMessageDone : 1;
		unsigned char bTakeLessDamage : 1;

		unsigned char bIsDamaged : 1;
		unsigned char bHasBeenOwnedByPlayer : 1;
		unsigned char bFadeOut : 1;
		unsigned char bIsBeingCarJacked : 1;
		unsigned char bCreateRoadBlockPeds : 1;
		unsigned char bCanBeDamaged : 1;
		unsigned char bOccupantsHaveBeenGenerated : 1;
		unsigned char bGunSwitchedOff : 1;

		unsigned char bVehicleColProcessed : 1;
		unsigned char bIsCarParkVehicle : 1;
		unsigned char bHasAlreadyBeenRecorded : 1;
		unsigned char bPartOfConvoy : 1;
		unsigned char bHeliMinimumTilt : 1;
		unsigned char bAudioChangingGear : 1;
		unsigned char bIsDrowning : 1;
		unsigned char bTyresDontBurst : 1;

		unsigned char bCreatedAsPoliceVehicle : 1;
		unsigned char bRestingOnPhysical : 1;
		unsigned char bParking : 1;
		unsigned char bCanPark : 1;
		unsigned char bFireGun : 1;
		unsigned char bDriverLastFrame : 1;
		unsigned char bNeverUseSmallerRemovalRange : 1;
		unsigned char bIsRCVehicle : 1;

		unsigned char bAlwaysSkidMarks : 1;
		unsigned char bEngineBroken : 1;
		unsigned char bVehicleCanBeTargetted : 1;
		unsigned char bPartOfAttackWave : 1;
		unsigned char bWinchCanPickMeUp : 1;
		unsigned char bImpounded : 1;
		unsigned char bVehicleCanBeTargettedByHS : 1;
		unsigned char bSirenOrAlarm : 1;

		unsigned char bHasGangLeaningOn : 1;
		unsigned char bGangMembersForRoadBlock : 1;
		unsigned char bDoesProvideCover : 1;
		unsigned char bMadDriver : 1;
		unsigned char bUpgradedStereo : 1;
		unsigned char bConsideredByPlayer : 1;
		unsigned char bPetrolTankIsWeakPoint : 1;
		unsigned char bDisableParticles : 1;

		unsigned char bHasBeenResprayed : 1;
		unsigned char bUseCarCheats : 1;
		unsigned char bDontSetColourWhenRemapping : 1;
		unsigned char bUsedForReplay : 1;
	} nFlags;

    unsigned int m_nCreationTime;

	uint8_t m_nPrimaryColor;
	uint8_t m_nSecondaryColor;
    uint8_t m_nTertiaryColor;
    uint8_t m_nQuaternaryColor;
	
	PADDING(_pad203, 40);

	PED_TYPE *m_pDriver;
	PED_TYPE *m_apPassengers[8];
	uint8_t m_nNumPassengers;

	PADDING(_pad204, 67);

	float m_fHealth;

	PADDING(_pad205, 56);

	uint32_t m_nDoorLock;

	/*PADDING(_pad206, 152);

	union {
		uint8_t m_nRenderLightsFlags;
		struct {
			uint8_t m_bRightFront : 1;
			uint8_t m_bLeftFront : 1;
			uint8_t m_bRightRear : 1;
			uint8_t m_bLeftRear : 1;
		} m_renderLights;
	};
	void *m_pCustomCarPlate; // (RwTexture *)

	PADDING(_pad207, 12);

	short m_nPreviousRemapTxd;
	short m_nRemapTxd;
	void *m_pRemapTexture; // (RwTexture *)*/
} VEHICLE_TYPE;

typedef struct _BULLET_DATA {
	uint32_t unk;
	VECTOR vecOrigin;
	VECTOR vecPos;
	VECTOR vecOffset;
	ENTITY_TYPE* pEntity;
} BULLET_DATA;

union tScriptParam {
	unsigned int uParam;
	int iParam;
	float fParam;
	void *pParam;
	char *szParam;
};

typedef struct _PED_MODEL
{
	uintptr_t vtable;
	uint8_t data[88];
} PED_MODEL;

typedef struct _OBJECT_MODEL {
	uintptr_t vtable;
	uint8_t data[52];
} OBJECT_MODEL;

typedef struct _PED_ACCESSORY {
	int			iModel;
	int			iBone;	
	VECTOR		vecOffset;
	VECTOR		vecRotation;
	VECTOR		vecScale;
	uint32_t 	uiFirstMaterialColor;
	uint32_t	uiSecondMaterialColor;
} PED_ACCESSORY;

typedef struct _PED_ANIM {
	union {
		struct {
			unsigned short	 m_nId : 16;
			unsigned char	 m_nFramedelta : 8;
			unsigned char   m_nLoopA : 1;
			unsigned char   m_nLockX : 1;
			unsigned char   m_nLockY : 1;
			unsigned char   m_nLockF : 1;
			unsigned char   m_nTime : 2;
		};
		int m_value;
	};
} PED_ANIM;

typedef struct _OBJECT_REMOVE {
	uint32_t dwModel;
	float fRange;
	VECTOR vecPosition;
} OBJECT_REMOVE;

#pragma pack(pop)

#define	VEHICLE_SUBTYPE_CAR				1
#define	VEHICLE_SUBTYPE_BIKE			2
#define	VEHICLE_SUBTYPE_HELI			3
#define	VEHICLE_SUBTYPE_BOAT			4
#define	VEHICLE_SUBTYPE_PLANE			5
#define	VEHICLE_SUBTYPE_PUSHBIKE		6
#define	VEHICLE_SUBTYPE_TRAIN			7

#define TRAIN_PASSENGER_LOCO			538
#define TRAIN_FREIGHT_LOCO				537
#define TRAIN_PASSENGER					570
#define TRAIN_FREIGHT					569
#define TRAIN_TRAM						449
#define HYDRA							520

#define ACTION_WASTED					55
#define ACTION_DEATH					54
#define ACTION_INCAR					50
#define ACTION_NORMAL					1
#define ACTION_SCOPE					12
#define ACTION_NONE						0 

#define WEAPON_MODEL_BRASSKNUCKLE		331 // was 332
#define WEAPON_MODEL_GOLFCLUB			333
#define WEAPON_MODEL_NITESTICK			334
#define WEAPON_MODEL_KNIFE				335
#define WEAPON_MODEL_BAT				336
#define WEAPON_MODEL_SHOVEL				337
#define WEAPON_MODEL_POOLSTICK			338
#define WEAPON_MODEL_KATANA				339
#define WEAPON_MODEL_CHAINSAW			341
#define WEAPON_MODEL_DILDO				321
#define WEAPON_MODEL_DILDO2				322
#define WEAPON_MODEL_VIBRATOR			323
#define WEAPON_MODEL_VIBRATOR2			324
#define WEAPON_MODEL_FLOWER				325
#define WEAPON_MODEL_CANE				326
#define WEAPON_MODEL_GRENADE			342 // was 327
#define WEAPON_MODEL_TEARGAS			343 // was 328
#define WEAPON_MODEL_MOLTOV				344 // was 329
#define WEAPON_MODEL_COLT45				346
#define WEAPON_MODEL_SILENCED			347
#define WEAPON_MODEL_DEAGLE				348
#define WEAPON_MODEL_SHOTGUN			349
#define WEAPON_MODEL_SAWEDOFF			350
#define WEAPON_MODEL_SHOTGSPA			351
#define WEAPON_MODEL_UZI				352
#define WEAPON_MODEL_MP5				353
#define WEAPON_MODEL_AK47				355
#define WEAPON_MODEL_M4					356
#define WEAPON_MODEL_TEC9				372
#define WEAPON_MODEL_RIFLE				357
#define WEAPON_MODEL_SNIPER				358
#define WEAPON_MODEL_ROCKETLAUNCHER		359
#define WEAPON_MODEL_HEATSEEKER			360
#define WEAPON_MODEL_FLAMETHROWER		361
#define WEAPON_MODEL_MINIGUN			362
#define WEAPON_MODEL_SATCHEL			363
#define WEAPON_MODEL_BOMB				364
#define WEAPON_MODEL_SPRAYCAN			365
#define WEAPON_MODEL_FIREEXTINGUISHER	366
#define WEAPON_MODEL_CAMERA				367
#define WEAPON_MODEL_NIGHTVISION		368	// newly added
#define WEAPON_MODEL_INFRARED			369	// newly added
#define WEAPON_MODEL_JETPACK			370	// newly added
#define WEAPON_MODEL_PARACHUTE			371

#define OBJECT_PARACHUTE				3131
#define OBJECT_CJ_CIGGY					1485
#define OBJECT_DYN_BEER_1				1486
#define OBJECT_CJ_BEER_B_2				1543
#define OBJECT_CJ_PINT_GLASS			1546

#define WEAPON_BRASSKNUCKLE				1
#define WEAPON_GOLFCLUB					2
#define WEAPON_NITESTICK				3
#define WEAPON_KNIFE					4
#define WEAPON_BAT						5
#define WEAPON_SHOVEL					6
#define WEAPON_POOLSTICK				7
#define WEAPON_KATANA					8
#define WEAPON_CHAINSAW					9
#define WEAPON_DILDO					10
#define WEAPON_DILDO2					11
#define WEAPON_VIBRATOR					12
#define WEAPON_VIBRATOR2				13
#define WEAPON_FLOWER					14
#define WEAPON_CANE						15
#define WEAPON_GRENADE					16
#define WEAPON_TEARGAS					17
#define WEAPON_MOLTOV					18
#define WEAPON_ROCKET					19
#define WEAPON_ROCKET_HS				20
#define WEAPON_FREEFALLBOMB				21
#define WEAPON_COLT45					22
#define WEAPON_SILENCED					23
#define WEAPON_DEAGLE					24
#define WEAPON_SHOTGUN					25
#define WEAPON_SAWEDOFF					26
#define WEAPON_SHOTGSPA					27
#define WEAPON_UZI						28
#define WEAPON_MP5						29
#define WEAPON_AK47						30
#define WEAPON_M4						31
#define WEAPON_TEC9						32
#define WEAPON_RIFLE					33
#define WEAPON_SNIPER					34
#define WEAPON_ROCKETLAUNCHER			35
#define WEAPON_HEATSEEKER				36
#define WEAPON_FLAMETHROWER				37
#define WEAPON_MINIGUN					38
#define WEAPON_SATCHEL					39
#define WEAPON_BOMB						40
#define WEAPON_SPRAYCAN					41
#define WEAPON_FIREEXTINGUISHER			42
#define WEAPON_CAMERA					43
#define WEAPON_NIGHTVISION				44
#define WEAPON_INFRARED					45
#define WEAPON_PARACHUTE				46
#define WEAPON_ARMOUR					47
#define WEAPON_VEHICLE					49
#define WEAPON_HELIBLADES				50
#define WEAPON_EXPLOSION				51
#define WEAPON_DROWN					53
#define WEAPON_COLLISION				54