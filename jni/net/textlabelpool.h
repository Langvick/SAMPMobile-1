#pragma once

#define MAX_TEXT_LABELS			1024
#define MAX_PLAYER_TEXT_LABELS	1024
#define INVALID_TEXT_LABEL		1025

struct textLabelData {
	char szText[4096];
	uint32_t uiColor;
	splittedText *splittedMainText;
	VECTOR vecPosition;
	float fDrawDistance;
	bool bDrawLOS;
	uint16_t usAttachedPlayerId;
	uint16_t usAttachedVehicleId;
};

class CLabelPool
{
private:
	textLabelData *m_pTextLabels[MAX_TEXT_LABELS + MAX_PLAYER_TEXT_LABELS + 2];
	bool m_bSlotState[MAX_TEXT_LABELS + MAX_PLAYER_TEXT_LABELS + 2];

public:
	CLabelPool();
	~CLabelPool();

	void CreateTextLabel(int labelId, char *szText, uint32_t uiColor, VECTOR vecPosition, float fDrawDistance, bool bDrawLOS, uint16_t attachedPlayerId, uint16_t attachedVehicleId);
	void Delete(int labelId);
	void Draw();
};