#pragma once


class Screenshot
{
public:
	bool initialize();
	void terminate();
	void Paint(HDC &hdc);
	bool CropAndGetRaw(std::string &strReturn);
	
	POINT pointCurrent;
	POINT pointStart;
	POINT pointEnd;

private:
	void DrawCursorCoords(Gdiplus::Graphics &gfx);
	void DrawBoundingBox(Gdiplus::Graphics &gfx);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	// Store the "screenshot" here when first launching the program
	Gdiplus::Bitmap * bmapFullscreen;

	Gdiplus::Rect * rectScreen;

	// Our objects that we will use when drawing
	Gdiplus::Pen * penBlack;
	Gdiplus::Pen * penLightGray;
	Gdiplus::SolidBrush * sbDarkGray;
	Gdiplus::SolidBrush * sbBlackTransparent;
	Gdiplus::Font * fConsolas;
	Gdiplus::FontFamily * ffConsolas;

};


extern Screenshot oSC;