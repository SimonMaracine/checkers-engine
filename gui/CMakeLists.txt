cmake_minimum_required(VERSION 3.20)

function(set_linux_defines target)
    # According to the code docs, this should be defined for Linux
    if(UNIX)
        target_compile_definitions(${target} PRIVATE "__WXGTK__")
    endif()
endfunction()

function(set_sanitizers target)
    target_compile_options(${target} PRIVATE "-fsanitize=address" "-fsanitize=undefined" "-g" "-fno-omit-frame-pointer")
    target_link_options(${target} PRIVATE "-fsanitize=address" "-fsanitize=undefined")

    message(STATUS "Building ${target} with sanitizers")
endfunction()

set(wxBUILD_SHARED ON)
set(wxBUILD_MONOLITHIC ON)
# set(wxBUILD_SAMPLES SOME)

add_subdirectory(extern/wxWidgets)

# Variable used by binary executables
if(WIN32)
    set(WINDOWS_APP WIN32)
endif()

add_subdirectory(common)
add_subdirectory(player)
add_subdirectory(tester)
