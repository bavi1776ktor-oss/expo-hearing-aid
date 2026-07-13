cmake_minimum_required(VERSION 3.22.1)
project(HearingAidEngine)

find_package(oboe REQUIRED CONFIG)

add_library(
        hearing_aid_engine
        SHARED
        src/main/cpp/AudioEngine.cpp
)

target_link_libraries(
        hearing_aid_engine
        oboe::oboe
        log
)
