#pragma once

#include <JuceHeader.h>
#include "../Core/PluginProcessor.h"
#include "LookAndFeel.h"

class TrebleMakerEditor : public juce::AudioProcessorEditor
{
public:
    TrebleMakerEditor (TrebleMakerAudioProcessor&);
    ~TrebleMakerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TrebleMakerAudioProcessor& audioProcessor;
    
    IndustrialLookAndFeel industrialLookAndFeel;

    // controls
    juce::Slider freqSlider;
    juce::Slider boostSlider;
    juce::Slider focusSlider;
    juce::TextButton reduceButton;
    
    // labels
    juce::Label freqLabel;
    juce::Label boostLabel;
    juce::Label focusLabel;
    juce::Label titleLabel;

    // attachments
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
    
    void updateCurve();
    void drawScreen(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrebleMakerEditor)
};
