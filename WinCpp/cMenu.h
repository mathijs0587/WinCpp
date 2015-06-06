#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <direct.h>



#pragma warning(disable:4244) //int to float conv. warning
#pragma once




struct Options{
	char *Name;
	char **Opt;
	int    *Function;
	BYTE Type;
	int Max;
	char *DescriptTXT;
	D3DXVECTOR2 Cords;
};


struct _sMenu{
	char *Title;
	int x;
	int y;
	int w;
};

#define T_FOLDER        1
#define T_OPTION        3

#define LineH            25 //Height of one Line

class cMenu
{
public:
	//MENU FUNCTIONS
	cMenu(void);
	//Adding Functions
	void AddFolder(char *Name, char **Opt, int *Pointer, char *Description = "Contains Items", int Max = 2);
	//Add a Item folder to the Menu Struct
	void AddOption(char *Name, char **Opt, int *Pointer, int *Folder, char *Description = "No Description avaible", int Max = 2);
	//Add a Menu Item(Option) to the Menu Struct
	bool Is_Ready();
	//Check for Menu Ready
	void Init_Menu(int x, int y, int w, char *Title);
	//Init the Menu (lol)
	void Draw_Menu(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont);
	//Draw's the Menu 
	void Draw_Hide(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont, char *Text);
	//Draw's a lil bar on the bottom of the Window when the Menu is hidden
	void Draw_FPS(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont, int x, int y);
	//Draws a FPS Box (See screenshot)
	void DrawBackground(LPDIRECT3DDEVICE9 pDevice);
	//To draw the Menu Background

	//Changing Thinks
	void Move_Menu(int x, int y);
	//Move Menu by mouse,LButton to stop
	void SetTitle(char *Title);
	//Change the Title

	//Misc
	bool Reset();
	//Set everything to 0
	bool Is_Visible();
	//Visible Check , returns true if menu visible
	int Return_Height();
	//returns the Menu Height
	void DrawDescBox(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont);
	//Draw the Description box
	void EnableItemBox(bool Enabled);
	//Draw a box around the selected item
	void EnableItemLine(bool Enabled);
	//Draw a line under the selected item
	void Navigation_Keyboard(); //only use one at once
	//Navigation by Keyboard(using arrowkeys)
	void Navigation_Mouse(); //only use one at once
	//Navigation by Mouse,LButton = + | RButton = - 
	D3DXVECTOR4 ReturnXYW();
	//Return menu cords


	//Drawing Functions
	void DrawTextA(int x, int y, DWORD color, char *text, ID3DXFont* pFont);
	void FillRGB(int x, int y, int w, int h, D3DCOLOR color, IDirect3DDevice9* pDevice);
	void DrawBorder(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y, int w, int h);
	void DrawBorderedBox(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Back_Col, D3DCOLOR Border_Col, int x, int y, int w, int h);
	void DrawLine(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y, int w);

	void DrawDot(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y);
	void D3DCursor(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color);
	void DrawExtraBox(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont, int x, int y, int w, int h, char *text, char *title);
	//Draw's a box with Headline & Border

	//Misc
	bool IsMouseInArea(int x, int y, int h, int w);
	//Nothing to say
	int Return_FPS();
	void Save_Items(void *Do_Menu);
	//Save the Menu Items
	void Load_Items(void *Do_Menu);
	//Load the Menu Items
	void Enable_Alpha(bool Enabled);
	//To enable/Disable the Alpha (Trans) for Box and so on
	HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32);
	//Generating Textures, credits i dont remember anymore :D


	//Menu Colors
	D3DCOLOR col_Title;
	D3DCOLOR col_Text;
	D3DCOLOR col_On;
	D3DCOLOR col_Off;
	D3DCOLOR col_Current;
	D3DCOLOR col_Folder;
	D3DCOLOR col_SubFolder;
	D3DCOLOR col_Items;
	D3DCOLOR col_Background;
	D3DCOLOR col_Border;





private:
	Options sOptions[250];
	_sMenu sMenu;
	int Items, Drawed_Items, Cur_Pos, display_items;
	bool vis, enable_ibox, line_cur, alhpa_enabled;
	float FPS_Last;
	int FPS_;
	void DrawRect(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y, int w, int h);
};