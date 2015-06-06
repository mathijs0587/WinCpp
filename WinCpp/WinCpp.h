#pragma once

#include "resource.h"
#include <string>
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <iostream>
#include <d3dx9.h>
#include <Dwmapi.h> 
#include <TlHelp32.h>
#include <tchar.h>
#include <strsafe.h>
#include <sstream>
#include <vector>
#include <algorithm>

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "Dwmapi.lib")

#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080

#define ESPheight	56000
#define ESPwidth	18100


//Offsets
#define LocalPlayer 0xA5F7F4
#define EntityList	0x4A01B54
#define EntitySize	0x10
#define InCrosshair	0x23DC
#define teamNumber	0xF0
#define HealthOff	0xFC
#define LifeStatus	0x25B
#define BoneMatrix	0xA78

#define PositionX	0x134
#define PositionY	0x138
#define PositionZ	0x13C

#define viewOffsetX	0x2384
#define viewOffsetY	0x2388

#define PunchX		0x13DC
#define PunchY		0x13E0

#define EPointer	0x50B054
#define EPosition	0x5B2524

#define ViewMatrix	0x49F70A4

//Functions
// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);    // closes Direct3D and releases memory
void DrawString(int x, int y, int FontSize, DWORD color, LPD3DXFONT g_pFont, LPCSTR Message);
void DrawLine(float Bx, float By, float Ex, float Ey, int A, int R, int G, int B);
void DrawRect(int x, int y, int width, int height, D3DCOLOR color);
void DrawESP(double hp, int x, int y, float distance, D3DCOLOR color);
bool WorldToScreen(float * from, float * to);
float Get3dDistance(float * myCoords, float * enemyCoords);
void DrawBox(int Bx, int By, int Ex, int Ey, D3DCOLOR color);
void GetBonePosition(int iBone, int player, HANDLE proc);
DWORD WINAPI mainThread();