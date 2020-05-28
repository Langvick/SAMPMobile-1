#include "../main.h"
#include "../util/armhook.h"
#include "RW/RenderWare.h"
#include "../playerslist.h"
#include "game.h"
#include "net/netgame.h"
#include "gui/gui.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CPlayersList *pPlayersList;
extern CGUI *pGUI;

bool g_bPlaySAMP = false;

extern bool bNetworkInited;
extern bool bGameInited;

extern int g_iLagCompensation;

void InitSAMP();
void InitInMenu();
void InitInGame();
void ProcessSAMPGraphic();
void ProcessSAMPGraphicFrame();
void InitNetwork();
void ProcessSAMPNetwork();
void HookCPad();
void InstallSCMHooks();

extern "C" uintptr_t get_lib() {
	return g_libGTASA;
}

struct stFile {
	int isFileExist;
	FILE *pFile;
};

stFile* (*NvFOpen)(const char*, const char*, int, int);
stFile* NvFOpen_hook(const char* r0, const char* r1, int r2, int r3)
{
	char path[0xFF] = { 0 };

	if(!strncmp(r1 + 12, "mainV1.scm", 10)) {
		sprintf(path, "%sSAMP/main.scm", g_pszStorage);
		Log("Loading mainV1.scm..");
		goto open;
	}

	if(!strncmp(r1 + 12, "SCRIPTV1.IMG", 12)) {
		sprintf(path, "%sSAMP/script.img", g_pszStorage);
		Log("Loading script.img..");
		goto open;
	}

	if(!strncmp(r1, "DATA/PEDS.IDE", 13)) {
		sprintf(path, "%s/SAMP/peds.ide", g_pszStorage);
		Log("Loading peds.ide..");
		goto open;
	}

	if(!strncmp(r1, "DATA/VEHICLES.IDE", 17)) {
		sprintf(path, "%s/SAMP/vehicles.ide", g_pszStorage);
		Log("Loading vehicles.ide..");
		goto open;
	}

	if(!strncmp(r1, "DATA/WEAPON.DAT", 15)) {
		sprintf(path, "%s/SAMP/weapon.dat", g_pszStorage);
		Log("Loading weapon.dat..");
		goto open;
	}

orig:
	return NvFOpen(r0, r1, r2, r3);

open:
	stFile *st = (stFile *)malloc(8);
	st->isFileExist = false;

	FILE *pFile = fopen(path, "rb");
	if(pFile) {
		st->isFileExist = true;
		st->pFile = pFile;
		return st;
	} else {
		Log("NVFOpen hook | Error: file not found (%s)", path);
		free(st);
		st = nullptr;
		return 0;
	}
}

int Init_hook(int r0, int r1, int r2)
{
	int result = ((int (*)(int, int, int))(g_libGTASA + 0x00244F2C + 1))(r0, r1, r2);

	InitSAMP();

	return result;
}

// Render2dStuff(void) — 0x0039AEF4
void (*Render2dStuff)(void);
void Render2dStuff_hook(void) {
	Render2dStuff();

	if(g_bPlaySAMP) {
		InitInGame();
	}
	return;
}

// Render2dStuffAfterFade(void) — 0x0039B098
void (*Render2dStuffAfterFade)(void);
void Render2dStuffAfterFade_hook(void) {
	// CHud::DrawAfterFade(void) — 0x003D7258
	((void (*)(void))(g_libGTASA + 0x003D7258 + 1))();
	
	// TemporaryFPSVisualization(void) — 0x0039B054
	((void (*)(void))(g_libGTASA + 0x0039B054 + 1))();

	if(g_bPlaySAMP) {
		ProcessSAMPGraphic();
	}

	// emu_GammaSet(uchar) — 0x00198010
	((void (*)(uint8_t))(g_libGTASA + 0x00198010 + 1))(1);

	// CMessages::Display(uchar) — 0x004D240C
	((void (*)(uint8_t))(g_libGTASA + 0x004D240C + 1))(0);

	// CFont::RenderFontBuffer(void) — 0x0053411C
	((void (*)(void))(g_libGTASA + 0x0053411C + 1))();

	// CCredits::Render(void) — 0x003FC43C
	((void (*)(void))(g_libGTASA + 0x003FC43C + 1))();

	// emu_GammaSet(uchar) — 0x00198010
	((void (*)(uint8_t))(g_libGTASA + 0x00198010 + 1))(0);
	
	if(g_bPlaySAMP) {
		ProcessSAMPGraphicFrame();
	}
	return;
}

// CRunningScript::Process(void) — 0x002E1E00
int32_t (*CRunningScript__Process)(uint32_t *_this);
int32_t CRunningScript__Process_hook(uint32_t *_this) {
	if(!bGameInited) {
		return CRunningScript__Process(_this);
	}

	InitNetwork();
	ProcessSAMPNetwork();
	return CRunningScript__Process(_this);
}

uint32_t dwShutDownTick;

void (*CGame__ShutDown)(void);
void CGame__ShutDown_hook(void)
{
	dwShutDownTick = GetTickCount() + 2000;
	QuitGame();

	while(GetTickCount() < dwShutDownTick) {
		sleep(100);
	}
	exit(0);
}

uint16_t wszGXTStr[0x7F];
static uint8_t bMenuRowIndex = -1, bMenuColumnIndex = -1;
uint16_t* (*CText_Get)(uintptr_t _this, const char* szText);
uint16_t* CText_Get_hook(uintptr_t _this, const char* szText)
{
	if(szText[0] == 'S' && szText[1] == 'A' && szText[2] == 'M' && szText[3] == 'P') 
	{
		const char* szCode = &szText[4];
		if(!pNetGame) {
			if(!strcmp(szCode, "MP"))  {
				CFont::AsciiToGxtChar("MultiPlayer", wszGXTStr);
			}
		} else {
			if (pNetGame->GetMenuPool()) {
				CFont::AsciiToGxtChar(pNetGame->GetMenuPool()->GetTextPointer((char*)szCode, &bMenuRowIndex, &bMenuColumnIndex), wszGXTStr);
			}
		}
		return wszGXTStr;
	}

	return CText_Get(_this, szText);
}

void MainMenu_OnStartSAMP()
{
	Log("MainMenu: MultiPlayer selected.");

	if(g_bPlaySAMP) {
		return;
	}
	
	InitInMenu();

	((void (*)())(g_libGTASA + 0x00261C8C + 1))();

	g_bPlaySAMP = true;
	return;
}

void (*MenuItem_add)(int r0, uintptr_t r1);
void MenuItem_add_hook(int r0, uintptr_t r1)
{
	static bool bMenuInited = false;
	char* name = *(char **)(r1 + 4);

	if(!strcmp(name, "FEP_STG") && !bMenuInited)
	{
		Log("Creating \"MultiPlayer\" button.. (struct: 0x%X)", r1);

		MenuItem_add(r0, r1);

		*(uintptr_t *)r1 = LoadTextureFromDB("samp", "menu_mainmp");
		*(char **)(r1 + 4) = "SAMP_MP";
		*(uintptr_t *)(r1 + 8) = (uintptr_t)MainMenu_OnStartSAMP;

		bMenuInited = true;
		goto ret;
	}

	if(g_bPlaySAMP && (!strcmp(name, "FEP_STG") || !strcmp(name, "FEH_STA") || !strcmp(name, "FEH_BRI"))) {
		return;
	}
	
ret:
	return MenuItem_add(r0, r1);
}

void (*InitialiseRenderWare)();
void InitialiseRenderWare_hook()
{
	Log("InitialiseRenderWare()");

	InitialiseRenderWare();

	((void (*)(const char *, int, int))(g_libGTASA + 0x001BF244 + 1))("samp", 0, 5);
	((void (*)(const char *, int, int))(g_libGTASA + 0x001BF244 + 1))("gtasa", 0, 5);
	return;
}

void RenderSplashScreen();
void (*CLoadingScreen__DisplayPCScreen)();
void CLoadingScreen__DisplayPCScreen_hook()
{
	RwCamera* camera = *(RwCamera **)(g_libGTASA + 0x0095B064);
	const float percent = *(float*)(g_libGTASA + 0x008F08C0);
	
	if(RwCameraBeginUpdate(camera)) {
		DefinedState2d();
		((void (*)())(g_libGTASA + 0x005519C8 + 1))(); // CSprite2d::InitPerFrame()
		RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
		((void (*)(bool))(g_libGTASA + 0x00198010 + 1))(false); // emu_GammaSet()
		
		RenderSplashScreen();
		RwCameraEndUpdate(camera);
		RwCameraShowRaster(camera, 0, 0);
	}
	return;
}

void (*TouchEvent)(int, int, int posX, int posY);
void TouchEvent_hook(int type, int num, int posX, int posY)
{
	if(pGUI) {
		bool bRet = pGUI->OnTouchEvent(type, num, posX, posY);
		if(bRet) {
			TouchEvent(type, num, posX, posY);
		}
		return;
	}
	
	return TouchEvent(type, num, posX, posY);
}

void (*CStreaming__InitImageList)();
void CStreaming__InitImageList_hook()
{
	char *ms_files = (char *)(g_libGTASA + 0x006702FC);
	ms_files[0] = 0;
	*(uint32_t *)&ms_files[44] = 0;
	ms_files[48] = 0;
	*(uint32_t *)&ms_files[92] = 0;
	ms_files[96] = 0;
	*(uint32_t *)&ms_files[140] = 0;
	ms_files[144] = 0;
	*(uint32_t *)&ms_files[188] = 0;
	ms_files[192] = 0;
	*(uint32_t *)&ms_files[236] = 0;
	ms_files[240] = 0;
	*(uint32_t *)&ms_files[284] = 0;
	ms_files[288] = 0;
	*(uint32_t *)&ms_files[332] = 0;
	ms_files[336] = 0;
	*(uint32_t *)&ms_files[380] = 0;

	((int (*)(const char *, bool))(g_libGTASA + 0x0028E7B0 + 1))("TEXDB\\GTA3.IMG", true); // CStreaming::AddImageToList [IMG]
	((int (*)(const char *, bool))(g_libGTASA + 0x0028E7B0 + 1))("TEXDB\\GTA_INT.IMG", true); // CStreaming::AddImageToList [IMG]

	((int (*)(char const *))(g_libGTASA + 0x00405690 + 1))("SAMP\\SAMP.IDE"); // CFileLoader::LoadObjectTypes [IDE]
	((int (*)(const char *, bool))(g_libGTASA + 0x0028E7B0 + 1))("TEXDB\\SAMP.IMG", true); // CStreaming::AddImageToList [IMG]
	((int (*)(const char *, bool))(g_libGTASA + 0x0028E7B0 + 1))("TEXDB\\SAMPCOL.IMG", true); // CStreaming::AddImageToList [IMG]
}

PED_MODEL pedsModels[315];
int pedsModelsCount = 0;

PED_MODEL* (*CModelInfo__AddPedModel)(int iModelId);
PED_MODEL* CModelInfo__AddPedModel_hook(int iModelId)
{
	PED_MODEL* pPedModel = &pedsModels[pedsModelsCount];
	memset(pPedModel, 0, sizeof(PED_MODEL));

	pPedModel->vtable = (uintptr_t)(g_libGTASA + 0x005C6E90);
	((uintptr_t (*)(PED_MODEL *))(*(void **)(pPedModel->vtable + 0x1C)))(pPedModel);
	*(PED_MODEL **)(g_libGTASA + 0x0087BF48 + (iModelId * 0x04)) = pPedModel;

	pedsModelsCount++;

	return pPedModel;
}

OBJECT_MODEL atomicModels[27342];
int atomicModelsCount = 0;
 
OBJECT_MODEL *(*CModelInfo__AddAtomicModel)(int iModelId);
OBJECT_MODEL *CModelInfo__AddAtomicModel_hook(int iModelId)
{
	OBJECT_MODEL *pObjectModel = &atomicModels[atomicModelsCount];
	memset(pObjectModel, 0, sizeof(OBJECT_MODEL));

	pObjectModel->vtable = (uintptr_t)(g_libGTASA + 0x005C6C68);
	((uintptr_t (*)(OBJECT_MODEL *))(*(void **)(pObjectModel->vtable + 0x1C)))(pObjectModel);
	*(OBJECT_MODEL **)(g_libGTASA + 0x0087BF48 + (iModelId * 0x04)) = pObjectModel;

	atomicModelsCount++;

	return pObjectModel;
}

uint32_t (*CRadar__GetRadarTraceColor)(uint32_t color, uint8_t bright, uint8_t friendly);
uint32_t CRadar__GetRadarTraceColor_hook(uint32_t color, uint8_t bright, uint8_t friendly)
{
	return TranslateColorCodeToRGBA(color);
}

int (*CRadar__SetCoordBlip)(int r0, float X, float Y, float Z, int r4, int r5, char* name);
int CRadar__SetCoordBlip_hook(int r0, float X, float Y, float Z, int r4, int r5, char* name)
{
	if(pNetGame && !strncmp(name, "CODEWAY", 7)) {
		float findZ = ((float (*)(float, float))(g_libGTASA + 0x003C3DD8 + 1))(X, Y);
		findZ += 1.5f;

		RakNet::BitStream bsSend;
		bsSend.Write(X);
		bsSend.Write(Y);
		bsSend.Write(findZ);
		pNetGame->GetRakClient()->RPC(RPC_MapMarker, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
	}

	return CRadar__SetCoordBlip(r0, X, Y, Z, r4, r5, name);
}

uint8_t bGZ = 0;
void (*CRadar__DrawRadarGangOverlay)(uint8_t v1);
void CRadar__DrawRadarGangOverlay_hook(uint8_t v1)
{
	bGZ = v1;
	if (pNetGame) {
		CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
		if(pGangZonePool) {
			pGangZonePool->Draw();
		}
	}
}

uint32_t dwParam;
extern "C" void doPickingUpPickup()
{
	if(pNetGame) {
		CPickupPool *pPickupPool = pNetGame->GetPickupPool();
		if(pPickupPool) {
			pPickupPool->PickedUp(((dwParam - (g_libGTASA + 0x0070E264)) / 0x20));
		}
	}
}

__attribute__((naked)) void PickupPickUp_hook()
{
	__asm__ volatile("push {lr}\n\t"
					"push {r0}\n\t"
					"blx get_lib\n\t"
					"add r0, #0x2D0000\n\t"
					"add r0, #0x009A00\n\t"
					"add r0, #1\n\t"
					"mov r1, r0\n\t"
					"pop {r0}\n\t"
					"pop {lr}\n\t"
					"push {r1}\n\t");

	__asm__ volatile("push {r0-r11, lr}\n\t"
					"mov %0, r4" : "=r" (dwParam));

	__asm__ volatile("blx doPickingUpPickup\n\t");

	__asm__ volatile("pop {r0-r11, lr}\n\t");

	__asm__ volatile("ldrb r1, [r4, #0x1C]\n\t"
					"sub.w r2, r1, #0xD\n\t"
					"sub.w r2, r1, #8\n\t"
					"cmp r1, #6\n\t"
					"pop {pc}\n\t");
}

extern "C" bool NotifyEnterVehicle(VEHICLE_TYPE *_pVehicle)
{
	if(!pNetGame) {
		return false;
	}
	
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool) {
		uint16_t vehicleId = pVehiclePool->FindIDFromGtaPtr(_pVehicle);
		CVehicle *pVehicle = pVehiclePool->GetAt(vehicleId);
		if(pVehicle) {
			if(pVehicle->m_bIsLocked) {
				return false;
			}

			if(pVehicle->m_pVehicle->entity.nModelIndex == TRAIN_PASSENGER) {
				return false;
			}

			if(pVehicle->m_pVehicle->m_pDriver) {
				if(pVehicle->m_pVehicle->m_pDriver->dwPedType != 0) {
					return false;
				}
			}
			
			CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
			if(pPlayerPool) {
				CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
				if(pLocalPlayer) {
					pLocalPlayer->SendEnterVehicleNotification(vehicleId, false);
				}
			}
		}
	}

	return true;
}

void (*CTaskComplexEnterCarAsDriver)(uint32_t _this, uint32_t pVehicle);
extern "C" void call_taskEnterCarAsDriver(uintptr_t a, uint32_t b)
{
	CTaskComplexEnterCarAsDriver(a, b);
}

void __attribute__((naked)) CTaskComplexEnterCarAsDriver_hook(uint32_t _this, uint32_t pVehicle)
{
	__asm__ volatile("push {r0-r11, lr}\n\t"
					"mov r2, lr\n\t"
					"blx get_lib\n\t"
					"add r0, #0x3A0000\n\t"
					"add r0, #0xEE00\n\t"
					"add r0, #0xF7\n\t"
					"cmp r2, r0\n\t"
					"bne 1f\n\t" 
					"mov r0, r1\n\t"
					"blx NotifyEnterVehicle\n\t" 
					"1:\n\t"  
					"pop {r0-r11, lr}\n\t"
					"push {r0-r11, lr}\n\t"
					"blx call_taskEnterCarAsDriver\n\t"
					"pop {r0-r11, pc}");
}

void (*CTaskComplexLeaveCar)(uintptr_t** _this, VEHICLE_TYPE *pVehicle, int iTargetDoor, int iDelayTime, bool bSensibleLeaveCar, bool bForceGetOut);
void CTaskComplexLeaveCar_hook(uintptr_t** _this, VEHICLE_TYPE *pVehicle, int iTargetDoor, int iDelayTime, bool bSensibleLeaveCar, bool bForceGetOut) 
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	if(dwRetAddr == 0x003AE905 || dwRetAddr == 0x003AE9CF) {
		if(pNetGame) {
			PED_TYPE *pLocalPed = pGame->FindPlayerPed()->GetGtaActor();
			if(pLocalPed) {
				if(pLocalPed->pVehicle == (uint32_t)pVehicle) {
					CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
					if(pVehiclePool) {
						CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
						if(pPlayerPool) {
							CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
							if(pLocalPlayer) {
								uint16_t vehicleId = pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)pLocalPed->pVehicle);
								pLocalPlayer->SendExitVehicleNotification(vehicleId);
							}
						}
					}
				}
			}
		}
	}

	(*CTaskComplexLeaveCar)(_this, pVehicle, iTargetDoor, iDelayTime, bSensibleLeaveCar, bForceGetOut);
}

void (*CPools__Initialise)(void);
void CPools__Initialise__hook(void)
{
	Log("GTA pools initializing..");

	struct PoolAllocator {

		struct Pool {
			void *objects;
			uint8_t *flags;
			uint32_t count;
			uint32_t top;
			uint32_t bInitialized;
		};
		static_assert(sizeof(Pool) == 0x14);

		static Pool* Allocate(size_t count, size_t size) {
			
			Pool *p = new Pool;

			p->objects = new char[size*count];
			p->flags = new uint8_t[count];
			p->count = count;
			p->top = 0xFFFFFFFF;
			p->bInitialized = 1;

			for (size_t i = 0; i < count; i++) {
				p->flags[i] |= 0x80;
				p->flags[i] &= 0x80;
			}

			return p;
		}
	};
	
	// 600000 / 75000 = 8
	static auto ms_pPtrNodeSingleLinkPool	= PoolAllocator::Allocate(100000,	8);		// 75000
	// 72000 / 6000 = 12
	static auto ms_pPtrNodeDoubleLinkPool	= PoolAllocator::Allocate(20000,	12);	// 6000
	// 10000 / 500 = 20
	static auto ms_pEntryInfoNodePool		= PoolAllocator::Allocate(20000,	20);	// 500
	// 279440 / 140 = 1996
	static auto ms_pPedPool					= PoolAllocator::Allocate(240,		1996);	// 140
	// 286440 / 110 = 2604
	static auto ms_pVehiclePool				= PoolAllocator::Allocate(2000,		2604);	// 110
	// 840000 / 14000 = 60
	static auto ms_pBuildingPool			= PoolAllocator::Allocate(20000,	60);	// 14000
	// 147000 / 350 = 420
	static auto ms_pObjectPool				= PoolAllocator::Allocate(3000,		420);	// 350
	// 210000 / 3500 = 60
	static auto ms_pDummyPool				= PoolAllocator::Allocate(40000,	60);	// 3500
	// 487200 / 10150 = 48
	static auto ms_pColModelPool			= PoolAllocator::Allocate(50000,	48);	// 10150
	// 64000 / 500 = 128
	static auto ms_pTaskPool				= PoolAllocator::Allocate(5000,		128);	// 500
	// 13600 / 200 = 68
	static auto ms_pEventPool				= PoolAllocator::Allocate(1000,		68);	// 200
	// 6400 / 64 = 100
	static auto ms_pPointRoutePool			= PoolAllocator::Allocate(200,		100);	// 64
	// 13440 / 32 = 420
	static auto ms_pPatrolRoutePool			= PoolAllocator::Allocate(200,		420);	// 32
	// 2304 / 64 = 36
	static auto ms_pNodeRoutePool			= PoolAllocator::Allocate(200,		36);	// 64
	// 512 / 16 = 32
	static auto ms_pTaskAllocatorPool		= PoolAllocator::Allocate(3000,		32);	// 16
	// 92960 / 140 = 664
	static auto ms_pPedIntelligencePool		= PoolAllocator::Allocate(240,		664);	// 140
	// 15104 / 64 = 236
	static auto ms_pPedAttractorPool		= PoolAllocator::Allocate(200,		236);	// 64

	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93E0) = ms_pPtrNodeSingleLinkPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93DC) = ms_pPtrNodeDoubleLinkPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93D8) = ms_pEntryInfoNodePool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93D4) = ms_pPedPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93D0) = ms_pVehiclePool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93CC) = ms_pBuildingPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93C8) = ms_pObjectPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93C4) = ms_pDummyPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93C0) = ms_pColModelPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93BC) = ms_pTaskPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93B8) = ms_pEventPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93B4) = ms_pPointRoutePool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93B0) = ms_pPatrolRoutePool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93AC) = ms_pNodeRoutePool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93A8) = ms_pTaskAllocatorPool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93A4) = ms_pPedIntelligencePool;
	*(PoolAllocator::Pool**)(g_libGTASA + 0x008B93A0) = ms_pPedAttractorPool;
}

uint32_t (*CWeapon__FireInstantHit)(WEAPON_SLOT_TYPE* _this, PED_TYPE* pFiringEntity, VECTOR* vecOrigin, VECTOR* muzzlePosn, ENTITY_TYPE* targetEntity, 
	VECTOR *target, VECTOR* originForDriveBy, int arg6, int muzzle);
uint32_t CWeapon__FireInstantHit_hook(WEAPON_SLOT_TYPE* _this, PED_TYPE* pFiringEntity, VECTOR* vecOrigin, VECTOR* muzzlePosn, ENTITY_TYPE* targetEntity, 
	VECTOR *target, VECTOR* originForDriveBy, int arg6, int muzzle)
{
	uintptr_t dwRetAddr = 0;
 	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));

 	dwRetAddr -= g_libGTASA;
 	if(	dwRetAddr == 0x569A84 + 1 ||
 		dwRetAddr == 0x569616 + 1 ||
 		dwRetAddr == 0x56978A + 1 ||
 		dwRetAddr == 0x569C06 + 1)
 	{
		PED_TYPE *pLocalPed = pGame->FindPlayerPed()->GetGtaActor();
		if(pLocalPed) {
			if(pFiringEntity != pLocalPed) {
				return muzzle;
			}

			if(pNetGame) {
				CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
				if(pPlayerPool) {
					pPlayerPool->ApplyCollisionChecking();
				}
			}

			if(pGame) {
				CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
				if(pPlayerPed) {
					pPlayerPed->FireInstant();
				}
			}

			if(pNetGame) {
				CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
				if(pPlayerPool) {
					pPlayerPool->ResetCollisionChecking();
				}
			}

			return muzzle;
		}
 	}

 	return CWeapon__FireInstantHit(_this, pFiringEntity, vecOrigin, muzzlePosn, targetEntity, target, originForDriveBy, arg6, muzzle);
}

extern CPlayerPed *g_pCurrentFiredPed;
extern BULLET_DATA *g_pCurrentBulletData;

void SendBulletSync(VECTOR *vecOrigin, VECTOR *vecEnd, VECTOR *vecPos, ENTITY_TYPE **ppEntity)
{
	static BULLET_DATA bulletData;
	memset(&bulletData, 0, sizeof(BULLET_DATA));

	bulletData.vecOrigin.X = vecOrigin->X;
  	bulletData.vecOrigin.Y = vecOrigin->Y;
  	bulletData.vecOrigin.Z = vecOrigin->Z;
  	bulletData.vecPos.X = vecPos->X;
  	bulletData.vecPos.Y = vecPos->Y;
  	bulletData.vecPos.Z = vecPos->Z;

  	if(ppEntity) {
  		static ENTITY_TYPE *pEntity;
  		pEntity = *ppEntity;
  		if(pEntity) {
  			if(pEntity->mat) {
  				if(g_iLagCompensation) {
  					bulletData.vecOffset.X = vecPos->X - pEntity->mat->pos.X;
		  			bulletData.vecOffset.Y = vecPos->Y - pEntity->mat->pos.Y;
		  			bulletData.vecOffset.Z = vecPos->Z - pEntity->mat->pos.Z;
  				} else {
  					static MATRIX4X4 mat1;
  					memset(&mat1, 0, sizeof(mat1));

  					static MATRIX4X4 mat2;
		  			memset(&mat2, 0, sizeof(mat2));

		  			RwMatrixOrthoNormalize(&mat2, pEntity->mat);
		  			RwMatrixInvert(&mat1, &mat2);
		  			ProjectMatrix(&bulletData.vecOffset, &mat1, vecPos);
  				}
  			}

  			bulletData.pEntity = pEntity;
  		}
  	}

  	pGame->FindPlayerPed()->ProcessBulletData(&bulletData);
}

uint32_t (*CWorld__ProcessLineOfSight)(VECTOR*,VECTOR*, VECTOR*, PED_TYPE**, bool, bool, bool, bool, bool, bool, bool, bool);
uint32_t CWorld__ProcessLineOfSight_hook(VECTOR* vecOrigin, VECTOR* vecEnd, VECTOR* vecPos, PED_TYPE** ppEntity, 
	bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8)
{
	uintptr_t dwRetAddr = 0;
 	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));

 	dwRetAddr -= g_libGTASA;
 	if(	dwRetAddr == 0x55E2FE + 1 || 
 		dwRetAddr == 0x5681BA + 1 ||  
 		dwRetAddr == 0x567AFC + 1)	
 	{
 		ENTITY_TYPE *pEntity = nullptr;
 		MATRIX4X4 *pMatrix = nullptr;
 		static VECTOR vecPosPlusOffset;

 		if(g_iLagCompensation != 2) {
 			if(g_pCurrentFiredPed != pGame->FindPlayerPed()) {
 				if(g_pCurrentBulletData) {
					if(g_pCurrentBulletData->pEntity) {
						if(!IsGameEntityArePlaceable(g_pCurrentBulletData->pEntity)) {
							pMatrix = g_pCurrentBulletData->pEntity->mat;
							if(pMatrix) {
								if(g_iLagCompensation) {
									vecPosPlusOffset.X = pMatrix->pos.X + g_pCurrentBulletData->vecOffset.X;
									vecPosPlusOffset.Y = pMatrix->pos.Y + g_pCurrentBulletData->vecOffset.Y;
									vecPosPlusOffset.Z = pMatrix->pos.Z + g_pCurrentBulletData->vecOffset.Z;
								} else {
									ProjectMatrix(&vecPosPlusOffset, pMatrix, &g_pCurrentBulletData->vecOffset);
								}

								vecEnd->X = vecPosPlusOffset.X - vecOrigin->X + vecPosPlusOffset.X;
								vecEnd->Y = vecPosPlusOffset.Y - vecOrigin->Y + vecPosPlusOffset.Y;
								vecEnd->Z = vecPosPlusOffset.Z - vecOrigin->Z + vecPosPlusOffset.Z;
							}
						}
					}
 				}
 			}
 		}

 		static uint32_t result = 0;
 		result = CWorld__ProcessLineOfSight(vecOrigin, vecEnd, vecPos, ppEntity, b1, b2, b3, b4, b5, b6, b7, b8);

 		if(g_iLagCompensation == 2) {
 			if(g_pCurrentFiredPed) {
 				if(g_pCurrentFiredPed == pGame->FindPlayerPed()) {
 					SendBulletSync(vecOrigin, vecEnd, vecPos, (ENTITY_TYPE**)ppEntity);
				}
 			}

 			return result;
 		}

 		if(g_pCurrentFiredPed) {
 			if(g_pCurrentFiredPed != pGame->FindPlayerPed()) {
 				if(g_pCurrentBulletData) {
 					if(!g_pCurrentBulletData->pEntity) {
 						PED_TYPE *pLocalPed = pGame->FindPlayerPed()->GetGtaActor();
 						if(*ppEntity == pLocalPed || (IN_VEHICLE(pLocalPed) &&  *(uintptr_t*)ppEntity == pLocalPed->pVehicle)) {
 							*ppEntity = nullptr;
 							vecPos->X = 0.0f;
 							vecPos->Y = 0.0f;
 							vecPos->Z = 0.0f;
 							return 0;
 						}
 					}
 				}
 			}
 		}

 		if(g_pCurrentFiredPed) {
 			if(g_pCurrentFiredPed == pGame->FindPlayerPed()) {
 				SendBulletSync(vecOrigin, vecEnd, vecPos, (ENTITY_TYPE **)ppEntity);
			}
 		}

 		return result;
 	}

	return CWorld__ProcessLineOfSight(vecOrigin, vecEnd, vecPos, ppEntity, b1, b2, b3, b4, b5, b6, b7, b8);
}

// CObject::Render(void) — 0x003EF518
int32_t (*CObject__Render)(ENTITY_TYPE *_this);
int32_t CObject__Render_hook(ENTITY_TYPE *_this)
{
	if(!_this) {
		return CObject__Render(_this);
	}

	if(!_this->m_pRpAtomic) {			
		return CObject__Render(_this);
	}

	if(!_this->m_pRpAtomic->geometry) {
		return CObject__Render(_this);
	}

	if(IsGameEntityArePlaceable(_this)) {
		return CObject__Render(_this);
	}

	if(!pNetGame) {
		return CObject__Render(_this);
	}

	if(!pNetGame->GetObjectPool()) {
		return CObject__Render(_this);
	}

	CObject *pSAMPObject = pNetGame->GetObjectPool()->GetObjectFromGtaPtr(_this);
	if(pSAMPObject) {
		static int32_t retnValue = 0;

		// reset prelight
		// DeActivateDirectional(void) — 0x00559EF8
		((void (*)(void))(g_libGTASA + 0x00559EF8 + 1))();

		// do entity texture change
		if(pSAMPObject->m_objectMaterial.isUsed) {			
			// store orig texture data
			if(!pSAMPObject->m_objectMaterial.origData.isStored) {
				for(int i = 0; i < _this->m_pRpAtomic->geometry->matList.numMaterials; ++i) {
					if(_this->m_pRpAtomic->geometry->matList.materials[i]) {
						if(i >= MAX_MATERIALS_PER_MODEL) {
							break;
						}
						
						// pSAMPObject->m_objectMaterial.origData.materialData[i].cColor = _this->m_pRpAtomic->geometry->matList.materials[i]->color;
						pSAMPObject->m_objectMaterial.origData.materialData[i].pTexture = _this->m_pRpAtomic->geometry->matList.materials[i]->texture;
					}
				}
				
				pSAMPObject->m_objectMaterial.origData.isStored = true;
			}

			// change textures in entity
			for(int i = 0; i < _this->m_pRpAtomic->geometry->matList.numMaterials; ++i) {
				if(_this->m_pRpAtomic->geometry->matList.materials[i]) {
					if(i >= MAX_MATERIALS_PER_MODEL) {
						break;
					}
					
					if(pSAMPObject->m_objectMaterial.materialData[i].isUsed) {
						// _this->m_pRpAtomic->geometry->matList.materials[i]->color = pSAMPObject->m_objectMaterial.materialData[i].cColor;
						if(pSAMPObject->m_objectMaterial.materialData[i].pTexture) {
							_this->m_pRpAtomic->geometry->matList.materials[i]->texture = pSAMPObject->m_objectMaterial.materialData[i].pTexture;
						}
					}
				}
			}

			// do render
			retnValue = CObject__Render(_this);
			
			// restore orig data
			for(int i = 0; i < _this->m_pRpAtomic->geometry->matList.numMaterials; ++i) {
				if(_this->m_pRpAtomic->geometry->matList.materials[i]) {
					if(i >= MAX_MATERIALS_PER_MODEL) {
						break;
					}
					
					// _this->m_pRpAtomic->geometry->matList.materials[i]->color = pSAMPObject->m_objectMaterial.origData.materialData[i].cColor;
					if(pSAMPObject->m_objectMaterial.origData.materialData[i].pTexture) {
						_this->m_pRpAtomic->geometry->matList.materials[i]->texture = pSAMPObject->m_objectMaterial.origData.materialData[i].pTexture;
					}
				}
			}
			
			// restore prelight
			// ActivateDirectional(void)	— 0x00559F14
			((void (*)(void))(g_libGTASA + 0x00559F14 + 1))();
		} else {
			// do render
			retnValue = CObject__Render(_this);

			// restore prelight
			// ActivateDirectional(void)	— 0x00559F14
			((void (*)(void))(g_libGTASA + 0x00559F14 + 1))();
		}
		return retnValue;
	}
	return CObject__Render(_this);
}

// CEntity::Render(void) — 0x00391E20
int32_t (*CEntity__Render)(ENTITY_TYPE *_this);
int32_t CEntity__Render_hook(ENTITY_TYPE *_this)
{
	if(!_this) {
		return CEntity__Render(_this);
	}
	
	if(IsGameEntityArePlaceable(_this)) {
		return CEntity__Render(_this);
	}
	
	std::vector<OBJECT_REMOVE> objectList = pGame->GetObjectsToRemoveList();
	for(int i = 0; i < objectList.size(); ++i)
	{
		if(_this->nModelIndex == objectList.at(i).dwModel)
		{
			MATRIX4X4 *pMatrix = _this->mat;
			if(pMatrix)
			{
				VECTOR vecObjectPosition = pMatrix->pos;
				if(IsValidPosition(vecObjectPosition))
				{
					VECTOR vecRemovePosition = objectList.at(i).vecPosition;
					if(IsValidPosition(vecRemovePosition))
					{
						if(GetDistanceFromVectorToVector(&vecObjectPosition, &vecRemovePosition) <= objectList.at(i).fRange) {
							_this->nEntityFlags.m_bIsVisible = 0;
							_this->nEntityFlags.m_bUsesCollision = 0;
						}
					}
				}
			}
		}
	}
	return CEntity__Render(_this);
}

void (*CWorld__ProcessPedsAfterPreRender)();
void CWorld__ProcessPedsAfterPreRender_hook()
{
	if(!pNetGame) {
		return CWorld__ProcessPedsAfterPreRender();
	}
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(!pPlayerPool) {
		return CWorld__ProcessPedsAfterPreRender();
	}

	if(pPlayerPool) {
		CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
		if(pPlayerPed) {
			pPlayerPed->UpdateAccessories();
		}

		for(int i = 0; i < MAX_PLAYERS; i++) {
			CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(i);
			if(pRemotePlayer) {
				CPlayerPed *pPlayerPed = pRemotePlayer->GetPlayerPed();
				if(pPlayerPed) {
					pPlayerPed->UpdateAccessories();
				}
			}
		}
	}

	return CWorld__ProcessPedsAfterPreRender();
}

void (*CFont__PrintString)(float x, float y, uint16_t *gxtString);
void CFont__PrintString_hook(float x, float y, uint16_t *gxtString) // crutch for calcucale menu's clickable area
{
	if(!pNetGame) {
		return CFont__PrintString(x, y, gxtString);
	}
	
	CMenuPool *pMenuPool = pNetGame->GetMenuPool();
	if(!pMenuPool) {
		return CFont__PrintString(x, y, gxtString);
	}
	
	CMenu *pMenu = pMenuPool->GetCurrentMenu();
	if(!pMenu) {
		return CFont__PrintString(x, y, gxtString);	
	}
	
	if(bMenuColumnIndex != -1 && bMenuRowIndex != INVALID_MENU_ROW_ID) {
		CFont__PrintString(x, y, gxtString);
		
		RECT rect;
		CFont::GetTextRect(&rect, x, y);

		float fHeight = CFont::GetHeight(false);
		// float fWidth = CFont::GetStringWidth(gxtString);

		rect.Y1 += fHeight / 4.0f;
		rect.Y2 = rect.Y1 + fHeight - fHeight / 4.0f; // WTF
		rect.X2 = rect.X1 + (CFont::GetStringWidth(gxtString) * 2);  // WTF

		pMenu->m_MenuItemRect[bMenuColumnIndex][bMenuRowIndex] = rect;
		bMenuRowIndex = INVALID_MENU_ROW_ID;
		bMenuColumnIndex = -1;
		return;
	}
	
	return CFont__PrintString(x, y, gxtString);
}

void InstallSpecialHooks()
{
	InstallMethodHook(g_libGTASA + 0x005DDC60, (uintptr_t)Init_hook);

	SetUpHook(g_libGTASA + 0x00269974, (uintptr_t)MenuItem_add_hook, (uintptr_t *) &MenuItem_add);
	SetUpHook(g_libGTASA + 0x004D3864, (uintptr_t)CText_Get_hook, (uintptr_t *) &CText_Get);
	SetUpHook(g_libGTASA + 0x0040C530, (uintptr_t)InitialiseRenderWare_hook, (uintptr_t *) &InitialiseRenderWare);
	SetUpHook(g_libGTASA + 0x003AF1A0, (uintptr_t)CPools__Initialise__hook, (uintptr_t *) &CPools__Initialise);
}

void InstallHooks()
{
	CodeInject(g_libGTASA + 0x002D99F4, (uintptr_t)PickupPickUp_hook, 1);

	SetUpHook(g_libGTASA + 0x0039AEF4, (uintptr_t)Render2dStuff_hook, (uintptr_t *) &Render2dStuff);
	SetUpHook(g_libGTASA + 0x0039B098, (uintptr_t)Render2dStuffAfterFade_hook, (uintptr_t *) &Render2dStuffAfterFade);
	SetUpHook(g_libGTASA + 0x002E1E00, (uintptr_t)CRunningScript__Process_hook, (uintptr_t *) &CRunningScript__Process);
	SetUpHook(g_libGTASA + 0x00398334, (uintptr_t)CGame__ShutDown_hook, (uintptr_t *) &CGame__ShutDown);

	SetUpHook(g_libGTASA + 0x0023B3DC, (uintptr_t)NvFOpen_hook, (uintptr_t *) &NvFOpen);
	SetUpHook(g_libGTASA + 0x00239D5C, (uintptr_t)TouchEvent_hook, (uintptr_t *) &TouchEvent);
	SetUpHook(g_libGTASA + 0x003D7CA8, (uintptr_t)CLoadingScreen__DisplayPCScreen_hook, (uintptr_t *) &CLoadingScreen__DisplayPCScreen);
	SetUpHook(g_libGTASA + 0x0028E83C, (uintptr_t)CStreaming__InitImageList_hook, (uintptr_t *) &CStreaming__InitImageList);
	SetUpHook(g_libGTASA + 0x00336690, (uintptr_t)CModelInfo__AddPedModel_hook, (uintptr_t *) &CModelInfo__AddPedModel);
	SetUpHook(g_libGTASA + 0x00336268, (uintptr_t)CModelInfo__AddAtomicModel_hook, (uintptr_t *) &CModelInfo__AddAtomicModel);
	SetUpHook(g_libGTASA + 0x003DBA88, (uintptr_t)CRadar__GetRadarTraceColor_hook, (uintptr_t *) &CRadar__GetRadarTraceColor);
	SetUpHook(g_libGTASA + 0x003DAF84, (uintptr_t)CRadar__SetCoordBlip_hook, (uintptr_t *) &CRadar__SetCoordBlip);
	SetUpHook(g_libGTASA + 0x003DE9A8, (uintptr_t)CRadar__DrawRadarGangOverlay_hook, (uintptr_t *) &CRadar__DrawRadarGangOverlay);
	SetUpHook(g_libGTASA + 0x00482E60, (uintptr_t)CTaskComplexEnterCarAsDriver_hook, (uintptr_t *) &CTaskComplexEnterCarAsDriver);
	SetUpHook(g_libGTASA + 0x004833CC, (uintptr_t)CTaskComplexLeaveCar_hook, (uintptr_t *) &CTaskComplexLeaveCar);
	SetUpHook(g_libGTASA + 0x00567964, (uintptr_t)CWeapon__FireInstantHit_hook, (uintptr_t *) &CWeapon__FireInstantHit);
	SetUpHook(g_libGTASA + 0x003C70C0, (uintptr_t)CWorld__ProcessLineOfSight_hook, (uintptr_t *) &CWorld__ProcessLineOfSight);
	SetUpHook(g_libGTASA + 0x003EF518, (uintptr_t)CObject__Render_hook, (uintptr_t *) &CObject__Render);
	SetUpHook(g_libGTASA + 0x00391E20, (uintptr_t)CEntity__Render_hook, (uintptr_t *) &CEntity__Render);
	SetUpHook(g_libGTASA + 0x003C1BF8, (uintptr_t)CWorld__ProcessPedsAfterPreRender_hook, (uintptr_t *) &CWorld__ProcessPedsAfterPreRender);
	SetUpHook(g_libGTASA + 0x005353B4, (uintptr_t)CFont__PrintString_hook, (uintptr_t *) &CFont__PrintString);

	HookCPad();
	InstallSCMHooks();
}