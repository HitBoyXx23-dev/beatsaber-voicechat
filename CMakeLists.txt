cmake_minimum_required(VERSION 3.19)
project(voicechat CXX)
set(CMAKE_CXX_STANDARD 17)

# Core logic and host tests: builds on any PC compiler.
add_library(voicecore STATIC src/voice_core.cpp)
target_include_directories(voicecore PUBLIC src)

add_executable(test_voice_core tests/test_voice_core.cpp)
target_link_libraries(test_voice_core PRIVATE voicecore)
enable_testing()
add_test(NAME voice_core COMMAND test_voice_core)

# Quest mod: only when building for Android (requires the NDK and qpm dependencies).
if(ANDROID)
    add_library(voicechat SHARED src/main.cpp)
    target_link_libraries(voicechat PRIVATE voicecore)
    set_target_properties(voicechat PROPERTIES OUTPUT_NAME voicechat)
endif()
