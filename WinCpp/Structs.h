#pragma once
#include "WinCpp.h"

typedef struct
{
	float flMatrix[4][4];
}WorldToScreenMatrix_t;

struct PlayersInfo
{
	DWORD playerAddress;

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

		float width = (18100 / distance) * 0.5;
		float height = (48000 / distance) * 0.65;

		AimXY[0] = aim[0];
		AimXY[1] = aim[1] - height;

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
};



PlayersInfo playersInfo[64];
LocalPlayerStruct localPlayerStruct;

int RectX = 50;
int RectY = 50;
float RectSize = 15;

int ESPx = 0;
int ESPy = 0;
float ESPd = 0;

int aimbotSize = 100;

struct CompareTargetEnArray
{
	//USE A COMPARATOR TO SORT OUR ARRAY nicely
	bool operator() (TargetsInfo & lhs, TargetsInfo & rhs)
	{
		return lhs.AimDistance < rhs.AimDistance;
	}
};