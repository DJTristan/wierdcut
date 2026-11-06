cmake_minimum_required(VERSION 3.22)
project(WarpCutLite VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(WarpCutLite COMPANY_NAME "Independent Audio" BUNDLE_ID "audio.independent.warpcutlite"
    PLUGIN_MANUFACTURER_CODE IndA PLUGIN_CODE WcL1
    FORMATS AU VST3 PRODUCT_NAME "WarpCut Lite"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(WarpCutLite)
target_sources(WarpCutLite PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h)
target_compile_definitions(WarpCutLite PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(WarpCutLite PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_audio_processors juce::juce_audio_plugin_client)
