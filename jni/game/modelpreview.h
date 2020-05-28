#pragma once

struct stModelPreviewStuff
{
	RwCamera 	*m_pRwCamera;
	RpLight 	*m_pRpLight;
	RwFrame 	*m_pRwFrame;
	RwRaster	*m_pRwRaster;
	
	
	stModelPreviewStuff()
	{
		m_pRwCamera = nullptr;
		m_pRpLight = nullptr;
		m_pRwFrame = nullptr;
		m_pRwRaster = nullptr;
	}
};

void InitModelPreviewStuff();
RwTexture * CreateModelPreviewForVehicle(uint16_t vehicleId, CRGBA color, VECTOR vecRot, float fZoom, int iColor1 = -1, int iColor2 = -1);