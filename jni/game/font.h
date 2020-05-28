#pragma once

#include <stdint.h>
#include "CRGBA.h"
#include "common.h"

class CFont
{
public:
	static void AsciiToGxtChar(const char *szASCII, uint16_t *wszGXT);
	static void PrintString(float fX, float fY, char *szText);
	static void SetColor(CRGBA color);
	static void SetDropColor(CRGBA color);
	static void SetEdge(bool on);
	static void SetJustify(bool on);
	static void SetScale(float fValue) ;
	static void SetScaleXY(float fValueX, float fValueY);
	static void SetOrientation(unsigned char ucValue);
	static void SetFontStyle(unsigned char ucValue);
	static void SetProportional(unsigned char ucValue);
	static void SetRightJustifyWrap(float fValue);
	static void SetBackground(bool enable, bool includeWrap);
	static void SetBackgroundColor(CRGBA uiColor);
	static void SetWrapx(float fValue);
	static void SetCentreSize(float fValue);
	static void SetDropShadowPosition(signed char scValue);
	static float GetHeight(bool unk);
	static float GetStringWidth(uint16_t* str);
	static void GetTextRect(PRECT rect, float x, float y);
};