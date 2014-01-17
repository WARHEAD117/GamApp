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
	property.diph.dwSize  = sizeof(DIPROPDWORD);
    property.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    property.diph.dwObj = 0;
	property.diph.dwHow  = DIPH_DEVICE;
    property.dwData = DINPUT_BUFFERSIZE;

	m_pMouse->SetProperty( DIPROP_BUFFERSIZE, &property.diph );
    m_pMouse->Acquire();
	
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
	ShowCursor(FALSE);     //隐藏鼠标
		
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

//-------------------------------------------
void Input::Clearup()
{
	if(m_pMouse)
	{
		TRelease(m_pMouse);
		m_pMouse = NULL;
	}
	if(pDI)
	{
		TRelease(pDI);
		pDI = NULL;
	}
	
}

