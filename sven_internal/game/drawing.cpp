// Drawing

#pragma comment(lib, "OpenGL32")
#pragma warning(disable : 4244)

#include <Windows.h>
#include <gl/GL.h>

#include "drawing.h"

#include "utils.h"
#include "../interfaces.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

//#define swap(a, b) s_swap_buffer = a; a = b; b = s_swap_buffer

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern vgui::ISurface *g_pSurface;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CDrawing g_Drawing;

vgui::HFont g_hESP;
vgui::HFont g_hESP2;
vgui::HFont g_hFontVotePopup;
//vgui::HFont MENU;

static char s_szBuffer[1024];
static wchar_t s_wszBuffer[1024];

static int s_swap_buffer = 0;

//-----------------------------------------------------------------------------
// CDrawing implementations
//-----------------------------------------------------------------------------

void CDrawing::SetupFonts()
{
	g_pSurface->AddGlyphSetToFont(g_hESP = g_pSurface->CreateFont(), "Choktoff", 12, FW_BOLD, NULL, NULL, vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0);
	g_pSurface->AddGlyphSetToFont(g_hESP2 = g_pSurface->CreateFont(), "Choktoff", 38, FW_MEDIUM, NULL, NULL, vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0);
	g_pSurface->AddGlyphSetToFont(g_hFontVotePopup = g_pSurface->CreateFont(), "Lucida-Console", 20, FW_EXTRABOLD, NULL, NULL, vgui::ISurface::FONTFLAG_NONE, 0, 0);
	//g_pSurface->AddGlyphSetToFont(MENU = g_pSurface->CreateFont(), "Arial", 14, FW_BOLD, NULL, NULL, FONTFLAG_NONE, 0, 0); //Main font
}

//-----------------------------------------------------------------------------
// Figures
//-----------------------------------------------------------------------------

void CDrawing::DrawCircle3D(Vector &position, float points, float radius, int r, int g, int b, int a)
{
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
	{
		Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
		Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

		Vector start2d, end2d;
		if (!WorldToScreen(start, start2d) || !WorldToScreen(end, end2d))
			return;

		DrawLine(start2d.x, start2d.y, end2d.x, end2d.y, r, g, b, a);
	}
}

void CDrawing::DrawCircle(float position[2], float points, float radius, int r, int g, int b, int a)
{
	float step = (float)M_PI * 2.0f / points;

	float start[2], end[2];

	for (float x = 0; x < (M_PI * 2.0f); x += step)
	{
		start[0] = radius * cosf(x) + position[0];
		start[1] = radius * sinf(x) + position[1];
		end[0] = radius * cosf(x + step) + position[0];
		end[1] = radius * sinf(x + step) + position[1];
		DrawLine(start[0], start[1], end[0], end[1], r, g, b, a);
	}
}

void CDrawing::DrawPlayerBox(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int x0 = x - w;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	//if (x1 < x0)
	//	swap(x1, x0);

	//if (y1 < y0)
	//	swap(y1, y0);

	g_pSurface->DrawSetColor(r, g, b, a);
	g_pSurface->DrawOutlinedRect(x0, y0, x1, y1);

	//if (cvar.esp_box_outline) 
	if (1) 
	{
		g_pSurface->DrawSetColor(0, 0, 0, a);
		g_pSurface->DrawOutlinedRect(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
		g_pSurface->DrawOutlinedRect(x0 + 1, y0 + 1, x1 - 1, y1 - 1);
	}
}

void CDrawing::DrawRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int x0 = x;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	//if (x1 < x0)
	//	swap(x1, x0);

	//if (y1 < y0)
	//	swap(y1, y0);

	g_pSurface->DrawSetColor(r, g, b, a);
	g_pSurface->DrawFilledRect(x0, y0, x1, y1);
}

void CDrawing::DrawOutlinedRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int x0 = x;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	//if (x1 < x0)
	//	swap(x1, x0);

	//if (y1 < y0)
	//	swap(y1, y0);

	g_pSurface->DrawSetColor(r, g, b, a);
	g_pSurface->DrawOutlinedRect(x0, y0, x1, y1);
}

void CDrawing::DrawLine(int x0, int y0, int x1, int y1, int r, int g, int b, int a)
{
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawLine(x0, y0, x1, y1);
}

void CDrawing::FillArea(int x, int y, int w, int h, int r, int g, int b, int a)
{
	glPushMatrix();

		glLoadIdentity();

		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4ub(r, g, b, a);

		glBegin(GL_QUADS);
			glVertex2i(x, y);
			glVertex2i(x + w, y);
			glVertex2i(x + w, y + h);
			glVertex2i(x, y + h);
		glEnd();

		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

	glPopMatrix();
}

void CDrawing::DrawBox(int x, int y, int w, int h, int r, int g, int b, int a)
{
	g_pSurface->DrawSetColor(r, g, b, a);
	g_pSurface->DrawOutlinedRect(x - w, y - h, x + w, y + h);
}

void CDrawing::Box(int x, int y, int w, int h, int lw, int r, int g, int b, int a)
{
	FillArea(x, y, w, lw, r, g, b, a);
	FillArea(x + w - lw, y + lw, lw, h - lw, r, g, b, a);
	FillArea(x, y + lw, lw, h - lw, r, g, b, a);
	FillArea(x + lw, y + h - lw, w - lw * 2, lw, r, g, b, a);
}

void CDrawing::BoxOutline(float x, float y, float w, float h, float lw, BYTE r, BYTE g, BYTE b, BYTE a)
{
	Box(x, y, w, h, lw, r, g, b, a);
	Box(x - 1, y - 1, w + 2, h + 2, 1, 0, 0, 0, a);
	Box(x + lw, y + lw, w - lw * 2, h - lw * 2, 1, 0, 0, 0, a);

}

void CDrawing::DrawCoalBox(int x, int y, int w, int h, int lw, int r, int g, int b, int a)
{
	int iw = w / 4;
	int ih = h / 1;

	DrawLine(x, y, x + iw, y, r, g, b, a);			// left
	DrawLine(x + w - iw, y, x + w, y, r, g, b, a);			// right
	DrawLine(x, y, x, y + ih, r, g, b, a);				// top left
	DrawLine(x + w, y, x + w, y + ih, r, g, b, a);	// top right
													// bottom
	DrawLine(x, y + h, x + iw, y + h, r, g, b, a);			// left
	DrawLine(x + w - iw, y + h, x + w, y + h, r, g, b, a);	// right
	DrawLine(x, y + h - ih, x, y + h, r, g, b, a);			// bottom left
	DrawLine(x + w, y + h - ih, x + w, y + h, r, g, b, a);	// bottom right
}

void CDrawing::DrawOutlineCoalBox(int x, int y, int w, int h, int  lw, BYTE r, BYTE g, BYTE b, BYTE a)
{
	int iw = w / 4;
	int ih = h / 4;

	DrawCoalBox(x - 1, y - 1, w, h, lw, 0, 0, 0, a);
	DrawCoalBox(x + 1, y + 1, w, h, lw, 0, 0, 0, a);
	DrawCoalBox(x - 1, y - 1, w, h, lw, 0, 0, 0, a);
	DrawCoalBox(x + 1, y + 1, w, h, lw, 0, 0, 0, a);

	DrawCoalBox(x - 1, y - 1, w, h, lw, 0, 0, 0, a);
	DrawCoalBox(x + 1, y + 1, w, h, lw, 0, 0, 0, a);
	DrawCoalBox(x - 1, y - 1, w, h, lw, 0, 0, 0, a);
	DrawCoalBox(x + 1, y + 1, w, h, lw, 0, 0, 0, a);


	DrawCoalBox(x, y, w, h, iw, r, g, b, a);
}

void CDrawing::BoxCorner(int x, int y, int w, int h, int lw, BYTE r, BYTE g, BYTE b, BYTE a)
{
	FillArea(x, y, w / 4, lw, r, g, b, a);
	FillArea(x + w - w / 4, y, w / 4, lw, r, g, b, a);
	FillArea(x, y + lw, lw, h / 4 - lw, r, g, b, a);
	FillArea(x, y + h - h / 4, lw, h / 4, r, g, b, a);
	FillArea(x + w - lw, y + lw, lw, h / 4 - lw, r, g, b, a);
	FillArea(x + w - lw, y + h - h / 4, lw, h / 4, r, g, b, a);
	FillArea(x + lw, y + h - lw, w / 4 - lw, lw, r, g, b, a);
	FillArea(x + w - w / 4, y + h - lw, w / 4 - lw, lw, r, g, b, a);
}

void CDrawing::BoxCornerOutline(int x, int y, int w, int h, int lw, BYTE r, BYTE g, BYTE b, BYTE a)
{
	BoxCorner(x - 1, y + 1, w, h, lw, 0, 0, 0, a);
	BoxCorner(x - 1, y - 1, w, h, lw, 0, 0, 0, a);
	BoxCorner(x + 1, y + 1, w, h, lw, 0, 0, 0, a);
	BoxCorner(x + 1, y - 1, w, h, lw, 0, 0, 0, a);

	BoxCorner(x, y, w, h, lw, r, g, b, a);
}

//-----------------------------------------------------------------------------
// Draw a texture
//-----------------------------------------------------------------------------

void CDrawing::DrawTexture(int id, int x0, int y0, int x1, int y1, int r, int g, int b, int a)
{
	g_pSurface->DrawSetColor(r, g, b, a);
	g_pSurface->DrawSetTexture(id);
	g_pSurface->DrawTexturedRect(x0, y0, x1, y1);
}

//-----------------------------------------------------------------------------
// Draw a formatted string
//-----------------------------------------------------------------------------

void CDrawing::DrawStringF(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char*pszString, ...)
{
	va_list va_alist;
	va_start(va_alist, pszString);
	_vsnprintf(s_szBuffer, sizeof(s_szBuffer), pszString, va_alist);
	va_end(va_alist);
	MultiByteToWideChar(CP_UTF8, 0, s_szBuffer, 256, s_wszBuffer, 256);

	int width, height;
	g_pSurface->GetTextSize(font, s_wszBuffer, width, height);

	ApplyTextAlignment(alignment, x, y, width, height);

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(s_wszBuffer, wcslen(s_wszBuffer));
}

void CDrawing::DrawStringACPF(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString, ...)
{
	va_list va_alist;
	va_start(va_alist, pszString);
	_vsnprintf(s_szBuffer, sizeof(s_szBuffer), pszString, va_alist);
	va_end(va_alist);
	MultiByteToWideChar(CP_ACP, 0, s_szBuffer, 256, s_wszBuffer, 256);

	int width, height;
	g_pSurface->GetTextSize(font, s_wszBuffer, width, height);

	ApplyTextAlignment(alignment, x, y, width, height);

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(s_wszBuffer, wcslen(s_wszBuffer));
}

void CDrawing::DrawWideStringF(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const wchar_t *pwszString, ...)
{
	va_list va_alist;
	va_start(va_alist, pwszString);
	swprintf(s_wszBuffer, sizeof(s_wszBuffer), pwszString, va_alist);
	va_end(va_alist);

	int width, height;
	g_pSurface->GetTextSize(font, s_wszBuffer, width, height);

	ApplyTextAlignment(alignment, x, y, width, height);

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(s_wszBuffer, wcslen(s_wszBuffer));
}

//-----------------------------------------------------------------------------
// Draw a string
//-----------------------------------------------------------------------------

void CDrawing::DrawString(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString)
{
	MultiByteToWideChar(CP_UTF8, 0, pszString, 256, s_wszBuffer, 256);

	int width, height;
	g_pSurface->GetTextSize(font, s_wszBuffer, width, height);

	ApplyTextAlignment(alignment, x, y, width, height);

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(s_wszBuffer, wcslen(s_wszBuffer));
}

void CDrawing::DrawStringACP(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString)
{
	MultiByteToWideChar(CP_ACP, 0, pszString, 256, s_wszBuffer, 256);

	int width, height;
	g_pSurface->GetTextSize(font, s_wszBuffer, width, height);

	ApplyTextAlignment(alignment, x, y, width, height);

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(s_wszBuffer, wcslen(s_wszBuffer));
}


void CDrawing::DrawWideString(vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const wchar_t *pwszString)
{
	int width, height;
	g_pSurface->GetTextSize(font, pwszString, width, height);

	ApplyTextAlignment(alignment, x, y, width, height);

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(pwszString, wcslen(pwszString));
}

//-----------------------------------------------------------------------------
// Misc. draw methods
//-----------------------------------------------------------------------------

void CDrawing::DrawCrosshair(int x, int y, int r, int g, int b, int a, int iSize /* = 10 */, int iGap /* = 4 */, int iThickness /* = 2 */)
{
	int thickness = iThickness - 1;

	// Left
	g_Drawing.FillArea(x - iGap - iSize + 1, y - (thickness / 2), iSize, iThickness, r, g, b, a);

	// Right
	g_Drawing.FillArea(x + iGap, y - (thickness / 2), iSize, iThickness, r, g, b, a);

	// Up
	g_Drawing.FillArea(x - (thickness / 2), y - iGap - iSize + 1, iThickness, iSize, r, g, b, a);

	// Down
	g_Drawing.FillArea(x - (thickness / 2), y + iGap, iThickness, iSize, r, g, b, a);
}

void CDrawing::DrawDot(int x, int y, int r, int g, int b, int a, int iThickness /* = 2 */)
{
	int thickness = iThickness - 1;

	g_Drawing.FillArea(x - (thickness / 2), y - (thickness / 2), iThickness, iThickness, r, g, b, a);
}

void CDrawing::DrawCrosshairShadow(int x, int y, int r, int g, int b, int a, int iSize /* = 10 */, int iGap /* = 4 */, int iThickness /* = 2 */, int iShadowThickness /* = 1 */)
{
	int thickness = iThickness - 1;

	// Left
	g_Drawing.FillArea(x - iGap - iSize + 1 - iShadowThickness + (iShadowThickness / 2),
					   y - (thickness / 2) - iShadowThickness + (iShadowThickness / 2),
					   iSize + iShadowThickness,
					   iThickness + iShadowThickness,
					   r, g, b, a);

	// Right
	g_Drawing.FillArea(x + iGap - iShadowThickness + (iShadowThickness / 2),
					   y - (thickness / 2) - iShadowThickness + (iShadowThickness / 2),
					   iSize + iShadowThickness,
					   iThickness + iShadowThickness,
					   r, g, b, a);

	// Up
	g_Drawing.FillArea(x - (thickness / 2) - iShadowThickness + (iShadowThickness / 2),
					   y - iGap - iSize + 1 - iShadowThickness + (iShadowThickness / 2),
					   iThickness + iShadowThickness,
					   iSize + iShadowThickness,
					   r, g, b, a);

	// Down
	g_Drawing.FillArea(x - (thickness / 2) - iShadowThickness + (iShadowThickness / 2),
					   y + iGap - iShadowThickness + (iShadowThickness / 2),
					   iThickness + iShadowThickness,
					   iSize + iShadowThickness,
					   r, g, b, a);
}

void CDrawing::DrawDotShadow(int x, int y, int r, int g, int b, int a, int iThickness /* = 2 */, int iShadowThickness /* = 1 */)
{
	int thickness = iThickness - 1;

	g_Drawing.FillArea(x - (thickness / 2) - iShadowThickness + (iShadowThickness / 2),
					   y - (thickness / 2) - iShadowThickness + (iShadowThickness / 2),
					   iThickness + iShadowThickness,
					   iThickness + iShadowThickness,
					   r, g, b, a);
}