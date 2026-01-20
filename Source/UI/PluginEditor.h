#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_graphics/juce_graphics.h>
#include "../Core/PluginProcessor.h"
#include "LookAndFeel.h"

namespace LayoutConstants
{
    const int width = 600;
    const int height = 450;
    const int titleHeight = 45;
    const float screenRatio = 0.55f;
    const int defaultPadding = 10;
    const int labelHeight = 20;
}

class TrebleMakerEditor : public juce::AudioProcessorEditor
{
public:
    TrebleMakerEditor (TrebleMakerAudioProcessor&);
    ~TrebleMakerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TrebleMakerAudioProcessor& audioProcessor;
    
    LookAndFeel lookAndFeel;

    juce::Slider freqSlider;
    juce::Slider boostSlider;
    juce::Slider focusSlider;
    juce::TextButton reduceButton;
    
    juce::Label freqLabel;
    juce::Label boostLabel;
    juce::Label focusLabel;
    juce::Label titleLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> freqAttachment;
    std::unique_ptr<SliderAttachment> boostAttachment;
    std::unique_ptr<SliderAttachment> focusAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reduceAttachment;
    
    // animation
    juce::VBlankAttachment vBlankAttachment;
    
    // curve Data
    std::vector<float> eqCurve;
    float phase = 0.0f;
    
    // smoothing state
    float smoothFreq = 1000.0f;
    float smoothBoost = 0.0f;
    float smoothFocus = 0.5f;
    
    void updateCurve();

    
    // drawing helpers
    void drawScreen(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawResponseCurve(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGloss(juce::Graphics& g, juce::Rectangle<float> bounds);

    void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text, const juce::String& id);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrebleMakerEditor)
};
