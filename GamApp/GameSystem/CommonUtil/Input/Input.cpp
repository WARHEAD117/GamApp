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
//���
//--------------------------------------------------
BOOL Input::ReadMouse()
{
	HRESULT		result;		//��Ϣ���ؾ��
	DIMOUSESTATE2 dims2;
	ShowCursor(FALSE);     //�������
		
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

