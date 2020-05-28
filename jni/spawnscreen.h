#pragma once

class CSpawnScreen
{
public:
	CSpawnScreen();
	~CSpawnScreen() {};

	void Render();
	void Show(bool bShow) { m_bEnabled = bShow; };
	bool GetState() { return m_bEnabled; };

private:
	bool	m_bEnabled;
	float	m_fPosX;
	float	m_fPosY;
	float 	m_fSizeX;
	float 	m_fSizeY;
	float 	m_fButWidth;
	float 	m_fButHeight;
};