#pragma once
#include "CommonUtil\\GlobalHeader.h"
#include "CommonUtil\\D3D9Header.h"

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

typedef struct MouseState
{
	bool   left;   //左键是否按下
	bool   right;  //右键是否按下
	POINT  pos;    //鼠标当前的位置
}MSTATE, *PMSTATE;

#define  DINPUT_BUFFERSIZE 16

class Input
{
public:
	LPDIRECTINPUT8			pDI;
	LPDIRECTINPUTDEVICE8    m_pMouse;
	 
	MSTATE                  m_OldMState;
	MSTATE                  m_CurMState;

	int                     MouseX;
	int                     MouseY;

//=======================================================
// 	//手柄设备声明
// 	// Game controller device.
// 	LPDIRECTINPUTDEVICE8 m_GameControl;
// 	DIJOYSTATE2 m_GC_State;
// 	DIJOYSTATE2 m_oldGC_State;
// 	char m_name[256];
// 	bool m_controllerFound;
// 	unsigned long m_numButtons;
// 
// 	// Left and right stick x and y positions.
// 	long m_xGCPos;
// 	long m_yGCPos;
// 	long m_xGCPos2;
// 	long m_yGCPos2;
//=======================================================

public:
	Input(void);
	~Input(void);
	BOOL       InitInput(HWND hWnd, HINSTANCE  hInstance);  //初始化
	BOOL       ReadMouse();									//读取鼠标函数
//	BOOL       ReadKeyboard();								//读取键盘函数（没用）

//==//==============================================================================================//
//	// Used to create the game controllers.//用于创建手柄（手柄是枚举的，必须要用回调函数）
//	BOOL EnumDeviceCallBack(const DIDEVICEINSTANCE *inst, void* pData);
//
//	// Game controller functions.
//	int ControllerButtonUp(unsigned int button);			//手柄按键松开
//	int ControllerButtonDown(unsigned int button);			//手柄按键按下
//
//	// Get controller main (left) and right stick position.
//	POINT GetLeftStickPos();								//左摇杆坐标
//	POINT GetRightStickPos();								//右摇杆坐标
//==================================================================================================//
	void       Clearup();
};

typedef CSingleton<Input> GAMEINPUT;