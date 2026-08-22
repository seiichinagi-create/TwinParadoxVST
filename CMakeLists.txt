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
    COPY_PLUGIN_AFTER_BUILD TRUE
    ICON_BIG                 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
    ICON_SMALL               "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
)

juce_generate_juce_header(TwinParadoxVST)

# The manual travels inside the binary, not beside it. docs/manual.html and
# docs/manual.pdf come from sn-lookandfeel/tools/build_manual.py, which reads
# the parameter table out of the BUILT plug-in - so after changing parameters,
# build, run that script, and build again. Its --check says if that is owed.
juce_add_binary_data(TwinParadoxVSTManual SOURCES
    docs/manual.html
    docs/manual.pdf
)
set_target_properties(TwinParadoxVSTManual PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

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
    TwinParadoxVSTManual
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
)
