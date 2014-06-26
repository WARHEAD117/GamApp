#include "Input.h"


Input::Input(void)
{
	pDI                  = NULL;
	m_pMouse             = NULL;

	//���ǰһ��״̬
	m_OldMState.left     = false;
	m_OldMState.right    = false;
	m_OldMState.pos.x    = 400;
	m_OldMState.pos.y    = 300;
	//��굱ǰ״̬
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
//Desc  ����DeviceInput
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
		MessageBox(NULL, "����DirectInput����ʧ��",NULL, MB_OK);
		return false;
	}

    //���ĳ�ʼ��
	pDI->CreateDevice( GUID_SysMouse, &m_pMouse, NULL );    //��������豸
	//����������ݸ�ʽ
	m_pMouse->SetDataFormat(&c_dfDIMouse2);
	//��������豸��Э���ȼ�
	m_pMouse->SetCooperativeLevel( hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE );
   
	//�趨��������С
	DIPROPDWORD       property;
	property.diph.dwSize = sizeof(DIPROPDWORD);
	property.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	property.diph.dwObj = 0;
	property.diph.dwHow = DIPH_DEVICE;
	property.dwData = DINPUT_BUFFERSIZE;

	m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &property.diph);
	m_pMouse->Acquire();


	//���̵ĳ�ʼ��
	pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);    //���������豸
	//���ü������ݸ�ʽ
	m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	//���������豸��Э���ȼ�
	m_pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	m_pKeyboard->Acquire();

	/////////////////////////////////////////////////////////////////////////////////////////////

	//Ϊ�������û��巽ʽ

	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DINPUT_BUFFERSIZE; // Arbitary buffer size

	m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

	//Ϊ���̰�װ�¼�֪ͨ��������׼����ȡ�ɼ�
	//m_pKeyboard->SetEventNotification(g_Event);
	m_pKeyboard->Acquire();
	//----------------------------------------------------------------------

	return true;
}

//--------------------------------------------------
//���
//--------------------------------------------------
BOOL Input::ReadMouse()
{
	HRESULT		result;		//��Ϣ���ؾ��
	DIMOUSESTATE2 dims2;
	ShowCursor(TRUE);     //�������
		
    if( m_pMouse == NULL )
	{
		MessageBox(NULL, "����豸����!", NULL, MB_OK);
		return false;
	}
    ZeroMemory( &dims2, sizeof(dims2) );
    result = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2),&dims2 );

  	if( result != DI_OK ) 
    {
		MessageBox(NULL, "������豸״̬ʧ��!", NULL, MB_OK);
		return false;
	}
    result = m_pMouse->Acquire();
    //��ѭ�������ͼ�����ʱ���ܱ�ϵͳ���� 
	while( result == DIERR_INPUTLOST )
	{
		result = m_pMouse->Acquire();			
	}	
	//�������ǰһ״̬
	m_OldMState.left   =  m_CurMState.left;
	m_OldMState.right  =  m_CurMState.right;
	m_OldMState.pos.x  =  m_CurMState.pos.x;
	m_OldMState.pos.y  =  m_CurMState.pos.y;
	
	//������굱ǰλ��
	m_CurMState.pos.x+=  dims2.lX;  
	m_CurMState.pos.y+=  dims2.lY; 
	//������굱ǰ��Ť״̬
	if(dims2.rgbButtons[0] & 0x80)        //����Ƿ���
	{
		m_CurMState.left  = true;
	}
	else
	{
		m_CurMState.left  = false;
	}

	if(dims2.rgbButtons[1] & 0x80)        //�Ҽ��Ƿ���
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
	HRESULT		result;		//��Ϣ���ؾ��

	if (m_pKeyboard == NULL)
	{
		MessageBox(NULL, "�����豸����!", NULL, MB_OK);
		return false;
	}
	ZeroMemory(m_LastFrameKeyboardState, sizeof(m_LastFrameKeyboardState));
	CopyMemory(m_LastFrameKeyboardState, m_KeyboardState, sizeof(m_KeyboardState));
	//ZeroMemory(m_KeyboardState, sizeof(m_KeyboardState));
	result = m_pKeyboard->GetDeviceState(sizeof(m_KeyboardState), (void**)&m_KeyboardState);

	if (result != DI_OK)
	{
		MessageBox(NULL, "���̻���豸״̬ʧ��!", NULL, MB_OK);
		return false;
	}
	result = m_pKeyboard->Acquire();
	//��ѭ�������ͼ�����ʱ���ܱ�ϵͳ���� 
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


