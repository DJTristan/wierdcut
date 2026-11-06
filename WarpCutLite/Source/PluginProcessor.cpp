#include "PluginProcessor.h"
#include <cmath>

juce::AudioProcessorValueTreeState::ParameterLayout WarpCutLiteAudioProcessor::makeParameters()
{
    using P = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> out;
    out.push_back(std::make_unique<P>(juce::ParameterID{"chance", 1}, "Cut Chance", juce::NormalisableRange<float>(0, 100, 1), 65));
    out.push_back(std::make_unique<P>(juce::ParameterID{"repeat", 1}, "Repeat Amount", juce::NormalisableRange<float>(0, 100, 1), 65));
    out.push_back(std::make_unique<P>(juce::ParameterID{"chaos", 1}, "Warp", juce::NormalisableRange<float>(0, 100, 1), 55));
    out.push_back(std::make_unique<C>(juce::ParameterID{"grid", 1}, "Minimum Slice", juce::StringArray{"1/8", "1/16", "1/32", "1/64"}, 1));
    out.push_back(std::make_unique<P>(juce::ParameterID{"fade", 1}, "Cut Fade ms", juce::NormalisableRange<float>(0, 20, 0.1f), 3));
    out.push_back(std::make_unique<P>(juce::ParameterID{"mix", 1}, "Mix", juce::NormalisableRange<float>(0, 100, 1), 100));
    return {out.begin(), out.end()};
}

WarpCutLiteAudioProcessor::WarpCutLiteAudioProcessor()
  : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    parameters(*this, nullptr, "WarpCutLite", makeParameters()) {}

bool WarpCutLiteAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == l.getMainOutputChannelSet()
        && (l.getMainInputChannelSet() == juce::AudioChannelSet::mono()
            || l.getMainInputChannelSet() == juce::AudioChannelSet::stereo());
}

void WarpCutLiteAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sr = sampleRate;
    history.setSize(2, juce::jmax(1024, (int)std::ceil(sr * 8.0)));
    history.clear();
    writePos = filled = cutPos = positionInPhrase = repeatsRemaining = 0;
    cutLength = 1;
    cutting = false;
}

void WarpCutLiteAudioProcessor::startNewCut(double bpm, int sampleRate)
{
    const int gridChoice = juce::jlimit(0, 3, (int)p("grid"));
    const int unitsPerBeat = 2 << gridChoice; // 1/8 to 1/64 in 4/4
    const int gridSamples = juce::jmax(16, (int)std::round(60.0 * sampleRate / (bpm * unitsPerBeat)));
    const float chaos = p("chaos") / 100.f;
    const float chance = p("chance") / 100.f;
    const float repetition = p("repeat") / 100.f;

    // A new phrase is built out of several unequal-length cuts.
    // Fills get more probable late in each phrase.
    const float fillBoost = positionInPhrase > 5 ? 0.22f : 0.f;
    cutting = filled > gridSamples * 8 && random01() < juce::jmin(1.f, chance + fillBoost);
    const int variableDivisor = random01() < chaos ? (random01() < 0.5f ? 1 : 2) : 1;
    cutLength = gridSamples * variableDivisor;
    if (random01() < chaos * 0.60f) cutLength = juce::jmax(16, cutLength / 2);
    if (random01() < chaos * 0.30f) cutLength *= 2;
    cutLength = juce::jlimit(16, juce::jmax(16, filled / 2), cutLength);
    cutPos = 0;

    if (cutting)
    {
        // Pull a slice from recently played audio; loop the same region for stutters.
        const int maxBack = juce::jmax(cutLength + 1, juce::jmin(filled - 1, gridSamples * 32));
        const int back = juce::jlimit(cutLength + 1, maxBack,
                           cutLength + 1 + (int)(random01() * juce::jmax(1, maxBack - cutLength - 1)));
        sourceStart = (writePos - back + history.getNumSamples()) % history.getNumSamples();
        repeatsRemaining = 1 + (int)std::floor(repetition * 7.f * random01());
    }
    positionInPhrase = (positionInPhrase + 1) % 8;
}

void WarpCutLiteAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (history.getNumSamples() == 0) return;
    const auto playhead = getPlayHead();
    if (playhead != nullptr)
    {
        const auto pos = playhead->getPosition();
        if (pos.hasValue())
        {
            if (const auto bpm = pos->getBpm(); bpm.hasValue() && *bpm > 20.0 && *bpm < 400.0)
                lastBpm = *bpm;
        }
    }
    const float wet = p("mix") * 0.01f;
    const int channels = buffer.getNumChannels();
    const int historySize = history.getNumSamples();
    const int fadeSamples = juce::jmax(1, (int)(sr * p("fade") * 0.001));
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        if (cutPos >= cutLength || cutLength <= 1)
        {
            if (cutting && repeatsRemaining > 0)
            {
                --repeatsRemaining;
                cutPos = 0;
            }
            else startNewCut(lastBpm, (int)sr);
        }
        for (int ch = 0; ch < channels; ++ch)
        {
            auto* output = buffer.getWritePointer(ch);
            const float dry = output[i];
            history.setSample(ch, writePos, dry);
            float cut = dry;
            if (cutting)
            {
                cut = history.getSample(ch, (sourceStart + cutPos) % historySize);
                // Small crossfade from original at each slice edge to limit clicks.
                const int edge = juce::jmin(cutPos, cutLength - 1 - cutPos);
                const float smooth = juce::jlimit(0.f, 1.f, (float)edge / (float)fadeSamples);
                cut = dry + (cut - dry) * smooth;
            }
            output[i] = dry + wet * (cut - dry);
        }
        ++cutPos;
        writePos = (writePos + 1) % historySize;
        filled = juce::jmin(historySize, filled + 1);
    }
}

juce::AudioProcessorEditor* WarpCutLiteAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void WarpCutLiteAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, dest);
}

void WarpCutLiteAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new WarpCutLiteAudioProcessor(); }
