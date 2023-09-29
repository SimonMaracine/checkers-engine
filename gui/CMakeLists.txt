cmake_minimum_required(VERSION 3.20)

set(wxBUILD_SHARED OFF)
# set(wxBUILD_SAMPLES ALL)

add_subdirectory(extern/wxWidgets)

add_subdirectory(common)
add_subdirectory(player)
add_subdirectory(tester)
