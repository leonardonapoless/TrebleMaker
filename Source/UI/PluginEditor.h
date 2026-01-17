#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_graphics/juce_graphics.h>
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
    
    void updateCurve();
    void drawScreen(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrebleMakerEditor)
};
