#pragma once

#include <JuceHeader.h>

namespace theme_colors
{
    const auto background    = juce::Colour::fromString("FFF2F2F2"); // off-white
    const auto gridLines     = juce::Colour::fromString("FFD0D0D0"); // subtle dark grey
    const auto textDark      = juce::Colour::fromString("FF222222"); 
    const auto textLight     = juce::Colour::fromString("FF666666"); 
    
    const auto screenBezelStart = juce::Colour::fromString("FFCCCCCC");
    const auto screenBezelEnd   = juce::Colour::fromString("FFF0F0F0");
    
    const auto screenBackground = juce::Colour::fromString("FFE8E0E0"); // light grey/pinkish
    const auto screenRed        = juce::Colour::fromString("FFFF3B30"); // vivid red
    const auto knobTick         = juce::Colour::fromString("FFFF3B30"); 
    const auto buttonBlue       = juce::Colour::fromString("FF101010"); // black
}

class IndustrialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    IndustrialLookAndFeel()
    {
        setColour(juce::Label::textColourId, theme_colors::textDark);
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }

    // "aluminum" knob
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 4.0f;
        auto center = bounds.getCentre();
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // drop shadow
        g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(0.35f), center.x, center.y + radius,
                                               juce::Colours::transparentBlack, center.x, center.y + radius + 8.0f, false));
        g.fillEllipse(center.x - radius, center.y - radius + 3.0f, radius * 2.0f, radius * 2.0f);

        // body
        juce::ColourGradient conicalGrad(juce::Colour::fromString("FFEEEEEE"), center.x, center.y,
                                         juce::Colour::fromString("FF999999"), center.x + radius, center.y + radius, true);
        conicalGrad.addColour(0.0, juce::Colour::fromString("FFCCCCCC"));
        conicalGrad.addColour(0.2, juce::Colour::fromString("FFFFFFFF"));
        conicalGrad.addColour(0.4, juce::Colour::fromString("FFAAAAAA"));
        conicalGrad.addColour(0.6, juce::Colour::fromString("FFDDDDDD"));
        conicalGrad.addColour(0.8, juce::Colour::fromString("FF999999"));
        conicalGrad.addColour(1.0, juce::Colour::fromString("FFCCCCCC"));
        
        g.setGradientFill(conicalGrad);
        g.fillEllipse(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);

        // milled texture
        g.setColour(juce::Colours::black.withAlpha(0.08f));
        for (float r = 4.0f; r < radius; r += 2.5f)
        {
            g.drawEllipse(center.x - r, center.y - r, r * 2.0f, r * 2.0f, 1.0f);
        }

        // top face
        auto faceRadius = radius * 0.9f;
        
        // subtle gradient for the top face
        juce::ColourGradient topGrad(juce::Colour::fromString("FFF5F5F5"), center.x - faceRadius, center.y - faceRadius,
                                     juce::Colour::fromString("FFBBBBBB"), center.x + faceRadius, center.y + faceRadius, false);
        g.setGradientFill(topGrad);
        g.fillEllipse(center.x - faceRadius, center.y - faceRadius, faceRadius * 2.0f, faceRadius * 2.0f);
        
        // bevel
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.drawEllipse(center.x - faceRadius, center.y - faceRadius, faceRadius * 2.0f, faceRadius * 2.0f, 1.5f);

        // pointer
        juce::Path p;
        auto tickW = 3.0f;
        auto tickH = faceRadius * 0.35f;
        p.addRectangle(-tickW * 0.5f, -faceRadius * 0.85f, tickW, tickH);
        
        // rotate
        p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(center.x, center.y));
        
        // tick shadow (for depth)
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillPath(p, juce::AffineTransform::translation(0.5f, 1.0f));

        // tick fill
        g.setColour(theme_colors::knobTick);
        g.fillPath(p);
    }
    
    // button
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, 
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(4.0f);
        auto cornerSize = bounds.getHeight() * 0.5f; // full pill shape
        bool isDown = button.getToggleState() || shouldDrawButtonAsDown;
        
        // drop shadow
        if (!isDown)
        {
            g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(0.4f), 0, bounds.getBottom(),
                                                   juce::Colours::transparentBlack, 0, bounds.getBottom() + 5.0f, false));
            g.fillRoundedRectangle(bounds.translated(0, 2.0f), cornerSize);
        }
        
        // metal bezel (outer ring)
        juce::ColourGradient bezelGrad(juce::Colour::fromString("FF444444"), 0, bounds.getY(),
                                       juce::Colour::fromString("FF222222"), 0, bounds.getBottom(), false);
        bezelGrad.addColour(0.5, juce::Colour::fromString("FF666666")); // Highlight in middle
        g.setGradientFill(bezelGrad);
        g.fillRoundedRectangle(bounds, cornerSize);
        
        // inner body (actual button surface)
        auto inner = bounds.reduced(2.0f);
        
        // gradient for the body
        auto topCol = juce::Colour::fromString("FF2B2B2B");
        auto botCol = juce::Colour::fromString("FF1A1A1A");
        
        if (isDown)
        {
            // darker and inverted gradient when pressed
            std::swap(topCol, botCol);
            topCol = topCol.darker(0.2f);
            botCol = botCol.darker(0.2f);
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            topCol = topCol.brighter(0.1f);
            botCol = botCol.brighter(0.1f);
        }
        
        juce::ColourGradient bodyGrad(topCol, 0, inner.getY(), botCol, 0, inner.getBottom(), false);
        g.setGradientFill(bodyGrad);
        g.fillRoundedRectangle(inner, cornerSize);
        
        // inner shadow/depth (inset effect)
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRoundedRectangle(inner, cornerSize, 1.0f);
        
        if (isDown)
        {
            // strong inner shadow at top to show it's pressed in
            g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(0.6f), 0, inner.getY(),
                                                   juce::Colours::transparentBlack, 0, inner.getY() + 10.0f, false));
            g.fillRoundedRectangle(inner, cornerSize);
        }
        else
        {
            // specular highlight (glossy top)
            // sharp white reflection at the top
            juce::Path highlight;
            auto highlightBounds = inner.removeFromTop(inner.getHeight() * 0.45f).reduced(2.0f, 0);
            highlight.addRoundedRectangle(highlightBounds, cornerSize);
            
            g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.15f), 0, highlightBounds.getY(),
                                                   juce::Colours::white.withAlpha(0.0f), 0, highlightBounds.getBottom(), false));
            g.fillPath(highlight);
            
            // rim light at the bottom
            g.setGradientFill(juce::ColourGradient(juce::Colours::transparentWhite, 0, inner.getBottom() - 5.0f,
                                                   juce::Colours::white.withAlpha(0.1f), 0, inner.getBottom(), false));
            g.fillRoundedRectangle(inner, cornerSize);
        }
    }
    
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        drawButtonBackground(g, button, theme_colors::buttonBlue, shouldDrawButtonAsHighlighted, button.getToggleState() || shouldDrawButtonAsDown);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions("Helvetica", 14.0f, juce::Font::bold)));
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
    }
    
    // override text button to use the custom background
    // override font for text buttons
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return juce::Font(juce::FontOptions("Helvetica", 13.0f, juce::Font::bold));
    }
};
