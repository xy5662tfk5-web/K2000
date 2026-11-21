cmake_minimum_required(VERSION 3.22)
project(K2000AU VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CI checks out JUCE into ./JUCE. Local builds may still use an installed JUCE package.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    if(NOT DEFINED JUCE_DIR AND DEFINED ENV{JUCE_DIR})
        set(JUCE_DIR "$ENV{JUCE_DIR}")
    endif()
    find_package(JUCE CONFIG REQUIRED)
endif()

juce_add_plugin(K2000
    COMPANY_NAME "K2000 Lab"
    BUNDLE_ID "audio.k2000lab.k2000"
    PLUGIN_MANUFACTURER_CODE K2Lb
    PLUGIN_CODE K2Au
    PRODUCT_NAME "K2000"
    DESCRIPTION "Kurzweil K2000R-inspired instrument"
    FORMATS AU VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(K2000 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)
target_compile_definitions(K2000 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)
target_link_libraries(K2000 PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
