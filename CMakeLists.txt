cmake_minimum_required(VERSION 3.22)

project(TrebleMaker VERSION 1.0.0 LANGUAGES C CXX)

# Create a compile_commands.json file for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# JUCE setup
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
)
FetchContent_MakeAvailable(juce)

FetchContent_Declare(
    clap-juce-extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(clap-juce-extensions)

# Build Options
option(JUCE_BUILD_EXTRAS "Build JUCE extras" OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE examples" OFF)

# Plugin Target
juce_add_plugin(TrebleMaker
    COMPANY_NAME "LeoCodes"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "LeoC"
    PLUGIN_CODE "Tblm"
    FORMATS VST3 LV2 AU Standalone
    LV2URI "http://leocodes.com/plugins/TrebleMaker"
    PRODUCT_NAME "TrebleMaker"
)

clap_juce_extensions_plugin(
    TARGET TrebleMaker
    CLAP_ID "com.leocodes.treblemaker"
    CLAP_FEATURES audio-effect equalizer
)

# Assets
juce_add_binary_data(TrebleMakerAssets
    HEADER_NAME "BinaryData.h"
    NAMESPACE BinaryData
    SOURCES
        Source/Assets/Helvetica-Bold.ttf
)


# Source Files
target_sources(TrebleMaker
    PRIVATE
        Source/Core/PluginProcessor.cpp
        Source/Core/PluginProcessor.h
        Source/UI/PluginEditor.cpp
        Source/UI/PluginEditor.h
        Source/UI/LookAndFeel.h
        Source/UI/LookAndFeel.cpp
)

# Link Libraries
target_compile_definitions(TrebleMaker
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(TrebleMaker
    PRIVATE
        TrebleMakerAssets
        clap_juce_extensions
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_events
        juce::juce_core
        juce::juce_data_structures
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
