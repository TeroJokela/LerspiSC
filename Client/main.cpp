/*
 *	Made by: Tero Jokela
 *	Purpose: A simple GDI screenshotting tool including uploading it to a server via a socket.
 *	   Bugs: They're all just features
 */


#include "Includes.h"


LRESULT APIENTRY WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);

		RECT rect;
		GetWindowRect(hwnd, &rect);

		HDC hdcMem;
		HPAINTBUFFER hpbPaint = BeginBufferedPaint(hdc, &rect, BPBF_COMPATIBLEBITMAP, NULL, &hdcMem);

		// Call our custom paint function
		oSC.Paint(hdcMem);
		
		EndBufferedPaint(hpbPaint, TRUE);
		EndPaint(hwnd, &ps);
		break;
	}

	// The user doesn't want to screenshot
	case WM_RBUTTONDOWN:
	case WM_KEYDOWN:
	{
#ifdef DEBUG
		oLog->writeLog("Capture interrupted by user");
#endif // DEBUG

		oSC.terminate();
		DestroyWindow(hwnd);
		PostQuitMessage(0);
		return 0;
	}

	// Start storing the area when left mouse button gets pressed down
	// (not holding, when just activating) so this only gets called once
	case WM_LBUTTONDOWN:
	{
		GetCursorPos(&oSC.pointStart);
		InvalidateRect(hwnd, NULL, NULL);
		break;
	}

	case WM_MOUSEMOVE:
	{
		// Start updating the end position
		if (GetAsyncKeyState(VK_LBUTTON))
			GetCursorPos(&oSC.pointEnd);

		InvalidateRect(hwnd, NULL, NULL);
		break;
	}

	// Time to crop the screenshot (that we took when the user first launched this program) and upload it to the server
	case WM_LBUTTONUP:
	{
#ifdef DEBUG
		oLog->writeLog("Starting to crop and upload the screenshot (WM_LBUTTONUP)");
#endif // DEBUG

		// We don't need the window anymore
		DestroyWindow(hwnd);

		// Get the end point one more time
		GetCursorPos(&oSC.pointEnd);
		
		// Don't accept 0 x 0 pictures
		if (oSC.pointStart.x == oSC.pointEnd.x && oSC.pointStart.y == oSC.pointStart.y)
		{
#ifdef DEBUG
			oLog->writeLog("The capture area was 0 x 0");
#endif // DEBUG

			oSC.terminate();
			PostQuitMessage(0);
			return 0;
		}

		// This will hold the pointer to the raw bytes from the cropped bitmap
		std::string strPictureBytes;
		
		if (!oSC.CropAndGetRaw(strPictureBytes) || strPictureBytes.empty())
		{
#ifdef DEBUG
			oLog->writeLog("Screenshot::CropAndGetRaw() - failed...");
#endif // DEBUG

			MessageBoxA((HWND)0, "Something went wrong...\nPlease contact the developer.", "LerspiSC - Error", MB_OK);
			oSC.terminate();
			PostQuitMessage(0);
			return 0;
		}

		// We don't need this anymore
		oSC.terminate();

#ifdef DEBUG
		oLog->writeLog("Upload::initialize() - starting");
#endif // DEBUG

		// First, initialize the connection
		if (!oUp.initialize())
		{
#ifdef DEBUG
			oLog->writeLog("Upload::initialize() - failed...");
#endif // DEBUG

			oUp.terminate();
			MessageBoxA((HWND)0, "Something went wrong...\nPlease contact the developer.", "LerspiSC - Error", MB_OK);
			PostQuitMessage(0);
			return 0;
		}

#ifdef DEBUG
		oLog->writeLog("Upload::initialize() - success");
#endif // DEBUG

#ifdef DEBUG
		oLog->writeLog("Upload::dataTransfer() - starting");
#endif // DEBUG

		std::string strAnswer;
		if (!oUp.dataTransfer(strPictureBytes, strAnswer))
		{
#ifdef DEBUG
			oLog->writeLog("Upload::dataTransfer() - failed...");
#endif // DEBUG

			oUp.terminate(); 

			if (strAnswer.substr(0, 5) == "Error")
				MessageBoxA((HWND)0, strAnswer.c_str(), "LerspiSC - Error", MB_OK);
			else
				MessageBoxA((HWND)0, "Something went wrong...\nPlease contact the developer.", "LerspiSC - Error", MB_OK);
			
			PostQuitMessage(0);
			return 0;
		}

#ifdef DEBUG
		oLog->writeLog("Upload::dataTransfer() - success");
#endif // DEBUG

		// We don't need this anymore
		oUp.terminate();

		MessageBoxA((HWND)0, strAnswer.c_str(), "LerspiSC - Answer from server", MB_OK);

		// We're done!
		PostQuitMessage(0);
		return 0;
	}

	// If DestroyWindow() was called
	case WM_DESTROY:
	case WM_NCDESTROY:
		break;

	// I don't think this really does anything, but I'll just leave this here just in case it does :)
	case WM_ERASEBKGND:
		return TRUE;

	default:
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


// Create a the window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	char className[] = "LerspiSC";
	HWND hwnd = NULL;

	// Our window class
	WNDCLASS wc;
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL; // TODO: add an icon
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);
	wc.hbrBackground = (HBRUSH)(0);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;

	// Register the class
	if (RegisterClass(&wc))
	{
		// Create our window
		hwnd = CreateWindowEx(
			WS_EX_TRANSPARENT | WS_EX_TOPMOST,
			className, NULL,
			WS_POPUP | WS_VISIBLE,
			0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			NULL, NULL, hInstance, NULL);
	}

	if (!hwnd)
		return FALSE;

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	return TRUE;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef DEBUG
	oLog->clearLog();
	oLog->writeLog("** Program started! **");
#endif // DEBUG
	
#ifdef DEBUG
	oLog->writeLog("Screenshot::initialize() - starting");
#endif // DEBUG

	if (!oSC.initialize())
	{
#ifdef DEBUG
		oLog->writeLog("Screenshot::initialize() - failed...");
#endif // DEBUG

		oSC.terminate();
		return 0;
	}

#ifdef DEBUG
	oLog->writeLog("Screenshot::initialize() - success");
#endif // DEBUG

#ifdef DEBUG
	oLog->writeLog("InitInstance() - starting");
#endif // DEBUG

	if (!InitInstance(hInstance, nCmdShow))
	{
#ifdef DEBUG
		oLog->writeLog("InitInstance() - failed... GetLastError(): " + std::to_string(GetLastError()));
#endif // DEBUG

		oSC.terminate();
		return 0;
	}

#ifdef DEBUG
	oLog->writeLog("InitInstance() - success");
#endif // DEBUG

	MSG msg;
	
	// The "main" loop
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

#ifdef DEBUG
	oLog->writeLog("** Closing... **");
#endif // DEBUG
	
	return msg.wParam;
}
