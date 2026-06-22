#include "PluginEditor.h"

static const juce::Colour kBgDark    { 0xff1a1a2e };
static const juce::Colour kBgPanel   { 0xff16213e };
static const juce::Colour kAccent    { 0xffe94560 };
static const juce::Colour kAccent2   { 0xff0f3460 };
static const juce::Colour kTextLight { 0xffccccdd };

TwinParadoxEditor::TwinParadoxEditor(TwinParadoxProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      drive1Att  (p.apvts, "drive1",    drive1Slider),
      drive2Att  (p.apvts, "drive2",    drive2Slider),
      filter1Att (p.apvts, "filter1fc", filter1Slider),
      filter2Att (p.apvts, "filter2fc", filter2Slider),
      modBlendAtt(p.apvts, "modblend",  modBlendSlider),
      mixAtt     (p.apvts, "mix",       mixSlider)
{
    setupKnob(drive1Slider,   drive1Label,   "Drive 1");
    setupKnob(drive2Slider,   drive2Label,   "Drive 2");
    setupKnob(filter1Slider,  filter1Label,  "Fwd Cut");
    setupKnob(filter2Slider,  filter2Label,  "Fbk Cut");
    setupKnob(modBlendSlider, modBlendLabel, "Mod Blend");
    setupKnob(mixSlider,      mixLabel,      "Mix");

    setSize(500, 330);
}

TwinParadoxEditor::~TwinParadoxEditor() {}

void TwinParadoxEditor::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);
    s.setColour(juce::Slider::thumbColourId,         kAccent);
    s.setColour(juce::Slider::rotarySliderFillColourId, kAccent);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, kAccent2);
    s.setColour(juce::Slider::textBoxTextColourId,   kTextLight);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(s);

    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(12.0f, juce::Font::bold));
    l.setColour(juce::Label::textColourId, kTextLight);
    addAndMakeVisible(l);
}

void TwinParadoxEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // Background
    g.fillAll(kBgDark);

    // Header bar
    g.setColour(kBgPanel);
    g.fillRoundedRectangle(8.0f, 8.0f, bounds.getWidth() - 16.0f, 48.0f, 8.0f);

    g.setColour(kAccent);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("Twin Paradox Distortion", 0, 10, getWidth(), 28, juce::Justification::centred);

    g.setColour(kTextLight.withAlpha(0.5f));
    g.setFont(juce::Font(10.0f));
    g.drawText("Dual-Nested Cross-Modulation Distortion  |  SeiNagi",
               0, 34, getWidth(), 18, juce::Justification::centred);

    // Stage 1 panel
    g.setColour(kAccent2.withAlpha(0.6f));
    g.fillRoundedRectangle(8.0f, 64.0f, 235.0f, 170.0f, 8.0f);
    g.setColour(kAccent.withAlpha(0.8f));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("STAGE 1  (Outer)", 8, 68, 235, 16, juce::Justification::centred);

    // Stage 2 panel
    g.setColour(kAccent2.withAlpha(0.6f));
    g.fillRoundedRectangle(258.0f, 64.0f, 235.0f, 170.0f, 8.0f);
    g.setColour(kAccent.withAlpha(0.8f));
    g.drawText("STAGE 2  (Inner)", 258, 68, 235, 16, juce::Justification::centred);

    // Signal flow arrow between stages
    g.setColour(kTextLight.withAlpha(0.3f));
    const float arrowY = 148.0f;
    g.drawArrow({ 244.0f, arrowY, 258.0f, arrowY }, 2.0f, 8.0f, 6.0f);

    // Bottom global section label
    g.setColour(kTextLight.withAlpha(0.4f));
    g.setFont(juce::Font(10.0f));
    g.drawText("GLOBAL", 0, 242, getWidth(), 14, juce::Justification::centred);
}

void TwinParadoxEditor::resized()
{
    constexpr int kW     = 100;
    constexpr int kH     = 100;
    constexpr int kLblH  = 16;
    constexpr int kYTop  = 82;

    // Stage 1: Drive1 + Filter1 (Fwd Cut)
    drive1Slider.setBounds(18,  kYTop, kW, kH);
    drive1Label .setBounds(18,  kYTop + kH, kW, kLblH);

    filter1Slider.setBounds(128, kYTop, kW, kH);
    filter1Label .setBounds(128, kYTop + kH, kW, kLblH);

    // Stage 2: Drive2 + Filter2 (Fbk Cut)
    drive2Slider.setBounds(268, kYTop, kW, kH);
    drive2Label .setBounds(268, kYTop + kH, kW, kLblH);

    filter2Slider.setBounds(378, kYTop, kW, kH);
    filter2Label .setBounds(378, kYTop + kH, kW, kLblH);

    // Global row: Mod Blend + Mix centered
    constexpr int kYBot = 254;
    modBlendSlider.setBounds(128, kYBot, kW, kH);
    modBlendLabel .setBounds(128, kYBot + kH, kW, kLblH);

    mixSlider.setBounds(268, kYBot, kW, kH);
    mixLabel .setBounds(268, kYBot + kH, kW, kLblH);
}
