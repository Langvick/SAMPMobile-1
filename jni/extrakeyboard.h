#pragma once

class CExtraKeyBoard
{
public:
	CExtraKeyBoard();
	~CExtraKeyBoard();

	void Render();
	void Clear();
	void Show(bool bShow);
	
private:
	float m_fButWidth;
	float m_fButHeight;
	bool m_bIsItemShow;
	bool m_bIsActive;
	bool m_tabactive;
};
