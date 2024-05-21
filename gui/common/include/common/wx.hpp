#pragma once

// Include wxWidgets while ignoring stupid warnings

#ifdef __GNUG__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <wx/wx.h>
#include <wx/timer.h>

#ifdef __GNUG__
    #pragma GCC diagnostic pop
#endif
