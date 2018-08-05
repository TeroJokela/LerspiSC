#pragma once

#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <Windows.h>
#include <string>
#include <random>
#include <fstream>

#include <winsock.h>
#pragma comment(lib, "wsock32.lib")

#include <gdiplus.h>
#pragma comment (lib, "Gdiplus.lib")
#include <uxtheme.h>
#pragma comment (lib, "uxtheme.lib")


#include "Upload.h"
#include "Screenshot.h"


// Uncomment this on for error-messages and logging
//#define DEBUG 1

#ifdef DEBUG
#include "DebugLogger.h"
#endif // DEBUG