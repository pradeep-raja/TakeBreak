#pragma once

#include "windows.h"

namespace Settings
{
    struct Data
    {
        UINT interval = 0;
    };

    void LaunchSettings();
    void GetData(Data* pData);
};