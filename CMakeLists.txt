cmake_minimum_required(VERSION 3.19)
project(voicechat CXX)
set(CMAKE_CXX_STANDARD 17)

# Core logic and host tests: builds on any PC compiler.
add_library(voicecore STATIC src/voice_core.cpp src/jitter_buffer.cpp)
target_include_directories(voicecore PUBLIC src)

add_executable(test_voice_core tests/test_voice_core.cpp)
target_link_libraries(test_voice_core PRIVATE voicecore)
add_executable(test_jitter_buffer tests/test_jitter_buffer.cpp)
target_link_libraries(test_jitter_buffer PRIVATE voicecore)
enable_testing()
add_test(NAME voice_core COMMAND test_voice_core)
add_test(NAME jitter_buffer COMMAND test_jitter_buffer)

# Quest mod: only when building for Android (requires the NDK and qpm dependencies).
if(ANDROID)
    file(GLOB_RECURSE VOICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    list(FILTER VOICE_SOURCES EXCLUDE REGEX ".*/(voice_core|jitter_buffer)\\.cpp$")
    add_library(voicechat SHARED ${VOICE_SOURCES})
    target_include_directories(voicechat PRIVATE include src)
    target_link_libraries(voicechat PRIVATE voicecore)
    set_target_properties(voicechat PROPERTIES OUTPUT_NAME voicechat)
endif()
