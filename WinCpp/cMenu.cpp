#include "cMenu.h"

#include "stdafx.h"
#include <d3d9.h>
#include <d3dx9.h>

#include <iostream>

cMenu::cMenu(void)
{
	this->vis = true;
}

void cMenu::AddFolder(char *Name, char **Opt, int *Pointer, char *Description, int Max)
{
	sOptions[this->Items].Name = Name;
	sOptions[this->Items].Opt = Opt;
	sOptions[this->Items].Function = Pointer;
	sOptions[this->Items].Type = T_FOLDER;
	sOptions[this->Items].Max = Max;
	sOptions[this->Items].DescriptTXT = Description;
	this->Items++;
}

void cMenu::AddOption(char *Name, char **Opt, int *Pointer, int *Folder, char *Description, int Max)
{
	if (*Folder == 0)
		return;
	sOptions[this->Items].Name = Name;
	sOptions[this->Items].Opt = Opt;
	sOptions[this->Items].Function = Pointer;
	sOptions[this->Items].Type = T_OPTION;
	sOptions[this->Items].Max = Max;
	sOptions[this->Items].DescriptTXT = Description;
	this->Items++;
}

bool cMenu::Is_Ready()
{
	if (this->Items == 0 || (!this->sMenu.Title))
		return false;
	return true;
}

void cMenu::Init_Menu(int x, int y, int w, char *Title)
{
	this->sMenu.Title = Title;
	this->sMenu.x = x;
	this->sMenu.y = y;
	this->sMenu.w = w;
	return;
}

void cMenu::Move_Menu(int x, int y)
{
	if ((!x) && (!y))
		return;
	this->sMenu.x = (int)x;
	this->sMenu.y = (int)y;
}

bool cMenu::Reset()
{
	for (int i = 0; i < this->Items; i++)
		*sOptions[i].Function = 0;
	this->Items = 0;
	this->Cur_Pos = 0;
	memset(&sOptions, 0, sizeof(sOptions));
	memset(&sMenu, 0, sizeof(sMenu));
	return true;
}

void cMenu::SetTitle(char *Title)
{
	this->sMenu.Title = Title;
}

void cMenu::DrawTextA(int x, int y, DWORD color, char *text, ID3DXFont* pFont)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, color);
}

void cMenu::DrawRect(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y, int w, int h)
{
	struct Vertex
	{
		float x, y, z, ht;
		DWORD Color;
	};
	DWORD ColorBG = 0;
	Vertex V[8];

	V[0].Color = V[1].Color = V[2].Color = V[3].Color = Color;
	V[0].z = V[1].z = V[2].z = V[3].z = 0.0f;
	V[0].ht = V[1].ht = V[2].ht = V[3].ht = 0.0f;

	V[0].x = V[1].x = (float)x;
	V[0].y = V[2].y = (float)(y + h);
	V[1].y = V[3].y = (float)y;
	V[2].x = V[3].x = (float)(x + w);

	pDevice->SetTexture(0, NULL);
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
}

void cMenu::FillRGB(int x, int y, int w, int h, D3DCOLOR color, IDirect3DDevice9* pDevice)
{
	if (this->alhpa_enabled == false)
	{
		D3DRECT rec = { x, y, x + w, y + h };
		pDevice->Clear(1, &rec, D3DCLEAR_TARGET, color, 0, 0);
	}
	else{
		this->DrawRect(pDevice, color, x, y, w, h);
	}
}

void cMenu::DrawBorder(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y, int w, int h)
{
	this->FillRGB(x, y, w, 2, Color, pDevice);
	this->FillRGB(x, y, 2, h, Color, pDevice);
	this->FillRGB((x + w), y, 2, h, Color, pDevice);
	this->FillRGB(x, (y + h), (w + 2), 2, Color, pDevice);
}

void cMenu::DrawBorderedBox(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Back_Col, D3DCOLOR Border_Col, int x, int y, int w, int h)
{
	this->FillRGB(x, y, w, h, Back_Col, pDevice);
	this->DrawBorder(pDevice, Border_Col, x, y, w, h);
}

void cMenu::DrawLine(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y, int w)
{
	this->FillRGB(x, y, w, 2, Color, pDevice);
}

void cMenu::DrawDot(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y)
{
	this->FillRGB(x, y, 1, 1, Color, pDevice);
}

void cMenu::D3DCursor(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color)
{
	POINT Cursor_Pos;
	if (GetCursorPos(&Cursor_Pos))
	{
		int x = Cursor_Pos.x;
		int y = Cursor_Pos.y;
#define DrawD(x,y) this->DrawDot(pDevice,Color,x,y)
		this->FillRGB(x, y, 1, 1, Color, pDevice);
		for (int i = 0; i < 10; i++)
		{
			this->FillRGB(x - i, y + i, 1 + (i * 2), 1, Color, pDevice);
		}
		this->FillRGB(x - 1, y + 11, 3, 3, Color, pDevice);
	}
}

bool cMenu::Is_Visible()
{
	if (vis == true)
		return true;
	return false;
}

void cMenu::Navigation_Keyboard()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		this->vis = !this->vis;

	if (!this->Is_Visible())
		return;

	int value = 0;

	if (GetAsyncKeyState(VK_DOWN) & 1)
	{
		this->Cur_Pos++;
		if (sOptions[this->Cur_Pos].Name == 0)
			this->Cur_Pos--;
	}

	if (GetAsyncKeyState(VK_UP) & 1)
	{
		this->Cur_Pos--;
		if (this->Cur_Pos == -1)
			this->Cur_Pos++;
	}

	else if (GetAsyncKeyState(VK_RIGHT) & 1)
	{
		if (*sOptions[this->Cur_Pos].Function < sOptions[this->Cur_Pos].Max - 1)
			value++;
	}

	else if ((GetAsyncKeyState(VK_LEFT) & 1) && *sOptions[this->Cur_Pos].Function >0)
	{
		value--;
	}


	if (value){
		*sOptions[this->Cur_Pos].Function += value;
		if (sOptions[this->Cur_Pos].Type == T_FOLDER)
		{
			memset(&sOptions, 0, sizeof(sOptions));
			this->Items = 0;
		}
	}


}

void cMenu::Navigation_Mouse()
{
	POINT Mouse_Pos;
	int value = NULL;
	if (GetAsyncKeyState(VK_INSERT) & 1)
		this->vis = !this->vis;
	if (!this->Is_Visible())
		return;

	if (GetCursorPos(&Mouse_Pos))
	{
		for (int i = 0; i < this->Items; i++)
		{
			if (this->IsMouseInArea(this->sOptions[i].Cords.x, this->sOptions[i].Cords.y, LineH, this->sMenu.w))
			{
				this->Cur_Pos = i;
				if (GetAsyncKeyState(VK_LBUTTON) & 1)
				{
					if (*this->sOptions[i].Function < this->sOptions[i].Max - 1)
						value = 1;
					else
						value = 0;
				}
				if (GetAsyncKeyState(VK_RBUTTON) & 1)
				{
					if (*this->sOptions[i].Function > 0)
						value = -1;
					else
						value = 0;
				}
			}

		}
	}
	if (value){
		*sOptions[this->Cur_Pos].Function += value;
		if (sOptions[this->Cur_Pos].Type == T_FOLDER)
		{
			memset(&sOptions, 0, sizeof(sOptions));
			this->Items = 0;
		}
	}

}


void cMenu::DrawExtraBox(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont, int x, int y, int w, int h, char *text, char *title)
{
	this->DrawBorderedBox(pDevice, this->col_Background, this->col_Border, x, y, w, h);
	this->DrawTextA(x + 5, y + 5, this->col_Title, title, pFont);
	this->FillRGB(x, y + 30, w, 2, this->col_Border, pDevice);
	this->DrawTextA(x + 5, y + 37, this->col_Text, text, pFont);
	return;
}

void cMenu::Draw_Menu(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont)
{
	if (!this->Is_Visible())
		return;
	this->Drawed_Items = 0;
	DWORD Cur_Color = NULL, Cur_Color2 = NULL;
	this->sMenu.x = 80;
	this->DrawTextA(sMenu.x + 25, sMenu.y, this->col_Title, sMenu.Title, pFont);
	for (int index = 0; index < this->Items; index++)
	{
		if (sOptions[index].Type == T_FOLDER)
			Cur_Color = this->col_Folder;
		else if (sOptions[index].Type == T_OPTION)
			Cur_Color = this->col_Items;


		if (*sOptions[index].Function == 0)
			Cur_Color2 = this->col_Off;
		else
			Cur_Color2 = this->col_On;
		if (index == this->Cur_Pos)
		{
			DWORD Off_On = Cur_Color2;
			Cur_Color2 = this->col_Current;
			if (this->enable_ibox == true)
				this->DrawBorder(pDevice, Cur_Color2, sMenu.x - 10, (sMenu.y + LineH * (index + 1) - 2), sMenu.w + 35, LineH + 2);
			if (this->line_cur == true)
				this->FillRGB(sMenu.x - 3, (sMenu.y + LineH * (index + 1)) + LineH - 5, sMenu.w + 20, 2, Cur_Color2, pDevice);


			this->DrawBorderedBox(pDevice, this->col_Background, this->col_Border, sMenu.x + sMenu.w + 30, sMenu.y + LineH*index, 100, 60);
			this->DrawTextA(sMenu.x + sMenu.w + 35, sMenu.y + LineH*index + 4, this->col_Text, "Value:", pFont);
			this->FillRGB(sMenu.x + sMenu.w + 30, sMenu.y + LineH*index + 30, 100, 2, this->col_Border, pDevice);
			this->DrawTextA(sMenu.x + sMenu.w + 40, (sMenu.y + LineH * (index + 1)) + 10, Off_On, sOptions[index].Opt[*sOptions[index].Function], pFont);
		}

		this->sOptions[index].Cords.x = sMenu.x;
		this->sOptions[index].Cords.y = sMenu.y + LineH * (index + 1);


		this->DrawTextA(sMenu.x + 10, (sMenu.y + LineH * (index + 1)), Cur_Color, sOptions[index].Name, pFont);
		this->FillRGB(sMenu.x, (sMenu.y + LineH * (index + 1)) + 5, 8, 8, Cur_Color2, pDevice);

		this->Drawed_Items++;
	}

}

void cMenu::Save_Items(void *Do_Menu)
{
	this->Cur_Pos = 0;
	for (int i = 0; i < this->Items; i++)
	{
		if (sOptions[i].Type == T_FOLDER)
		{
			*sOptions[i].Function = 1;
		}
	}
	char Pfad[255];
	_getcwd(Pfad, 255);
	strcat_s(Pfad, "\\Save.ini");
	this->Items = 0;
	__asm call[Do_Menu];
	for (int i = 0; i < this->Items; i++)
	{
		if (sOptions[i].Type != T_FOLDER)
		{
			int Value = *sOptions[i].Function;
			char TMP[25];
			sprintf_s(TMP, 25, "%i", Value);
			WritePrivateProfileStringA("Save_File", sOptions[i].Name, TMP, Pfad);
		}
	}
}

void cMenu::Load_Items(void *Do_Menu)
{
	this->Cur_Pos = 0;
	for (int i = 0; i < this->Items; i++)
	{
		if (sOptions[i].Type == T_FOLDER)
		{
			*sOptions[i].Function = 1;
		}
	}
	this->Items = 0;
	__asm call[Do_Menu];
	char Pfad[255];
	_getcwd(Pfad, 255);
	strcat_s(Pfad, "\\Save.ini");
	for (int i = 0; i < this->Items; i++)
	{
		if (sOptions[i].Type != T_FOLDER)
		{
			int val = GetPrivateProfileIntA("Save_File", sOptions[i].Name, -255, Pfad);
			if (val != -255)
				*sOptions[i].Function = val;
		}
	}

}

void cMenu::Enable_Alpha(bool Enabled)
{
	this->alhpa_enabled = Enabled;
}

int OldFPS = 0;
int cMenu::Return_FPS()
{
	float FPS;

	FPS = clock() * 0.001f;
	FPS_++;

	if ((FPS - FPS_Last) > 1.0f) {
		FPS_Last = FPS;
		OldFPS = FPS_;
		FPS_ = 0;
		return OldFPS;
	}
	return OldFPS;
}

char TMP[25] = "FPS: x |";
void cMenu::Draw_Hide(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont, char *Text)
{
	if (this->Is_Visible())
		return;
	D3DVIEWPORT9 pViewport;
	pDevice->GetViewport(&pViewport);
	int y = (pViewport.Height - 30);
	int x = (pViewport.X);
	this->FillRGB(x, y, pViewport.Width, 30, this->col_Background, pDevice);
	this->DrawBorder(pDevice, this->col_Border, x, y, pViewport.Width, 30);
	this->DrawTextA(x + 105, y + 5, this->col_Text, Text, pFont);

	sprintf_s(TMP, 25, "FPS: %i |", this->Return_FPS());
	this->DrawTextA(x + 5, y + 5, this->col_Text, TMP, pFont);

}

void cMenu::Draw_FPS(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont, int x, int y)
{
	D3DCOLOR Cur_Col = NULL;
	char TEMP_FPS[25];
	int FPS2 = NULL;
	FPS2 = this->Return_FPS();
	sprintf_s(TEMP_FPS, 25, "FPS:%i", FPS2);
	this->FillRGB(x, y, 75, 200, D3DCOLOR_ARGB(255, 0, 0, 0), pDevice);
	if (FPS2 >= 100)
	{
		Cur_Col = D3DCOLOR_ARGB(255, 0, 255, 0);
		this->FillRGB(x, y, 75, 199, Cur_Col, pDevice);
	}
	else{
		Cur_Col = D3DCOLOR_ARGB(255, 0, 255, 0);
		if (FPS2 < 50)
			Cur_Col = D3DCOLOR_ARGB(255, 255, 255, 000);
		if (FPS2 < 30)
			Cur_Col = D3DCOLOR_ARGB(255, 255, 0, 0);
		this->FillRGB(x, y + 200 - FPS2 * 2, 75, FPS2 * 2, Cur_Col, pDevice);
	}
	this->DrawTextA(x + 1, y + 5, D3DCOLOR_ARGB(255, 255, 255, 255), TEMP_FPS, pFont);
	this->DrawBorder(pDevice, Cur_Col, x + 1, y + 1, 73, 198);
}

void cMenu::DrawBackground(LPDIRECT3DDEVICE9 pDevice)
{
	if (!this->Is_Ready() || !this->Is_Visible())
		return;

	D3DXVECTOR4 Cords = this->ReturnXYW();
	Cords.z = this->Return_Height();
	this->FillRGB(Cords.x - 20, Cords.y - 10, Cords.w + 40, Cords.z + 20, this->col_Background, pDevice);
	this->FillRGB(Cords.x - 20, Cords.y + LineH - 5, Cords.w + 40, 2, this->col_Border, pDevice);
	this->DrawBorder(pDevice, this->col_Border, Cords.x - 20, Cords.y - 10, Cords.w + 40, Cords.z + 20);
}

void cMenu::DrawDescBox(LPDIRECT3DDEVICE9 pDevice, ID3DXFont* pFont)
{
	if (!this->Is_Visible())
		return;
	this->DrawExtraBox(pDevice, pFont, sMenu.x + sMenu.w + 140, sMenu.y + LineH*this->Cur_Pos, 350, 60, this->sOptions[this->Cur_Pos].DescriptTXT, "Description:");
}

void cMenu::EnableItemBox(bool Enabled)
{
	this->enable_ibox = Enabled;
	this->line_cur = !Enabled;
}

void cMenu::EnableItemLine(bool Enabled)
{
	this->enable_ibox = !Enabled;
	this->line_cur = Enabled;
}


bool cMenu::IsMouseInArea(int x, int y, int h, int w)
{
	POINT m_pos;
	if (GetCursorPos(&m_pos))
	{

		if ((m_pos.x >= x) && (m_pos.x <= (x + w)))
		if (m_pos.y >= y && m_pos.y <= (y + h))
			return true;
	}
	return false;
}

int cMenu::Return_Height()
{
	return (this->Drawed_Items + 1) * LineH;
}

D3DXVECTOR4 cMenu::ReturnXYW()
{
	D3DXVECTOR4 XYW;
	XYW.x = this->sMenu.x;
	XYW.y = this->sMenu.y;
	XYW.w = this->sMenu.w;
	return XYW;
}



//Credits to the Guy who made it, i think some1 @uc-forums
HRESULT cMenu::GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32)
{
	if (FAILED(pD3Ddev->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL)))
		return E_FAIL;

	WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12)
		| (WORD)(((colour32 >> 20) & 0xF) << 8)
		| (WORD)(((colour32 >> 12) & 0xF) << 4)
		| (WORD)(((colour32 >> 4) & 0xF) << 0);

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	WORD *pDst16 = (WORD*)d3dlr.pBits;

	for (int xy = 0; xy < 8 * 8; xy++)
		*pDst16++ = colour16;

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}