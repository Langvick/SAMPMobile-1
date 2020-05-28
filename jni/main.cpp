/*SC BY FERRY DO NOT DELETE THIS  TEXT!!*/ 


#include <jni.h>
#include <android/log.h>
#include <ucontext.h>
#include <pthread.h>

#include "main.h"
#include "game/game.h"
#include "game/RW/RenderWare.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "chatwindow.h"
#include "spawnscreen.h"
#include "playertags.h"
#include "dialog.h"
#include "extrakeyboard.h"
#include "keyboard.h"
#include "settings.h"
#include "playerslist.h"

#include "util/armhook.h"
#include "checkfilehash.h"
#include "timer.hpp"

const short iNetGameCookie = 0x6969;
const short iNetGameVersion = 0x0FD9;

uintptr_t g_libGTASA = 0;
uintptr_t g_libSAMP = 0;
const char *g_pszStorage = nullptr;

bool bNetworkInited = false;
bool bGameInited = false;

bool bQuitGame = false;
uint32_t dwStartQuitTick = 0;

const auto encryptedAddress = cryptor::create("40.70.19.6", 16);
const auto encryptedPassword = cryptor::create("E2gle123123", 16);
unsigned short usPort = 7777;

CGame *pGame = nullptr;
CNetGame *pNetGame = nullptr;
CChatWindow *pChatWindow = nullptr;
CSpawnScreen *pSpawnScreen = nullptr;
CPlayerTags *pPlayerTags = nullptr;
CDialogWindow *pDialogWindow = nullptr;
CPlayersList *pPlayersList = nullptr;
CGUI *pGUI = nullptr;
CKeyBoard *pKeyBoard = nullptr;
CExtraKeyBoard *pExtraKeyBoard = nullptr;
CSettings *pSettings = nullptr;

void InitHookStuff();
void InstallSpecialHooks();
void InitRenderWareFunctions();
void ApplyInGamePatches();
void ApplyPatches_level0();
void InitModelPreviewStuff();
void QuitGame();

void InitSAMP() {
	g_pszStorage = (const char *)(g_libGTASA + 0x0063C4B8);
	if(!g_pszStorage) {
		std::terminate();
		return;
	}
	
	Log("Initialising \"libSAMP.so\"...");
	Log("Storage: %s", g_pszStorage);

	pSettings = new CSettings();

	if(!FileCheckSum()) {
		std::terminate();
		return;
	}
}

void InitInMenu() {	
	pGame = new CGame();
	pGame->InitInMenu();

	pGUI = new CGUI();
	pDialogWindow = new CDialogWindow();
	pKeyBoard = new CKeyBoard();
	pExtraKeyBoard = new CExtraKeyBoard();
	pChatWindow = new CChatWindow();
	pPlayersList = new CPlayersList();
	pSpawnScreen = new CSpawnScreen();
	pPlayerTags = new CPlayerTags();
}

void InitInGame() {
	if(!bGameInited) {
		pGame->InitInGame();
		pGame->SetMaxStats();
		InitModelPreviewStuff();

		bGameInited = true;
	}
}

void ProcessSAMPGraphic() {
	if(pNetGame) {
		CTextDrawPool *pTextDrawPool = pNetGame->GetTextDrawPool();
		if(pTextDrawPool) {
			pTextDrawPool->Draw();
		}
	}
}

void ProcessSAMPGraphicFrame() {
	if(pGUI) {
		pGUI->Render();
	}
}

void InitNetwork() {
	if(!bNetworkInited && pSettings->Get().iFontOutline)
	{
		pNetGame = new CNetGame(encryptedAddress.decrypt(), usPort, pSettings->Get().szNickName, "");

		bNetworkInited = true;
	}
}

void ProcessSAMPNetwork() {
	if(pNetGame) {
		pNetGame->Process();
	}
	
	if(bQuitGame){
		if((GetTickCount() - dwStartQuitTick) > 1000) {
			if(pNetGame)
			{
				delete pNetGame;
				pNetGame = NULL;
			}
			exit(0);
		}
		return;
	}
}

void handler(int signum, siginfo_t *info, void *contextPtr) {
	ucontext *context = (ucontext_t *)contextPtr;
	if(info->si_signo == SIGSEGV) {
		Log("SIGSEGV | Fault address: 0x%X", info->si_addr);
		Log("libGTASA base address: 0x%X", g_libGTASA);
		
		Log("register states:");
		Log("r0: 0x%X, r1: 0x%X, r2: 0x%X, r3: 0x%X", context->uc_mcontext.arm_r0, context->uc_mcontext.arm_r1, context->uc_mcontext.arm_r2, context->uc_mcontext.arm_r3);
		Log("r4: 0x%x, r5: 0x%x, r6: 0x%x, r7: 0x%x", context->uc_mcontext.arm_r4, context->uc_mcontext.arm_r5, context->uc_mcontext.arm_r6, context->uc_mcontext.arm_r7);
		Log("r8: 0x%x, r9: 0x%x, sl: 0x%x, fp: 0x%x", context->uc_mcontext.arm_r8, context->uc_mcontext.arm_r9, context->uc_mcontext.arm_r10, context->uc_mcontext.arm_fp);
		Log("ip: 0x%x, sp: 0x%x, lr: 0x%x, pc: 0x%x", context->uc_mcontext.arm_ip, context->uc_mcontext.arm_sp, context->uc_mcontext.arm_lr, context->uc_mcontext.arm_pc);

		Log("backtrace (libGTASA | libSAMP):");
		Log("1: 0x%X (0x%X | 0x%X)", context->uc_mcontext.arm_pc, context->uc_mcontext.arm_pc - g_libGTASA, context->uc_mcontext.arm_pc - g_libSAMP);
		Log("2: 0x%X (0x%X | 0x%X)", context->uc_mcontext.arm_lr, context->uc_mcontext.arm_lr - g_libGTASA, context->uc_mcontext.arm_lr - g_libSAMP);

		exit(0);
	}
	return;
}

void QuitGame() {
	if(pNetGame && pNetGame->GetGameState() == GAMESTATE_CONNECTED) {
		pNetGame->GetRakClient()->Disconnect(500);
	}	
	bQuitGame = true;
	dwStartQuitTick = GetTickCount();
}

void *Init(void *p) {
	ApplyPatches_level0();

	pthread_exit(0);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {	
	Log("SAMP library loaded! Build time: " __DATE__ " " __TIME__);

	g_libGTASA = FindLibrary("libGTASA.so");
	if(g_libGTASA == 0) {
		Log("ERROR: libGTASA.so address not found!");
		return 0;
	}

	Log("libGTASA.so image base address: 0x%X", g_libGTASA);

	g_libSAMP = FindLibrary("libsamp.so");
	Log("libsamp.so image base address: 0x%X", g_libSAMP);

	srand(time(0));

	InitHookStuff();
	InitRenderWareFunctions();
	InstallSpecialHooks();

	pthread_t thread;
	pthread_create(&thread, 0, Init, 0);

	struct sigaction act;
	act.sa_sigaction = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, 0);

	return JNI_VERSION_1_6;
}

void Log(const char *fmt, ...) {	
	char buffer[0xFF];
	static FILE* flLog = nullptr;

	if(flLog == nullptr && g_pszStorage != nullptr)
	{
		sprintf(buffer, "%sSAMP/execution.log", g_pszStorage);
		flLog = fopen(buffer, "w");
	}

	memset(buffer, 0, sizeof(buffer));

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	va_end(arg);

	__android_log_write(ANDROID_LOG_INFO, "ExecutionLog", buffer);

	if(flLog == nullptr) return;
	fprintf(flLog, "%s\n", buffer);
	fflush(flLog);

	return;
}

uint32_t GetTickCount() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
