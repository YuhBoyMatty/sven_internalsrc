// Drawing

#pragma comment(lib, "OpenGL32")
#pragma warning(disable : 4244)

#include <Windows.h>
#include <gl/GL.h>

#include "drawing.h"

#include "utils.h"
#include "../interfaces.h"

//-----------------------------------------------------------------------------

#define swap(a, b) __swap_buffer = a; a = b; b = __swap_buffer

//-----------------------------------------------------------------------------

static int __swap_buffer = 0;

CDrawing g_Drawing;

vgui::HFont g_hESP;
vgui::HFont g_hESP2;
//vgui::HFont MENU;

extern vgui::ISurface *g_pSurface;

//-----------------------------------------------------------------------------

void CDrawing::SetupFonts()
{
	g_pSurface->AddGlyphSetToFont(g_hESP = g_pSurface->CreateFont(), "Choktoff", 12, FW_BOLD, NULL, NULL, FONTFLAG_DROPSHADOW, 0, 0);
	g_pSurface->AddGlyphSetToFont(g_hESP2 = g_pSurface->CreateFont(), "Choktoff", 38, FW_MEDIUM, NULL, NULL, FONTFLAG_DROPSHADOW, 0, 0);
	//g_pSurface->AddGlyphSetToFont(MENU = g_pSurface->CreateFont(), "Arial", 14, FW_BOLD, NULL, NULL, FONTFLAG_NONE, 0, 0); //Main font
}

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

	if (x1 < x0)
		swap(x1, x0);

	if (y1 < y0)
		swap(y1, y0);

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

void CDrawing::DrawTexture(int id, int x0, int y0, int x1, int y1, int r, int g, int b, int a)
{
	g_pSurface->DrawSetColor(r, g, b, a);
	g_pSurface->DrawSetTexture(id);
	g_pSurface->DrawTexturedRect(x0, y0, x1, y1);
}

void CDrawing::DrawRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int x0 = x;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	if (x1 < x0)
		swap(x1, x0);

	if (y1 < y0)
		swap(y1, y0);

	g_pSurface->DrawSetColor(r, g, b, a);
	g_pSurface->DrawFilledRect(x0, y0, x1, y1);
}

void CDrawing::DrawOutlinedRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int x0 = x;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	if (x1 < x0)
		swap(x1, x0);

	if (y1 < y0)
		swap(y1, y0);

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

void CDrawing::DrawString(vgui::HFont font, int x, int y, int r, int g, int b, int a, DWORD alignment, const char* msg, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int width, height;
	g_pSurface->GetTextSize(font, wbuf, width, height);

	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(wbuf, wcslen(wbuf));
}

void CDrawing::DrawStringACP(vgui::HFont font, int x, int y, int r, int g, int b, int a, DWORD alignment, const char* msg, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, 256, wbuf, 256);

	int width, height;
	g_pSurface->GetTextSize(font, wbuf, width, height);

	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(wbuf, wcslen(wbuf));
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

void CDrawing::DrawCrosshair(int x, int y, int r, int g, int b, int a, int Size /* = 10 */, int Gap /* = 4 */, int Thickness /* = 2 */)
{
	int thickness = Thickness - 1;

	g_Drawing.FillArea(x - Gap - Size + 1 - thickness, y - thickness, Size, Thickness, r, g, b, a);
	g_Drawing.FillArea(x + Gap, y - thickness, Size, Thickness, r, g, b, a);
	g_Drawing.FillArea(x - thickness, y - Gap - Size + 1 - thickness, Thickness, Size, r, g, b, a);
	g_Drawing.FillArea(x - thickness, y + Gap, Thickness, Size, r, g, b, a);
}

void CDrawing::DrawDot(int x, int y, int r, int g, int b, int a, int Thickness /* = 2 */)
{
	int thickness = Thickness - 1;

	g_Drawing.FillArea(x - thickness, y - thickness, Thickness, Thickness, r, g, b, a);
}