#pragma once
#pragma once

#include "windows.h"

class Preferences
{
public:
    Preferences();

    BOOL IsFirstTime();

    void Save(int selection);
    int Get();

    void RunOnStartUp(BOOL flag);
    BOOL IsRunOnStartUp();

private:
    WCHAR m_path[MAX_PATH];
};

