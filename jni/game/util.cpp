#include "../main.h"
#include "../chatwindow.h"
#include "game.h"
#include <cmath>

extern CGame *pGame;
extern CChatWindow *pChatWindow;

#define PI 3.14159265

uintptr_t dwPlayerPedPtrs[PLAYER_PED_SLOTS];

extern char* PLAYERS_REALLOC;

PED_TYPE* GamePool_FindPlayerPed()
{
	return *(PED_TYPE**)PLAYERS_REALLOC;
}

PED_TYPE *GamePool_Ped_GetAt(int iPedId)
{
	return ((PED_TYPE *(*)(int))(g_libGTASA + 0x0041DD7C + 1))(iPedId);
}

int GamePool_Ped_GetIndex(PED_TYPE *pActor)
{
	return ((int (*)(PED_TYPE*))(g_libGTASA + 0x0041DD60 + 1))(pActor);
}

VEHICLE_TYPE *GamePool_Vehicle_GetAt(int iVehicleId)
{
	return ((VEHICLE_TYPE *(*)(int))(g_libGTASA + 0x0041DD44 + 1))(iVehicleId);
}

int GamePool_Vehicle_GetIndex(VEHICLE_TYPE *pVehicle)
{
	return ((int (*)(VEHICLE_TYPE*))(g_libGTASA + 0x0041DD28 + 1))(pVehicle);
}

ENTITY_TYPE *GamePool_Object_GetAt(int iObjectId)
{
	return ((ENTITY_TYPE *(*)(int))(g_libGTASA + 0x0041DDB4 + 1))(iObjectId);
} 

bool IsPedModel(uint32_t uiModelId) {
	if(uiModelId < 0 || uiModelId > 20000) {
		return false;
	}

	uintptr_t *dwModelArray = (uintptr_t *)(g_libGTASA + 0x0087BF48);
	uintptr_t dwModelInfo = dwModelArray[uiModelId];
	if(dwModelInfo && *(uintptr_t *)dwModelInfo == (uintptr_t)(g_libGTASA + 0x005C6E90/*CPedModelInfo vtable*/)) {
		return true;
	}
	return false;
}

bool IsValidGameEntity(ENTITY_TYPE *pEntity) {
	// IsEntityPointerValid(CEntity *) — 0x00393870
	if(((bool (*)(ENTITY_TYPE *))(g_libGTASA + 0x00393870 + 1))(pEntity)) {
		return true;
	}
	return false;
}

bool IsGameEntityArePlaceable(ENTITY_TYPE *pEntity) {
	if(pEntity) {
		if(pEntity->vtable == g_libGTASA + 0x005C7358) {
			return true;
		}
	}
	return false;
}

uint16_t GetModelReferenceCount(uint32_t uiModelId) {
	if(uiModelId < 0 || uiModelId > 20000) {
		return 0;
	}

	uintptr_t *dwModelArray = (uintptr_t *)(g_libGTASA + 0x0087BF48);
	uint8_t *pModelInfoStart = (uint8_t *)dwModelArray[uiModelId];
	return *(uint16_t *)(pModelInfoStart + 0x1E);
}

bool PreloadAnimFile(char *szAnimFile, uint16_t usSleep, uint16_t usHowTimes) {
	if(pGame->IsAnimationLoaded(szAnimFile)) {
		return true;
	}
	
	pGame->RequestAnimation(szAnimFile);

	int iWaitAnimLoad = 0;
	while(!pGame->IsAnimationLoaded(szAnimFile)) {
		usleep(usSleep);
		if(usHowTimes != -1) {
			if(++iWaitAnimLoad > usHowTimes) {
				return false;
			}
		}
	}
	return true;
}

void InitPlayerPedPtrRecords()
{
	memset(&dwPlayerPedPtrs[0], 0, sizeof(uintptr_t) * PLAYER_PED_SLOTS);
}

void SetPlayerPedPtrRecord(uint8_t bytePlayer, uintptr_t dwPedPtr)
{
	dwPlayerPedPtrs[bytePlayer] = dwPedPtr;
}

uint8_t FindPlayerNumFromPedPtr(uintptr_t dwPedPtr)
{
	uint8_t x = 0;
	while(x != PLAYER_PED_SLOTS) {
		if(dwPlayerPedPtrs[x] == dwPedPtr) {
			return x;
		}
		x++;
	}

	return 0;
}

uintptr_t FindRwTexture(std::string szTexDb, std::string szTexName) {
	uintptr_t pRwTexture = GetTexture(szTexName + std::string("_") + szTexDb);
	if(!pRwTexture) {
		pRwTexture = GetTexture(szTexDb + std::string("_") + szTexName);
		if(!pRwTexture) {
			pRwTexture = GetTexture(szTexName);
			if(!pRwTexture) {
				return 0;
			}
		}
	}
	return pRwTexture;
}

uintptr_t GetTexture(std::string szTexName) {
	uintptr_t pRwTexture = ((uintptr_t (*)(char const *))(g_libGTASA + 0x001BE990 + 1))(szTexName.c_str());
	if(pRwTexture) {
		++*(uint32_t *)(pRwTexture + 0x54);
	}
	return pRwTexture;
}

uintptr_t LoadTextureFromDB(std::string szTexDb, std::string szTexName)
{
	// TextureDatabaseRuntime::GetDatabase(char const*)
	char const *pszTexDb = szTexDb.c_str();
	uintptr_t pTextureDatabaseRuntime = ((uintptr_t (*)(char const *))(g_libGTASA + 0x001BF530 + 1))(pszTexDb);
	if(!pTextureDatabaseRuntime) {
		Log("Error: Database not found! (%s)", pszTexDb);
		return 0;
	}

	// TextureDatabaseRuntime::Register(TextureDatabaseRuntime *)
	((void (*)(uintptr_t))(g_libGTASA + 0x001BE898 + 1))(pTextureDatabaseRuntime);

	uintptr_t pRwTexture = GetTexture(szTexName);
	if(!pRwTexture) {
		Log("Error: Texture (%s) not found in database (%s)", szTexName.c_str(), pszTexDb);
	}
	
	// TextureDatabaseRuntime::Unregister(TextureDatabaseRuntime *)
	((void (*)(uintptr_t))(g_libGTASA + 0x001BE938 + 1))(pTextureDatabaseRuntime);
	return pRwTexture;
}

void DefinedState2d()
{
	return (( void (*)())(g_libGTASA + 0x005590B0 + 1))();
}

void SetScissorRect(void* pRect)
{
}

float DegToRad(float fDegrees)
{
	if (fDegrees > 360.0f || fDegrees < 0.0f) return 0.0f;

	if (fDegrees > 180.0f) return (float)(-(PI - (((fDegrees - 180.0f) * PI) / 180.0f)));
	else return (float)((fDegrees * PI) / 180.0f);
}

float FloatOffset(float f1, float f2)
{   
	if(f1 >= f2) return f1 - f2;
	else return (f2 - f1);
}

int GameGetWeaponModelIDFromWeaponID(int iWeaponID)
{
	switch(iWeaponID)
	{
	case WEAPON_BRASSKNUCKLE:
		return WEAPON_MODEL_BRASSKNUCKLE;

	case WEAPON_GOLFCLUB:
		return WEAPON_MODEL_GOLFCLUB;

	case WEAPON_NITESTICK:
		return WEAPON_MODEL_NITESTICK;

	case WEAPON_KNIFE:
		return WEAPON_MODEL_KNIFE;

	case WEAPON_BAT:
		return WEAPON_MODEL_BAT;

	case WEAPON_SHOVEL:
		return WEAPON_MODEL_SHOVEL;

	case WEAPON_POOLSTICK:
		return WEAPON_MODEL_POOLSTICK;

	case WEAPON_KATANA:
		return WEAPON_MODEL_KATANA;

	case WEAPON_CHAINSAW:
		return WEAPON_MODEL_CHAINSAW;

	case WEAPON_DILDO:
		return WEAPON_MODEL_DILDO;

	case WEAPON_DILDO2:
		return WEAPON_MODEL_DILDO2;

	case WEAPON_VIBRATOR:
		return WEAPON_MODEL_VIBRATOR;

	case WEAPON_VIBRATOR2:
		return WEAPON_MODEL_VIBRATOR2;

	case WEAPON_FLOWER:
		return WEAPON_MODEL_FLOWER;

	case WEAPON_CANE:
		return WEAPON_MODEL_CANE;

	case WEAPON_GRENADE:
		return WEAPON_MODEL_GRENADE;

	case WEAPON_TEARGAS:
		return WEAPON_MODEL_TEARGAS;

	case WEAPON_MOLTOV:
		return WEAPON_MODEL_MOLTOV;

	case WEAPON_COLT45:
		return WEAPON_MODEL_COLT45;

	case WEAPON_SILENCED:
		return WEAPON_MODEL_SILENCED;

	case WEAPON_DEAGLE:
		return WEAPON_MODEL_DEAGLE;

	case WEAPON_SHOTGUN:
		return WEAPON_MODEL_SHOTGUN;

	case WEAPON_SAWEDOFF:
		return WEAPON_MODEL_SAWEDOFF;

	case WEAPON_SHOTGSPA:
		return WEAPON_MODEL_SHOTGSPA;

	case WEAPON_UZI:
		return WEAPON_MODEL_UZI;

	case WEAPON_MP5:
		return WEAPON_MODEL_MP5;

	case WEAPON_AK47:
		return WEAPON_MODEL_AK47;

	case WEAPON_M4:
		return WEAPON_MODEL_M4;

	case WEAPON_TEC9:
		return WEAPON_MODEL_TEC9;

	case WEAPON_RIFLE:
		return WEAPON_MODEL_RIFLE;

	case WEAPON_SNIPER:
		return WEAPON_MODEL_SNIPER;

	case WEAPON_ROCKETLAUNCHER:
		return WEAPON_MODEL_ROCKETLAUNCHER;

	case WEAPON_HEATSEEKER:
		return WEAPON_MODEL_HEATSEEKER;

	case WEAPON_FLAMETHROWER:
		return WEAPON_MODEL_FLAMETHROWER;

	case WEAPON_MINIGUN:
		return WEAPON_MODEL_MINIGUN;

	case WEAPON_SATCHEL:
		return WEAPON_MODEL_SATCHEL;

	case WEAPON_BOMB:
		return WEAPON_MODEL_BOMB;

	case WEAPON_SPRAYCAN:
		return WEAPON_MODEL_SPRAYCAN;

	case WEAPON_FIREEXTINGUISHER:
		return WEAPON_MODEL_FIREEXTINGUISHER;

	case WEAPON_CAMERA:
		return WEAPON_MODEL_CAMERA;

	case WEAPON_NIGHTVISION:
		return WEAPON_MODEL_NIGHTVISION;

	case WEAPON_INFRARED:
		return WEAPON_MODEL_INFRARED;

	case WEAPON_PARACHUTE:
		return WEAPON_MODEL_PARACHUTE;

	}

	return -1;
}

int Weapon_FireSniper(WEAPON_SLOT_TYPE* pWeaponSlot, PED_TYPE* pPed)
{
	return ((int (*)(WEAPON_SLOT_TYPE*, PED_TYPE*))(g_libGTASA + 0x0056668C + 1))(pWeaponSlot, pPed);
}

uintptr_t GetWeaponInfo(int iWeapon, int iSkill)
{
	// CWeaponInfo::GetWeaponInfo
	return ((uintptr_t (*)(int, int))(g_libGTASA + 0x0056BD60 + 1))(iWeapon, iSkill);
}

void ProjectMatrix(VECTOR* vecOut, MATRIX4X4* mat, VECTOR *vecPos)
{
	vecOut->X = mat->at.X * vecPos->Z + mat->up.X * vecPos->Y + mat->right.X * vecPos->X + mat->pos.X;
	vecOut->Y = mat->at.Y * vecPos->Z + mat->up.Y * vecPos->Y + mat->right.Y * vecPos->X + mat->pos.Y;
	vecOut->Z = mat->at.Z * vecPos->Z + mat->up.Z * vecPos->Y + mat->right.Z * vecPos->X + mat->pos.Z;
}

void RwMatrixOrthoNormalize(MATRIX4X4 *matIn, MATRIX4X4 *matOut)
{
	((void (*)(MATRIX4X4*, MATRIX4X4*))(g_libGTASA + 0x001B8CC8 + 1))(matIn, matOut);
}

void RwMatrixInvert(MATRIX4X4 *matOut, MATRIX4X4 *matIn)
{
	((void (*)(MATRIX4X4*, MATRIX4X4*))(g_libGTASA + 0x001B91CC + 1))(matOut, matIn);
}

void RwMatrixRotate(MATRIX4X4 *mat, int axis, float angle)
{
	static float rMat[3][3] = 
	{
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f }
	};

	// RwMatrixRotate
	(( void (*)(MATRIX4X4*, float*, float, int))(g_libGTASA + 0x001B9118 + 1))(mat, rMat[axis], angle, 1);
}

void RwMatrixScale(MATRIX4X4 *matrix, VECTOR *vecScale)
{
	matrix->right.X *= vecScale->X;
	matrix->right.Y *= vecScale->X;
	matrix->right.Z *= vecScale->X;

	matrix->up.X *= vecScale->Y;
	matrix->up.Y *= vecScale->Y;
	matrix->up.Z *= vecScale->Y;

	matrix->at.X *= vecScale->Z;
	matrix->at.Y *= vecScale->Z;
	matrix->at.Z *= vecScale->Z;

	matrix->flags &= 0xFFFDFFFC;
}

float GetDistanceFromVectorToVector(VECTOR *vecFrom, VECTOR *vecTo) {
	float fX = (vecFrom->X - vecTo->X) * (vecFrom->X - vecTo->X);
	float fY = (vecFrom->Y - vecTo->Y) * (vecFrom->Y - vecTo->Y);
	float fZ = (vecFrom->Z - vecTo->Z) * (vecFrom->Z - vecTo->Z);

	return (float)sqrt(fX + fY + fZ);
}

bool IsValidPosition(VECTOR const& vecPosition) {
    if (vecPosition.X < -16000 || vecPosition.X > 16000 || std::isnan(vecPosition.X) || 
		vecPosition.Y < -16000 || vecPosition.Y > 16000 || std::isnan(vecPosition.Y) || 
		vecPosition.Z < -5000 || vecPosition.Z > 100000 || std::isnan(vecPosition.Z)) {
        return false;
	}
    return true;
}