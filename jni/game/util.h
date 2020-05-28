#pragma once

PED_TYPE *GamePool_FindPlayerPed();

PED_TYPE *GamePool_Ped_GetAt(int iPedId);
int GamePool_Ped_GetIndex(PED_TYPE *pActor);

VEHICLE_TYPE *GamePool_Vehicle_GetAt(int iVehicleId);
int GamePool_Vehicle_GetIndex(VEHICLE_TYPE *pVehicle);

ENTITY_TYPE *GamePool_Object_GetAt(int iObjectId);

bool IsPedModel(uint32_t uiModelId);
bool IsValidGamePed(PED_TYPE *pPed);
bool IsValidGameEntity(ENTITY_TYPE *pEntity);
bool IsGameEntityArePlaceable(ENTITY_TYPE *pEntity);
uint16_t GetModelReferenceCount(uint32_t uiModelId);
bool PreloadAnimFile(char *szAnimFile, uint16_t usSleep, uint16_t usHowTimes);

void InitPlayerPedPtrRecords();
void SetPlayerPedPtrRecord(uint8_t bytePlayer, uintptr_t dwPedPtr);
uint8_t FindPlayerNumFromPedPtr(uintptr_t dwPedPtr);

uintptr_t FindRwTexture(std::string szTexDb, std::string szTexName);
uintptr_t GetTexture(std::string szTexName);
uintptr_t LoadTextureFromDB(std::string szTexDb, std::string szTexName);

void DefinedState2d();
void SetScissorRect(void* pRect);
float DegToRad(float fDegrees);

float FloatOffset(float f1, float f2);

int GameGetWeaponModelIDFromWeaponID(int iWeaponID);

int Weapon_FireSniper(WEAPON_SLOT_TYPE* pWeaponSlot, PED_TYPE* pPed);
uintptr_t GetWeaponInfo(int iWeapon, int iSkill);

void ProjectMatrix(VECTOR* vecOut, MATRIX4X4* mat, VECTOR *vecPos);
void RwMatrixOrthoNormalize(MATRIX4X4 *matIn, MATRIX4X4 *matOut);
void RwMatrixInvert(MATRIX4X4 *matOut, MATRIX4X4 *matIn);
void RwMatrixRotate(MATRIX4X4 *mat, int axis, float angle);
void RwMatrixScale(MATRIX4X4 *matrix, VECTOR *vecScale);

float GetDistanceFromVectorToVector(VECTOR *vecFrom, VECTOR *vecTo);
bool IsValidPosition(VECTOR const& vecPosition);