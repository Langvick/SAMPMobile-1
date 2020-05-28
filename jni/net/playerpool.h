#pragma once

#define NO_TEAM 255

class CPlayerPool
{
public:
	CPlayerPool();
	~CPlayerPool();

	bool Process();

	// LOCAL
	void SetLocalPlayerName(const char* szName) { strcpy(m_szLocalPlayerName, szName); }
	char* GetLocalPlayerName() { return m_szLocalPlayerName; }
	void SetLocalPlayerID(uint16_t MyPlayerID) {
		strcpy(m_szPlayerNames[MyPlayerID], m_szLocalPlayerName);
		m_LocalPlayerID = MyPlayerID;
	}
	uint16_t GetLocalPlayerID() { return m_LocalPlayerID; }
	CLocalPlayer* GetLocalPlayer() { return m_pLocalPlayer; }

	// remote
	bool New(uint16_t playerId, char* szPlayerName, bool bIsNPC);
	bool Delete(uint16_t playerId, uint8_t byteReason);
	
	bool IsValidPlayerId(uint16_t playerId) {
		if(playerId >= 0 && playerId < MAX_PLAYERS) {
			return true;
		}
		return false;
	};
	
	bool GetSlotState(uint16_t playerId) {
		if(IsValidPlayerId(playerId)) {
			return m_bPlayerSlotState[playerId];
		}
		return false;
	};
	
	CRemotePlayer *GetAt(uint16_t playerId) {
		if(!GetSlotState(playerId)) {
			return nullptr;
		}
		return m_pPlayers[playerId];
	};
	
	void UpdateScore(uint16_t playerId, int iScore)
	{ 
		if (playerId == m_LocalPlayerID)
		{
			m_iLocalPlayerScore = iScore;
		} else {
			if (playerId >= MAX_PLAYERS) { return; }
			m_iPlayerScores[playerId] = iScore;
		}
	};

	void UpdatePing(uint16_t playerId, uint32_t dwPing) { 
		if (playerId == m_LocalPlayerID)
		{
			m_dwLocalPlayerPing = dwPing;
		} else {
			if (playerId >= MAX_PLAYERS) { return; }
			m_dwPlayerPings[playerId] = dwPing;
		}
	};

	void UpdateIPAddress(uint16_t playerId, unsigned long ulIPAddress) {
		if (playerId >= MAX_PLAYERS) { return; }
		m_ulIPAddresses[playerId] = ulIPAddress;
	}

	int GetLocalPlayerScore() {
		return m_iLocalPlayerScore;
	};

	uint32_t GetLocalPlayerPing() {
		return m_dwLocalPlayerPing;
	};

	int GetPlayerScore(uint16_t playerId) {
		if (playerId >= MAX_PLAYERS) { return 0; }
		return m_iPlayerScores[playerId];
	};

	uint32_t GetPlayerPing(uint16_t playerId)
	{
		if (playerId >= MAX_PLAYERS) { return 0; }
		return m_dwPlayerPings[playerId];
	};

	long GetPlayerIP(uint16_t playerId) {
		if (playerId >= MAX_PLAYERS) { return 0; }
		return m_ulIPAddresses[playerId];
	};

	void SetPlayerName(uint16_t playerId, char* szName) { strcpy(m_szPlayerNames[playerId], szName); }
	char* GetPlayerName(uint16_t playerId){ return m_szPlayerNames[playerId]; }

	uint16_t FindRemotePlayerIDFromGtaPtr(PED_TYPE *pActor);
	CPlayerPed *FindRemotePlayerPedFromGtaPtr(PED_TYPE *pActor);

	void ApplyCollisionChecking();
	void ResetCollisionChecking();

private:
	// LOCAL
	uint16_t		m_LocalPlayerID;
	CLocalPlayer	*m_pLocalPlayer;
	char			m_szLocalPlayerName[MAX_PLAYER_NAME+1];

	// REMOTE
	CRemotePlayer	*m_pPlayers[MAX_PLAYERS];
	bool			m_bPlayerSlotState[MAX_PLAYERS];
	char			m_szPlayerNames[MAX_PLAYERS][MAX_PLAYER_NAME+1];
	bool 			m_bCollisionChecking[MAX_PLAYERS];
	int			    m_iLocalPlayerScore;
	int		    	m_iPlayerScores[MAX_PLAYERS];
	uint32_t		m_dwLocalPlayerPing;
	uint32_t		m_dwPlayerPings[MAX_PLAYERS];
	unsigned long	m_ulIPAddresses[MAX_PLAYERS];
};