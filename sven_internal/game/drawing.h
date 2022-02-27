// Drawing

#pragma once

#include "../sdk.h"

#include "utils/vgui2/include/IPanel.h"
#include "utils/vgui2/include/ISurface.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum FontAlignFlags_t
{
	FONT_ALIGN_LEFT = 0,
	FONT_ALIGN_RIGHT = (1 << 0),
	FONT_ALIGN_CENTER = (1 << 1),
	FONT_ALIGN_LEFT_BOT = (1 << 2),
	FONT_ALIGN_LEFT_TOP = (1 << 3)
};

//-----------------------------------------------------------------------------
// Class for drawing
//-----------------------------------------------------------------------------

class CDrawing
{
public:
	void SetupFonts();

	void FillArea(int x, int y, int w, int h, int r, int g, int b, int a);

	void DrawRect(int x, int y, int w, int h, int r, int g, int b, int a);
	void DrawOutlinedRect(int x, int y, int w, int h, int r, int g, int b, int a);

	void DrawCircle(float position[2], float points, float radius, int r, int g, int b, int a);
	void DrawCircle3D(Vector &position, float points, float radius, int r, int g, int b, int a);
	void DrawLine(int x0, int y0, int x1, int y1, int r, int g, int b, int a);

	void BoxCorner(int x, int y, int w, int h, int lw, BYTE r, BYTE g, BYTE b, BYTE a);
	void BoxCornerOutline(int x, int y, int w, int h, int lw, BYTE r, BYTE g, BYTE b, BYTE a);

	void DrawCoalBox(int x, int y, int w, int h, int lw, int r, int g, int b, int a);
	void DrawOutlineCoalBox(int x, int y, int w, int h, int  lw, BYTE r, BYTE g, BYTE b, BYTE a);
	void DrawPlayerBox(int x, int y, int w, int h, int r, int g, int b, int a);
	void DrawBox(int x, int y, int w, int h, int r, int g, int b, int a);
	void BoxOutline(float x, float y, float w, float h, float lw, BYTE r, BYTE g, BYTE b, BYTE a);
	void Box(int x, int y, int w, int h, int lw, int r, int g, int b, int a);

	void DrawTexture(int id, int x0, int y0, int x1, int y1, int r = 255, int g = 255, int b = 255, int a = 255);

	void DrawStringF(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString, ...);
	void DrawStringACPF(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString, ...);
	void DrawWideStringF(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const wchar_t *pwzString, ...);

	void DrawString(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString);
	void DrawStringACP(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString);
	void DrawWideString(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const wchar_t *pwszString);

	void DrawCrosshair(int x, int y, int r, int g, int b, int a, int iSize = 10, int iGap = 4, int iThickness = 2);
	void DrawDot(int x, int y, int r, int g, int b, int a, int iThickness = 2);
	void DrawCrosshairShadow(int x, int y, int r, int g, int b, int a, int iSize = 10, int Giap = 4, int iThickness = 2, int iShadowThickness = 1);
	void DrawDotShadow(int x, int y, int r, int g, int b, int a, int iThickness = 2, int iShadowThickness = 1);

private:
	void ApplyTextAlignment(FontAlignFlags_t alignment, int &x, int &y, int textWidth, int textHeight);
};

inline void CDrawing::ApplyTextAlignment(FontAlignFlags_t alignment, int &x, int &y, int textWidth, int textHeight)
{
	if (alignment & FONT_ALIGN_RIGHT)
		x -= textWidth;

	if (alignment & FONT_ALIGN_CENTER)
		x -= textWidth / 2;

	if (alignment & FONT_ALIGN_LEFT_BOT)
		y += textHeight / 2;

	if (alignment & FONT_ALIGN_LEFT_TOP)
		y -= textHeight / 2;
}

//-----------------------------------------------------------------------------
// Exports
//-----------------------------------------------------------------------------

extern CDrawing g_Drawing;

extern vgui::HFont g_hESP;
extern vgui::HFont g_hESP2;
extern vgui::HFont g_hFontVotePopup;
//extern vgui::HFont MENU;