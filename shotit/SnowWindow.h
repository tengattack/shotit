
#ifndef _SNOW_CUTE_SNOW_WINDOW_H_
#define _SNOW_CUTE_SNOW_WINDOW_H_ 1

#include "base/common.h"
#include "base/view.h"
#include "base/frame/mainwindow.h"

//control
#include "base/control/button.h"
#include "base/control/radio.h"
#include "base/control/check.h"
#include "base/control/text.h"
#include "base/control/listview.h"

//operation
#include "base/operation/threadpool.h"

//core
#include "core/users.h"

//base
#include "base/file/file.h"
#include "base/file/filedata.h"
#include "base/lock.h"

namespace view {
	namespace frame {

		class CSettingsWindow : public CDialog {
		public:
			CSettingsWindow();
			~CSettingsWindow();

			//function:
			static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
			int ListAllWindows();

		//protected:
			virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			int m_window_count;

		protected:

			CListView listview;
			CText text[2];
			CButton button[3];

			virtual int GetInitialWidth() const
			{
				return 520;
			}
			virtual int GetInitialHeight() const
			{
				return 345;
			}
		};

		class CSnowWindow : public CMainWindow {
		public:
			CSnowWindow();
			~CSnowWindow();

			void TopWindow(bool enable_);
			void doSettings();

		//protected:
			virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			CButton button[2];

		protected:
			//CRadio radio[2];
			CCheck check[1];
			CText text[3];

			//Lock m_lock;
			//unsigned long m_count;

			virtual int GetInitialWidth() const
			{
				return 160;
			}
			virtual int GetInitialHeight() const
			{
				return 250;
			}
		};

	};
};

#endif