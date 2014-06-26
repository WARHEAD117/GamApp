#include "Input.h"


Input::Input(void)
{
	pDI                  = NULL;
	m_pMouse             = NULL;

	//鼠标前一个状态
	m_OldMState.left     = false;
	m_OldMState.right    = false;
	m_OldMState.pos.x    = 400;
	m_OldMState.pos.y    = 300;
	//鼠标当前状态
	m_CurMState.left     = false;
	m_CurMState.right    = false;
	m_CurMState.pos.x    = 400;
	m_CurMState.pos.y    = 300;
}

Input::~Input(void)
{
	Clearup();
}

//-------------------------------------------------
//Name  InitDInput
//Desc  创建DeviceInput
//-------------------------------------------------
BOOL Input::InitInput( HWND  hWnd, HINSTANCE  hInstance )
{
	HRESULT           result;   
   
	result  = DirectInput8Create( hInstance,
		                       DIRECTINPUT_VERSION,
		                       IID_IDirectInput8,
							     (LPVOID*)&pDI, 
								  NULL);
    if(result != DI_OK)
	{
		MessageBox(NULL, "建立DirectInput对象失败",NULL, MB_OK);
		return false;
	}

    //鼠标的初始化
	pDI->CreateDevice( GUID_SysMouse, &m_pMouse, NULL );    //创建鼠标设备
	//设置鼠标数据格式
	m_pMouse->SetDataFormat(&c_dfDIMouse2);
	//创建鼠标设备的协作等级
	m_pMouse->SetCooperativeLevel( hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE );
   
	//设定缓冲区大小
	DIPROPDWORD       property;
	property.diph.dwSize = sizeof(DIPROPDWORD);
	property.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	property.diph.dwObj = 0;
	property.diph.dwHow = DIPH_DEVICE;
	property.dwData = DINPUT_BUFFERSIZE;

	m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &property.diph);
	m_pMouse->Acquire();


	//键盘的初始化
	pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);    //创建键盘设备
	//设置键盘数据格式
	m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	//创建键盘设备的协作等级
	m_pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	m_pKeyboard->Acquire();

	/////////////////////////////////////////////////////////////////////////////////////////////

	//为键盘设置缓冲方式

	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DINPUT_BUFFERSIZE; // Arbitary buffer size

	m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

	//为键盘安装事件通知关联，并准备获取采集
	//m_pKeyboard->SetEventNotification(g_Event);
	m_pKeyboard->Acquire();
	//----------------------------------------------------------------------

	return true;
}

//--------------------------------------------------
//鼠标
//--------------------------------------------------
BOOL Input::ReadMouse()
{
	HRESULT		result;		//信息返回句柄
	DIMOUSESTATE2 dims2;
	ShowCursor(TRUE);     //隐藏鼠标
		
    if( m_pMouse == NULL )
	{
		MessageBox(NULL, "鼠标设备出错!", NULL, MB_OK);
		return false;
	}
    ZeroMemory( &dims2, sizeof(dims2) );
    result = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2),&dims2 );

  	if( result != DI_OK ) 
    {
		MessageBox(NULL, "鼠标获得设备状态失败!", NULL, MB_OK);
		return false;
	}
    result = m_pMouse->Acquire();
    //死循环，鼠标和键盘随时可能被系统夺走 
	while( result == DIERR_INPUTLOST )
	{
		result = m_pMouse->Acquire();			
	}	
	//设置鼠标前一状态
	m_OldMState.left   =  m_CurMState.left;
	m_OldMState.right  =  m_CurMState.right;
	m_OldMState.pos.x  =  m_CurMState.pos.x;
	m_OldMState.pos.y  =  m_CurMState.pos.y;
	
	//设置鼠标当前位置
	m_CurMState.pos.x+=  dims2.lX;  
	m_CurMState.pos.y+=  dims2.lY; 
	//设置鼠标当前按扭状态
	if(dims2.rgbButtons[0] & 0x80)        //左键是否按下
	{
		m_CurMState.left  = true;
	}
	else
	{
		m_CurMState.left  = false;
	}

	if(dims2.rgbButtons[1] & 0x80)        //右键是否按下
	{
		m_CurMState.right  = true;
	}
	else
	{
		m_CurMState.right  = false;
	}

	return true;
}

BOOL Input::ReadKeyboard()
{
	HRESULT		result;		//信息返回句柄

	if (m_pKeyboard == NULL)
	{
		MessageBox(NULL, "键盘设备出错!", NULL, MB_OK);
		return false;
	}
	ZeroMemory(m_LastFrameKeyboardState, sizeof(m_LastFrameKeyboardState));
	CopyMemory(m_LastFrameKeyboardState, m_KeyboardState, sizeof(m_KeyboardState));
	//ZeroMemory(m_KeyboardState, sizeof(m_KeyboardState));
	result = m_pKeyboard->GetDeviceState(sizeof(m_KeyboardState), (void**)&m_KeyboardState);

	if (result != DI_OK)
	{
		MessageBox(NULL, "键盘获得设备状态失败!", NULL, MB_OK);
		return false;
	}
	result = m_pKeyboard->Acquire();
	//死循环，鼠标和键盘随时可能被系统夺走 
	while (result == DIERR_INPUTLOST)
	{
		result = m_pKeyboard->Acquire();
	}

	return true;
}

//-------------------------------------------
void Input::Clearup()
{
	SafeRelease(m_pMouse);

	SafeRelease(pDI);	
}

bool Input::KeyUp(int keyCode)
{
	return (m_KeyboardState[keyCode] & 0x80) == 0;
}

bool Input::KeyDown(int keyCode)
{
	return (m_KeyboardState[keyCode] & 0x80) != 0;
}

bool Input::KeyPressed(int keyCode)
{
	bool LastFrameUp = (m_LastFrameKeyboardState[keyCode] & 0x80) == 0;
	bool CurFrameDown = (m_KeyboardState[keyCode] & 0x80) != 0;
	return LastFrameUp & CurFrameDown;
}

bool Input::KeyReleased(int keyCode)
{	
	bool LastFramePressed = (m_LastFrameKeyboardState[keyCode] & 0x80) != 0;
	bool CurFrameReleased = (m_KeyboardState[keyCode] & 0x80) == 0;
	return LastFramePressed & CurFrameReleased;
}

bool Input::UpdateInputState()
{
	bool mouseState, keyBoardState;
	keyBoardState = ReadKeyboard();
	mouseState = ReadMouse();
	return keyBoardState & mouseState;
}

D3DXVECTOR2 Input::GetMouseMove()
{
	float dx = m_CurMState.pos.x - m_OldMState.pos.x;
	float dy = m_CurMState.pos.y - m_OldMState.pos.y;
	D3DXVECTOR2 move = D3DXVECTOR2(dx, dy);
	return move;
}


