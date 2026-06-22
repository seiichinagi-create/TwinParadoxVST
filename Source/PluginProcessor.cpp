#include "PluginProcessor.h"
#include "PluginEditor.h"

TwinParadoxProcessor::TwinParadoxProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameters())
{
}

TwinParadoxProcessor::~TwinParadoxProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout TwinParadoxProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive1",    "Drive 1",    0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive2",    "Drive 2",    0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filter1fc", "Filter 1 Fc",
        juce::NormalisableRange<float>(200.0f, 8000.0f, 1.0f, 0.3f), 4000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filter2fc", "Filter 2 Fc",
        juce::NormalisableRange<float>(200.0f, 8000.0f, 1.0f, 0.3f), 4000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "modblend",  "Mod Blend",  0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix",       "Mix",        0.0f, 1.0f, 1.0f));

    return { params.begin(), params.end() };
}

void TwinParadoxProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dspL.prepare(sampleRate, samplesPerBlock);
    dspR.prepare(sampleRate, samplesPerBlock);
}

void TwinParadoxProcessor::releaseResources()
{
    dspL.reset();
    dspR.reset();
}

void TwinParadoxProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Map normalized drive [0,1] to useful gain range [0.5, 8.0]
    auto mapDrive = [](float v) { return 0.5f + v * 7.5f; };

    const float d1  = mapDrive(apvts.getRawParameterValue("drive1")->load());
    const float d2  = mapDrive(apvts.getRawParameterValue("drive2")->load());
    const float f1  = apvts.getRawParameterValue("filter1fc")->load();
    const float f2  = apvts.getRawParameterValue("filter2fc")->load();
    const float mod = apvts.getRawParameterValue("modblend")->load();
    const float mx  = apvts.getRawParameterValue("mix")->load();

    for (auto* dsp : { &dspL, &dspR })
    {
        dsp->drive1    = d1;
        dsp->drive2    = d2;
        dsp->filter1Fc = f1;
        dsp->filter2Fc = f2;
        dsp->modBlend  = mod;
        dsp->mix       = mx;
    }

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    auto* left  = buffer.getWritePointer(0);
    auto* right = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        left[i] = dspL.processSample(left[i]);
        if (right)
            right[i] = dspR.processSample(right[i]);
    }
}

juce::AudioProcessorEditor* TwinParadoxProcessor::createEditor()
{
    return new TwinParadoxEditor(*this);
}

void TwinParadoxProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TwinParadoxProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TwinParadoxProcessor();
}
