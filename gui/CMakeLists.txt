cmake_minimum_required(VERSION 3.20)

function(set_linux_defines TARGET)
    # According to the code docs, this should be defined for Linux
    if(UNIX)
        target_compile_definitions(${TARGET} PRIVATE "__WXGTK__")
    endif()
endfunction()

function(set_sanitizers TARGET)
    target_compile_options(${TARGET} PRIVATE "-fsanitize=undefined" "-g")
    target_link_options(${TARGET} PRIVATE "-static-libasan" "-fsanitize=undefined")
endfunction()

set(wxBUILD_SHARED ON)
set(wxBUILD_MONOLITHIC ON)
# set(wxBUILD_SAMPLES SOME)
set(wxUSE_LIBSDL ON)

add_subdirectory(extern/wxWidgets)

# Variable used by binary executables
if(WIN32)
    set(WINDOWS_APP WIN32)
endif()

add_subdirectory(common)
add_subdirectory(player)
add_subdirectory(tester)
