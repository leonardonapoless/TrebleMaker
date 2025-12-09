#include "../Core/PluginProcessor.h"
#include "PluginEditor.h"
#include <complex>
#include <cmath>

TrebleMakerEditor::TrebleMakerEditor (TrebleMakerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      vBlankAttachment(this, [this] { updateCurve(); })
{
    setLookAndFeel(&industrialLookAndFeel);

    auto setupSlider = [this](juce::Slider& s, const juce::String& id, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(s);
        
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setFont(juce::Font(juce::FontOptions("Helvetica", 12.0f, juce::Font::bold)));
        addAndMakeVisible(l);
    };

    setupSlider(freqSlider,  "freq",  freqLabel,  "FREQ");
    setupSlider(boostSlider, "boost", boostLabel, "BOOST");
    setupSlider(focusSlider, "focus", focusLabel, "FOCUS");

    reduceButton.setButtonText("REDUCE");
    reduceButton.setClickingTogglesState(true);
    addAndMakeVisible(reduceButton);

    freqAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "freq",  freqSlider);
    boostAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "gain", boostSlider);
    focusAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "q", focusSlider);
    reduceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "mode", reduceButton);
    
    titleLabel.setText("TrebleMaker", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions("Helvetica", 18.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, theme_colors::textDark);
    addAndMakeVisible(titleLabel);

    // initialize Curve
    eqCurve.resize(200, 0.0f);

    // window
    setSize (600, 450);
    setResizeLimits(600, 450, 10000, 10000);
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
    
    auto screenArea = bounds.removeFromTop(bounds.getHeight() * 0.55f).reduced(25.0f);
    screenArea.removeFromTop(20.0f); // Space for title
    
    drawScreen(g, screenArea);
}

void TrebleMakerEditor::drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(theme_colors::gridLines);
    float gridSize = 40.0f;
    
    // draw grid
    for (float x = 0; x < bounds.getWidth(); x += gridSize)
        g.drawVerticalLine((int)x, 0.0f, bounds.getHeight());
        
    for (float y = 0; y < bounds.getHeight(); y += gridSize)
        g.drawHorizontalLine((int)y, 0.0f, bounds.getWidth());
}

void TrebleMakerEditor::drawScreen(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // bezel
    juce::ColourGradient bezelGrad(theme_colors::screenBezelStart, 0, bounds.getY(),
                                   theme_colors::screenBezelEnd, 0, bounds.getBottom(), false);
    g.setGradientFill(bezelGrad);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    // inner screen rect
    auto inner = bounds.reduced(10.0f); // thick bezel
    
    g.setColour(theme_colors::screenBackground);
    g.fillRoundedRectangle(inner, 4.0f);
    
    // inner grid 
    g.saveState();
    g.reduceClipRegion(inner.toNearestInt());
    g.setColour(juce::Colours::black.withAlpha(0.05f));
    float gridSize = 20.0f;
    for (float x = inner.getX(); x < inner.getRight(); x += gridSize)
        g.drawVerticalLine((int)x, inner.getY(), inner.getBottom());
    for (float y = inner.getY(); y < inner.getBottom(); y += gridSize)
        g.drawHorizontalLine((int)y, inner.getX(), inner.getRight());
        
    // inner shadow (top and left) for recessed look
    // top shadow
    g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(0.25f), 0, inner.getY(),
                                           juce::Colours::transparentBlack, 0, inner.getY() + 20.0f, false));
    g.fillRect(inner.getX(), inner.getY(), inner.getWidth(), 20.0f);
    
    // left shadow
    g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(0.2f), inner.getX(), 0,
                                           juce::Colours::transparentBlack, inner.getX() + 20.0f, 0, false));
    g.fillRect(inner.getX(), inner.getY(), 20.0f, inner.getHeight());
    
    // EQ Curve
    if (!eqCurve.empty())
    {
        juce::Path p;
        
        float zeroDbY = inner.getCentreY();
        float scaleY = inner.getHeight() / 24.0f; // +/- 12dB range
        
        p.startNewSubPath(inner.getX(), inner.getBottom());
        p.lineTo(inner.getX(), zeroDbY - eqCurve[0] * scaleY); // first point
        
        for (size_t i = 1; i < eqCurve.size(); ++i)
        {
            float x = juce::jmap((float)i, 0.0f, (float)eqCurve.size() - 1.0f, inner.getX(), inner.getRight());
            float db = eqCurve[i];
            float y = zeroDbY - db * scaleY;
            
            // clamp to screen
            y = juce::jlimit(inner.getY(), inner.getBottom(), y);
            
            // smooth curve to avoid jaggedness
            // using simple lineTo for now, but points are dense enough (200)
            p.lineTo(x, y);
        }
        
        p.lineTo(inner.getRight(), inner.getBottom());
        p.closeSubPath();
        
        g.setGradientFill(juce::ColourGradient(theme_colors::screenRed.withAlpha(0.5f), 0, inner.getBottom(),
                                               theme_colors::screenRed.withAlpha(0.1f), 0, inner.getY(), false));
        g.fillPath(p);
        
        juce::Path strokePath;
        strokePath.startNewSubPath(inner.getX(), zeroDbY - eqCurve[0] * scaleY);
        for (size_t i = 1; i < eqCurve.size(); ++i)
        {
            float x = juce::jmap((float)i, 0.0f, (float)eqCurve.size() - 1.0f, inner.getX(), inner.getRight());
            float db = eqCurve[i];
            float y = zeroDbY - db * scaleY;
            y = juce::jlimit(inner.getY(), inner.getBottom(), y);
            strokePath.lineTo(x, y);
        }
        
        g.setColour(theme_colors::screenRed);
        g.strokePath(strokePath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    
    g.restoreState();
    
    // bezel inner border (highlight)
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 8.0f, 1.0f);
    
    // screen frame border 
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawRoundedRectangle(inner, 4.0f, 1.0f);
}

void TrebleMakerEditor::resized()
{
    auto bounds = getLocalBounds();
    
    titleLabel.setBounds(25, 15, 200, 30);
    
    // knobs area
    auto bottomArea = bounds.removeFromBottom(150);
    
    int knobSize = 90;
    int gap = 30;
    int startX = 50;
    int y = bottomArea.getY() + 10;
    
    auto setKnob = [&](juce::Slider& s, juce::Label& l, int index)
    {
        s.setBounds(startX + index * (knobSize + gap), y, knobSize, knobSize);
        l.setBounds(s.getX(), s.getBottom() + 5, knobSize, 20);
    };
    
    setKnob(freqSlider, freqLabel, 0);
    setKnob(boostSlider, boostLabel, 1);
    setKnob(focusSlider, focusLabel, 2);
    
    // button
    // to the right of the knobs
    reduceButton.setBounds(startX + 3 * (knobSize + gap) + 20, y + 25, 120, 40);
}

void TrebleMakerEditor::updateCurve()
{
    // smooth parameters for elastic animation
    static float smoothFreq = 1000.0f;
    static float smoothBoost = 0.0f;
    static float smoothFocus = 0.5f; // Q
    
    float targetFreq = freqSlider.getValue();
    float targetBoost = boostSlider.getValue();
    float targetFocus = focusSlider.getValue();
    bool isReduce = reduceButton.getToggleState();
    
    // simple smoothing coefficient
    float alpha = 0.15f;
    
    smoothFreq += (targetFreq - smoothFreq) * alpha;
    smoothBoost += (targetBoost - smoothBoost) * alpha;
    smoothFocus += (targetFocus - smoothFocus) * alpha;
    
    double sampleRate = 44100.0;
    double w0 = 2.0 * juce::MathConstants<double>::pi * smoothFreq / sampleRate;
    double Q = juce::jmax(0.1f, smoothFocus);
    double alpha_filter = std::sin(w0) / (2.0 * Q);
    double cos_w0 = std::cos(w0);
    
    // RBJ highpass coefficients (to match TPT Highpass topology roughly)
    double b0 =  (1.0 + cos_w0) / 2.0;
    double b1 = -(1.0 + cos_w0);
    double b2 =  (1.0 + cos_w0) / 2.0;
    double a0 =   1.0 + alpha_filter;
    double a1 =  -2.0 * cos_w0;
    double a2 =   1.0 - alpha_filter;

    // normalize
    b0 /= a0; b1 /= a0; b2 /= a0;
    a1 /= a0; a2 /= a0;
    
    float gainVal = smoothBoost; // dB
    float linearGain = juce::Decibels::decibelsToGain(gainVal);
    
    // calculate magnitude for each point
    for (size_t i = 0; i < eqCurve.size(); ++i)
    {
        double f = 20.0 * std::pow(1000.0, (double)i / (double)eqCurve.size()); // log scale 20Hz to 20kHz
        if (f > sampleRate / 2.0) f = sampleRate / 2.0;
        
        double w = 2.0 * juce::MathConstants<double>::pi * f / sampleRate;
        
        // H_hp(z)
        std::complex<double> z = std::polar(1.0, w);
        std::complex<double> h_hp = (b0 + b1 / z + b2 / (z * z)) / (1.0 + a1 / z + a2 / (z * z));
        
        std::complex<double> h_total;
        
        if (!isReduce)
        {
            // boost: dry + HP * (G - 1)
            h_total = 1.0 + h_hp * ((double)linearGain - 1.0);
        }
        else
        {
            // reduce: dry - HP * G
            h_total = 1.0 - h_hp * (double)linearGain;
        }
        
        double mag = std::abs(h_total);
        eqCurve[i] = (float)juce::Decibels::gainToDecibels(mag);
        
        // wave animation
        // add a subtle ripple that moves
        float wave = std::sin(phase + (float)i * 0.3f) * 0.15f; // reduced amplitude, higher frequency
        eqCurve[i] += wave;
    }
    
    phase += 0.05f; // slower speed
    
    // Update button text
    if (reduceButton.getToggleState())
        reduceButton.setButtonText("BOOST");
    else
        reduceButton.setButtonText("REDUCE");
    
    // repaint only the screen area to save CPU
    repaint();
}
