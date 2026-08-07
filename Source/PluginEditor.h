#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "vendor/SnLookAndFeel.h"

class TwinParadoxEditor : public juce::AudioProcessorEditor
{
public:
    explicit TwinParadoxEditor(TwinParadoxProcessor&);
    ~TwinParadoxEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    sn::KnobLookAndFeel snLaf;   // ★メンバの先頭=最後に壊れる(子より長生きさせる)
    TwinParadoxProcessor& processor;

    juce::Slider drive1Slider, drive2Slider;
    juce::Slider filter1Slider, filter2Slider;
    juce::Slider modBlendSlider, mixSlider;

    juce::Label drive1Label, drive2Label;
    juce::Label filter1Label, filter2Label;
    juce::Label modBlendLabel, mixLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    Attachment drive1Att, drive2Att, filter1Att, filter2Att, modBlendAtt, mixAtt;

    void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TwinParadoxEditor)
};
