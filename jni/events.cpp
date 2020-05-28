#include "main.h"
#include "game/game.h"
#include "net/netgame.h"
#include "chatwindow.h"

#define EVENT_TYPE_PAINTJOB 1
#define EVENT_TYPE_CARCOMPONENT 2
#define EVENT_TYPE_CARCOLOR 3
#define EVENT_ENTEREXIT_MODSHOP 4

extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
extern CGame *pGame;

void ProcessIncommingEvent(uint16_t playerId, int iEventType, uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3)
{
	if(!pNetGame) return;
	
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	switch(iEventType) {
		case EVENT_TYPE_PAINTJOB: {
			int iVehicleId = pVehiclePool->FindGtaIDFromID(dwParam1);
			if(iVehicleId != INVALID_VEHICLE_ID)
			{
				ScriptCommand(&change_car_skin, iVehicleId, dwParam2);
			}
			break;
		}

		case EVENT_TYPE_CARCOMPONENT: {
			if(!IS_VEHICLE_MOD(dwParam2)) {
				return;
			}

			if(!pGame->HasModelLoaded(dwParam2)) {
				pGame->RequestVehicleUpgrade(dwParam2, 10);
				pGame->LoadRequestedModels(false);
			}

			VEHICLE_TYPE *pVehicle = pVehiclePool->FindGtaVehicleFromId(dwParam1);
			if(pVehicle) {
				((void (*)(VEHICLE_TYPE *, int32_t))(g_libGTASA + 0x00516090 + 1))(pVehicle, dwParam2);
			}
			
			
			break;
		}

		case EVENT_TYPE_CARCOLOR: {
			pVehiclePool->GetAt(dwParam1)->SetColor(dwParam2, dwParam3);
			break;
		}
		
		case EVENT_ENTEREXIT_MODSHOP: {
			if(playerId < 0 || playerId > MAX_PLAYERS) {
				return;
			}
			
			pNetGame->GetPlayerPool()->GetAt(playerId)->m_iIsInAModShop = (int)dwParam2;
			break;
		}
	}
}

void SendScmEvent(int iEventType, uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3)
{
	RakNet::BitStream bsSend;
	bsSend.Write(iEventType);
	bsSend.Write(dwParam1);
	bsSend.Write(dwParam2);
	bsSend.Write(dwParam3);
	pNetGame->GetRakClient()->RPC(RPC_ScmEvent, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false);
}

void ProcessOutgoingEvent(int iEventType, uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3)
{
	if(!pNetGame) {
		return;
	}
	
	int iVehicleId = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(GamePool_Vehicle_GetAt(dwParam1));
	if(iVehicleId != INVALID_VEHICLE_ID) {
		switch(iEventType) {
			case EVENT_TYPE_PAINTJOB: {
				SendScmEvent(EVENT_TYPE_PAINTJOB, iVehicleId, dwParam2, 0);		
				break;
			}

			case EVENT_TYPE_CARCOMPONENT: {
				SendScmEvent(EVENT_TYPE_CARCOMPONENT, iVehicleId, dwParam2, 0);
				break;
			}

			case EVENT_TYPE_CARCOLOR: {
				SendScmEvent(EVENT_TYPE_CARCOLOR, iVehicleId, dwParam2, dwParam3);
				break;
			}

			case EVENT_ENTEREXIT_MODSHOP: {
				SendScmEvent(EVENT_ENTEREXIT_MODSHOP, iVehicleId, dwParam2, 0);
				break;
			}
		}
	}
}

void OnSCMEvent(int iCommandId) {
	switch(iCommandId) {
		case 0x09C6: {
			// CRunningScript::CollectParameters(short) — 0x002E1254
			((void (*)(short))(g_libGTASA + 0x002E1254 + 1))(4);

			// ScriptParams — 0x007159E8
			tScriptParam *pScriptParams = (tScriptParam *)(g_libGTASA + 0x007159E8);
			if (pScriptParams) {
				ProcessOutgoingEvent(pScriptParams[0].iParam, pScriptParams[1].uParam, pScriptParams[2].uParam, pScriptParams[3].uParam);
			}
			break;
		}
		case 0x09D3: {
			int IsDriving;
			uint32_t ScmInst;
			
			__asm__ volatile ("mov %0, lr" : "=r" (ScmInst));
			
			IsDriving = 1;
			
			if(pGame)
			{
				CPlayerPed *pDriverPed = pGame->FindPlayerPed();
				if(pDriverPed->IsInVehicle() && !pDriverPed->IsAPassenger()) {
					IsDriving = 0;
				}
			}
	
			// CRunningScript::UpdateCompareFlag() - 0x00300210 
			((void (*)(uint32_t, int))(g_libGTASA + 0x00300210 + 1))(ScmInst, IsDriving);
			break;
		}
	}
}