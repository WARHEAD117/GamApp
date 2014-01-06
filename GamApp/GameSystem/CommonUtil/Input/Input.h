#pragma once
#include "CommonUtil\\GlobalHeader.h"
#include "CommonUtil\\D3D9Header.h"

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

typedef struct MouseState
{
	bool   left;   //����Ƿ���
	bool   right;  //�Ҽ��Ƿ���
	POINT  pos;    //��굱ǰ��λ��
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
// 	//�ֱ��豸����
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
	BOOL       InitInput(HWND hWnd, HINSTANCE  hInstance);  //��ʼ��
	BOOL       ReadMouse();									//��ȡ��꺯��
//	BOOL       ReadKeyboard();								//��ȡ���̺�����û�ã�

//==//==============================================================================================//
//	// Used to create the game controllers.//���ڴ����ֱ����ֱ���ö�ٵģ�����Ҫ�ûص�������
//	BOOL EnumDeviceCallBack(const DIDEVICEINSTANCE *inst, void* pData);
//
//	// Game controller functions.
//	int ControllerButtonUp(unsigned int button);			//�ֱ������ɿ�
//	int ControllerButtonDown(unsigned int button);			//�ֱ���������
//
//	// Get controller main (left) and right stick position.
//	POINT GetLeftStickPos();								//��ҡ������
//	POINT GetRightStickPos();								//��ҡ������
//==================================================================================================//
	void       Clearup();
};

typedef CSingleton<Input> GAMEINPUT;