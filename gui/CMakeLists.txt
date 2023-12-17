cmake_minimum_required(VERSION 3.20)

function(set_linux_defines TARGET)
    # According to the code docs, this should be defined for Linux
    if(UNIX)
        target_compile_definitions(${TARGET} PRIVATE "__WXGTK__")
    endif()
endfunction()

set(wxBUILD_SHARED ON)
set(wxBUILD_MONOLITHIC ON)

add_subdirectory(extern/wxWidgets)

# Variable used by binary executables
if(WIN32)
    set(WINDOWS_APP WIN32)
endif()

add_subdirectory(common)
add_subdirectory(player)
add_subdirectory(tester)
