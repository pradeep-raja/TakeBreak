#include "stdafx.h"
#include "Settings.h"
#include "resource.h"
#include <Shlobj.h>
#include "Shlwapi.h"

extern HINSTANCE hInst;

struct Option
{
    Option(int i, WCHAR* pStr) : value(i), pDisplay(pStr){}
    int value = 0;
    WCHAR* pDisplay;
};

static Option options[] =
{
    Option(30, L"30 minutes"),
    Option(45, L"45 minutes"),
    Option(60, L"1 hour"),
    Option(75, L"1 hour 15 minutes"),
    Option(90, L"1 hour 30 minutes"),
    Option(105, L"1 hour 45 minutes"),
    Option(120, L"2 Hours")
};

class Preferences
{
public:
    Preferences()
    {
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, m_path)))
        {
            if (PathAppend(m_path, L"TakeBreak"))
            {
                CreateDirectory(m_path, NULL);
            }
        }
    }

    void Save(int selection)
    {
        WCHAR file[256] = { 0 };
        wcscpy_s(file, sizeof(file)/sizeof(WCHAR), m_path);
        PathAppend(file, L"data");

        FILE* fp = NULL;
        _wfopen_s(&fp, file, L"wb");

        if (fp)
        {
            char buf[128] = { 0 };
            _itoa_s(selection, buf, 10);
            fwrite(buf, 1, sizeof(buf) / sizeof(char), fp);
            fclose(fp);
        }
    }

    int Get()
    {
        WCHAR file[256] = { 0 };
        wcscpy_s(file, sizeof(file)/sizeof(WCHAR), m_path);
        PathAppend(file, L"data");

        int sel = -1;

        if (PathFileExists(file))
        {
            FILE* fp = NULL;
            _wfopen_s(&fp, file, L"rb+");;

            if (fp)
            {
                char buf[128] = { 0 };
                fread(buf, 1, sizeof(buf) / sizeof(char), fp);
                fclose(fp);
                sel = atoi(buf);
            }
        }
        
        if (-1 == sel)
        {
            sel = 1;
            Save(sel);
        }

        return sel;
    }

private:
    WCHAR m_path[MAX_PATH];
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

    void GetData(Data* pData)
    {
        pData->interval = options[prefObj.Get()].value;
    }

    BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
                HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
                SetClassLongPtr(hwndDlg, GCLP_HBRBACKGROUND, (LONG)brush);

                HWND hwndTimeDropdown = GetDlgItem(hwndDlg, IDC_COMBO1);
                const int entries = sizeof(options) / sizeof(options[0]);

                for (int k = 0; k < entries; k++)
                {
                    SendMessage(hwndTimeDropdown, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)options[k].pDisplay);
                }

                SendMessage(hwndTimeDropdown, CB_SETCURSEL, (WPARAM)prefObj.Get(), 0);
                return TRUE;
            }

            case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                    case IDOK:
                    {
                        int sel = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO1), (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

                        if (prefObj.Get() != sel)
                        {
                            prefObj.Save(sel);
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