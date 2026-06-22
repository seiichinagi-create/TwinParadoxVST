cmake_minimum_required(VERSION 3.22)
project(TwinParadoxVST VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory("C:/JUCE" "${CMAKE_CURRENT_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)

juce_add_plugin(TwinParadoxVST
    PLUGIN_MANUFACTURER_CODE Snag
    PLUGIN_CODE TwPD
    FORMATS VST3 Standalone
    PRODUCT_NAME "Twin Paradox Distortion"
    COMPANY_NAME "SeiNagi"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
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
