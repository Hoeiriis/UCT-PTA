add_library(environments_objLib OBJECT)
target_sources(environments_objLib
    PRIVATE
        UppaalEnvironmentInterface.cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/UppaalEnvironmentInterface.h>
        $<INSTALL_INTERFACE:include>)

Set_Multiple_PUBLIC_HEADER(environments_objLib
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UppaalEnvironmentInterface.h)

target_include_directories(environments_objLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)


target_compile_features(environments_objLib
    PUBLIC
        cxx_std_17)

add_library(Environments_uctpta STATIC)
target_link_libraries(Environments_uctpta
    PUBLIC
        environments_objLib
    PRIVATE
        MCTS::MCTS)