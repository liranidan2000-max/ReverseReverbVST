cmake_minimum_required(VERSION 3.22)
project(ReverseReverb VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Plugin target
juce_add_plugin(ReverseReverb
    COMPANY_NAME           "LiranRoneKalifa"
    IS_SYNTH               TRUE
    NEEDS_MIDI_INPUT       TRUE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE            Rvrs
    FORMATS                VST3 Standalone
    PRODUCT_NAME           "ReverseReverb"
    COPY_PLUGIN_AFTER_BUILD FALSE
    ICON_BIG               "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
    ICON_SMALL             "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
)

# Generate JuceHeader.h (needed because source files use #include <JuceHeader.h>)
juce_generate_juce_header(ReverseReverb)

target_sources(ReverseReverb
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Embed background image as binary data
juce_add_binary_data(ReverseReverbBinaryData
    SOURCES
        "Resources/BACKGROUND REVERSE PNG.png"
)

target_compile_definitions(ReverseReverb
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(ReverseReverb
    PRIVATE
        ReverseReverbBinaryData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
