#include "../main.h"
#include "../settings.h"
#include "../util/armhook.h"
#include "common.h"

extern CSettings *pSettings;

char* PLAYERS_REALLOC = nullptr;

void ApplyPatches_level0()
{
	// reallocate CWorld::Players[]
	//PLAYERS_REALLOC = new char[404*PLAYER_PED_SLOTS];
	PLAYERS_REALLOC = (( char* (*)(int))(g_libGTASA + 0x00179B40))(404*MAX_PLAYERS);
	UnFuck(g_libGTASA + 0x005D021C);
	*(char**)(g_libGTASA + 0x005D021C) = PLAYERS_REALLOC;
	Log("CWorld::Player new address = 0x%X", PLAYERS_REALLOC);

	// CdStreamInit(6);
	WriteMemory(g_libGTASA + 0x003981EC, (uintptr_t)"\x06\x20", 2);
}

void ApplyPatches()
{
	Log("Installing patches..");

	// CAudioEngine::StartLoadingTune
	NOP(g_libGTASA + 0x0056C150, 2);

	// DefaultPCSaveFileName
	char* DefaultPCSaveFileName = (char*)(g_libGTASA + 0x0060EAE8);
	memcpy((char*)DefaultPCSaveFileName, "GTASAMP", 8);

	// CWidgetRegionSteeringSelection::Draw
	WriteMemory(g_libGTASA + 0x00284BB8, (uintptr_t)"\xF7\x46", 2);

	// CHud::SetHelpMessage
	WriteMemory(g_libGTASA + 0x003D4244, (uintptr_t)"\xF7\x46", 2);
	// CHud::SetHelpMessageStatUpdate
	WriteMemory(g_libGTASA + 0x003D42A8, (uintptr_t)"\xF7\x46", 2);
	// CVehicleRecording::Load
	WriteMemory(g_libGTASA + 0x002DC8E0, (uintptr_t)"\xF7\x46", 2);

	// nop calling CRealTimeShadowManager::ReturnRealTimeShadow from ~CPhysical
	NOP(g_libGTASA + 0x003A019C, 2);

 	// CPed pool (old: 140, new: 210)
 	/* 	MOVW R0, #0x5EC8
 		MOVT R0, #6 */
 	WriteMemory(g_libGTASA + 0x003AF2D0, (uintptr_t)"\x45\xF6\xC8\x60\xC0\xF2\x06\x00", 8); // MOV  R0, #0x65EC8 | size=0x7C4 (old: 0x43F30)
 	WriteMemory(g_libGTASA + 0x003AF2DE, (uintptr_t)"\xD2\x20", 2); // MOVS R0, #0xD2
 	WriteMemory(g_libGTASA + 0x003AF2E4, (uintptr_t)"\xD2\x22", 2); // MOVS R2, #0xD2
 	WriteMemory(g_libGTASA + 0x003AF310, (uintptr_t)"\xD2\x2B", 2); // CMP  R3, #0xD2

 	// CPedIntelligence pool (old: 140, new: 210)
	// movw r0, #0x20B0
 	// movt r0, #2
 	// nop
 	WriteMemory(g_libGTASA + 0x003AF7E6, (uintptr_t)"\x42\xF2\xB0\x00\xC0\xF2\x02\x00\x00\x46", 10); // MOVS R0, #0x220B0 | size=0x298 (old: 0x16B20)
 	WriteMemory(g_libGTASA + 0x003AF7F6, (uintptr_t)"\xD2\x20", 2); // MOVS R0, #0xD2
 	WriteMemory(g_libGTASA + 0x003AF7FC, (uintptr_t)"\xD2\x22", 2); // MOVS R2, #0xD2
 	WriteMemory(g_libGTASA + 0x003AF824, (uintptr_t)"\xD2\x2B", 2); // CMP  R3, 0xD2

 	// Task pool (old: 500, new: 1524 (1536))
 	WriteMemory(g_libGTASA + 0x003AF4EA, (uintptr_t)"\x4F\xF4\x40\x30", 4); // MOV.W R0, #30000 | size = 0x80 (old: 0xFA00)
 	WriteMemory(g_libGTASA + 0x003AF4F4, (uintptr_t)"\x4F\xF4\xC0\x60", 4); // MOV.W R0, #0x600
 	WriteMemory(g_libGTASA + 0x003AF4FC, (uintptr_t)"\x4F\xF4\xC0\x62", 4); // MOV.W R2, #0x600
 	WriteMemory(g_libGTASA + 0x003AF52A, (uintptr_t)"\xB3\xF5\xC0\x6F", 4); // CMP.W R3, #0x600

 	// Event pool (old: 200, new: 512)
 	

 	// ColModel pool (old:10150, new: 32511)
 	// mov r0, #0xCFD0
 	// movt r0, #0x17
 	WriteMemory(g_libGTASA + 0x003AF48E, (uintptr_t)"\x4C\xF6\xD0\x70\xC0\xF2\x17\x00", 8); // MOV R0, #0x17CFD0 | size=0x30 (old: 0x76F20)
 	WriteMemory(g_libGTASA + 0x003AF49C, (uintptr_t)"\x47\xF6\xFF\x60", 4); // MOVW R0, #0x7EFF
 	WriteMemory(g_libGTASA + 0x003AF4A4, (uintptr_t)"\x47\xF6\xFF\x62", 4); // MOVW R2, #0x7EFF

 	// VehicleStruct increase (0x32C*0x50 = 0xFDC0)
    WriteMemory(g_libGTASA + 0x00405338, (uintptr_t)"\x4F\xF6\xC0\x50", 4);	// MOV  R0, #0xFDC0
    WriteMemory(g_libGTASA + 0x00405342, (uintptr_t)"\x50\x20", 2);			// MOVS R0, #0x50
    WriteMemory(g_libGTASA + 0x00405348, (uintptr_t)"\x50\x22", 2);			// MOVS R2, #0x50
    WriteMemory(g_libGTASA + 0x00405374, (uintptr_t)"\x50\x2B", 2);			// CMP  R3, #0x50

    // Increase matrix count in CPlaceable::InitMatrixArray 
 	WriteMemory(g_libGTASA + 0x003ABB0A, (uintptr_t)"\x4F\xF4\x7A\x61", 4); // MOV.W R1, #4000
	
	// Frame Limiter
	switch(pSettings->Get().iFrameLimit) {
		case 20:
		WriteMemory(g_libGTASA + 0x00463FE8, (uintptr_t)"\x14", 1);
		WriteMemory(g_libGTASA + 0x0056C1F6, (uintptr_t)"\x14", 1);
		WriteMemory(g_libGTASA + 0x0056C126, (uintptr_t)"\x14", 1);
		WriteMemory(g_libGTASA + 0x0095B074, (uintptr_t)"\x14", 1);
		break;
		
		case 25:
		WriteMemory(g_libGTASA + 0x00463FE8, (uintptr_t)"\x19", 1);
		WriteMemory(g_libGTASA + 0x0056C1F6, (uintptr_t)"\x19", 1);
		WriteMemory(g_libGTASA + 0x0056C126, (uintptr_t)"\x19", 1);
		WriteMemory(g_libGTASA + 0x0095B074, (uintptr_t)"\x19", 1);
		break;
		
		case 30:
		WriteMemory(g_libGTASA + 0x00463FE8, (uintptr_t)"\x1E", 1);
		WriteMemory(g_libGTASA + 0x0056C1F6, (uintptr_t)"\x1E", 1);
		WriteMemory(g_libGTASA + 0x0056C126, (uintptr_t)"\x1E", 1);
		WriteMemory(g_libGTASA + 0x0095B074, (uintptr_t)"\x1E", 1);
		break;
		
		case 48:
		WriteMemory(g_libGTASA + 0x00463FE8, (uintptr_t)"\x30", 1);
		WriteMemory(g_libGTASA + 0x0056C1F6, (uintptr_t)"\x30", 1);
		WriteMemory(g_libGTASA + 0x0056C126, (uintptr_t)"\x30", 1);
		WriteMemory(g_libGTASA + 0x0095B074, (uintptr_t)"\x30", 1);
		break;
		
		case 60:
		WriteMemory(g_libGTASA + 0x00463FE8, (uintptr_t)"\x3C", 1);
		WriteMemory(g_libGTASA + 0x0056C1F6, (uintptr_t)"\x3C", 1);
		WriteMemory(g_libGTASA + 0x0056C126, (uintptr_t)"\x3C", 1);
		WriteMemory(g_libGTASA + 0x0095B074, (uintptr_t)"\x3C", 1);
		break;
	}
}

void ApplyInGamePatches()
{
	Log("Installing patches (ingame)..");

	/* Разблокировка карты */
	// CTheZones::ZonesVisited[100]
	memset((void*)(g_libGTASA + 0x008EA7B0), 1, 100);
	
	// CTheZones::ZonesRevealed
	*(uint32_t*)(g_libGTASA + 0x008EA7A8) = 100;

	// CTaskSimplePlayerOnFoot::PlayIdleAnimations 
	WriteMemory(g_libGTASA + 0x004BDB18, (uintptr_t)"\x70\x47", 2);

	// CarCtl::GenerateRandomCars nulled from CGame::Process
	UnFuck(g_libGTASA + 0x00398A3A);
	NOP(g_libGTASA + 0x00398A3A, 2);

	// CTheCarGenerators::Process nulled from CGame::Process
	UnFuck(g_libGTASA + 0x00398A34);
	NOP(g_libGTASA + 0x00398A34, 2);

	// множитель для MaxHealth
	UnFuck(g_libGTASA + 0x003BAC68);
	*(float*)(g_libGTASA + 0x003BAC68) = 176.0f;
	
	// множитель для Armour
	UnFuck(g_libGTASA + 0x0027D884);
	*(float*)(g_libGTASA + 0x0027D884) = 176.0f;

	// CEntryExit::GenerateAmbientPeds nulled from CEntryExit::TransitionFinished
	UnFuck(g_libGTASA + 0x002C2C22);
	NOP(g_libGTASA + 0x002C2C22, 4);

	// ПОТРАЧЕНО
	WriteMemory(g_libGTASA + 0x003D62FC, (uintptr_t)"\xF7\x46", 2);

	// CPlayerPed::CPlayerPed task fix
	WriteMemory(g_libGTASA + 0x00458ED1, (uintptr_t)"\xE0", 1);

	// ReapplyPlayerAnimation (хз зачем)
	NOP(g_libGTASA + 0x0045477E, 5);

	// radar draw blips
    UnFuck(g_libGTASA + 0x003DCA90);
    NOP(g_libGTASA + 0x003DCA90, 2);
    UnFuck(g_libGTASA + 0x003DD4A4);
    NOP(g_libGTASA + 0x003DD4A4, 2);
	
    // CCamera::CamShake from CExplosion::AddExplosion
    NOP(g_libGTASA + 0x0055EFB8, 2);
    NOP(g_libGTASA + 0x0055F8F8, 2);

    // camera_on_actor path
    UnFuck(g_libGTASA + 0x002F7B68);
    *(uint8_t*)(g_libGTASA + 0x002F7B6B) = 0xBE;

    // CPed::RemoveWeaponWhenEnteringVehicle (GetPlayerInfoForThisPlayerPed)
    UnFuck(g_libGTASA + 0x00434D94);
    NOP(g_libGTASA + 0x00434D94, 6);

    // CBike::ProcessAI
    UnFuck(g_libGTASA + 0x004EE200);
    *(uint8_t*)(g_libGTASA + 0x004EE200) = 0x9B;

    // CWidgetPlayerInfo::DrawWanted
    WriteMemory(g_libGTASA + 0x0027D8D0, (uintptr_t)"\x4F\xF0\x00\x08", 4);

    // no vehicle audio processing
    UnFuck(g_libGTASA + 0x004E31A6);
    NOP(g_libGTASA + 0x004E31A6, 2);
    UnFuck(g_libGTASA + 0x004EE7D2);
    NOP(g_libGTASA + 0x004EE7D2, 2);
    UnFuck(g_libGTASA + 0x004F741E);
    NOP(g_libGTASA + 0x004F741E, 2);
    UnFuck(g_libGTASA + 0x0050AB4A);
    NOP(g_libGTASA + 0x0050AB4A, 2);
	
	// CPed::SetWeaponLockOnTarget
	// UnFuck(g_libGTASA + 0x00438DB4);
	// WriteMemory(g_libGTASA + 0x00438DB4, (uintptr_t)"\x00\x20\xF7\x46", 4);
	
	// Display FPS
	*(bool *)(g_libGTASA + 0x008ED875) = pSettings->Get().bDrawFPS ? true : false;
	
	// Disable vehicle name rendering
	NOP(g_libGTASA + 0x003D6FDC, 2);
}
