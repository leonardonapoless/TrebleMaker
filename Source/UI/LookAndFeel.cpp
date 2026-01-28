#include "LookAndFeel.h"

namespace theme_colors {
const juce::Colour background = juce::Colour(0xFFF2F2F2);
const juce::Colour gridLines = juce::Colour(0xFFD0D0D0);
const juce::Colour textDark = juce::Colour(0xFF222222);
const juce::Colour textLight = juce::Colour(0xFF666666);

const juce::Colour screenBezelStart = juce::Colour(0xFFCCCCCC);
const juce::Colour screenBezelEnd = juce::Colour(0xFFF0F0F0);

const juce::Colour screenBackground = juce::Colour(0xFFE8E0E0);
const juce::Colour screenRed = juce::Colour(0xFFFF3B30);
const juce::Colour knobTick = juce::Colour(0xFFFF3B30);
const juce::Colour buttonBlue = juce::Colour(0xFF101010);

const juce::Colour knobBodyLight = juce::Colour(0xFFF0F0F0);
const juce::Colour knobBodyDark = juce::Colour(0xFFB0B0B0);
const juce::Colour knobBodyMid = juce::Colour(0xFFD0D0D0);
const juce::Colour knobFaceNormal = juce::Colour(0xFFE0E0E0);
const auto knobFaceShadow = juce::Colour(0xFFBBBBBB);
} 

LookAndFeel::LookAndFeel() {
  setColour(juce::Label::textColourId, theme_colors::textDark);
  setColour(juce::TextButton::textColourOffId, juce::Colours::white);
  setColour(juce::TextButton::textColourOnId, juce::Colours::white);
}

void LookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                                   int height, float sliderPos,
                                   const float rotaryStartAngle,
                                   const float rotaryEndAngle, juce::Slider &) {
  auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
  auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 4.0f;
  auto center = bounds.getCentre();
  auto toAngle =
      rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

  g.setGradientFill(
      juce::ColourGradient(juce::Colours::black.withAlpha(0.35f), center.x,
                           center.y + radius, juce::Colours::transparentBlack,
                           center.x, center.y + radius + 8.0f, false));
  g.fillEllipse(center.x - radius, center.y - radius + 3.0f, radius * 2.0f,
                radius * 2.0f);
  juce::ColourGradient conicalGrad(theme_colors::knobBodyLight, center.x,
                                   center.y, theme_colors::knobBodyDark,
                                   center.x + radius, center.y + radius, true);

  conicalGrad.addColour(0.0, theme_colors::knobBodyMid);
  conicalGrad.addColour(0.2, juce::Colours::white);
  conicalGrad.addColour(0.4, juce::Colour(0xFFAAAAAA));
  conicalGrad.addColour(0.6, juce::Colour(0xFFDDDDDD));
  conicalGrad.addColour(0.8, theme_colors::knobBodyDark);
  conicalGrad.addColour(1.0, theme_colors::knobBodyMid);

  g.setGradientFill(conicalGrad);
  g.fillEllipse(center.x - radius, center.y - radius, radius * 2.0f,
                radius * 2.0f);

  g.setColour(juce::Colours::black.withAlpha(0.08f));
  for (float r = 4.0f; r < radius; r += 2.5f) {
    g.drawEllipse(center.x - r, center.y - r, r * 2.0f, r * 2.0f, 1.0f);
  }

  auto faceRadius = radius * 0.9f;

  juce::ColourGradient topGrad(
      theme_colors::knobFaceNormal, center.x - faceRadius,
      center.y - faceRadius, theme_colors::knobFaceShadow,
      center.x + faceRadius, center.y + faceRadius, false);
  g.setGradientFill(topGrad);
  g.fillEllipse(center.x - faceRadius, center.y - faceRadius, faceRadius * 2.0f,
                faceRadius * 2.0f);

  g.setColour(juce::Colours::white.withAlpha(0.6f));
  g.drawEllipse(center.x - faceRadius, center.y - faceRadius, faceRadius * 2.0f,
                faceRadius * 2.0f, 1.5f);

  juce::Path p;
  auto tickW = 3.0f;
  auto tickH = faceRadius * 0.35f;
  p.addRectangle(-tickW * 0.5f, -faceRadius * 0.85f, tickW, tickH);

  p.applyTransform(
      juce::AffineTransform::rotation(toAngle).translated(center.x, center.y));

  g.setColour(juce::Colours::black.withAlpha(0.3f));
  g.fillPath(p, juce::AffineTransform::translation(0.5f, 1.0f));

  g.setColour(theme_colors::knobTick);
  g.fillPath(p);
}

void LookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                       const juce::Colour &,
                                       bool shouldDrawButtonAsHighlighted,
                                       bool shouldDrawButtonAsDown) {
  auto bounds = button.getLocalBounds().toFloat().reduced(4.0f);
  auto cornerSize = bounds.getHeight() * 0.5f;
  bool isDown = button.getToggleState() || shouldDrawButtonAsDown;

  if (button.getComponentID() == "bypass") {
    bool pressed = button.getToggleState() || isDown;
    auto baseColour = juce::Colour(0xFFF4F4F4);

    if (pressed) {
      g.setColour(baseColour.darker(0.1f));
      g.fillRoundedRectangle(bounds, cornerSize);

      g.setGradientFill(juce::ColourGradient(
          juce::Colours::black.withAlpha(0.2f), 0, bounds.getY(),
          juce::Colours::transparentBlack, 0, bounds.getY() + 5.0f, false));
      g.fillRoundedRectangle(bounds, cornerSize);

      g.setGradientFill(juce::ColourGradient(
          juce::Colours::transparentWhite, 0, bounds.getBottom() - 2.0f,
          juce::Colours::white.withAlpha(0.3f), 0, bounds.getBottom(), false));
      g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    } else {
      g.setGradientFill(juce::ColourGradient(
          juce::Colours::black.withAlpha(0.35f), 0, bounds.getBottom(),
          juce::Colours::transparentBlack, 0, bounds.getBottom() + 5.0f,
          false));
      g.fillRoundedRectangle(bounds.translated(0, 2.0f), cornerSize);

      g.setGradientFill(juce::ColourGradient(baseColour, 0, bounds.getY(),
                                             baseColour.darker(0.05f), 0,
                                             bounds.getBottom(), false));
      g.fillRoundedRectangle(bounds, cornerSize);

      g.setGradientFill(juce::ColourGradient(
          juce::Colours::white.withAlpha(0.9f), 0, bounds.getY(),
          juce::Colours::transparentWhite, 0, bounds.getY() + 2.0f, false));
      g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    }
    return;
  }

  if (!isDown) {
    g.setGradientFill(juce::ColourGradient(
        juce::Colours::black.withAlpha(0.4f), 0, bounds.getBottom(),
        juce::Colours::transparentBlack, 0, bounds.getBottom() + 5.0f, false));
    g.fillRoundedRectangle(bounds.translated(0, 2.0f), cornerSize);
  }

  juce::ColourGradient bezelGrad(juce::Colour(0xFF444444), 0, bounds.getY(),
                                 juce::Colour(0xFF222222), 0,
                                 bounds.getBottom(), false);
  bezelGrad.addColour(0.5, juce::Colour(0xFF666666));
  g.setGradientFill(bezelGrad);
  g.fillRoundedRectangle(bounds, cornerSize);

  auto inner = bounds.reduced(2.0f);

  auto topCol = juce::Colour(0xFF2B2B2B);
  auto botCol = juce::Colour(0xFF1A1A1A);

  if (isDown) {
    std::swap(topCol, botCol);
    topCol = topCol.darker(0.2f);
    botCol = botCol.darker(0.2f);
  } else if (shouldDrawButtonAsHighlighted) {
    topCol = topCol.brighter(0.1f);
    botCol = botCol.brighter(0.1f);
  }

  juce::ColourGradient bodyGrad(topCol, 0, inner.getY(), botCol, 0,
                                inner.getBottom(), false);
  g.setGradientFill(bodyGrad);
  g.fillRoundedRectangle(inner, cornerSize);

  g.setColour(juce::Colours::black.withAlpha(0.5f));
  g.drawRoundedRectangle(inner, cornerSize, 1.0f);

  if (isDown) {
    g.setGradientFill(juce::ColourGradient(
        juce::Colours::black.withAlpha(0.6f), 0, inner.getY(),
        juce::Colours::transparentBlack, 0, inner.getY() + 10.0f, false));
    g.fillRoundedRectangle(inner, cornerSize);
  } else {
    juce::Path highlight;
    auto highlightBounds =
        inner.removeFromTop(inner.getHeight() * 0.45f).reduced(2.0f, 0);
    highlight.addRoundedRectangle(highlightBounds, cornerSize);

    g.setGradientFill(juce::ColourGradient(
        juce::Colours::white.withAlpha(0.15f), 0, highlightBounds.getY(),
        juce::Colours::white.withAlpha(0.0f), 0, highlightBounds.getBottom(),
        false));
    g.fillPath(highlight);

    g.setGradientFill(juce::ColourGradient(
        juce::Colours::transparentWhite, 0, inner.getBottom() - 5.0f,
        juce::Colours::white.withAlpha(0.1f), 0, inner.getBottom(), false));
    g.fillRoundedRectangle(inner, cornerSize);
  }
}

void LookAndFeel::drawButtonText(juce::Graphics &g, juce::TextButton &button,
                                 bool, bool) {
  g.setFont(getTextButtonFont(button, button.getHeight()));
  g.setColour(button.findColour(juce::TextButton::textColourOffId));

  auto textBounds = button.getLocalBounds();

  if (button.getComponentID() == "bypass") {
    g.setColour(juce::Colour(0xFF222222));

    if (button.getToggleState() || button.isDown()) {
      textBounds.translate(0, 1);
    }
  }

  g.drawText(button.getButtonText(), textBounds, juce::Justification::centred);
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &button,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown) {
  drawButtonBackground(g, button, theme_colors::buttonBlue,
                       shouldDrawButtonAsHighlighted,
                       button.getToggleState() || shouldDrawButtonAsDown);

  g.setColour(juce::Colours::white);
  g.setFont(
      juce::Font(juce::FontOptions("Helvetica", 14.0f, juce::Font::bold)));
  g.drawText(button.getButtonText(), button.getLocalBounds(),
             juce::Justification::centred, true);
}

juce::Font LookAndFeel::getTextButtonFont(juce::TextButton &, int) {
  return juce::Font(juce::FontOptions("Helvetica", 13.0f, juce::Font::bold));
}
