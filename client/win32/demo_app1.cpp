// demo_app1.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "demo_app1.h"
#include "mainapp.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/win32socketserver.h"

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
CMainApp	APP;

#define Char2WChar(lpMultiByteStr,cbMultiByte,lpWideCharStr,cbWideChar) \
	MultiByteToWideChar(CP_ACP,0,lpMultiByteStr,cbMultiByte,lpWideCharStr,cbWideChar)
#define WChar2Char(lpWideCharStr,cbWideChar,lpMultiByteStr,cbMultiByte) \
	WideCharToMultiByte(CP_ACP,0,lpWideCharStr,cbWideChar,lpMultiByteStr,cbMultiByte,NULL,NULL)

INT_PTR CALLBACK    AppProcess(HWND, UINT, WPARAM, LPARAM);
typedef INT_PTR __stdcall CBPTR(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;
	HWND hdlg = CreateDialog(hInstance,
		MAKEINTRESOURCE(IDD_MAINDLG),
		GetDesktopWindow(),
		AppProcess
	);

	if (!hdlg)
	{
		return 0;
	}

	ShowWindow(hdlg, SW_SHOW);
	UpdateWindow(hdlg);

	//allocate console
	AllocConsole();
	freopen("conout$", "w", stdout);
	freopen("conerr$", "w", stderr);

	//init app
	rtc::EnsureWinsockInit();
	rtc::Win32Thread w32_thread;
	rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);
	APP.Init();

	//msg dispatch
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


INT_PTR CALLBACK AppProcess(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG: 
	{		
		//TODO set item default text
		wchar_t wturn[] = _T("192.168.8.254");
		wchar_t wtport[] = _T("3100");
		wchar_t wsig[] = _T("192.168.8.254");
		wchar_t wsport[] = _T("3102");
		SendMessage(GetDlgItem(hDlg, IDC_SIGNALING_SERVER), WM_SETTEXT, 32, (LPARAM)(wsig));
		SendMessage(GetDlgItem(hDlg, IDC_SIGNALING_PORT), WM_SETTEXT, 32, (LPARAM)(wsport));
		SendMessage(GetDlgItem(hDlg, IDC_TURN_SERVER), WM_SETTEXT, 32, (LPARAM)(wturn));
		SendMessage(GetDlgItem(hDlg, IDC_TURN_PORT), WM_SETTEXT, 32, (LPARAM)(wtport));
		//set view
		APP.setRemoteView(GetDlgItem(hDlg, IDC_REMOTE_1));
		APP.setLocalView(GetDlgItem(hDlg, IDC_LOCAL_VIEW));

		return (INT_PTR)TRUE;
	}
	case WM_CREATE:
	{

		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
	{
		int32_t ret = 0;
		switch (LOWORD(wParam))
		{
		case IDC_BTN_CONNECT:
		{
			//connect to server
			HWND hTurn, hSig, hTPort, hSPort;
			hTurn = GetDlgItem(hDlg, IDC_TURN_SERVER);
			hTPort = GetDlgItem(hDlg, IDC_TURN_PORT);
			hSig = GetDlgItem(hDlg, IDC_SIGNALING_SERVER);
			hSPort = GetDlgItem(hDlg, IDC_SIGNALING_PORT);

			wchar_t wturn[32], wsig[32], wtport[16], wsport[16];
			SendMessage(hTurn, WM_GETTEXT, 32, (LPARAM)wturn);
			SendMessage(hTPort, WM_GETTEXT, 32, (LPARAM)wtport);
			SendMessage(hSig, WM_GETTEXT, 32, (LPARAM)wsig);
			SendMessage(hSPort, WM_GETTEXT, 32, (LPARAM)wsport);

			char turn[32], sig[32], tport[16], sport[16];
			WChar2Char(wturn, -1, turn, 32);
			WChar2Char(wsig, -1, sig, 32);
			WChar2Char(wtport, -1, tport, 16);
			WChar2Char(wsport, -1, sport, 16);
			ret = APP.ConnectToServer(sig, atoi(sport), turn, atoi(tport));
			break;
		}
		case IDC_BTN_CLOSE:
			//disconnect to server
			APP.DisConnect();
			break;
		case IDC_BTN_REG:
		{
			HWND hReg = GetDlgItem(hDlg, IDC_USER_ID);
			wchar_t wreg[32];
			char reg[32];

			SendMessage(hReg, WM_GETTEXT, 32, (LPARAM)wreg);
			WChar2Char(wreg, -1, reg, 32);

			ret = APP.Register(reg);
			break;
		}
		case IDC_BTN_UNREG:
			ret = APP.UnRegister();
			break;
		case IDC_BTN_CALL:
		{
			HWND hDst, hCallid;
			wchar_t wdst[16], wcallid[16];
			char dst[16], callid[16];

			hDst = GetDlgItem(hDlg, IDC_DSTID);
			hCallid = GetDlgItem(hDlg, IDC_CALL_ID);
			SendMessage(hDst, WM_GETTEXT, 16, (LPARAM)wdst);
			SendMessage(hCallid, WM_GETTEXT, 16, (LPARAM)wcallid);
			WChar2Char(wdst, -1, dst, 16);
			WChar2Char(wcallid, -1, callid, 16);

			ret = APP.Offer(dst, callid);
			break;
		}
		case IDC_BTN_ANSWER:
			ret = APP.Answer();
			break;
		case IDC_BTN_REJECT:
			ret = APP.Reject();
			break;
		case IDC_BTN_CANCLE:
			ret = APP.Bye();
			break;
		case IDC_BTN_LOCALOPEN:
			APP.setLocalView(::GetDlgItem(hDlg, IDC_LOCAL_VIEW));
			::EnableWindow(::GetDlgItem(hDlg, IDC_BTN_LOCALOPEN), FALSE);
			::EnableWindow(::GetDlgItem(hDlg, IDC_BTN_LOCALCLOSE), TRUE);
			break;
		case IDC_BTN_LOCALCLOSE:
			::EnableWindow(::GetDlgItem(hDlg, IDC_BTN_LOCALOPEN), TRUE);
			::EnableWindow(::GetDlgItem(hDlg, IDC_BTN_LOCALCLOSE), FALSE);
			break;
		default:
			//error
			DefWindowProc(hDlg, message, wParam, lParam);
		}
		return (INT_PTR)TRUE;
	}
	case WM_NEW_PACKAGE:
	{
		char buff[4 * 1024];
		memset(buff, 0, 4 * 1024);
		memcpy(buff, (char*)(lParam), size_t(wParam));
		::SendMessage(::GetDlgItem(hDlg, IDC_USER_ID), WM_SETTEXT, 32, (LPARAM)buff);
		break;
	}
	case WM_DESTROY:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
	default:
		DefWindowProc(hDlg, message, wParam, lParam);
		return (INT_PTR)FALSE;
	}

	return (INT_PTR)TRUE;
}