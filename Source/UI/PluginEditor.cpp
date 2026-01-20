#include "../Core/PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_dsp/juce_dsp.h>

TrebleMakerEditor::TrebleMakerEditor (TrebleMakerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      vBlankAttachment(this, [this] { updateCurve(); })
{
    setLookAndFeel(&lookAndFeel);

    setupKnob(freqSlider,  freqLabel,  "FREQ",  PID::freq);
    setupKnob(boostSlider, boostLabel, "BOOST", PID::gain);
    setupKnob(focusSlider, focusLabel, "FOCUS", PID::q);

    reduceButton.setButtonText("REDUCE");
    reduceButton.setClickingTogglesState(true);
    addAndMakeVisible(reduceButton);
    
    bypassButton.setButtonText("bypass");
    bypassButton.setComponentID(PID::bypass);
    bypassButton.setClickingTogglesState(true);
    addAndMakeVisible(bypassButton);

    freqAttachment   = std::make_unique<SliderAttachment>(audioProcessor.apvts, PID::freq,  freqSlider);
    boostAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, PID::gain, boostSlider);
    focusAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, PID::q,    focusSlider);
    reduceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, PID::mode, reduceButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, PID::bypass, bypassButton);
    
    titleLabel.setText("TrebleMaker", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions("Helvetica", 18.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, theme_colors::textDark);
    addAndMakeVisible(titleLabel);

    eqCurve.resize(200, 0.0f);

    setSize (LayoutConstants::width, LayoutConstants::height);
    setResizeLimits(LayoutConstants::width, LayoutConstants::height, 10000, 10000);
    setResizable(true, true);
}

TrebleMakerEditor::~TrebleMakerEditor()
{
    setLookAndFeel(nullptr);
}

void TrebleMakerEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.fillAll(theme_colors::background);
    drawGrid(g, bounds);
    
    auto screenArea = bounds.removeFromTop(bounds.getHeight() * LayoutConstants::screenRatio).reduced(25.0f);
    screenArea.removeFromTop(20.0f); 
    
    drawScreen(g, screenArea);
}

void TrebleMakerEditor::drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(theme_colors::gridLines);
    constexpr float gridSize = 40.0f;
    
    for (float x = 0; x < bounds.getWidth(); x += gridSize)
        g.drawVerticalLine((int)x, 0.0f, bounds.getHeight());
    
    for (float y = 0; y < bounds.getHeight(); y += gridSize)
        g.drawHorizontalLine((int)y, 0.0f, bounds.getWidth());
}

void TrebleMakerEditor::drawScreen(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    juce::ColourGradient bezelGrad(theme_colors::screenBezelStart, 0, bounds.getY(),
                                   theme_colors::screenBezelEnd, 0, bounds.getBottom(), false);
    g.setGradientFill(bezelGrad);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    auto inner = bounds.reduced(10.0f);
    
    juce::Path screenClip;
    screenClip.addRoundedRectangle(inner, 4.0f);
    
    {
        juce::Graphics::ScopedSaveState save(g);
        g.reduceClipRegion(screenClip);
        
        g.setColour(theme_colors::screenBackground);
        g.fillAll(); 
        
        // grid
        g.setColour(juce::Colours::black.withAlpha(0.05f));
        constexpr float cell = 20.0f;
        
        for (float x = inner.getX(); x < inner.getRight(); x += cell)
            g.drawVerticalLine((int)x, inner.getY(), inner.getBottom());
            
        for (float y = inner.getY(); y < inner.getBottom(); y += cell)
            g.drawHorizontalLine((int)y, inner.getX(), inner.getRight());

        if (!eqCurve.empty())
            drawResponseCurve(g, inner);
        
        drawGloss(g, inner);
    }
    
    // inner shadow/inset effect
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawRoundedRectangle(inner, 4.0f, 1.0f);
    
    // outer highlight
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 8.0f, 1.0f);
}

void TrebleMakerEditor::drawResponseCurve(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    juce::Path p;
    
    const float zeroDbY = bounds.getCentreY();
    const float scaleY = bounds.getHeight() / 24.0f; // 12dB range +/-
    
    p.startNewSubPath(bounds.getX(), bounds.getBottom());
    p.lineTo(bounds.getX(), zeroDbY - eqCurve[0] * scaleY);
    
    const float xInc = bounds.getWidth() / (float)(eqCurve.size() - 1);
    
    for (size_t i = 1; i < eqCurve.size(); ++i)
    {
        float x = bounds.getX() + (float)i * xInc;
        float y = zeroDbY - eqCurve[i] * scaleY;
        
        p.lineTo(x, std::clamp(y, bounds.getY(), bounds.getBottom()));
    }
    
    juce::Path fillPath = p;
    fillPath.lineTo(bounds.getRight(), bounds.getBottom());
    fillPath.lineTo(bounds.getX(), bounds.getBottom());
    fillPath.closeSubPath();
    
    auto baseColor = bypassButton.getToggleState() ? juce::Colours::grey : theme_colors::screenRed;

    g.setGradientFill(juce::ColourGradient(baseColor.withAlpha(0.5f), 0, bounds.getBottom(),
                                           baseColor.withAlpha(0.1f), 0, bounds.getY(), false));
    g.fillPath(fillPath);
    
    g.setColour(baseColor);
    g.strokePath(p, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void TrebleMakerEditor::drawGloss(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // top shadow
    g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(0.25f), 0, bounds.getY(),
                                           juce::Colours::transparentBlack, 0, bounds.getY() + 20.0f, false));
    g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 20.0f);
    
    // left edge shadow
    g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(0.2f), bounds.getX(), 0,
                                           juce::Colours::transparentBlack, bounds.getX() + 20.0f, 0, false));
    g.fillRect(bounds.getX(), bounds.getY(), 20.0f, bounds.getHeight());
}

void TrebleMakerEditor::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text, const juce::String& id)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setComponentID(id);
    addAndMakeVisible(s);
    
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(juce::FontOptions("Helvetica", 12.0f, juce::Font::bold)));
    addAndMakeVisible(l);
}

void TrebleMakerEditor::resized()
{
    auto area = getLocalBounds();
    
    titleLabel.setBounds(area.removeFromTop(LayoutConstants::titleHeight).reduced(25, 5));

    area.removeFromBottom(50);
    auto bottomArea = area.removeFromBottom(150).reduced(LayoutConstants::defaultPadding);
    auto buttonArea = bottomArea.removeFromRight(140).reduced(LayoutConstants::defaultPadding, 30);
    
    auto bypassArea = buttonArea.removeFromTop(30);
    buttonArea.removeFromTop(5); 
    
    bypassButton.setBounds(bypassArea.reduced(10, 0)); 
    reduceButton.setBounds(buttonArea);

    const int knobWidth = bottomArea.getWidth() / 3;
    
    auto performLayout = [&](juce::Slider& s, juce::Label& l)
    {
        auto slot = bottomArea.removeFromLeft(knobWidth).reduced(LayoutConstants::defaultPadding);
        s.setBounds(slot);
        l.setBounds(s.getX(), s.getBottom() + 5, s.getWidth(), LayoutConstants::labelHeight);
    };

    performLayout(freqSlider, freqLabel);
    performLayout(boostSlider, boostLabel);
    performLayout(focusSlider, focusLabel);
}

void TrebleMakerEditor::updateCurve()
{
    constexpr float alpha = 0.15f;
    smoothFreq  += ((float)freqSlider.getValue()  - smoothFreq)  * alpha;
    smoothBoost += ((float)boostSlider.getValue() - smoothBoost) * alpha;
    smoothFocus += ((float)focusSlider.getValue() - smoothFocus) * alpha;
    
    bool isReduce = reduceButton.getToggleState();
    
    double sampleRate = 44100.0; 
    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, smoothFreq, juce::jmax(0.1f, smoothFocus));

    float gainVal = smoothBoost; 
    float linearGain = juce::Decibels::decibelsToGain(gainVal);
    
    std::vector<double> freqs;
    freqs.reserve(eqCurve.size());
    for (size_t i = 0; i < eqCurve.size(); ++i)
    {
        // logarithmic scale 20Hz-20kHz
        freqs.push_back(20.0 * std::pow(1000.0, (double)i / (double)eqCurve.size()));
    }
    
    std::vector<double> mags;
    mags.resize(freqs.size());
    
    coeffs->getMagnitudeForFrequencyArray(freqs.data(), mags.data(), freqs.size(), sampleRate);
    
    for (size_t i = 0; i < eqCurve.size(); ++i)
    {
        float mag = (float)mags[i];
        float db = 0.0f;
        
        if (!isReduce)
        {
            db = juce::Decibels::gainToDecibels(1.0f + mag * (linearGain - 1.0f));
        }
        else
        {
            float combined = 1.0f - mag * linearGain;
            db = juce::Decibels::gainToDecibels(std::abs(combined));
        }
        
        // animation
        float wave = std::sin(phase + (float)i * 0.3f) * 0.15f;
        eqCurve[i] = db + wave;
    }
    
    if (!bypassButton.getToggleState())
        phase += 0.05f;
    
    const auto text = reduceButton.getToggleState() ? "BOOST" : "REDUCE";
    if (reduceButton.getButtonText() != text)
        reduceButton.setButtonText(text); 
        
    repaint();
}
