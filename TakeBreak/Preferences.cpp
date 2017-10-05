#include "stdafx.h"
#include "Preferences.h"
#include <Shlobj.h>
#include "Shlwapi.h"

#define REG_KEY         _T("TakeBreak")
#define REG_KEY_PATH    _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define PREF_FILE       _T("data")

Preferences::Preferences()
{
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, m_path)))
    {
        if (PathAppend(m_path, L"TakeBreak"))
        {
            CreateDirectory(m_path, NULL);
        }
    }

    if (IsFirstTime())
    {
        RunOnStartUp(TRUE);
    }
}

BOOL Preferences::IsFirstTime()
{
    WCHAR file[256] = { 0 };
    wcscpy_s(file, sizeof(file) / sizeof(WCHAR), m_path);
    PathAppend(file, PREF_FILE);

    return !PathFileExists(file);
}

void Preferences::Save(int selection)
{
    WCHAR file[256] = { 0 };
    wcscpy_s(file, sizeof(file) / sizeof(WCHAR), m_path);
    PathAppend(file, PREF_FILE);

    FILE* fp = NULL;
    _wfopen_s(&fp, file, L"wb");

    if (fp)
    {
        char buf[128] = { 0 };
        _itoa_s(selection, buf, 10);
        fwrite(buf, sizeof(char), strlen(buf), fp);
        fclose(fp);
    }
}

int Preferences::Get()
{
    WCHAR file[256] = { 0 };
    wcscpy_s(file, sizeof(file) / sizeof(WCHAR), m_path);
    PathAppend(file, PREF_FILE);

    int sel = -1;

    if (PathFileExists(file))
    {
        FILE* fp = NULL;
        _wfopen_s(&fp, file, L"rb");;

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

void Preferences::RunOnStartUp(BOOL flag)
{
    WCHAR cmd[MAX_PATH] = { 0 };

    if (flag)
    {
        WCHAR path[MAX_PATH] = { 0 };
        GetModuleFileName(0, path, sizeof(path));
        wsprintf(cmd, L"add HKCU\\%s /v %s /t REG_SZ /d \"\"\"%s\"\"\" /f", REG_KEY_PATH, REG_KEY, path);
    }
    else
    {
        wsprintf(cmd, L"delete HKCU\\%s /v %s /f", REG_KEY_PATH, REG_KEY);
    }

    // Launch as administrator. 
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = L"reg";
    sei.hwnd = NULL;
    sei.nShow = SW_HIDE;
    sei.lpParameters = cmd;


    if (!ShellExecuteEx(&sei))
    {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_CANCELLED)
        {
           // MessageBox(GetForegroundWindow(), L"You have change your mood", L"TakeBreak", MB_OK);
        }
    }
}

BOOL Preferences::IsRunOnStartUp()
{
    BOOL flag = FALSE;
    HKEY hKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_PATH, 0, KEY_ALL_ACCESS, &hKey))
    {
        flag = (ERROR_SUCCESS == RegQueryValueEx(hKey, REG_KEY, NULL, NULL, NULL, NULL));
        RegCloseKey(hKey);
    }

    return flag;
}