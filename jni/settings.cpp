#include "main.h"
#include "settings.h"
#include "vendor/inih/cpp/INIReader.h"

#include "vendor/SimpleIni/SimpleIni.h"


CSettings::CSettings()
{
	Log("Loading settings..");

	char buff[0x7F];
	sprintf(buff, "%sSAMP/settings.ini", g_pszStorage);

	INIReader reader(buff);

	if(reader.ParseError() < 0) {
		Log("Error: can't load %s", buff);
		std::terminate();
		return;
	}

	unsigned long length = 0;

	// Client
	sprintf(buff, "_SGB%i%i", rand() % 1000, rand() % 1000);
	length = reader.Get("client", "name", buff).copy(m_Settings.szNickName, MAX_PLAYER_NAME);
	m_Settings.szNickName[length] = '\0';
	length = reader.Get("client", "password", "").copy(m_Settings.szPassword, MAX_SETTINGS_STRING);
	m_Settings.szPassword[length] = '\0';
	
	m_Settings.iFrameLimit = reader.GetInteger("client", "fpslimit", 20);
	
	// debug
	m_Settings.bDrawFPS = reader.GetBoolean("debug", "display_fps", false);

	// gui
	length = reader.Get("gui", "Font", "Arial.ttf").copy(m_Settings.szFont, 35);
	m_Settings.szFont[length] = '\0';
	
	m_Settings.fFontSize = reader.GetReal("gui", "FontSize", 30.0f);
	m_Settings.iFontOutline = reader.GetInteger("gui", "FontOutline", 2);

	// chat
	m_Settings.fChatPosX = reader.GetReal("gui", "ChatPosX", 325.0f);
	m_Settings.fChatPosY = reader.GetReal("gui", "ChatPosY", 20.0f);
	m_Settings.fChatSizeX = reader.GetReal("gui", "ChatSizeX", 1150.0f);
	m_Settings.fChatSizeY = reader.GetReal("gui", "ChatSizeY", 220.0f);
	m_Settings.iChatMaxMessages = reader.GetInteger("gui", "ChatMaxMessages", 6);

	// spawnscreen
	m_Settings.fSpawnScreenPosX = reader.GetReal("gui", "SpawnScreenPosX", 660.0f);
	m_Settings.fSpawnScreenPosY = reader.GetReal("gui", "SpawnScreenPosY", 950.0f);
	m_Settings.fSpawnScreenSizeX = reader.GetReal("gui", "SpawnScreenSizeX", 600.0f);
	m_Settings.fSpawnScreenSizeY = reader.GetReal("gui", "SpawnScreenSizeY", 100.0f);

	// nametags
	m_Settings.fHealthBarWidth = reader.GetReal("gui", "HealthBarWidth", 60.0f);
	m_Settings.fHealthBarHeight = reader.GetReal("gui", "HealthBarHeight", 10.0f);
}

bool CSettings::SaveNameAndServerPassword(const char *name)
{
	char buff[0x7F];
	sprintf(buff, "%sSAMP/settings.ini", g_pszStorage);

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(buff);
	SI_Error rc;
	if (name) {
		rc = ini.SetValue("client", "name", name);
	}
	rc = ini.SaveFile(buff);
	return (rc >= 0);
}
