#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Forward declarations
class ReverseReverbAudioProcessor;

// Modern 3D LookAndFeel with advanced visual effects
// Features: Glass morphism, depth shadows, gradient lighting, specular highlights
class Modern3DLookAndFeel : public juce::LookAndFeel_V4
{
public:
 Modern3DLookAndFeel()
 {
 // Modern color scheme - deep space theme
 setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff0a0a0f));
 setColour(juce::Slider::thumbColourId, juce::Colour(0xffff006e));
 setColour(juce::Slider::trackColourId, juce::Colour(0xffb537f2));
 }
 
 // Animation phase for pulsing effects
 void setAnimationPhase(float phase) { animationPhase = phase; }
 
 // Helper methods
 void drawMultiLayerGlow(juce::Graphics& g, float centerX, float centerY, 
 float radius, const juce::Colour& colour, 
 float intensity, int layers)
 {
 for (int i = layers; i >= 1; --i)
 {
 float glowRadius = radius + i * 4.0f * intensity;
 float alpha = (0.15f * intensity) / i;
 g.setColour(colour.withAlpha(alpha));
 g.fillEllipse(centerX - glowRadius, centerY - glowRadius,
 glowRadius * 2.0f, glowRadius * 2.0f);
 }
 }
 
 void drawGlowingDot(juce::Graphics& g, float centerX, float centerY,
 float dotRadius, const juce::Colour& colour, float intensity)
 {
 for (int i = 3; i >= 1; --i)
 {
 float glowSize = dotRadius + i * 2.0f;
 float alpha = (0.3f * intensity) / i;
 g.setColour(colour.withAlpha(alpha));
 g.fillEllipse(centerX - glowSize, centerY - glowSize,
 glowSize * 2.0f, glowSize * 2.0f);
 }
 
 juce::ColourGradient dotGradient(
 juce::Colours::white.withAlpha(0.9f),
 centerX, centerY - dotRadius * 0.5f,
 colour,
 centerX, centerY + dotRadius * 0.5f,
 false
 );
 g.setGradientFill(dotGradient);
 g.fillEllipse(centerX - dotRadius, centerY - dotRadius,
 dotRadius * 2.0f, dotRadius * 2.0f);
 }
 
 void draw3DPointer(juce::Graphics& g, float centerX, float centerY,
 float radius, float angle, const juce::Colour& colour, float intensity)
 {
 auto pointerLength = radius * 0.55f;
 auto pointerThickness = 4.0f;
 
 juce::Path pointer;
 pointer.addRectangle(-pointerThickness * 0.5f, -radius + 8, 
 pointerThickness, pointerLength);
 pointer.applyTransform(juce::AffineTransform::rotation(angle)
 .translated(centerX, centerY));
 
 juce::Path shadowPointer = pointer;
 shadowPointer.applyTransform(juce::AffineTransform::translation(2, 2));
 g.setColour(juce::Colours::black.withAlpha(0.3f));
 g.fillPath(shadowPointer);
 
 for (int i = 2; i >= 1; --i)
 {
 juce::Path glowPointer = pointer;
 glowPointer.applyTransform(juce::AffineTransform::scale(1.0f + i * 0.1f, 1.0f + i * 0.1f, 
 centerX, centerY));
 float alpha = (0.3f * intensity) / i;
 g.setColour(colour.withAlpha(alpha));
 g.fillPath(glowPointer);
 }
 
 juce::ColourGradient pointerGradient(
 juce::Colours::white.withAlpha(0.95f),
 centerX, centerY - radius,
 colour.brighter(0.2f),
 centerX, centerY - radius + pointerLength,
 false
 );
 g.setGradientFill(pointerGradient);
 g.fillPath(pointer);

 g.setColour(juce::Colours::white.withAlpha(0.5f * intensity));
 juce::Path highlight;
 highlight.addRectangle(-pointerThickness * 0.25f, -radius + 8,
 pointerThickness * 0.5f, pointerLength * 0.6f);
 highlight.applyTransform(juce::AffineTransform::rotation(angle)
 .translated(centerX, centerY));
 g.fillPath(highlight);
 }
 
 void drawChromeRing(juce::Graphics& g, float centerX, float centerY,
 float radius, const juce::Colour& colour, float intensity)
 {
 auto ringRadius = radius * 0.98f;

 // Outer dark ring
 g.setColour(juce::Colour(0xff0a0a0a));
 g.drawEllipse(centerX - ringRadius, centerY - ringRadius,
 ringRadius * 2.0f, ringRadius * 2.0f, 2.5f);

 // Chrome metallic shimmer ring (grey/silver)
 float shimmer = 0.7f + 0.3f * std::sin(animationPhase * 2.5f);
 g.setColour(juce::Colour(0xffcccccc).withAlpha(shimmer * intensity * 0.4f));
 g.drawEllipse(centerX - ringRadius, centerY - ringRadius,
 ringRadius * 2.0f, ringRadius * 2.0f, 1.5f);

 // Colored accent ring (inner)
 auto innerRadius = ringRadius - 2.5f;
 g.setColour(colour.withAlpha(0.25f * intensity));
 g.drawEllipse(centerX - innerRadius, centerY - innerRadius,
 innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

 // Inner white highlight (very subtle)
 auto highlight = innerRadius - 1.0f;
 g.setColour(juce::Colours::white.withAlpha(0.08f));
 g.drawEllipse(centerX - highlight, centerY - highlight,
 highlight * 2.0f, highlight * 2.0f, 0.5f);
 }
 
 void drawNotches(juce::Graphics& g, float centerX, float centerY,
 float radius, const juce::Colour& colour)
 {
 const int numNotches = 12;
 const float notchLength = 5.0f;
 const float notchThickness = 1.5f;
 
 for (int i = 0; i < numNotches; ++i)
 {
 float angle = juce::MathConstants<float>::twoPi * i / numNotches - juce::MathConstants<float>::halfPi;
 
 float innerX = centerX + std::cos(angle) * radius;
 float innerY = centerY + std::sin(angle) * radius;
 float outerX = centerX + std::cos(angle) * (radius + notchLength);
 float outerY = centerY + std::sin(angle) * (radius + notchLength);
 
 juce::ColourGradient notchGradient(
 colour.withAlpha(0.3f),
 innerX, innerY,
 colour.withAlpha(0.1f),
 outerX, outerY,
 false
 );
 
 g.setGradientFill(notchGradient);
 g.drawLine(innerX, innerY, outerX, outerY, notchThickness);
 }
 }
 
 // Override LookAndFeel methods
 void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
 float sliderPosProportional, float rotaryStartAngle,
 float rotaryEndAngle, juce::Slider& slider) override;
 
 void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
 float sliderPos, float minSliderPos, float maxSliderPos,
 const juce::Slider::SliderStyle style, juce::Slider& slider) override;
 
 void drawButtonBackground(juce::Graphics& g, juce::Button& button,
 const juce::Colour& backgroundColour,
 bool shouldDrawButtonAsHighlighted,
 bool shouldDrawButtonAsDown) override;

private:
 float animationPhase = 0.0f;
 
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Modern3DLookAndFeel)
};

// Static Waveform Display - shows the processed sample result
// Uses a thread-safe snapshot approach: pre-computes min/max per pixel
// on the message thread, then paint() only reads from the local snapshot.
class WaveformDisplay : public juce::Component
{
public:
 WaveformDisplay()
 {
 // Let mouse events pass through to the parent editor
 // (for drag-to-DAW, file browser click, fade handles)
 setInterceptsMouseClicks(false, false);
 }

 void paint(juce::Graphics& g) override
 {
 auto bounds = getLocalBounds().toFloat();
 
 // Base background
 g.setColour(juce::Colour(0xff000000));
 g.fillRoundedRectangle(bounds, 4.0f);

 // Animated border glow (breathing effect)
 const float pulse = 0.15f + 0.10f * std::sin(animationPhase * 1.7f);
 if (isDragOver)
 {
  // Stronger hot-pink glow while dragging a file in
  auto alpha = juce::jlimit(0.4f, 0.9f, 0.65f + pulse);
  g.setColour(juce::Colour(0xffff006e).withAlpha(alpha));
  g.drawRoundedRectangle(bounds.expanded(1.5f), 5.5f, 2.2f);
 }
 else
 {
  // Idle soft purple shimmer
  auto alpha = 0.18f + pulse * 0.6f;
  g.setColour(juce::Colour(0xffb537f2).withAlpha(alpha));
  g.drawRoundedRectangle(bounds, 4.0f, 1.3f);
 }

 auto area = bounds.reduced(6.0f);
 auto width = (int)area.getWidth();
 auto height = area.getHeight();
 auto centerY = area.getCentreY();

 if (width <= 0 || cachedMin.empty())
 {
  // Draw drag & drop prompt when no sample is loaded
  const float textPulse = 0.4f + 0.2f * std::sin(animationPhase * 1.3f);

  g.setColour(juce::Colour(0xffb537f2).withAlpha(0.55f + 0.25f * textPulse));
  g.setFont(15.0f);
  auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.55f);
  g.drawText("Drag & Drop Audio", topArea,
   juce::Justification::centredBottom);

  g.setColour(juce::Colours::grey.withAlpha(0.35f + 0.25f * textPulse));
  g.setFont(11.0f);
  g.drawText("Or click here - WAV / MP3 / AIFF / FLAC - Max 8s",
   bounds, juce::Justification::centredTop);
  return;
 }

 // Center line
 g.setColour(juce::Colours::white.withAlpha(0.08f));
 g.drawHorizontalLine((int)centerY, area.getX(), area.getRight());

 // Build waveform paths from cached min/max data
 juce::Path waveTop, waveBottom;
 int numPoints = (int)cachedMax.size();

 for (int i = 0; i < numPoints; ++i)
 {
  float x = area.getX() + (float)i;
  float yTop = centerY - cachedMax[(size_t)i] * cachedGain * height * 0.45f;
  float yBot = centerY - cachedMin[(size_t)i] * cachedGain * height * 0.45f;

  if (i == 0) { waveTop.startNewSubPath(x, yTop); waveBottom.startNewSubPath(x, yBot); }
  else { waveTop.lineTo(x, yTop); waveBottom.lineTo(x, yBot); }
 }

 // Create filled shape
 juce::Path filledWave;
 filledWave.addPath(waveTop);
 for (int i = numPoints - 1; i >= 0; --i)
 {
  float x = area.getX() + (float)i;
  float yBot = centerY - cachedMin[(size_t)i] * cachedGain * height * 0.45f;
  filledWave.lineTo(x, yBot);
 }
 filledWave.closeSubPath();

 // Gradient fill
 juce::ColourGradient fillGrad(
  juce::Colour(0xffb537f2).withAlpha(0.3f), area.getCentreX(), centerY - height * 0.4f,
  juce::Colour(0xffff006e).withAlpha(0.15f), area.getCentreX(), centerY + height * 0.4f, false);
 g.setGradientFill(fillGrad);
 g.fillPath(filledWave);

 // Stroke top and bottom lines
 g.setColour(juce::Colour(0xffb537f2).withAlpha(0.9f));
 g.strokePath(waveTop, juce::PathStrokeType(1.5f));
 g.setColour(juce::Colour(0xffff006e).withAlpha(0.6f));
 g.strokePath(waveBottom, juce::PathStrokeType(1.0f));

 // Draw playback position indicator
 if (playbackProgress > 0.0f && playbackProgress < 1.0f)
 {
  float posX = area.getX() + playbackProgress * area.getWidth();
  g.setColour(juce::Colours::white.withAlpha(0.7f));
  g.drawVerticalLine((int)posX, area.getY(), area.getBottom());
 }
 }

 // Thread-safe: takes a snapshot of the buffer data into local arrays.
 // Call ONLY from the message thread when the buffer is known to be stable.
 void setAudioBuffer(const juce::AudioBuffer<float>* buffer)
 {
 int width = getWidth() - 12; // match area.reduced(6)
 if (width <= 0 || buffer == nullptr || buffer->getNumSamples() == 0)
 {
  cachedMin.clear();
  cachedMax.clear();
  repaint();
  return;
 }

 auto numSamples = buffer->getNumSamples();
 auto numChannels = buffer->getNumChannels();
 float samplesPerPixel = (float)numSamples / (float)width;

 cachedMin.resize((size_t)width);
 cachedMax.resize((size_t)width);

 for (int px = 0; px < width; ++px)
 {
  int sampleStart = (int)(px * samplesPerPixel);
  int sampleEnd = juce::jmin((int)((px + 1) * samplesPerPixel), numSamples);

  float minVal = 0.0f, maxVal = 0.0f;
  for (int s = sampleStart; s < sampleEnd; ++s)
  {
   float v = 0.0f;
   for (int ch = 0; ch < numChannels; ++ch)
    v += buffer->getSample(ch, s);
   v /= numChannels;
   minVal = juce::jmin(minVal, v);
   maxVal = juce::jmax(maxVal, v);
  }
  cachedMin[(size_t)px] = minVal;
  cachedMax[(size_t)px] = maxVal;
 }

 repaint();
 }

 void setGain(float g)
 {
 if (cachedGain != g) { cachedGain = g; repaint(); }
 }

 void setPlaybackProgress(float p)
 {
 if (playbackProgress != p) { playbackProgress = p; repaint(); }
 }

 void setDraggingOver(bool dragging)
 {
 if (isDragOver != dragging) { isDragOver = dragging; repaint(); }
 }

 // Animation phase from editor timer for subtle breathing effects
 void setAnimationPhase(float phase)
 {
 if (animationPhase != phase) { animationPhase = phase; repaint(); }
 }

private:
 // Thread-safe cached waveform data (owned by this component, no shared pointers)
 std::vector<float> cachedMin;
 std::vector<float> cachedMax;
 float cachedGain = 1.0f;
 float playbackProgress = 0.0f;
 bool isDragOver = false;
 float animationPhase = 0.0f;

 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

// AboutWindow - animated about dialog with glowing effects
class AboutWindow : public juce::Component, private juce::Timer
{
public:
 AboutWindow()
 {
 setSize(420, 340);

 // Close button
 closeButton.setButtonText(juce::CharPointer_UTF8("\xc3\x97")); // multiplication sign as X
 closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
 closeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff006e).withAlpha(0.3f));
 closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffb537f2));
 closeButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
 closeButton.onClick = [this] {
 juce::Desktop::getInstance().getAnimator().fadeOut(this, 250);
 juce::Timer::callAfterDelay(260, [this] { delete this; });
 };
 addAndMakeVisible(closeButton);

 // Generate sparkle positions
 juce::Random rng;
 for (int i = 0; i < 18; ++i)
 {
  sparkles.push_back({ rng.nextFloat(), rng.nextFloat(),
                        rng.nextFloat() * 6.28f, 0.3f + rng.nextFloat() * 0.7f });
 }

 startTimerHz(30);
 }

 ~AboutWindow() override { stopTimer(); }

 void timerCallback() override
 {
 phase += 0.04f;
 repaint();
 }

 void paint(juce::Graphics& g) override
 {
 auto bounds = getLocalBounds().toFloat();

 // Background gradient - deep space purple to dark blue
 juce::ColourGradient bgGrad(
  juce::Colour(0xff0d0520), bounds.getCentreX(), 0,
  juce::Colour(0xff1a0a30), bounds.getCentreX(), bounds.getHeight(), false);
 bgGrad.addColour(0.5, juce::Colour(0xff120828));
 g.setGradientFill(bgGrad);
 g.fillRoundedRectangle(bounds, 12.0f);

 // Animated border glow
 float borderPhase = 0.6f + 0.4f * std::sin(phase * 1.2f);
 auto borderColour = juce::Colour(0xffb537f2).interpolatedWith(
  juce::Colour(0xffff006e), 0.5f + 0.5f * std::sin(phase * 0.8f));

 // Outer glow layers
 for (int i = 3; i >= 1; --i)
 {
  float expand = i * 1.5f;
  g.setColour(borderColour.withAlpha(0.06f * borderPhase / i));
  g.drawRoundedRectangle(bounds.expanded(expand), 12.0f + expand, 1.5f);
 }

 // Main border
 g.setColour(borderColour.withAlpha(0.7f * borderPhase));
 g.drawRoundedRectangle(bounds.reduced(0.5f), 12.0f, 1.5f);

 // Animated sparkles
 for (size_t i = 0; i < sparkles.size(); ++i)
 {
  auto& s = sparkles[i];
  float sparklePhase = std::sin(phase * s.speed + s.offset);
  float alpha = juce::jlimit(0.0f, 1.0f, sparklePhase * 0.35f);
  float x = s.x * bounds.getWidth();
  float y = s.y * bounds.getHeight();
  float sz = 1.0f + sparklePhase * 1.5f;

  g.setColour(juce::Colour(0xffb537f2).withAlpha(alpha * 0.5f));
  g.fillEllipse(x - sz * 2, y - sz * 2, sz * 4, sz * 4);
  g.setColour(juce::Colours::white.withAlpha(alpha * 0.8f));
  g.fillEllipse(x - sz * 0.5f, y - sz * 0.5f, sz, sz);
 }

 // Horizontal separator line with glow
 float sepY = 70.0f;
 float lineGlow = 0.5f + 0.3f * std::sin(phase * 1.5f);
 juce::ColourGradient lineGrad(
  borderColour.withAlpha(0.0f), bounds.getX() + 20, sepY,
  borderColour.withAlpha(lineGlow * 0.6f), bounds.getCentreX(), sepY, false);
 g.setGradientFill(lineGrad);
 g.fillRect(bounds.getX() + 20, sepY, bounds.getWidth() * 0.5f - 20, 1.0f);
 juce::ColourGradient lineGrad2(
  borderColour.withAlpha(lineGlow * 0.6f), bounds.getCentreX(), sepY,
  borderColour.withAlpha(0.0f), bounds.getRight() - 20, sepY, false);
 g.setGradientFill(lineGrad2);
 g.fillRect(bounds.getCentreX(), sepY, bounds.getWidth() * 0.5f - 20, 1.0f);

 // Title with animated glow
 float titleGlow = 0.8f + 0.2f * std::sin(phase * 0.6f);
 auto titleColour = juce::Colour(0xffb537f2).interpolatedWith(
  juce::Colour(0xffff006e), 0.3f + 0.2f * std::sin(phase * 0.5f));

 // Title shadow
 g.setColour(titleColour.withAlpha(0.15f * titleGlow));
 g.setFont(juce::Font(28.0f, juce::Font::bold));
 g.drawText("ReverseReverb", bounds.withY(16).withHeight(40).translated(1, 1),
  juce::Justification::centred);

 // Title text
 g.setColour(juce::Colours::white.withAlpha(0.95f * titleGlow));
 g.drawText("ReverseReverb", bounds.withY(16).withHeight(40),
  juce::Justification::centred);

 // Version badge
 juce::String versionStr = juce::String("v") + JucePlugin_VersionString;
 g.setFont(juce::Font(11.0f));
 auto vBadge = juce::Rectangle<float>(
  bounds.getCentreX() - 30, 52, 60, 16);
 g.setColour(borderColour.withAlpha(0.15f));
 g.fillRoundedRectangle(vBadge, 8.0f);
 g.setColour(borderColour.withAlpha(0.5f));
 g.drawRoundedRectangle(vBadge, 8.0f, 0.5f);
 g.setColour(juce::Colours::white.withAlpha(0.7f));
 g.drawText(versionStr, vBadge, juce::Justification::centred);

 // Content area
 float contentY = 90.0f;
 float lineH = 26.0f;

 // Author
 g.setFont(juce::Font(15.0f, juce::Font::bold));
 g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.9f));
 g.drawText("Created by", bounds.withY(contentY).withHeight(lineH),
  juce::Justification::centred);

 contentY += lineH;
 g.setFont(juce::Font(17.0f, juce::Font::bold));
 g.setColour(juce::Colours::white.withAlpha(0.95f));
 g.drawText("Liran Rone Kalifa", bounds.withY(contentY).withHeight(lineH),
  juce::Justification::centred);

 // Second separator
 contentY += lineH + 10;
 float sep2Y = contentY;
 g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.15f));
 g.fillRect(bounds.getX() + 40, sep2Y, bounds.getWidth() - 80, 1.0f);

 // Description
 contentY += 18;
 g.setFont(juce::Font(13.0f));
 g.setColour(juce::Colours::white.withAlpha(0.6f));
 g.drawText("Reverse Reverb Effect Plugin", bounds.withY(contentY).withHeight(22),
  juce::Justification::centred);
 contentY += 22;
 g.drawText("VST3 + Standalone", bounds.withY(contentY).withHeight(22),
  juce::Justification::centred);

 // Features with icons
 contentY += 34;
 float iconX = bounds.getCentreX() - 110;
 g.setFont(juce::Font(12.0f));

 // Feature 1
 g.setColour(juce::Colour(0xff00ff88).withAlpha(0.8f));
 g.fillEllipse(iconX, contentY + 3, 6, 6);
 g.setColour(juce::Colours::white.withAlpha(0.55f));
 g.drawText("Drag & drop audio files", juce::Rectangle<float>(iconX + 14, contentY, 200, 18),
  juce::Justification::centredLeft);

 contentY += 20;
 g.setColour(juce::Colour(0xffff006e).withAlpha(0.8f));
 g.fillEllipse(iconX, contentY + 3, 6, 6);
 g.setColour(juce::Colours::white.withAlpha(0.55f));
 g.drawText("Tweak reverb parameters", juce::Rectangle<float>(iconX + 14, contentY, 200, 18),
  juce::Justification::centredLeft);

 contentY += 20;
 g.setColour(juce::Colour(0xffb537f2).withAlpha(0.8f));
 g.fillEllipse(iconX, contentY + 3, 6, 6);
 g.setColour(juce::Colours::white.withAlpha(0.55f));
 g.drawText("Drag result to your DAW", juce::Rectangle<float>(iconX + 14, contentY, 200, 18),
  juce::Justification::centredLeft);
 }

 void resized() override
 {
 closeButton.setBounds(getWidth() - 38, 8, 28, 28);
 }

private:
 juce::TextButton closeButton;
 float phase = 0.0f;

 struct Sparkle { float x, y, offset, speed; };
 std::vector<Sparkle> sparkles;

 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutWindow)
};

// DragVisualizer - for drag-to-DAW functionality
class DragVisualizer : public juce::Component
{
public:
 DragVisualizer(ReverseReverbAudioProcessor& p) : processor(p)
 {
 setInterceptsMouseClicks(true, false);
 }
 
 void paint(juce::Graphics& g) override
 {
 auto bounds = getLocalBounds().toFloat().reduced(2.0f);
 auto cx = bounds.getCentreX();
 auto cy = bounds.getCentreY();
 auto size = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.38f;

 // Diamond shape
 juce::Path diamond;
 diamond.startNewSubPath(cx, cy - size);
 diamond.lineTo(cx + size, cy);
 diamond.lineTo(cx, cy + size);
 diamond.lineTo(cx - size, cy);
 diamond.closeSubPath();

 // Subtle fill
 g.setColour(juce::Colour(0xffb537f2).withAlpha(0.12f));
 g.fillPath(diamond);

 // Border
 g.setColour(juce::Colour(0xffb537f2).withAlpha(0.55f));
 g.strokePath(diamond, juce::PathStrokeType(1.5f));

 // Arrow inside pointing down-right
 float as = size * 0.45f;
 juce::Path arrow;
 arrow.startNewSubPath(cx - as * 0.3f, cy - as * 0.3f);
 arrow.lineTo(cx + as * 0.4f, cy + as * 0.4f);
 g.setColour(juce::Colours::white.withAlpha(0.65f));
 g.strokePath(arrow, juce::PathStrokeType(1.5f));

 // Arrow head lines
 juce::Path head;
 head.startNewSubPath(cx + as * 0.4f, cy + as * 0.4f);
 head.lineTo(cx + as * 0.4f - 4.0f, cy + as * 0.4f - 3.0f);
 head.startNewSubPath(cx + as * 0.4f, cy + as * 0.4f);
 head.lineTo(cx + as * 0.4f - 3.0f, cy + as * 0.4f + 2.0f);
 g.strokePath(head, juce::PathStrokeType(1.5f));
 }

 void mouseDrag(const juce::MouseEvent& event) override;
 void mouseUp(const juce::MouseEvent&) override { isDragging = false; }

 // Animation phase from editor timer for hover / idle motion
 void setAnimationPhase(float phase)
 {
 if (animationPhase != phase) { animationPhase = phase; repaint(); }
 }

private:
 ReverseReverbAudioProcessor& processor;
 bool isDragging = false;
 float animationPhase = 0.0f;
 
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DragVisualizer)
};

// Glowing Knob LookAndFeel - legacy compatibility
class GlowingKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
 GlowingKnobLookAndFeel() = default;
 
private:
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlowingKnobLookAndFeel)
};

// Glowing Button LookAndFeel - legacy compatibility 
class GlowingButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
 GlowingButtonLookAndFeel() = default;
 
private:
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlowingButtonLookAndFeel)
};

// Main editor class
class ReverseReverbAudioProcessorEditor : public juce::AudioProcessorEditor,
 public juce::FileDragAndDropTarget,
 public juce::DragAndDropContainer,
 public juce::Slider::Listener,
 public juce::Button::Listener,
 public juce::ComboBox::Listener,
 private juce::Timer
{
public:
 ReverseReverbAudioProcessorEditor (ReverseReverbAudioProcessor&);
 ~ReverseReverbAudioProcessorEditor() override;

 void paint (juce::Graphics&) override;
 void resized() override;
 
 // FileDragAndDropTarget
 bool isInterestedInFileDrag (const juce::StringArray& files) override;
 void fileDragExit (const juce::StringArray& files) override;
 void filesDropped (const juce::StringArray& files, int x, int y) override;
 
 // Slider::Listener
 void sliderValueChanged (juce::Slider* slider) override;
 
 // Button::Listener
 void buttonClicked (juce::Button* button) override;
 
 // ComboBox::Listener
 void comboBoxChanged (juce::ComboBox* comboBox) override;
 
 // Timer callback for throttled processing
 void timerCallback() override;
 
 // Mouse handling for drop zone
 void mouseDown (const juce::MouseEvent& event) override;
 void mouseMove (const juce::MouseEvent& event) override;
 void mouseDrag (const juce::MouseEvent& event) override;
 void mouseUp (const juce::MouseEvent& event) override;
 void mouseDoubleClick (const juce::MouseEvent& event) override;
 
 // Public method for DragVisualizer to update status
 void updateStatus(const juce::String& message);

private:
 ReverseReverbAudioProcessor& audioProcessor;
 
 // Modern 3D LookAndFeel and waveform display
 std::unique_ptr<Modern3DLookAndFeel> modern3DLAF;
 std::unique_ptr<WaveformDisplay> waveformDisplay;
 
 // Custom LookAndFeel for glowing knobs and buttons (legacy - keeping for compatibility)
 GlowingKnobLookAndFeel glowingKnobLAF;
 GlowingButtonLookAndFeel glowingButtonLAF;
 
 // Background image
 juce::Image backgroundImage;

 // Logo image (set via loadLogoImage or drag-drop)
 juce::Image logoImage;
 juce::Rectangle<int> logoArea;
 
 // UI Components
 juce::Label titleLabel;
 juce::Label dropLabel;
 juce::Label statusLabel;
 
 juce::Slider reverbSizeSlider;
 juce::Label reverbSizeLabel;
 
 juce::Slider dryWetSlider;
 juce::Label dryWetLabel;
 
 juce::Slider tailDivisionSlider;
 juce::Label tailDivisionLabel;
 juce::Label bpmDisplayLabel;
 juce::Slider bpmSlider;
 
 juce::Slider stereoWidthSlider;
 juce::Label stereoWidthLabel;
 
 juce::Slider lowCutSlider;
 juce::Label lowCutLabel;
 
 juce::TextButton playButton;
 
 juce::TextButton transitionModeButton;
 juce::Label transitionModeLabel;
 
 // About button
 juce::TextButton aboutButton;
 
 // Drag visualizer for drag-to-DAW (replaces export button)
 DragVisualizer dragVisualizer;
 
 // Drop zone area
 juce::Rectangle<int> dropZone;
 juce::Rectangle<int> waveformArea;
 juce::Rectangle<int> tremoloBoxBounds;
 bool isDraggingOver = false;
 bool waveformClickPending = false; // For drag-to-DAW vs click-to-browse
 bool isDraggingToDAW = false; // True when performing external file drag
 bool tremoloExpanded = false; // Collapsible tremolo section
 
 // OPTIMIZATION: Cache waveform path to avoid recalculating every paint
 juce::Path cachedWaveformPath;
 bool waveformNeedsUpdate = true;
 
 // Throttled processing
 bool processingScheduled = false;
 std::atomic<bool> isCurrentlyProcessing { false };
 int processingThrottleCounter = 0;
 
 // Shared animation phase for synchronized animations
 float animationPhase = 0.0f;
 
 // Fade controls
 float fadeInAmount = 0.0f;
 float fadeOutAmount = 0.0f;
 bool isDraggingFadeIn = false;
 bool isDraggingFadeOut = false;
 
 // Default values for double-click reset
 const float defaultReverbSize = 1.0f;
 const float defaultDryWet = 1.0f;
 const int defaultTailDivision = 3;    // 1 Bar
 const float defaultManualBpm = 120.0f;
 const float defaultStereoWidth = 0.5f;
 const float defaultLowCut = 20.0f;
 
 // Tremolo UI Controls
 juce::TextButton tremoloEnableButton;
 juce::Label tremoloLabel;
 
 juce::Slider tremoloDepthSlider;
 juce::Label tremoloDepthLabel;
 
 juce::Slider tremoloRateSlider;
 juce::Label tremoloRateLabel;
 
 juce::ComboBox tremoloWaveformCombo;
 juce::Label tremoloWaveformLabel;
 
 juce::ToggleButton tremoloSyncToggle;
 juce::Label tremoloSyncLabel;
 
 juce::ComboBox tremoloSyncDivisionCombo;
 juce::Label tremoloSyncDivisionLabel;
 
 // Tremolo Rate Ramp Controls
 juce::ToggleButton tremoloRateRampToggle;
 juce::Label tremoloRateRampLabel;
 
 juce::ComboBox tremoloStartDivisionCombo;
 juce::Label tremoloStartDivisionLabel;
 
 juce::ComboBox tremoloEndDivisionCombo;
 juce::Label tremoloEndDivisionLabel;

 juce::TextButton tremoloExpandButton; // Arrow button to expand/collapse tremolo
 
 void paintWaveform(juce::Graphics& g);
 void openFileBrowser();
 void performDragToDAW();
 void drawCircuitBoardPattern(juce::Graphics& g, juce::Rectangle<int> area);
 void updateWaveformWithTremolo(); // Update waveform display with tremolo preview

 // Buffer for tremolo-modulated waveform display
 juce::AudioBuffer<float> tremoloDisplayBuffer;

 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverseReverbAudioProcessorEditor)
};

