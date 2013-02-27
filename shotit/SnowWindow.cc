
#include "stdafx.h"

#include <base/common.h>
#include <base/view.h>
#include <base/frame/frame_window.h>
#include <base/frame/mainwindow.h>

#include <base/string/stringprintf.h>
#include <base/string/string_number_conversions.h>

#include <base/file/file.h>
#include <base/file/filedata.h>
#include <base/json/json_reader.h>
#include <base/json/json_writer.h>
#include <base/json/values.h>
#include <base/json-config.h>

#include <base/operation/fileselect.h>

#include <filecommon/file_path.h>

#include "SnowWindow.h"

#ifndef _DEBUG
#include <png.h>
#pragma comment(lib, "libpng15.lib")
#pragma comment(lib, "zlib.lib")
#endif

#define TEXT_BUTTON_SHOT L"SHOT"

enum _ID_CONTROL {
	//window
	ID_WINDOW_SETTINGS = 1000,
	//button
	ID_BUTTON_OK = 1001,
	ID_BUTTON_CANCEL,
	ID_BUTTON_BROWSE,
	ID_BUTTON_EXIT,
	ID_BUTTON_SHOT,
	ID_BUTTON_SETTINGS,
	//check
	ID_BUTTON_ONTOP,
	//text
	ID_TEXT_PATH,
	//litview
	ID_LISTVIEW_WINDOWS,
	//Static
	ID_STATIC_SETTINGS,
};

HWND g_hShotWnd = NULL;
std::wstring g_szSavePath;
std::wstring g_szTargetTitle;
std::wstring g_szConfigfile;
std::wstring g_szGlobalConfigfile;
unsigned long g_lFileindex = 0;
bool g_alwaysontop = false;
bool g_findwindow = false;

void ReadGlobalConfigfile()
{
	g_lFileindex = 0;
	wchar_t exepath[MAX_PATH];
	int len = ::GetModuleFileName(NULL, exepath, MAX_PATH);
	for (int i = len - 1; i > 0; i--) {
		if (exepath[i] == '\\' || exepath[i] == '/') {
			exepath[i + 1] = 0;
			break;
		}
	}

	g_szGlobalConfigfile = exepath;
	g_szGlobalConfigfile += L"config.json";

	DictionaryValue* v = NULL;
	if (base::ReadJsonFile(g_szGlobalConfigfile.c_str(), (Value **)&v)) {
		if (v) {
			if (v->GetType() == Value::TYPE_DICTIONARY) {
				std::wstring path;
				v->GetString("path", &path);
				if (path.length() > 0) {
					g_szSavePath = path;
				}

				v->GetBoolean("alwaysontop", &g_alwaysontop);
				v->GetString("targettitle", &g_szTargetTitle);
			}
			delete v;
		}
	}
}

void WriteGlobalConfigfile()
{
	std::string jsontext;
	DictionaryValue* v = new DictionaryValue();
	v->SetString("path", g_szSavePath);
	v->SetString("targettitle", g_szTargetTitle);
	v->SetBoolean("alwaysontop", g_alwaysontop);
	base::JSONWriter::Write(v, true, &jsontext);
	delete v;

	base::CFile f;
	if (f.Open(base::kFileCreate, g_szGlobalConfigfile.c_str())) {
		f.Write((unsigned char *)jsontext.c_str(), jsontext.length());
		f.Close();
	}
}

void ReadConfigfile()
{
	g_lFileindex = 0;
	g_szConfigfile = g_szSavePath + L"shotit.json";
	
	DictionaryValue* v = NULL;
	if (base::ReadJsonFile(g_szConfigfile.c_str(), (Value **)&v)) {
		if (v) {
			if (v->GetType() == Value::TYPE_DICTIONARY) {
				int index = 0;
				v->GetInteger("fileindex", &index);
				if (index > 0) {
					g_lFileindex = index;
				}
			}

			delete v;
		}
	}
}

void WriteConfigfile()
{
	std::string jsontext;
	DictionaryValue* v = new DictionaryValue();
	v->SetInteger("fileindex", g_lFileindex);
	base::JSONWriter::Write(v, true, &jsontext);
	delete v;

	base::CFile f;
	if (f.Open(base::kFileCreate, g_szConfigfile.c_str())) {
		f.Write((unsigned char *)jsontext.c_str(), jsontext.length());
		f.Close();
	}
}

HBITMAP PrintWindowByRect(HDC *phMemDC, LPRECT lpRect)
{
	HDC       hScrDC, hMemDC;      
	// ��Ļ���ڴ��豸������
	HBITMAP    hBitmap, hOldBitmap;   
	// λͼ���
	int       nX, nY, nX2, nY2;      
	// ѡ����������
	int       nWidth, nHeight;
	
	// ȷ��ѡ������Ϊ�վ���
	if (IsRectEmpty(lpRect))
		return NULL;

	int m_xScreen = GetSystemMetrics(SM_CXSCREEN);
	int m_yScreen = GetSystemMetrics(SM_CYSCREEN);

	//Ϊ��Ļ�����豸������
	hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	//Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
	hMemDC = CreateCompatibleDC(hScrDC);
	// ���ѡ����������
	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	//ȷ��ѡ�������ǿɼ���
	if (nX < 0)
		nX = 0;
	if (nY < 0)
		nY = 0;
	if (nX2 > m_xScreen)
		nX2 = m_xScreen;
	if (nY2 > m_yScreen)
		nY2 = m_yScreen;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;

	// ����һ������Ļ�豸��������ݵ�λͼ
	hBitmap = CreateCompatibleBitmap
		(hScrDC, nWidth, nHeight);
	// ����λͼѡ���ڴ��豸��������
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	// ����Ļ�豸�����������ڴ��豸��������
	BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			hScrDC, nX, nY, SRCCOPY);

	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
	//�õ���Ļλͼ�ľ��
	//��� 
	DeleteDC(hScrDC);

	/*if(bSave)
	{
		if (OpenClipboard()) 
		{
			//��ռ�����
			EmptyClipboard();
			//����Ļ����ճ������������,
			//hBitmap Ϊ�ղŵ���Ļλͼ���
			SetClipboardData(CF_BITMAP, hBitmap);
			//�رռ�����
			CloseClipboard();
		}
	}*/
	// ����λͼ���
	*phMemDC = hMemDC;
	return hBitmap;
}

bool ToPngFile(BYTE *bImageData, int width, int height, DWORD dwSize, LPCTSTR lpszPngFile)
{
	bool bret = false;
#ifndef _DEBUG
	png_bytepp row_p = NULL;
	png_bytep trans_palette=NULL;

	row_p = (png_bytepp)malloc(height * sizeof(png_bytep));

	//�ȷ������������Ķ���ָ��
	png_structp png_ptr = png_create_write_struct
		(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));

	FILE *fp = _tfopen(lpszPngFile, _T("wb"));
	if (!fp)
	{
		//_snprintf(errstr,ERRSTR_LEN,"�޷���д���ļ���%s��",sfile);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		goto exit_l;
	}
	png_init_io(png_ptr, fp);		//set up the output code
	//png_set_write_status_fn(png_ptr, write_row_callback);
	png_set_IHDR(png_ptr, info_ptr, width, height,	//����IHDR��
		8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	//png_set_pHYs(png_ptr, info_ptr, 0xb12, 0xb12, PNG_RESOLUTION_METER);

	int bpl = width*4;
	for(int i = 0; i < height; i++) {
		//bmpdata
		int line = height - i - 1;
		row_p[line] = (png_bytep)malloc(bpl);	//�������ڴ�й¶

		for (int k = 0; k < width; k+=4) {
			row_p[line][k] = (bImageData + i * bpl)[k+2];
			row_p[line][k+1] = (bImageData + i * bpl)[k+1];
			row_p[line][k+2] = (bImageData + i * bpl)[k];
			row_p[line][k+3] = (bImageData + i * bpl)[k+3];
		}
		row_p[line] = (bImageData + i * bpl);
				
		//raw data
		//row_p[i] = (bImageData + i * bpl);
	}

	png_set_rows(png_ptr, info_ptr, row_p);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	bret = true;

exit_l:
	if (row_p) {
		//bmpdata
		/*for(int i=0;i<height;i++)
			if(row_p[i]) {
				free(row_p[i]);
				row_p[i] = NULL;
			}*/
		free(row_p);
	}
	if(fp) fclose(fp);
#endif

	return bret;
}

bool WriteBMPFile(HDC hDC, HBITMAP bitmap, LPCTSTR filename)
{
	BITMAP bmp; 
	PBITMAPINFO pbmi; 
	WORD cClrBits; 
	HANDLE hf; // file handle 
	BITMAPFILEHEADER hdr; // bitmap file-header 
	PBITMAPINFOHEADER pbih; // bitmap info-header 
	LPBYTE lpBits; // memory pointer 
	DWORD dwTotal; // total count of bytes 
	DWORD cb; // incremental count of bytes 
	BYTE *hp; // byte pointer 
	DWORD dwTmp; 

	// create the bitmapinfo header information

	if (!GetObject(bitmap, sizeof(BITMAP), (LPSTR)&bmp)){
		//AfxMessageBox("Could not retrieve bitmap info");
		return false;
	}

	// Convert the color format to a count of bits. 
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
	if (cClrBits == 1) 
		cClrBits = 1; 
	else if (cClrBits <= 4) 
		cClrBits = 4; 
	else if (cClrBits <= 8) 
		cClrBits = 8; 
	else if (cClrBits <= 16) 
		cClrBits = 16; 
	else if (cClrBits <= 24) 
		cClrBits = 24; 
	else cClrBits = 32; 

	// Allocate memory for the BITMAPINFO structure.
	if (cClrBits != 24) {
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<< cClrBits)); 
	} else {
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER)); 
	}
	// Initialize the fields in the BITMAPINFO structure. 

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
	pbmi->bmiHeader.biWidth = bmp.bmWidth; 
	pbmi->bmiHeader.biHeight = bmp.bmHeight; 
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
	if (cClrBits < 24) 
	pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

	// If the bitmap is not compressed, set the BI_RGB flag. 
	pbmi->bmiHeader.biCompression = BI_RGB; 

	// Compute the number of bytes in the array of color 
	// indices and store the result in biSizeImage. 
	pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8 * pbmi->bmiHeader.biHeight * cClrBits; 
	// Set biClrImportant to 0, indicating that all of the 
	// device colors are important. 
	pbmi->bmiHeader.biClrImportant = 0; 

	// now open file and save the data
	pbih = (PBITMAPINFOHEADER) pbmi; 
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits) {
		//AfxMessageBox("writeBMP::Could not allocate memory");
		return false;
	}

	// Retrieve the color table (RGBQUAD array) and the bits 
	if (!GetDIBits(hDC, HBITMAP(bitmap), 0, (WORD) pbih->biHeight, lpBits, pbmi, DIB_RGB_COLORS)) {
		//AfxMessageBox("writeBMP::GetDIB error");
		return false;
	}

	// Create the .BMP file. 
	/*hf = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, (DWORD) 0, 
	NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
	(HANDLE) NULL); 
	if (hf == INVALID_HANDLE_VALUE){
	//AfxMessageBox("Could not create file for writing");
	return;
	}
	hdr.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M" 
	// Compute the size of the entire file. 
	hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
	pbih->biSize + pbih->biClrUsed 
	* sizeof(RGBQUAD) + pbih->biSizeImage); 
	hdr.bfReserved1 = 0; 
	hdr.bfReserved2 = 0; 

	// Compute the offset to the array of color indices. 
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
	pbih->biSize + pbih->biClrUsed 
	* sizeof (RGBQUAD); 

	// Copy the BITMAPFILEHEADER into the .BMP file. 
	if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
	(LPDWORD) &dwTmp, NULL)) {
	//AfxMessageBox("Could not write in to file");
	return;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
	if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
	+ pbih->biClrUsed * sizeof (RGBQUAD), 
	(LPDWORD) &dwTmp, ( NULL))){
	//AfxMessageBox("Could not write in to file");
	return;
	}


	// Copy the array of color indices into the .BMP file. 
	dwTotal = cb = pbih->biSizeImage; 
	hp = lpBits; 
	if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)){
	//AfxMessageBox("Could not write in to file");
	return;
	}

	// Close the .BMP file. 
	if (!CloseHandle(hf)){
	//AfxMessageBox("Could not close file");
	return;
	}*/

	//write png files
	bool bret = ToPngFile(lpBits, pbih->biWidth, pbih->biHeight, pbih->biSizeImage, filename);

	// Free memory. 
	GlobalFree((HGLOBAL)lpBits);

	return bret;
}

namespace view{
	namespace frame{

		CSettingsWindow::CSettingsWindow()
			: m_window_count(0)
		{
			AddStaticText(ID_STATIC_SETTINGS, 10, 10, 485, 20, L"Select Window");
			AddStaticText(ID_STATIC_SETTINGS, 10, 190, 485, 20, L"Save Path");
		}

		CSettingsWindow::~CSettingsWindow()
		{
		}

		BOOL CALLBACK CSettingsWindow::EnumWindowsProc(HWND hWnd, LPARAM lParam)
		{
			if(::GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE) {

				CSettingsWindow* pSettings = (CSettingsWindow *)lParam;
				wchar_t sztext[256] = {0};
				wsprintf(sztext, L"0x%08x", hWnd);

				int iItem = pSettings->listview.InsertItem(pSettings->m_window_count, sztext);

				::GetWindowText(hWnd, sztext, 256);
				pSettings->listview.SetItemText(iItem, 1, sztext);

				if (g_hShotWnd == hWnd) {
					pSettings->listview.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					pSettings->listview.EnsureVisible(iItem, false);
					g_findwindow = true;
				}

				pSettings->m_window_count++;//count start
			}
			return TRUE;
		}

		int CSettingsWindow::ListAllWindows()
		{
			m_window_count = 0;
			g_findwindow = false;
			EnumWindows(EnumWindowsProc, (LPARAM)this);
			if (!g_findwindow && g_szTargetTitle.length() > 0) {
				//check title
				int count = listview.GetItemCount();
				std::wstring title;
				for (int i = 0; i < count; i++) {
					title = L"";
					listview.GetItemText(i, 1, title);
					if (title == g_szTargetTitle) {
						listview.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						listview.EnsureVisible(i, false);
						g_findwindow = true;
						break;
					}
				}
			}
			return m_window_count;
		}

		void CSettingsWindow::InitWindow()
		{
			listview.CreateListView(m_hWnd, L"All Windows", ID_LISTVIEW_WINDOWS, 10, 30, 485, 150); 
			listview.InsertColumn(0, L"hWnd", 100);
			listview.InsertColumn(1, L"Name", 360);

			button[0].CreateButton(m_hWnd, L"SAVE", ID_BUTTON_OK, 335, 245, 80, 30);
			button[1].CreateButton(m_hWnd, L"CANCEL", ID_BUTTON_CANCEL, 420, 245, 80, 30);
			button[2].CreateButton(m_hWnd, L"BROWSE", ID_BUTTON_BROWSE, 430, 210, 65, 25);

			text[0].CreateText(m_hWnd, g_szSavePath.c_str(), ID_TEXT_PATH, 10, 210, 415, 25);

			ListAllWindows();
		}

		LRESULT CALLBACK CSettingsWindow::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					handled = true;
					switch (LOWORD(wParam))
					{
					case ID_BUTTON_OK:			//SAVE
						if (listview.GetSelectedCount() > 0) {
							int count = listview.GetItemCount();
							int iItem = 0;
							HWND hNewWnd = NULL;
							std::wstring hwndtext, path;

							for (int i = 0; i < count; i++) {
								if (listview.IsItemSelected(i)) {
									listview.GetItemText(i, 0, hwndtext);
									if (hwndtext.length() > 2) {
										hNewWnd = (HWND)wcstoul(hwndtext.c_str() + 2, NULL, 16);
										iItem = i;
									}
									break;
								}
							}
							if (hNewWnd != NULL) {
								text[0].GetText(path);
								int length = path.length();
								if (length > 0) {

									g_hShotWnd = hNewWnd;
									if (path[length - 1] != L'\\') {
										path += L"\\";
									}

									g_szSavePath = path;
									CreateDir(path.c_str(), true);

									listview.GetItemText(iItem, 1, g_szTargetTitle);

									//config
									ReadConfigfile();
									WriteGlobalConfigfile();

									Close();
								}
							}
						}
						break;
					case ID_BUTTON_CANCEL:
						Close();
						break;
					case ID_BUTTON_BROWSE:
						operation::CFileSelect fs(m_hWnd, operation::kDir, NULL, L"Please select a save path");
						if (fs.Select())
						{
							text[0].SetText(fs.GetPath().c_str());
						}
						break;
					}
				}
				return 0;
				break;
			}

			return CDialog::OnWndProc(message, wParam, lParam, handled);
		}

		DWORD WINAPI ShotProc(LPVOID lParam)
		{
			CSnowWindow* pWindow = (CSnowWindow *)lParam;
			bool bshot = false;
			DWORD starttime = ::GetTickCount();

			if (g_hShotWnd) {

				std::wstring path = g_szSavePath;
				wchar_t szfilename[25];

				RECT wsize = {0};
				::SetWindowPos(g_hShotWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
				::SetFocus(g_hShotWnd);
				::GetWindowRect(g_hShotWnd, &wsize);

				//HDC hdc = GetDC(m_hWnd);
				//HDC hMemDC = ::CreateCompatibleDC(NULL);
				//HBITMAP hMemBitmap = ::CreateCompatibleBitmap(hdc, wsize.right - wsize.left, wsize.bottom - wsize.top);
				//HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hMemBitmap);
								
				HDC hMemDC = NULL;
				HBITMAP hMemBitmap = PrintWindowByRect(&hMemDC, &wsize);	//print all, with work area

				g_lFileindex++;
				wsprintf(szfilename, L"%08d.png", g_lFileindex);
				path += szfilename;

				bshot = WriteBMPFile(hMemDC, hMemBitmap, path.c_str());
				//DeleteObject(hOldBitmap);
				DeleteObject(hMemBitmap);
				DeleteDC(hMemDC);
				//ReleaseDC(m_hWnd, hdc);

				WriteConfigfile();

				//PrintWindow: http://msdn.microsoft.com/en-us/library/dd162869(VS.85).aspx
				//PrintWindow PW_CLIENTONLY
			}

			if (pWindow) {
				if (!bshot) {
					pWindow->button[0].SetText(L"FAILED!");
					Sleep(1000);
					pWindow->button[0].SetText(TEXT_BUTTON_SHOT);
				} else {
					DWORD usedtime = ::GetTickCount() - starttime;
					if (usedtime < 1000) {
						Sleep(1000 - usedtime);
					}
				}
				pWindow->button[0].Enable(true);
			}
			return 0;
		}


		CSnowWindow::CSnowWindow()
		{
			ReadGlobalConfigfile();

			//hide max button
			m_skin.GetCaptionButton(1)->SetShow(false);
			m_skin.GetCaptionButton(1)->SetEnabled(false);

			//AddStaticText(
		}

		CSnowWindow::~CSnowWindow()
		{
		}

		void CSnowWindow::TopWindow(bool enable_)
		{
			if (enable_) {
				::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			} else {
				::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			}
		}

		void CSnowWindow::InitWindow()
		{
			button[0].CreateButton(m_hWnd, TEXT_BUTTON_SHOT, ID_BUTTON_SHOT, 30, 5, 100, 100);
			button[1].CreateButton(m_hWnd, L"SETTINGS", ID_BUTTON_SETTINGS, 30, 115, 100, 50);

			check[0].CreateCheck(m_hWnd, L"ON TOP", ID_BUTTON_ONTOP, 48, 175, 100, 20);

			if (g_alwaysontop) {
				TopWindow(true);
				check[0].SetCheck(true);
			}
		}

		LRESULT CALLBACK CSnowWindow::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					handled = true;
					switch (LOWORD(wParam))
					{
					/*case ID_BUTTON_OK:
						break;
					case ID_BUTTON_EXIT:
						Close();
						break;*/
					case ID_BUTTON_ONTOP:
						if (check[0].GetCheck()) {
							g_alwaysontop = true;
							TopWindow(true);
						} else {
							g_alwaysontop = false;
							TopWindow(false);
						}
						WriteGlobalConfigfile();
						break;
					//���
					case ID_BUTTON_SHOT:
						{
							if (g_hShotWnd) {

								button[0].Enable(false);
								::SetWindowPos(g_hShotWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
								::SetFocus(g_hShotWnd);

								HANDLE hThread = ::CreateThread(NULL, NULL, ShotProc, this, NULL, NULL);
								if (hThread != INVALID_HANDLE_VALUE) {
									CloseHandle(hThread);
								} else {
									button[0].Enable(true);
								}

							} else {
								goto l_showsettings;
							}
						}
						break;
					case ID_BUTTON_SETTINGS:
						{
l_showsettings:
							CSettingsWindow sw;
							//sw.DoMedal();
							sw.CreateDialog(m_hWnd, L"Settings", true);
							sw.DoModal();
						}
						break;
					}
				}
				return 0;
				break;
			}

			return CMainWindow::OnWndProc(message, wParam, lParam, handled);
		}
	};
};
