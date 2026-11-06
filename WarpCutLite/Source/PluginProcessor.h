#pragma once
#include <JuceHeader.h>
#include <array>
#include <random>

class WarpCutLiteAudioProcessor final : public juce::AudioProcessor
{
public:
    WarpCutLiteAudioProcessor();
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "WarpCut Lite"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    float p(const char* id) const { return *parameters.getRawParameterValue(id); }
    void startNewCut(double bpm, int sampleRate);
    float random01() { return std::uniform_real_distribution<float>(0.f, 1.f)(rng); }
    std::mt19937 rng { 0xBADC0DEu };
    juce::AudioBuffer<float> history;
    int writePos = 0, filled = 0, cutPos = 0, cutLength = 1, sourceStart = 0;
    int repeatsRemaining = 0, positionInPhrase = 0;
    bool cutting = false;
    double sr = 44100.0, lastBpm = 120.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WarpCutLiteAudioProcessor)
};
