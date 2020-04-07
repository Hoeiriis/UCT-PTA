add_library(environments_uctpta_objLib OBJECT)
target_sources(environments_uctpta_objLib
    PRIVATE
        UppaalEnvironmentInterface.cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/UppaalEnvironmentInterface.h>
        $<INSTALL_INTERFACE:include>)

Set_Multiple_PUBLIC_HEADER(environments_uctpta_objLib
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UppaalEnvironmentInterface.h)

target_include_directories(environments_uctpta_objLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)


target_compile_features(environments_uctpta_objLib
    PUBLIC
        cxx_std_17)

add_library(Environments_uctpta STATIC)
target_link_libraries(Environments_uctpta
    PUBLIC
        environments_uctpta_objLib
    PRIVATE
        MCTS::MCTS)