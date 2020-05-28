/*
SGBTEAM lib Thanks to Delvin Hatanaskov santrope-rp to help me about blabla indinahoy stupid :v 


Файзал Р.Ивальди, Дельвин Хатанасков

*/

#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include <list>
#include <unistd.h>
#include <stdint.h>

#define DEBUG 1

#define SAMP_VERSION "0.3.7"
#define AUTH_BS "FF2BE5E6F5D9392F57C4E66F7AD78767277C6E4F6B"
//#define AUTH_BS "15121F6F18550C00AC4B4F8A167D0379BB0ACA99043"

#define MAX_PLAYERS 1004
#define MAX_PLAYER_NAME	24

#define MAX_VEHICLES 2000

#define LOCKING_DISTANCE		200.0f
#define CSCANNER_DISTANCE		200.0f
#define PSCANNER_DISTANCE		600.0f

#include "vendor/RakNet/SAMP.h"

#include "util/util.h"

#include "str_obfuscator_no_template.hpp"

extern uintptr_t g_libGTASA;
extern const char* g_pszStorage;
extern const short iNetGameCookie;
extern const short iNetGameVersion;

// Quit Game / Exit Game
void QuitGame();

void Log(const char *fmt, ...);
uint32_t GetTickCount();