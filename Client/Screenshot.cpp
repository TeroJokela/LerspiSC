#include "Includes.h"


bool Screenshot::initialize()
{
	// Start GDI+
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Store our screen resolution in here
	rectScreen = new Gdiplus::Rect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	// Setup some compatible graphics stuff
	HWND hwndDesktop = GetDesktopWindow();
	HDC hdcDesktop = GetDC(hwndDesktop);
	HDC hdcCapture = CreateCompatibleDC(hdcDesktop);

	// Take a screenshot and store it to 'hbmFullscreen'
	HBITMAP hbmFullscreen = CreateCompatibleBitmap(hdcDesktop, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	HANDLE hTemp = SelectObject(hdcCapture, hbmFullscreen);
	BOOL b = BitBlt(hdcCapture, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), hdcDesktop, 0, 0, SRCCOPY | CAPTUREBLT);

	// Free some resources/memory
	DeleteObject(hTemp);
	ReleaseDC(hwndDesktop, hdcDesktop);
	DeleteDC(hdcCapture);

	// We will use this for drawing the overlay and cropping this on CropAndGetRaw()
	bmapFullscreen = new Gdiplus::Bitmap(hbmFullscreen, (HPALETTE)0);
	
	// We don't need this anymore
	DeleteObject(hbmFullscreen);

	if (!b)
	{
#ifdef DEBUG
		oLog->writeLog("BitBlt() - failed... GetLastError: " + GetLastError());
#endif // DEBUG

		return false;
	}

	// Setup our drawing tools and fonts
	penBlack = new Gdiplus::Pen(Gdiplus::Color::Black);
	penLightGray = new Gdiplus::Pen(Gdiplus::Color::LightGray);
	sbDarkGray = new Gdiplus::SolidBrush(Gdiplus::Color::DarkGray);
	sbBlackTransparent = new Gdiplus::SolidBrush(Gdiplus::Color(50, 0, 0, 0));
	fConsolas = new Gdiplus::Font(L"Consolas", (Gdiplus::REAL)14);
	ffConsolas = new Gdiplus::FontFamily(L"Consolas");

	return true;
}


void Screenshot::terminate()
{
#ifdef DEBUG
	oLog->writeLog("Screenshot::terminate() - starting");
#endif // DEBUG

	delete penBlack;
	delete penLightGray;
	delete sbDarkGray;
	delete sbBlackTransparent;
	delete fConsolas;
	delete ffConsolas;
	delete rectScreen;
	delete bmapFullscreen;
	Gdiplus::GdiplusShutdown(gdiplusToken);

#ifdef DEBUG
	oLog->writeLog("Screenshot::terminate() - success");
#endif // DEBUG
}


// This draws the cursor coordinates close to the cursor
void Screenshot::DrawCursorCoords(Gdiplus::Graphics &gfx)
{

	std::wstring wstrX = std::to_wstring(abs(pointCurrent.x - pointStart.x));
	std::wstring wstrY = std::to_wstring(abs(pointCurrent.y - pointStart.y));

	// These are for drawing the outline for the coordinates
	Gdiplus::GraphicsPath gpOutline;

	Gdiplus::REAL yPos = pointCurrent.y;

	bool bDrawBox = GetAsyncKeyState(VK_LBUTTON);

	if (pointCurrent.y > pointStart.y && bDrawBox)
		yPos = pointCurrent.y - 38;

	// Draw outline for the coordinates
	gpOutline.AddString(wstrX.c_str(), (INT)wstrX.length(), ffConsolas, Gdiplus::FontStyleRegular, (Gdiplus::REAL)19, Gdiplus::PointF((Gdiplus::REAL)((pointStart.x < pointCurrent.x && bDrawBox) ? pointCurrent.x - 12.5 * wstrX.length() : pointCurrent.x), yPos), NULL);
	gpOutline.AddString(wstrY.c_str(), (INT)wstrY.length(), ffConsolas, Gdiplus::FontStyleRegular, (Gdiplus::REAL)19, Gdiplus::PointF((Gdiplus::REAL)((pointStart.x < pointCurrent.x && bDrawBox) ? pointCurrent.x - 12.5 * wstrY.length() : pointCurrent.x), yPos + 16), NULL);
	gfx.DrawPath(penBlack, &gpOutline);

	// Draw the coordinates
	gfx.DrawString(wstrX.c_str(), (INT)wstrX.length(), fConsolas, Gdiplus::PointF((Gdiplus::REAL)((pointStart.x < pointCurrent.x && bDrawBox) ? pointCurrent.x - 12.5 * wstrX.length() : pointCurrent.x), yPos), sbDarkGray);
	gfx.DrawString(wstrY.c_str(), (INT)wstrY.length(), fConsolas, Gdiplus::PointF((Gdiplus::REAL)((pointStart.x < pointCurrent.x && bDrawBox) ? pointCurrent.x - 12.5 * wstrY.length() : pointCurrent.x), yPos + 16), sbDarkGray);
}


void Screenshot::DrawBoundingBox(Gdiplus::Graphics &gfx)
{
	Gdiplus::Rect r;

	if (pointStart.x < pointCurrent.x)
	{
		r.X = (INT)pointStart.x;
		r.Width = (INT)(pointCurrent.x - pointStart.x);
	}
	else if (pointStart.x > pointCurrent.x)
	{
		r.X = (INT)pointCurrent.x;
		r.Width = (INT)(pointStart.x - pointCurrent.x);
	}

	if (pointStart.y < pointCurrent.y)
	{
		r.Y = (INT)pointStart.y;
		r.Height = (INT)(pointCurrent.y - pointStart.y);
	}
	else if (pointStart.y > pointCurrent.y)
	{
		r.Y = (INT)pointCurrent.y;
		r.Height = (INT)(pointStart.y - pointCurrent.y);
	}

	gfx.FillRectangle(sbBlackTransparent, r);
	gfx.DrawRectangle(penBlack, r);
}


// Paint our stuff, pretty simple
void Screenshot::Paint(HDC &hdc)
{
	GetCursorPos(&pointCurrent);

	Gdiplus::Graphics gfx(hdc);

	// These should speed up Gdiplus just a tiny bit, but the amount is (almost?) unnoticeable
	gfx.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);
	gfx.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
	gfx.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
	gfx.SetSmoothingMode(Gdiplus::SmoothingModeNone);
	gfx.SetInterpolationMode(Gdiplus::InterpolationModeDefault);

	gfx.DrawImage(bmapFullscreen, *rectScreen);

	if (GetAsyncKeyState(VK_LBUTTON))
		DrawBoundingBox(gfx);

	DrawCursorCoords(gfx);
}


bool Screenshot::CropAndGetRaw(std::string &strReturn)
{
	HBITMAP hbmFullscreen;

	if (bmapFullscreen->GetHBITMAP(Gdiplus::Color::Transparent, &hbmFullscreen) != Gdiplus::Status::Ok)
	{
#ifdef DEBUG
		oLog->writeLog("Bitmap->GetHBITMAP() - failed... GetLastStatus: " + bmapFullscreen->GetLastStatus());
#endif // DEBUG

		return false;
	}

	// Crop the hbmFullscreen bitmap and store it to to hbmClone
	HBITMAP hbmClone = (HBITMAP)CopyImage(hbmFullscreen, IMAGE_BITMAP, abs(pointStart.x - pointEnd.x), abs(pointStart.y - pointEnd.y), LR_CREATEDIBSECTION);

	if (!hbmClone)
	{
#ifdef DEBUG
		oLog->writeLog("CopyImage() - failed... GetLastError: " + GetLastError());
#endif // DEBUG

		return false;
	}

	// Create 2 HDCs that are compatible with the display driver
	HDC hdcTemp = CreateCompatibleDC(0);
	HDC hdcTemp2 = CreateCompatibleDC(0);

	// We could have the SelectObject() functions not to return to a variable, but this way we can delete the handles explicitly
	HANDLE hTemp = (HANDLE)SelectObject(hdcTemp, hbmFullscreen);
	HANDLE hTemp2 = (HANDLE)SelectObject(hdcTemp2, hbmClone);

	// Holy fuck this one-liner
	BOOL b = BitBlt(hdcTemp2, 0, 0, abs(pointStart.x - pointEnd.x), abs(pointStart.y - pointEnd.y), hdcTemp, (pointStart.x < pointEnd.x) ? pointStart.x : pointEnd.x, (pointStart.y < pointEnd.y) ? pointStart.y : pointEnd.y, SRCCOPY);

	// Free resources/memory
	DeleteObject(hbmFullscreen);
	DeleteObject(hTemp);
	DeleteObject(hTemp2);
	DeleteDC(hdcTemp);
	DeleteDC(hdcTemp2);

	// Store the cropped bitmap here
	Gdiplus::Bitmap bmap(hbmClone, (HPALETTE)0);
	
	// We don't need this anymore
	DeleteObject(hbmClone);

	// check if BitBlt failed
	if (!b)
	{
#ifdef DEBUG
		oLog->writeLog("BitBlt() - failed... GetLastError: " + GetLastError());
#endif // DEBUG

		return false;
	}
	
	CLSID clsidPNG;

	// Get the PNG encoder
	HRESULT hrCLSIDFromString = CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsidPNG);
	if (hrCLSIDFromString != NOERROR)
	{
#ifdef DEBUG
		oLog->writeLog("CLSIDFromString() - failed... Error: " + std::to_string(hrCLSIDFromString));
#endif // DEBUG

		return false;
	}

	// We will write the encoded picture data here
	IStream * istreamFile = NULL;
	
	HRESULT hrCreateStreamOnHGlobal = CreateStreamOnHGlobal(NULL, TRUE, &istreamFile);
	if (hrCreateStreamOnHGlobal != NOERROR)
	{
#ifdef DEBUG
		oLog->writeLog("CreateStreamOnHGlobal() - failed... Error: " + std::to_string(hrCreateStreamOnHGlobal));
#endif // DEBUG

		return false;
	}

#ifdef DEBUG
	bmap.Save(L"Debug.png", &clsidPNG);
#endif // DEBUG

	// Save the data to isFile
	if (bmap.Save(istreamFile, &clsidPNG) != Gdiplus::Status::Ok)
	{
#ifdef DEBUG
		oLog->writeLog("Bitmap::Save() - failed... GetLastStatus: " + std::to_string(bmap.GetLastStatus()));
#endif // DEBUG

		return false;
	}

	// The memory handle associated with istreamFile
	HGLOBAL hgMemHandle = NULL;
	HRESULT hrGetHGlobalFromStream = GetHGlobalFromStream(istreamFile, &hgMemHandle);
	if (hrGetHGlobalFromStream != NOERROR)
	{
#ifdef DEBUG
		oLog->writeLog("GetHGlobalFromStream() - failed... Error: " + std::to_string(hrGetHGlobalFromStream));
#endif // DEBUG

		return false;
	}

	// Get the size
	size_t s_tSize = GlobalSize(hgMemHandle);

	// Resize the string to hold all the data correctly
	strReturn.resize(s_tSize);

	// Lock the memory and get the pointer to the first byte of the array of bytes
	LPVOID ptrData = GlobalLock(hgMemHandle);

	// Copy the PNG data from memory to our strReturn
	memcpy(&strReturn[0], ptrData, s_tSize);

	// Unlock the data
	if (GlobalUnlock(hgMemHandle) != NOERROR)
	{
#ifdef DEBUG
		oLog->writeLog("GlobalUnlock() - failed... Error: " + std::to_string(GetLastError()));
#endif // DEBUG

		return false;
	}

	istreamFile->Release();

	return true;
}


Screenshot oSC;

