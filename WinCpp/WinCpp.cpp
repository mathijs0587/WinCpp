// WinCpp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WinCpp.h"

#define MAX_LOADSTRING 100

#pragma warning(disable:4244) //int to float conv. warning

const MARGINS  margin = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };


DWORD Module(LPSTR ModuleName);

std::string GameName = "Counter-Strike: Global Offensive";
LPCSTR LGameWindow = "Counter-Strike: Global Offensive";
LPCSTR lWnd = "Yes";
std::string GameStatus;

bool IsGameAvail;


DWORD dwProcID = NULL;

DWORD vInCross = NULL;
DWORD vLTeamNum = NULL;
DWORD vLHealth = NULL;
DWORD dwLocalPlayer;

bool toggle = false;
bool radar = false;
bool trigger = false;
bool norecoil = false;

#define MAX_THREADS 1
#define BUF_SIZE 255

// global declarations
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class

LPD3DXFONT pFont;

RECT m_Rect;


float PlayerXY[3];
float EnemyXY[3];
float FriendlyXY[3];
float BoneXY[3];

int readBone = 10;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

typedef struct
{
	float flMatrix[4][4];
}WorldToScreenMatrix_t;

typedef struct
{
	float bMatrix[3][4];
}Matrix3x4;

struct PlayersInfo
{
	DWORD playerAddress;
	DWORD boneMatrixAddress;
	DWORD teamNum;
	int Health;
	float Position[3];
	float x;
	float y;
	float z;
	float pitch;
	float yaw;
	int lifeState;
	float distance;
	DWORD vis;
	float boneXYZ[3];
	DWORD Name;
};

struct TargetsInfo
{
	float AimXY[2];
	float distance;
	float AimDistance;

	TargetsInfo()
	{

	}

	TargetsInfo(float myCoords[], float enemyCoords[], float aim[])
	{
		distance = Get3dDistance(myCoords, enemyCoords);

		float width = (ESPwidth / distance) * 0.5;
		float height = (ESPheight / distance) * 0.65;

		AimXY[0] = aim[0];
		AimXY[1] = aim[1];		

		float xdist = (AimXY[0] - (SCREEN_WIDTH / 2));
		float ydist = (AimXY[1] - (SCREEN_HEIGHT / 2));
		AimDistance = sqrt(pow(ydist, 2) + pow(xdist, 2));

		
		
	}
};

struct LocalPlayerStruct
{
	WorldToScreenMatrix_t WorldToScreenMatrix;
	DWORD teamNum;
	int Health;
	float Position[3];
	float x;
	float y;
	float z;
	float pitch;
	float yaw;
	std::vector<float> viewAng;
	std::vector<float> punchAng;
};

PlayersInfo playersInfo[64];
LocalPlayerStruct localPlayerStruct;

float RectSize = 15;

int aimbotSize = 100;

struct CompareTargetEnArray
{
	//USE A COMPARATOR TO SORT OUR ARRAY nicely
	bool operator() (TargetsInfo & lhs, TargetsInfo & rhs)
	{
		return lhs.AimDistance < rhs.AimDistance;
	}
};

void GetBonePosition(int iBone, int player, HANDLE proc)
{
	ReadProcessMemory(proc, (LPCVOID)(playersInfo[player].boneMatrixAddress + 0x30 * iBone + 0xC), &playersInfo[player].boneXYZ[0], sizeof(playersInfo[player].boneXYZ[0]), 0);
	ReadProcessMemory(proc, (LPCVOID)(playersInfo[player].boneMatrixAddress + 0x30 * iBone + 0x1C), &playersInfo[player].boneXYZ[1], sizeof(playersInfo[player].boneXYZ[1]), 0);
	ReadProcessMemory(proc, (LPCVOID)(playersInfo[player].boneMatrixAddress + 0x30 * iBone + 0x2C), &playersInfo[player].boneXYZ[2], sizeof(playersInfo[player].boneXYZ[2]), 0);
}

float Get3dDistance(float * myCoords, float * enemyCoords)
{
	return sqrt(
		pow(double(enemyCoords[0] - myCoords[0]), 2.0) +
		pow(double(enemyCoords[1] - myCoords[1]), 2.0) +
		pow(double(enemyCoords[2] - myCoords[2]), 2.0));

}

bool WorldToScreen(float * from, float * to)
{
	float w = 0.0f;

	to[0] = localPlayerStruct.WorldToScreenMatrix.flMatrix[0][0] * from[0] + localPlayerStruct.WorldToScreenMatrix.flMatrix[0][1] * from[1] + localPlayerStruct.WorldToScreenMatrix.flMatrix[0][2] * from[2] + localPlayerStruct.WorldToScreenMatrix.flMatrix[0][3];
	to[1] = localPlayerStruct.WorldToScreenMatrix.flMatrix[1][0] * from[0] + localPlayerStruct.WorldToScreenMatrix.flMatrix[1][1] * from[1] + localPlayerStruct.WorldToScreenMatrix.flMatrix[1][2] * from[2] + localPlayerStruct.WorldToScreenMatrix.flMatrix[1][3];
	w = localPlayerStruct.WorldToScreenMatrix.flMatrix[3][0] * from[0] + localPlayerStruct.WorldToScreenMatrix.flMatrix[3][1] * from[1] + localPlayerStruct.WorldToScreenMatrix.flMatrix[3][2] * from[2] + localPlayerStruct.WorldToScreenMatrix.flMatrix[3][3];

	if (w < 0.01f)
		return false;

	float invw = 1.0f / w;
	to[0] *= invw;
	to[1] *= invw;

	int width = (int)(m_Rect.right - m_Rect.left);
	int height = (int)(m_Rect.bottom - m_Rect.top);

	float x = width / 2;
	float y = height / 2;

	x += 0.5 * to[0] * width + 0.5;
	y -= 0.5 * to[1] * height + 0.5;

	to[0] = x + m_Rect.left;
	to[1] = y + m_Rect.top;

	return true;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	wc.lpszClassName = "WindowClass";

	RegisterClassEx(&wc);	

		hWnd = CreateWindowEx(NULL,
			"WindowClass",
			"DirectX Test",
			WS_EX_TOPMOST | WS_POPUP,
			0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			NULL,
			NULL,
			hInstance,
			NULL);


	SetWindowLong(hWnd, GWL_EXSTYLE, (int)GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	//SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	initD3D(hWnd);	

	DWORD   dwThreadIdArray[MAX_THREADS];
	HANDLE  hThreadArray[MAX_THREADS];
	
	for (int i = 0; i < MAX_THREADS; i++)
	{
		hThreadArray[i] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)mainThread,
			NULL,
			0,
			&dwThreadIdArray[i]);
	}
	
	m_Rect.left = 0;
	m_Rect.top = 0;
	m_Rect.right = SCREEN_WIDTH;
	m_Rect.bottom = SCREEN_HEIGHT;

	D3DXCreateFont(d3ddev, 10, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &pFont);
	// enter the main loop:
	MSG msg;
	::SetWindowPos(FindWindow(NULL, LGameWindow), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	while (TRUE)
	{		
		Sleep(10);		
		::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, NULL);		
		
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		HWND hGameWindow = NULL;
		HANDLE hProcHandle = NULL;
		float vPunchX = NULL;
		float vPunchY = NULL;
		float vPunchZ = NULL;

		if (GetAsyncKeyState(VK_OEM_PLUS))
		{
			RectSize += 0.1f;
		}

		if (GetAsyncKeyState(VK_OEM_MINUS))
		{
			if (RectSize > 1)
			{
				RectSize -= 0.1f;
			}
		}

		if (GetAsyncKeyState(VK_NUMPAD7))
		{
			aimbotSize += 1;
		}
		if (GetAsyncKeyState(VK_NUMPAD8))
		{
			aimbotSize -= 1;
		}

		if (GetAsyncKeyState(VK_NUMPAD5))
		{
			readBone += 1;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD6) && readBone != 0)
		{
			readBone -= 1;
			Sleep(200);
		}

		hGameWindow = FindWindow(NULL, LGameWindow);
		if (hGameWindow)
		{
			GetWindowThreadProcessId(hGameWindow, &dwProcID);
			if (dwProcID != 0)
			{
				hProcHandle = OpenProcess(PROCESS_ALL_ACCESS, false, dwProcID);
				if (hProcHandle == INVALID_HANDLE_VALUE || hProcHandle == NULL)
				{
					IsGameAvail = false;
				}
				else
				{
					IsGameAvail = true;
				}
			}
		}

		if (IsGameAvail)
		{
			GetWindowRect(FindWindow(NULL, LGameWindow), &m_Rect);

			if (GetAsyncKeyState(VK_NUMPAD9))
			{
				toggle = false;
				radar = false;
				trigger = false;
				norecoil = false;
			}

			if (GetAsyncKeyState(VK_NUMPAD1))
			{
				toggle = true;
			}

			if (GetAsyncKeyState(VK_NUMPAD2))
			{
				radar = true;
			}

			if (GetAsyncKeyState(VK_NUMPAD3))
			{
				trigger = true;
			}

			if (GetAsyncKeyState(VK_NUMPAD4))
			{
				norecoil = true;
			}

			if (toggle)
			{

				DWORD ClientBase = Module("client.dll");
				DWORD temp1 = ClientBase + LocalPlayer;
				ReadProcessMemory(hProcHandle, (LPCVOID)temp1, &dwLocalPlayer, 4, NULL);

				/* Reads the value of InCross */
				DWORD inCross = dwLocalPlayer + InCrosshair;
				DWORD ReadInCross = inCross;

				DWORD Lpitch = dwLocalPlayer + viewOffsetX;
				DWORD Lyaw = dwLocalPlayer + viewOffsetY;

				ReadProcessMemory(hProcHandle, (LPCVOID)Lpitch, &localPlayerStruct.pitch, sizeof(localPlayerStruct.viewAng), 0);
				ReadProcessMemory(hProcHandle, (LPCVOID)Lyaw, &localPlayerStruct.yaw, sizeof(localPlayerStruct.yaw), 0);

				ReadProcessMemory(hProcHandle, (LPCVOID)ReadInCross, &vInCross, sizeof(vInCross), 0);

				/* Read the value of LocalPlayer-TeamNum */
				DWORD LTeamNum = dwLocalPlayer + teamNumber;
				DWORD ReadLTeamNum = LTeamNum;
				
				ReadProcessMemory(hProcHandle, (LPCVOID)ReadLTeamNum, &vLTeamNum, sizeof(vLTeamNum), 0);

				DWORD LPosX = dwLocalPlayer + PositionX;
				DWORD LPosY = dwLocalPlayer + PositionY;
				DWORD LPosZ = dwLocalPlayer + PositionZ;

				ReadProcessMemory(hProcHandle, (LPCVOID)LPosX, &localPlayerStruct.Position[0], sizeof(localPlayerStruct.Position[0]), 0);
				ReadProcessMemory(hProcHandle, (LPCVOID)LPosY, &localPlayerStruct.Position[1], sizeof(localPlayerStruct.Position[1]), 0);
				ReadProcessMemory(hProcHandle, (LPCVOID)LPosZ, &localPlayerStruct.Position[2], sizeof(localPlayerStruct.Position[2]), 0);

				localPlayerStruct.teamNum = vLTeamNum;


				DWORD temp51 = ClientBase + ViewMatrix;
				ReadProcessMemory(hProcHandle, (LPCVOID)temp51, &localPlayerStruct.WorldToScreenMatrix, sizeof(localPlayerStruct.WorldToScreenMatrix), 0);

				TargetsInfo* targetsInfo = new TargetsInfo[64];

				DWORD temp100 = Module("client.dll") + 0x4A3554C + 0x50;
				DWORD temp101;

				ReadProcessMemory(hProcHandle, (LPCVOID)temp100, &temp101, sizeof(temp101), NULL);

				int targetLoop = 0;
				/* Read the Value of EnityList-TeamNum */
				for (int i = 0; i < 64; i++)
				{
					DWORD temp2 = ClientBase + EntityList + (i * EntitySize);
					ReadProcessMemory(hProcHandle, (LPCVOID)temp2, &playersInfo[i].playerAddress, sizeof(playersInfo[i].playerAddress), NULL);
					DWORD temp3 = playersInfo[i].playerAddress + teamNumber;
					ReadProcessMemory(hProcHandle, (LPCVOID)temp3, &playersInfo[i].teamNum, sizeof(playersInfo[i].teamNum), NULL);

					DWORD posX = playersInfo[i].playerAddress + PositionX;
					DWORD posY = playersInfo[i].playerAddress + PositionY;
					DWORD posZ = playersInfo[i].playerAddress + PositionZ;

					DWORD temp102 = temp101 + 0x1E0 * i;

					DWORD temp103 = temp102 + 0x24;

					ReadProcessMemory(hProcHandle, (LPCVOID)temp103, &playersInfo[i].Name, sizeof(playersInfo[i].Name), NULL);

					ReadProcessMemory(hProcHandle, (LPCVOID)posX, &playersInfo[i].Position[0], sizeof(playersInfo[i].Position[0]), NULL);
					ReadProcessMemory(hProcHandle, (LPCVOID)posY, &playersInfo[i].Position[1], sizeof(playersInfo[i].Position[1]), NULL);
					ReadProcessMemory(hProcHandle, (LPCVOID)posZ, &playersInfo[i].Position[2], sizeof(playersInfo[i].Position[2]), NULL);

					DWORD LifeState = playersInfo[i].playerAddress + LifeStatus;

					ReadProcessMemory(hProcHandle, (LPCVOID)LifeState, &playersInfo[i].lifeState, sizeof(playersInfo[i].lifeState), NULL);

					DWORD boneAddress = playersInfo[i].playerAddress + BoneMatrix;

					ReadProcessMemory(hProcHandle, (LPCVOID)boneAddress, &playersInfo[i].boneMatrixAddress, sizeof(playersInfo[i].boneMatrixAddress), NULL);

					GetBonePosition(readBone, i, hProcHandle);

					// Read the value of Health
					DWORD LHealth = playersInfo[i].playerAddress + HealthOff;
					DWORD ReadLHealth = LHealth;
					ReadProcessMemory(hProcHandle, (LPCVOID)ReadLHealth, &vLHealth, sizeof(vLHealth), 0);
					playersInfo[i].Health = vLHealth;

					playersInfo[i].distance = Get3dDistance(localPlayerStruct.Position, playersInfo[i].Position);

					float width = (ESPwidth / playersInfo[i].distance) * 0.5;
					float height = (ESPheight / playersInfo[i].distance) * 0.65;

					if (playersInfo[i].teamNum == localPlayerStruct.teamNum)
						continue;

					if (playersInfo[i].Health < 1)
						continue;

					if (playersInfo[i].lifeState == 0)
						continue;

					if (!WorldToScreen(playersInfo[i].boneXYZ, BoneXY))
						continue;

					if (BoneXY[0] < (SCREEN_WIDTH / 2) - aimbotSize || BoneXY[0] > (SCREEN_WIDTH / 2) + aimbotSize)
						continue;

					if (BoneXY[1] < (SCREEN_HEIGHT / 2) - aimbotSize || BoneXY[1] > (SCREEN_HEIGHT / 2) + aimbotSize)
						continue;

					targetsInfo[targetLoop] = TargetsInfo(localPlayerStruct.Position, playersInfo[i].Position, BoneXY);
					

					targetLoop++;

					
				}
				
				if (targetLoop > 0)
				{
					std::sort(targetsInfo, targetsInfo + targetLoop, CompareTargetEnArray());

					if (GetAsyncKeyState(0x45))
					{
						mouse_event(MOUSEEVENTF_ABSOLUTE, targetsInfo[0].AimXY[0] - (m_Rect.right / 2),
							targetsInfo[0].AimXY[1] - (m_Rect.bottom / 2), 0, 0);
					}
				}

				targetLoop = 0;

				delete [] targetsInfo;

				if (vInCross < 64 && vInCross > 0)
				{
					if (vLTeamNum != playersInfo[vInCross - 1].teamNum
						&& playersInfo[vInCross - 1].lifeState != 0
						&& !GetAsyncKeyState(0x01)
						&& trigger)
					{
						mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
						mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
					}
				}
				/*
				if (GetAsyncKeyState(0x01) && norecoil)
				{
					DWORD Lpitch = dwLocalPlayer + viewOffsetX;
					DWORD Lyaw = dwLocalPlayer + viewOffsetY;

					ReadProcessMemory(hProcHandle, (LPCVOID)Lpitch, &localPlayerStruct.pitch, sizeof(localPlayerStruct.viewAng), 0);
					ReadProcessMemory(hProcHandle, (LPCVOID)Lyaw, &localPlayerStruct.yaw, sizeof(localPlayerStruct.yaw), 0);

					DWORD anglePunchX = dwLocalPlayer + PunchX;
					DWORD anglePunchY = dwLocalPlayer + PunchY;

					float aPunchX;
					float aPunchY;

					ReadProcessMemory(hProcHandle, (LPCVOID)anglePunchX, &aPunchX, sizeof(aPunchX), NULL);
					ReadProcessMemory(hProcHandle, (LPCVOID)anglePunchY, &aPunchY, sizeof(aPunchY), NULL);

					DWORD temp40 = Module("engine.dll") + EPointer;
					DWORD temp41;
					
					ReadProcessMemory(hProcHandle, (LPCVOID)temp40, &temp41, sizeof(temp41), NULL);

					DWORD temp42 = temp41 + 0x4C90;
					DWORD temp43 = temp41 + 0x4C94;

					if (aPunchX < 0.001f && aPunchY < 0.001f)
						continue;

					localPlayerStruct.pitch -= aPunchX * 2.0f;
					localPlayerStruct.yaw -= aPunchY * 2.0f;
					WriteProcessMemory(hProcHandle, (LPVOID)temp42, &localPlayerStruct.pitch, sizeof(localPlayerStruct.pitch), NULL);
					WriteProcessMemory(hProcHandle, (LPVOID)temp43, &localPlayerStruct.yaw, sizeof(localPlayerStruct.yaw), NULL);
					
				}*/
			}
		}
	}

	// clean up DirectX and COM
	cleanD3D();
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
					 DwmExtendFrameIntoClientArea(hWnd, &margin);
	}break;

	case WM_DESTROY:
	{
					   PostQuitMessage(0);
					   return 0;
	} break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;    // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = SCREEN_WIDTH;    // set the width of the buffer
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;    // set the height of the buffer

	// create a device class using this information and information from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);
}


void render_frame(void)
{


	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();


	if (radar)
	{
		for (int i = 0; i < 64; i++)
		{
			int playerX = playersInfo[i].Position[0] - localPlayerStruct.Position[0];
			int playerY = playersInfo[i].Position[1] - localPlayerStruct.Position[1];

			float radarPosX = 116;
			float radarPosY = 150;

			if (localPlayerStruct.Position != playersInfo[i].Position)
			{
				if (playersInfo[i].teamNum == vLTeamNum && playersInfo[i].Health > 0 && playersInfo[i].Health <= 100 && playersInfo[i].lifeState != 0)
				{
					DrawRect((radarPosX + (playerX / RectSize)), (radarPosY + -(playerY / RectSize)), 5, 5, D3DCOLOR_ARGB(255, 0, 0, 255));
					
					if (WorldToScreen(playersInfo[i].Position, PlayerXY))
					{
						DrawESP(playersInfo[i].Health, PlayerXY[0] - m_Rect.left, PlayerXY[1] - m_Rect.top, Get3dDistance(localPlayerStruct.Position, playersInfo[i].Position), D3DCOLOR_XRGB(0, 0, 255));
					}
					
				}
				else if (playersInfo[i].teamNum != vLTeamNum && playersInfo[i].Health > 0 && playersInfo[i].Health <= 100 && playersInfo[i].lifeState != 0)
				{
					DrawRect((radarPosX + (playerX / RectSize)), (radarPosY + -(playerY / RectSize)), 5, 5, D3DCOLOR_ARGB(255, 0, 255, 0));
					if (WorldToScreen(playersInfo[i].Position, EnemyXY))
					{
						std::stringstream os;
						os << playersInfo[i].Health;
						DrawESP(playersInfo[i].Health, EnemyXY[0] - m_Rect.left, EnemyXY[1] - m_Rect.top, Get3dDistance(localPlayerStruct.Position, playersInfo[i].Position), D3DCOLOR_XRGB(255, 0, 0));
						DrawString(EnemyXY[0] - m_Rect.left, EnemyXY[1] - m_Rect.top, 12, D3DCOLOR_XRGB(255, 255, 0), pFont, (LPCSTR)os.str().c_str());
					}
					if (WorldToScreen(playersInfo[i].boneXYZ, BoneXY))
					{
						DrawRect(BoneXY[0], BoneXY[1], 5, 5, D3DCOLOR_XRGB(255, 0, 0));
					}
				}
			}
			

			//std::stringstream Test;
			//Test << playersInfo[i].vis ;

			

			//playersInfo[i].x = 0;
			//playersInfo[i].y = 0;
			//playersInfo[i].Health = 0;
			playersInfo[i].lifeState = 0;
		}
	}
	
	
	//DrawESP(ESPx, ESPy, ESPd, D3DCOLOR_XRGB(255, 255, 255));

	DrawBox((SCREEN_WIDTH / 2) - aimbotSize, (SCREEN_HEIGHT / 2) - aimbotSize, aimbotSize * 2, aimbotSize * 2, D3DCOLOR_XRGB(255, 0, 180));
	
	d3ddev->EndScene();

	d3ddev->Present(NULL, NULL, NULL, NULL);

}

void cleanD3D(void)
{
	d3ddev->Release();    // close and release the 3D device
	d3d->Release();    // close and release Direct3D
}

void DrawString(int x, int y, int FontSize, DWORD color, LPD3DXFONT g_pFont, LPCSTR Message)
{
	D3DXCreateFont(d3ddev, FontSize, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &pFont);

	RECT FontPos = { x, y, x + 120, y + 16 };

	pFont->DrawTextA(NULL, Message, -1, (LPRECT)&FontPos, 0, color);
	pFont->Release();
}

void DrawLine(float Bx, float By, float Ex, float Ey, int A, int R, int G, int B)
{
	LPD3DXLINE line;
	D3DXCreateLine(d3ddev, &line);
	D3DXVECTOR2 lines[] = { D3DXVECTOR2(Bx, By), D3DXVECTOR2(Ex, Ey) };
	
	line->Draw(lines, 2, D3DCOLOR_ARGB(A, R, G, B));
	line->Release();
	
}

void DrawRect(int x, int y, int width, int height, D3DCOLOR color)
{
	D3DRECT Rect = {x, y, x + width, y + height};
	d3ddev->Clear(1, &Rect, D3DCLEAR_TARGET, color, 1.0f, NULL);
}

void DrawESP(double hp,int x, int y, float distance, D3DCOLOR color)
{
	int width = ESPwidth / distance;
	int height = ESPheight / distance;

	x = x - (width / 2);
	y = y - height;

	D3DRECT Rect1 = { x, y, x + width, y + 1 };
	D3DRECT Rect2 = { x, y, x + 1, y+ height };
	D3DRECT Rect3 = { x + width, y, x + width + 1, y + height };
	D3DRECT Rect4 = { x, y + height, x + 1 + width, y + height + 1 };
	D3DRECT Rect5 = { x , y + height + 3, x + width * (hp / 100), y + height + 5 };
	
	d3ddev->Clear(1, &Rect1, D3DCLEAR_TARGET, color, 1.0f, NULL); //Links
	d3ddev->Clear(1, &Rect2, D3DCLEAR_TARGET, color, 1.0f, NULL); //Boven
	d3ddev->Clear(1, &Rect3, D3DCLEAR_TARGET, color, 1.0f, NULL); //Rechts
	d3ddev->Clear(1, &Rect4, D3DCLEAR_TARGET, color, 1.0f, NULL); //onder
	d3ddev->Clear(1, &Rect5, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 255, 0), 1.0f, NULL); //HP
}

void DrawBox(int Bx, int By, int Ex, int Ey, D3DCOLOR color)
{
	D3DRECT Rect1 = { Bx, By, Bx + Ex, By + 1 };
	D3DRECT Rect2 = { Bx, By, Bx + 1, By + Ey };
	D3DRECT Rect3 = { Bx + Ex, By, Bx + Ey + 1, By + Ey };
	D3DRECT Rect4 = { Bx, By + Ey, Bx + 1 + Ex, By + Ey + 1 };

	d3ddev->Clear(1, &Rect1, D3DCLEAR_TARGET, color, 1.0f, NULL);
	d3ddev->Clear(1, &Rect2, D3DCLEAR_TARGET, color, 1.0f, NULL);
	d3ddev->Clear(1, &Rect3, D3DCLEAR_TARGET, color, 1.0f, NULL);
	d3ddev->Clear(1, &Rect4, D3DCLEAR_TARGET, color, 1.0f, NULL);
}

DWORD Module(LPSTR ModuleName){

	//Variables
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcID); //Take A Module Snapshot Of The Process (Grab All Loaded Modules)
	MODULEENTRY32 mEntry; //Declare Module Entry Structure
	mEntry.dwSize = sizeof(mEntry); //Declare Structure Size And Populate It With Loaded Modules
	
	//Scan For Module By Name
	do
	if (!strcmp(mEntry.szModule, ModuleName))
	{
		CloseHandle(hModule);
		return (DWORD)mEntry.modBaseAddr;
	}
	while (Module32Next(hModule, &mEntry));

	std::cout << "\nMODULE: Process Platform Invalid\n";
	return 0;
}

DWORD WINAPI mainThread()
{
	while (TRUE)
	{
		render_frame();
		//Sleep(10);
		
	}
	return 0;
}

