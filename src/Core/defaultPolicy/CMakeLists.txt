add_library(defaultpolicy_uctpta_objLib OBJECT)
target_sources(defaultpolicy_uctpta_objLib
    PRIVATE
        UPPAAL_RandomSamplingDefaultPolicy.cpp
        DelaySamplingDefaultPolicy.cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/UPPAAL_RandomSamplingDefaultPolicy.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/DelaySamplingDefaultPolicy.h>
        $<INSTALL_INTERFACE:include>)

Set_Multiple_PUBLIC_HEADER(defaultpolicy_uctpta_objLib
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UPPAAL_RandomSamplingDefaultPolicy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/DelaySamplingDefaultPolicy.h)

target_include_directories(defaultpolicy_uctpta_objLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(defaultpolicy_uctpta_objLib
    PRIVATE
        MCTS::MCTS)