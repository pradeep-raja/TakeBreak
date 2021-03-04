#include "stdafx.h"
#include "Commctrl.h"
#include "Settings.h"
#include "Preferences.h"
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

extern HINSTANCE hInst;

struct Option
{
    Option(int i, WCHAR* pStr) : value(i), pDisplay(pStr) {}
    int value = 0;
    WCHAR* pDisplay;
};

static Preferences prefObj;
static bool isRunning = false;

namespace Settings
{
BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

void LaunchSettings()
{
    if (!isRunning)
    {
        HWND hwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SETTINGS_DLG), NULL, (DLGPROC)DlgProc);
        ShowWindow(hwnd, SW_SHOW);
        isRunning = true;
    }
}

BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TAKEBREAK));
        if (hIcon)
        {
            SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }

        BOOL bFirstTime = prefObj.IsFirstTime();

        HWND hwndRunOnStartCheckBox = GetDlgItem(hwndDlg, IDC_CHECK_STARTUP_RUN);

        if (bFirstTime)
        {
            Button_SetElevationRequiredState(GetDlgItem(hwndDlg, IDOK), TRUE);
            SendMessage(hwndRunOnStartCheckBox, BM_SETCHECK, BST_CHECKED, 0);
        }
        else
        {
            SendMessage(hwndRunOnStartCheckBox, BM_SETCHECK, prefObj.IsRunOnStartUp() ? BST_CHECKED : BST_UNCHECKED, 0);
        }
        return TRUE;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_CHECK_STARTUP_RUN:
        {
            BOOL checked = SendDlgItemMessage(hwndDlg, IDC_CHECK_STARTUP_RUN, BM_GETCHECK, 0, 0);
            Button_SetElevationRequiredState(GetDlgItem(hwndDlg, IDOK), checked != prefObj.IsRunOnStartUp());
            break;
        }

        case IDOK:
        {
            int sel = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO1), (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

            if (prefObj.Get() != sel)
            {
                prefObj.Save(sel);
            }

            BOOL checked = SendDlgItemMessage(hwndDlg, IDC_CHECK_STARTUP_RUN, BM_GETCHECK, 0, 0);
            if (checked != prefObj.IsRunOnStartUp())
            {
                prefObj.RunOnStartUp(checked);
            }

            isRunning = false;
            DestroyWindow(hwndDlg);
            return TRUE;
        }

        case IDCANCEL:
        {
            isRunning = false;
            DestroyWindow(hwndDlg);
            return TRUE;
        }
        }
    }
    }
    return FALSE;
}
};