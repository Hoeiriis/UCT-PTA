add_library(defaultpolicy_uctpta_objLib)
target_sources(defaultpolicy_uctpta_objLib
    PRIVATE
        UPPAAL_RandomSamplingDefaultPolicy.cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/UPPAAL_RandomSamplingDefaultPolicy.h>
        $<INSTALL_INTERFACE:include>)

Set_Multiple_PUBLIC_HEADER(defaultpolicy_uctpta_objLib
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UPPAAL_RandomSamplingDefaultPolicy.h)

target_include_directories(defaultpolicy_uctpta_objLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(defaultpolicy_uctpta_objLib
    PRIVATE
        MCTS::MCTS)