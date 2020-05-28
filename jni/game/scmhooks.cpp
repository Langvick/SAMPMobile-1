#include "../main.h"
#include "../util/armhook.h"

void OnSCMEvent(int iCommandId);

/*int(*CRunningScript__ProcessCommands0To99)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands0To99_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands0To99(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands100To199)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands100To199_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands100To199(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands200To299)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands200To299_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands200To299(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands300To399)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands300To399_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands300To399(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands400To499)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands400To499_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands400To499(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands500To599)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands500To599_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands500To599(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands600To699)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands600To699_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands600To699(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands700To799)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands700To799_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands700To799(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands800To899)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands800To899_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands800To899(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands900To999)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands900To999_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands900To999(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1000To1099)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1000To1099_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1000To1099(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1100To1199)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1100To1199_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1100To1199(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1200To1299)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1200To1299_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1200To1299(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1300To1399)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1300To1399_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1300To1399(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1400To1499)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1400To1499_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1400To1499(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1500To1599)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1500To1599_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1500To1599(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1600To1699)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1600To1699_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1600To1699(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1700To1799)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1700To1799_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1700To1799(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1800To1899)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1800To1899_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1800To1899(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands1900To1999)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands1900To1999_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands1900To1999(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands2000To2099)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands2000To2099_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands2000To2099(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands2100To2199)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands2100To2199_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands2100To2199(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands2200To2299)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands2200To2299_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands2200To2299(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands2300To2399)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands2300To2399_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands2300To2399(_this, iCommandId);
}

int(*CRunningScript__ProcessCommands2400To2499)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands2400To2499_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands2400To2499(_this, iCommandId);
}*/

int(*CRunningScript__ProcessCommands2500To2599)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands2500To2599_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands2500To2599(_this, iCommandId);
}

/*int(*CRunningScript__ProcessCommands2600To2699)(uint32_t *_this, int iCommandId);
int CRunningScript__ProcessCommands2600To2699_hook(uint32_t *_this, int iCommandId) {
	OnSCMEvent(iCommandId);
	return CRunningScript__ProcessCommands2600To2699(_this, iCommandId);
}*/

void InstallSCMHooks() {
	/*SetUpHook(g_libGTASA + 0x002E4A90, (uintptr_t)CRunningScript__ProcessCommands0To99_hook, (uintptr_t *) &CRunningScript__ProcessCommands0To99);
	SetUpHook(g_libGTASA + 0x002E23F4, (uintptr_t)CRunningScript__ProcessCommands100To199_hook, (uintptr_t *) &CRunningScript__ProcessCommands100To199);
	SetUpHook(g_libGTASA + 0x002E55C0, (uintptr_t)CRunningScript__ProcessCommands200To299_hook, (uintptr_t *) &CRunningScript__ProcessCommands200To299);
	SetUpHook(g_libGTASA + 0x002F7910, (uintptr_t)CRunningScript__ProcessCommands300To399_hook, (uintptr_t *) &CRunningScript__ProcessCommands300To399);
	SetUpHook(g_libGTASA + 0x002F87F0, (uintptr_t)CRunningScript__ProcessCommands400To499_hook, (uintptr_t *) &CRunningScript__ProcessCommands400To499);
	SetUpHook(g_libGTASA + 0x002FB184, (uintptr_t)CRunningScript__ProcessCommands500To599_hook, (uintptr_t *) &CRunningScript__ProcessCommands500To599);
	SetUpHook(g_libGTASA + 0x002F9664, (uintptr_t)CRunningScript__ProcessCommands600To699_hook, (uintptr_t *) &CRunningScript__ProcessCommands600To699);
	SetUpHook(g_libGTASA + 0x002F9D24, (uintptr_t)CRunningScript__ProcessCommands700To799_hook, (uintptr_t *) &CRunningScript__ProcessCommands700To799);
	SetUpHook(g_libGTASA + 0x002FE444, (uintptr_t)CRunningScript__ProcessCommands800To899_hook, (uintptr_t *) &CRunningScript__ProcessCommands800To899);
	SetUpHook(g_libGTASA + 0x002FC6D8, (uintptr_t)CRunningScript__ProcessCommands900To999_hook, (uintptr_t *) &CRunningScript__ProcessCommands900To999);
	SetUpHook(g_libGTASA + 0x00305028, (uintptr_t)CRunningScript__ProcessCommands1000To1099_hook, (uintptr_t *) &CRunningScript__ProcessCommands1000To1099);
	SetUpHook(g_libGTASA + 0x00303CCC, (uintptr_t)CRunningScript__ProcessCommands1100To1199_hook, (uintptr_t *) &CRunningScript__ProcessCommands1100To1199);
	SetUpHook(g_libGTASA + 0x00305EC4, (uintptr_t)CRunningScript__ProcessCommands1200To1299_hook, (uintptr_t *) &CRunningScript__ProcessCommands1200To1299);
	SetUpHook(g_libGTASA + 0x00307428, (uintptr_t)CRunningScript__ProcessCommands1300To1399_hook, (uintptr_t *) &CRunningScript__ProcessCommands1300To1399);
	SetUpHook(g_libGTASA + 0x00308640, (uintptr_t)CRunningScript__ProcessCommands1400To1499_hook, (uintptr_t *) &CRunningScript__ProcessCommands1400To1499);
	SetUpHook(g_libGTASA + 0x0030A5EC, (uintptr_t)CRunningScript__ProcessCommands1500To1599_hook, (uintptr_t *) &CRunningScript__ProcessCommands1500To1599);
	SetUpHook(g_libGTASA + 0x0030DAE0, (uintptr_t)CRunningScript__ProcessCommands1600To1699_hook, (uintptr_t *) &CRunningScript__ProcessCommands1600To1699);
	SetUpHook(g_libGTASA + 0x0030F834, (uintptr_t)CRunningScript__ProcessCommands1700To1799_hook, (uintptr_t *) &CRunningScript__ProcessCommands1700To1799);
	SetUpHook(g_libGTASA + 0x002E6FAC, (uintptr_t)CRunningScript__ProcessCommands1800To1899_hook, (uintptr_t *) &CRunningScript__ProcessCommands1800To1899);
	SetUpHook(g_libGTASA + 0x002E8E84, (uintptr_t)CRunningScript__ProcessCommands1900To1999_hook, (uintptr_t *) &CRunningScript__ProcessCommands1900To1999);
	SetUpHook(g_libGTASA + 0x002EC040, (uintptr_t)CRunningScript__ProcessCommands2000To2099_hook, (uintptr_t *) &CRunningScript__ProcessCommands2000To2099);
	SetUpHook(g_libGTASA + 0x002EDE10, (uintptr_t)CRunningScript__ProcessCommands2100To2199_hook, (uintptr_t *) &CRunningScript__ProcessCommands2100To2199);
	SetUpHook(g_libGTASA + 0x002F2E4C, (uintptr_t)CRunningScript__ProcessCommands2200To2299_hook, (uintptr_t *) &CRunningScript__ProcessCommands2200To2299);
	SetUpHook(g_libGTASA + 0x002EF788, (uintptr_t)CRunningScript__ProcessCommands2300To2399_hook, (uintptr_t *) &CRunningScript__ProcessCommands2300To2399);
	SetUpHook(g_libGTASA + 0x002F11FC, (uintptr_t)CRunningScript__ProcessCommands2400To2499_hook, (uintptr_t *) &CRunningScript__ProcessCommands2400To2499);*/
	SetUpHook(g_libGTASA + 0x002F5EB4, (uintptr_t)CRunningScript__ProcessCommands2500To2599_hook, (uintptr_t *) &CRunningScript__ProcessCommands2500To2599);
	//SetUpHook(g_libGTASA + 0x002F4658, (uintptr_t)CRunningScript__ProcessCommands2600To2699_hook, (uintptr_t *) &CRunningScript__ProcessCommands2600To2699);
}