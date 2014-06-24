#pragma once
#include "CommonUtil\GlobalHeader.h"
#include "MainGame\MainGame.h"
#include "D3D9Device.h"
#include "CommonUtil\Input\Input.h"

LRESULT MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static bool minOrMaxed = false;

	RECT clientRect = {0, 0, 0, 0};

    switch(msg)
    {
        case WM_LBUTTONDOWN:
//		        ctl.GetInstance()->LButtondown();
		    break;
	    case WM_RBUTTONDOWN:
//		        ctl.GetInstance()->RButtondown();
		    break;
		case WM_RBUTTONUP:
//		    ctl.GetInstance()->RButtonup();
		    break;

		case WM_KEYDOWN:
			return 0;

        case WM_ACTIVATE:
            return 0;
                        
            
        case WM_SIZE:
		    return 0;	
		
		
        case WM_EXITSIZEMOVE:
		    return 0;
		    
        // WM_CLOSE is sent when the user presses the 'X' button in the
        // caption bar menu.
        case WM_CLOSE:
			DestroyWindow(hwnd);
            return 0;

        // WM_DESTROY is sent when the window is being destroyed.
        case WM_DESTROY:
            PostQuitMessage(0);
        return 0;
  
    }
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return MessageProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF |
		_CRTDBG_LEAK_CHECK_DF);
#endif

	//TODO
	//Read basic game config file
	int wndWidth = 1024;
	int wndHeght = 786;
	std::string wndName = "New Game";
	LPCSTR className = "New Game";

	//Create window
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = className;
	
	//Register window
	if (!RegisterClass(&wc))
	{
		MessageBox(0, "RegisterClass FAILED", 0, 0);
		PostQuitMessage(0);
	}

	// Default to a window with a client area read from config file
	RECT wndRect = { 0, 0, wndWidth, wndHeght };
	AdjustWindowRect(&wndRect, WS_OVERLAPPEDWINDOW, false);
	HWND hWnd = CreateWindow(className, wndName.c_str(),
		WS_OVERLAPPEDWINDOW, 50, 50, wndRect.right, wndRect.bottom,
		0, 0, hInstance, 0);

	if (!hWnd)
	{
		MessageBox(0, "Create Window FAILED", 0, 0);
		PostQuitMessage(0);
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	//InitRenderDevice and Input
	RENDERDEVICE::Instance().InitD3DDevice(hWnd);
	GAMEINPUT::Instance().InitInput(hWnd, hInstance);

	//Main Game Loop
	MainGame mainGame;
	mainGame.GameStart();
	mainGame.GameLoad();

	MSG msg;
	msg.message = WM_NULL;
	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			// If the application is paused then free some CPU
			// cycles to other applications and then continue on
			// to the next frame.
			//if (mAppPaused)
			//{
			//	Sleep(20);
			//	continue;
			//}

			//CheckDevice
			//if (!isDeviceLost())
			{
				GAMEINPUT::Instance().UpdateInputState();
				mainGame.GameLoop();
			}
		}
	}
	mainGame.GameEnd();
	return (int)msg.wParam;
}