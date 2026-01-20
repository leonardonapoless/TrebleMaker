#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_core/juce_core.h>

namespace theme_colors
{
    extern const juce::Colour background;
    extern const juce::Colour gridLines;
    extern const juce::Colour textDark;
    extern const juce::Colour textLight;
    
    extern const juce::Colour screenBezelStart;
    extern const juce::Colour screenBezelEnd;
    
    extern const juce::Colour screenBackground;
    extern const juce::Colour screenRed;
    extern const juce::Colour knobTick;
    extern const juce::Colour buttonBlue;
}

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                              const juce::Colour&,
                              bool shouldDrawButtonAsHighlighted,  
                              bool shouldDrawButtonAsDown) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&, bool isMouseOverButton, bool isButtonDown) override;

    juce::Font getTextButtonFont(juce::TextButton&, int) override;
};
