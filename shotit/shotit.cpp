// CuteUITemplate.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "shotit.h"
#include "cuteui.h"
#include "SnowWindow.h"

#include <base/string/stringprintf.h>

#include <common/strconv.cpp>

#ifdef _DEBUG
//#include <vld.h>
#endif

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	// ��ʼ��
	global::Init(hInstance);
	view::frame::CMainWindow::InitMainWindow();

	view::frame::CSnowWindow mw;
	if (mw.CreateMainWindow(L"Shotit!")) {
		view::MessageLoop();
	}

	global::Uninit();
	return 0;
}
