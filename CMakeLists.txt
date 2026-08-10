cmake_minimum_required(VERSION 3.22)
project(TwinParadoxVST VERSION 0.2.0)
set(CMAKE_CXX_STANDARD 17)

# JUCE の場所（プラットフォーム別・-DJUCE_DIR で上書き可）
if(NOT DEFINED JUCE_DIR)
    if(APPLE)
        set(JUCE_DIR "$ENV{HOME}/JUCE")
    else()
        set(JUCE_DIR "C:/JUCE")
    endif()
endif()
add_subdirectory("${JUCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)

juce_add_plugin(TwinParadoxVST
    PLUGIN_MANUFACTURER_CODE Snag
    PLUGIN_CODE TwPD
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Twin Paradox Distortion"
    COMPANY_NAME "SeiNagi"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    ICON_BIG                 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
    ICON_SMALL               "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
)

juce_generate_juce_header(TwinParadoxVST)

target_sources(TwinParadoxVST PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/TwinParadoxDSP.cpp
)

if(MSVC)
    target_compile_options(TwinParadoxVST PRIVATE /utf-8)
endif()

target_compile_definitions(TwinParadoxVST PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TwinParadoxVST
    PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
)
