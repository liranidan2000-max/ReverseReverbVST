#include "PluginProcessor.h"
#include "PluginEditor.h"

// ========================================
// Design Color Palette
// ========================================
namespace DesignColours
{
    const juce::Colour background     (0xff0a0a0f);
    const juce::Colour backgroundLight(0xff0a0e12);
    const juce::Colour circuitTeal    (0xff0a3a3a);
    const juce::Colour hotPink        (0xffff006e);
    const juce::Colour electricCyan   (0xff00d9ff);
    const juce::Colour purple         (0xffb537f2);
    const juce::Colour lightPurple    (0xff9d4edd);
    const juce::Colour deepPurple     (0xff7209b7);
    const juce::Colour orange         (0xffffa500);
    const juce::Colour green          (0xff00ff88);
    const juce::Colour darkFace       (0xff0f0f18);
    const juce::Colour trackBg        (0xff1a1a2e);
    const juce::Colour darkPanel      (0xff2a1a3f);
}

// ========================================
// Modern3DLookAndFeel Implementation
// ========================================

void Modern3DLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
 float sliderPosProportional, float rotaryStartAngle,
 float rotaryEndAngle, juce::Slider& slider)
{
 // Safety: JUCE can occasionally call this with a fully-clipped
 // component that has a "sentinel" negative size in debug builds.
 // Bail out early in that case to avoid triggering the JUCE
 // coordsToRectangle assertion in juce_GraphicsContext.cpp.
 if (width <= 0 || height <= 0
     || !std::isfinite((float) width)
     || !std::isfinite((float) height))
     return;

 auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10.0f);
 auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
 auto centerX = bounds.getCentreX();
 auto centerY = bounds.getCentreY();
 auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
 
 // Get slider color
 auto colour = slider.findColour(juce::Slider::thumbColourId);
 auto trackColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
 
 // Calculate intensity based on value
 float intensity = 0.5f + sliderPosProportional * 0.5f;
 
 // Draw multi-layer glow
 drawMultiLayerGlow(g, centerX, centerY, radius, colour, intensity, 4);
 
 // Draw the background circle with gradient - darker face for chrome contrast
 juce::ColourGradient backgroundGradient(
 juce::Colour(0xff141420),
 centerX, centerY - radius,
 juce::Colour(0xff0a0a0f),
 centerX, centerY + radius,
 false
 );
 g.setGradientFill(backgroundGradient);
 g.fillEllipse(centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f);
 
 // Draw track arc
 juce::Path track;
 track.addCentredArc(centerX, centerY, radius * 0.85f, radius * 0.85f,
 0.0f, rotaryStartAngle, rotaryEndAngle, true);
 
 g.setColour(juce::Colour(0xff2a2a3f));
 g.strokePath(track, juce::PathStrokeType(3.0f));
 
 // Draw filled arc
 juce::Path filledArc;
 filledArc.addCentredArc(centerX, centerY, radius * 0.85f, radius * 0.85f,
 0.0f, rotaryStartAngle, angle, true);
 
 juce::ColourGradient arcGradient(
 trackColour.brighter(0.2f),
 centerX, centerY - radius,
 trackColour,
 centerX, centerY + radius,
 false
 );
 g.setGradientFill(arcGradient);
 g.strokePath(filledArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
 
 // Draw chrome ring
 drawChromeRing(g, centerX, centerY, radius, colour, intensity);
 
 // Draw notches
 drawNotches(g, centerX, centerY, radius * 0.9f, colour);
 
 // Draw 3D pointer
 draw3DPointer(g, centerX, centerY, radius, angle, colour, intensity);
 
 // Draw center dot
 drawGlowingDot(g, centerX, centerY, 6.0f, colour, intensity);
}

void Modern3DLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
 float sliderPos, float minSliderPos, float maxSliderPos,
 const juce::Slider::SliderStyle style, juce::Slider& slider)
{
 // Same safety guard as in drawRotarySlider – protect against
 // fully-clipped components that report invalid sizes.
 if (width <= 0 || height <= 0
     || !std::isfinite((float) width)
     || !std::isfinite((float) height))
     return;

 if (slider.isHorizontal())
 {
 auto trackHeight = 6.0f;
 auto trackY = y + height * 0.5f - trackHeight * 0.5f;
 auto thumbRadius = 9.0f;

 auto trackColour = slider.findColour(juce::Slider::trackColourId);
 auto thumbColour = slider.findColour(juce::Slider::thumbColourId);

 // Track background (dark)
 g.setColour(juce::Colour(0xff1a1a2e));
 g.fillRoundedRectangle((float)x, trackY, (float)width, trackHeight, trackHeight * 0.5f);

 // Filled portion with gradient
 float fillWidth = sliderPos - x;
 if (fillWidth > 0)
 {
 juce::ColourGradient fillGrad(
 trackColour.darker(0.2f), (float)x, trackY,
 trackColour, sliderPos, trackY, false);
 g.setGradientFill(fillGrad);
 g.fillRoundedRectangle((float)x, trackY, fillWidth, trackHeight, trackHeight * 0.5f);
 }

 // Thumb glow
 float thumbCY = y + height * 0.5f;
 for (int i = 3; i >= 1; --i)
 {
 float glowR = thumbRadius + i * 2.5f;
 float alpha = 0.15f / i;
 g.setColour(thumbColour.withAlpha(alpha));
 g.fillEllipse(sliderPos - glowR, thumbCY - glowR, glowR * 2, glowR * 2);
 }

 // Thumb body
 g.setColour(thumbColour);
 g.fillEllipse(sliderPos - thumbRadius, thumbCY - thumbRadius, thumbRadius * 2, thumbRadius * 2);

 // Specular highlight
 g.setColour(juce::Colours::white.withAlpha(0.5f));
 g.fillEllipse(sliderPos - thumbRadius * 0.35f, thumbCY - thumbRadius * 0.5f,
 thumbRadius * 0.5f, thumbRadius * 0.5f);
 }
 else
 {
 auto trackWidth = 6.0f;
 auto trackX = x + width * 0.5f - trackWidth * 0.5f;

 g.setColour(juce::Colour(0xff1a1a2e));
 g.fillRoundedRectangle(trackX, (float)y, trackWidth, (float)height, trackWidth * 0.5f);

 auto fillColour = slider.findColour(juce::Slider::trackColourId);
 g.setColour(fillColour);
 g.fillRoundedRectangle(trackX, sliderPos, trackWidth, y + height - sliderPos, trackWidth * 0.5f);

 drawGlowingDot(g, x + width * 0.5f, sliderPos, 9.0f,
 slider.findColour(juce::Slider::thumbColourId), 0.8f);
 }
}

void Modern3DLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
 const juce::Colour& backgroundColour,
 bool shouldDrawButtonAsHighlighted,
 bool shouldDrawButtonAsDown)
{
 auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
 auto cornerRadius = bounds.getHeight() * 0.5f; // Pill shape

 auto baseColour = backgroundColour
 .withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
 .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

 float glowIntensity = 0.6f;
 if (shouldDrawButtonAsDown || button.getToggleState())
 {
 baseColour = baseColour.brighter(0.3f);
 glowIntensity = 1.0f;
 }
 else if (shouldDrawButtonAsHighlighted)
 {
 baseColour = baseColour.brighter(0.15f);
 glowIntensity = 0.8f;
 }

 // Outer glow layers
 for (int i = 3; i >= 1; --i)
 {
 float expand = i * 2.5f;
 float alpha = (0.08f * glowIntensity) / i;
 g.setColour(baseColour.withAlpha(alpha));
 g.fillRoundedRectangle(bounds.expanded(expand), cornerRadius + expand);
 }

 // Dark semi-transparent fill
 g.setColour(juce::Colour(0xff0a0a0f).withAlpha(0.7f));
 g.fillRoundedRectangle(bounds, cornerRadius);

 // Border stroke - main visual element
 float borderThickness = shouldDrawButtonAsDown ? 2.5f : 2.0f;
 g.setColour(baseColour);
 g.drawRoundedRectangle(bounds, cornerRadius, borderThickness);

 // Inner highlight for depth on hover/press
 if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
 {
 g.setColour(baseColour.brighter(0.5f).withAlpha(0.12f));
 g.drawRoundedRectangle(bounds.reduced(1.0f), cornerRadius - 1.0f, 0.5f);
 }
}

// ========================================
// ReverseReverbAudioProcessorEditor Implementation
// ========================================

ReverseReverbAudioProcessorEditor::ReverseReverbAudioProcessorEditor (ReverseReverbAudioProcessor& p)
 : AudioProcessorEditor (&p), audioProcessor (p), dragVisualizer(p),
 modern3DLAF(new Modern3DLookAndFeel()), // Create new 3D LookAndFeel
 waveformDisplay(new WaveformDisplay()) // Create waveform display
{
 // Set window size - compact design with logo banner + collapsible tremolo
 setSize (700, 600);

 // Make the plugin resizable with constraints (no fixed aspect ratio - height changes with tremolo)
 setResizable (true, true);
 setResizeLimits (560, 500, 1050, 1020);
 
// // Load background image
// juce::File imageFile("/Users/liranronekalifa/Library/CloudStorage/Dropbox/reverse reverb.png");
// if (imageFile.existsAsFile())
// {
// backgroundImage = juce::ImageFileFormat::loadFrom(imageFile);
// DBG("Background image loaded successfully!");
// }
// else
// {
// DBG("Warning: Background image not found at: " + imageFile.getFullPathName());
// }
 
 // Title Label - REMOVED (commented out for logo space)
 // titleLabel.setText ("Reverse Reverb VST", juce::dontSendNotification);
 // titleLabel.setFont (juce::Font (24.0f, juce::Font::bold));
 // titleLabel.setJustificationType (juce::Justification::centred);
 // titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
 // addAndMakeVisible (titleLabel);
 
 // Drop zone label - lighter purple/grey - BIGGER!
 dropLabel.setText ("Drag & Drop Audio (WAV, MP3, AIFF, FLAC - Max 8s)", juce::dontSendNotification);
 dropLabel.setFont (juce::Font (13.0f)); // Bigger font - was 11
 dropLabel.setJustificationType (juce::Justification::centred);
 dropLabel.setColour (juce::Label::textColourId, juce::Colour (0xffb8b8d0)); // Light purple-grey
 dropLabel.setInterceptsMouseClicks (false, false); // Allow clicks to pass through
 addAndMakeVisible (dropLabel);
 
 // Status label - Purple/magenta theme
 statusLabel.setText ("No sample loaded", juce::dontSendNotification);
 statusLabel.setFont (juce::Font (12.0f));
 statusLabel.setJustificationType (juce::Justification::centred);
 statusLabel.setColour (juce::Label::textColourId, juce::Colour (0xffb537f2)); // Vibrant purple
 addAndMakeVisible (statusLabel);
 
 // Reverb Size Slider - Purple/Magenta theme
 reverbSizeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
 reverbSizeSlider.setRange (0.0, 1.0, 0.001); // Finer resolution
 reverbSizeSlider.setValue (audioProcessor.getReverbSize());
 reverbSizeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
 reverbSizeSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xffb537f2)); // Vibrant purple
 reverbSizeSlider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffb537f2));
 reverbSizeSlider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff2a1a3f));
 reverbSizeSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
 reverbSizeSlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
 reverbSizeSlider.setMouseDragSensitivity (200); // Higher = smoother/slower movement
 reverbSizeSlider.setVelocityBasedMode (true); // Smooth velocity-based dragging
 reverbSizeSlider.setVelocityModeParameters (1.0, 1, 0.05, false); // Sensitivity, threshold, offset, invert
 reverbSizeSlider.setLookAndFeel (modern3DLAF.get()); // Apply modern 3D look!
 reverbSizeSlider.addListener (this);
 addAndMakeVisible (reverbSizeSlider);
 
 reverbSizeLabel.setText ("Room Size", juce::dontSendNotification);
 reverbSizeLabel.setJustificationType (juce::Justification::centred);
 reverbSizeLabel.setColour (juce::Label::textColourId, juce::Colour (0xffb537f2));
 reverbSizeLabel.setFont (juce::Font (14.0f, juce::Font::bold)); // Bigger and bold!
 addAndMakeVisible (reverbSizeLabel);
 
 // Dry/Wet Slider - Hot Pink/Magenta (center knob - brightest) - NOW GAIN CONTROL
 dryWetSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
 dryWetSlider.setRange (0.0, 2.0, 0.001); // 0.0 to 2.0 (0.5 = -6dB, 1.0 = 0dB, 2.0 = +6dB)
 dryWetSlider.setValue (audioProcessor.getDryWet());
 dryWetSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
 dryWetSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xffff006e)); // Hot pink
 dryWetSlider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffff006e));
 dryWetSlider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff2a1a3f));
 dryWetSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
 dryWetSlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
 dryWetSlider.setMouseDragSensitivity (200); // Higher = smoother/slower movement
 dryWetSlider.setVelocityBasedMode (true); // Smooth velocity-based dragging
 dryWetSlider.setVelocityModeParameters (1.0, 1, 0.05, false); // Sensitivity, threshold, offset, invert
 dryWetSlider.setLookAndFeel (modern3DLAF.get()); // Apply modern 3D look!
 dryWetSlider.addListener (this);
 addAndMakeVisible (dryWetSlider);
 
 dryWetLabel.setText ("Gain", juce::dontSendNotification); // Changed from "Output Level" to "Gain"
 dryWetLabel.setJustificationType (juce::Justification::centred);
 dryWetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffff006e));
 dryWetLabel.setFont (juce::Font (14.0f, juce::Font::bold)); // Bigger and bold!
 addAndMakeVisible (dryWetLabel);
 
 // Tail Length Division Knob - Light purple, snaps to divisions
 tailDivisionSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
 tailDivisionSlider.setRange(0.0, 8.0, 1.0); // 9 steps: 0=8Bar .. 8=1/32
 tailDivisionSlider.setValue((double)audioProcessor.getTailDivision());
 tailDivisionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
 tailDivisionSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff9d4edd));
 tailDivisionSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff9d4edd));
 tailDivisionSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a1a3f));
 tailDivisionSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
 tailDivisionSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
 tailDivisionSlider.setMouseDragSensitivity(200);
 tailDivisionSlider.setLookAndFeel(modern3DLAF.get());
 tailDivisionSlider.textFromValueFunction = [](double value) -> juce::String {
     const char* names[] = { "8 Bar", "4 Bar", "2 Bar", "1 Bar", "1/2", "1/4", "1/8", "1/16", "1/32" };
     int idx = juce::jlimit(0, 8, (int)value);
     return names[idx];
 };
 tailDivisionSlider.valueFromTextFunction = [](const juce::String& text) -> double {
     if (text == "8 Bar") return 0.0;
     if (text == "4 Bar") return 1.0;
     if (text == "2 Bar") return 2.0;
     if (text == "1 Bar") return 3.0;
     if (text == "1/2")   return 4.0;
     if (text == "1/4")   return 5.0;
     if (text == "1/8")   return 6.0;
     if (text == "1/16")  return 7.0;
     if (text == "1/32")  return 8.0;
     return 3.0;
 };
 tailDivisionSlider.addListener(this);
 addAndMakeVisible(tailDivisionSlider);

 tailDivisionLabel.setText("Tail Length", juce::dontSendNotification);
 tailDivisionLabel.setJustificationType(juce::Justification::centred);
 tailDivisionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9d4edd));
 tailDivisionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
 addAndMakeVisible(tailDivisionLabel);

 // BPM controls
 if (audioProcessor.isStandalone())
 {
     bpmSlider.setSliderStyle(juce::Slider::LinearHorizontal);
     bpmSlider.setRange(20.0, 300.0, 1.0);
     bpmSlider.setValue(audioProcessor.getManualBpm());
     bpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 18);
     bpmSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff9d4edd));
     bpmSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff9d4edd).withAlpha(0.5f));
     bpmSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2a1a3f));
     bpmSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
     bpmSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
     bpmSlider.setLookAndFeel(modern3DLAF.get());
     bpmSlider.addListener(this);
     addAndMakeVisible(bpmSlider);

     bpmDisplayLabel.setText("BPM", juce::dontSendNotification);
     bpmDisplayLabel.setJustificationType(juce::Justification::centred);
     bpmDisplayLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9d4edd).withAlpha(0.7f));
     bpmDisplayLabel.setFont(juce::Font(11.0f, juce::Font::bold));
     addAndMakeVisible(bpmDisplayLabel);
 }
 else
 {
     bpmDisplayLabel.setText("BPM: Auto", juce::dontSendNotification);
     bpmDisplayLabel.setJustificationType(juce::Justification::centred);
     bpmDisplayLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9d4edd).withAlpha(0.5f));
     bpmDisplayLabel.setFont(juce::Font(10.0f));
     addAndMakeVisible(bpmDisplayLabel);
 }
 
 // Stereo Width Slider - Horizontal Fader - Cyan/Electric Blue theme
 stereoWidthSlider.setSliderStyle (juce::Slider::LinearHorizontal);
 stereoWidthSlider.setRange (0.0, 1.0, 0.001); // 0.0 = mono, 0.5 = normal, 1.0 = max width
 stereoWidthSlider.setValue (audioProcessor.getStereoWidth());
 stereoWidthSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 30); // Bigger text box
 stereoWidthSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff00d9ff)); // Electric cyan
 stereoWidthSlider.setColour (juce::Slider::trackColourId, juce::Colour (0xff00d9ff).withAlpha(0.6f));
 stereoWidthSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a1a3f).withAlpha(0.5f));
 stereoWidthSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
 stereoWidthSlider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff1a0f2e).withAlpha(0.8f));
 stereoWidthSlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff00d9ff).withAlpha(0.3f));
 stereoWidthSlider.setMouseDragSensitivity (100); // Was 200, now 100 - faster response!
 stereoWidthSlider.setVelocityBasedMode (false); // Was true, now false - direct control!
 stereoWidthSlider.setLookAndFeel (modern3DLAF.get()); // Apply modern 3D look!
 stereoWidthSlider.addListener (this);
 addAndMakeVisible (stereoWidthSlider);
 
 stereoWidthLabel.setText ("Stereo Width", juce::dontSendNotification);
 stereoWidthLabel.setJustificationType (juce::Justification::centredLeft);
 stereoWidthLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00d9ff));
 stereoWidthLabel.setFont (juce::Font (14.0f, juce::Font::bold)); // Slightly bigger and bold
 addAndMakeVisible (stereoWidthLabel);
 
 DBG("Stereo Width Slider initialized at value: " + juce::String(audioProcessor.getStereoWidth()));
 
 // Low Cut Slider - Horizontal Fader - Orange/Gold theme
 lowCutSlider.setSliderStyle (juce::Slider::LinearHorizontal);
 lowCutSlider.setRange (20.0, 500.0, 1.0); // 20Hz to 500Hz
 lowCutSlider.setValue (audioProcessor.getLowCutFreq());
 lowCutSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 30);
 lowCutSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xffffa500)); // Orange
 lowCutSlider.setColour (juce::Slider::trackColourId, juce::Colour (0xffffa500).withAlpha(0.6f));
 lowCutSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a1a3f).withAlpha(0.5f));
 lowCutSlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
 lowCutSlider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff1a0f2e).withAlpha(0.8f));
 lowCutSlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xffffa500).withAlpha(0.3f));
 lowCutSlider.setMouseDragSensitivity (100);
 lowCutSlider.setVelocityBasedMode (false);
 lowCutSlider.setTextValueSuffix (" Hz");
 lowCutSlider.setLookAndFeel (modern3DLAF.get()); // Apply modern 3D look!
 lowCutSlider.addListener (this);
 addAndMakeVisible (lowCutSlider);
 
 lowCutLabel.setText ("Low Cut", juce::dontSendNotification);
 lowCutLabel.setJustificationType (juce::Justification::centredLeft);
 lowCutLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffa500));
 lowCutLabel.setFont (juce::Font (14.0f, juce::Font::bold));
 addAndMakeVisible (lowCutLabel);
 
 DBG("Low Cut Slider initialized at value: " + juce::String(audioProcessor.getLowCutFreq()));
 
 // Play Button - Modern purple theme - BIGGER WITH GLOW!
 playButton.setButtonText ("PLAY");
 playButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffff006e)); // Hot pink/magenta border
 playButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffff006e).brighter(0.3f)); // Brighter when clicked
 playButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
 playButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
 playButton.setLookAndFeel (modern3DLAF.get()); // Apply modern 3D look!
 playButton.addListener (this);
 playButton.setEnabled (true);
 addAndMakeVisible (playButton);
 
 // Transition Mode Toggle Button - CYAN/GREEN GLOW!
 transitionModeButton.setButtonText ("REVERSE ONLY");
 transitionModeButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff00d9ff)); // Cyan when OFF
 transitionModeButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff00ff88)); // Green when ON
 transitionModeButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white); // White text when OFF
 transitionModeButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white); // White text when ON
 transitionModeButton.setLookAndFeel (modern3DLAF.get()); // Apply modern 3D look!
 transitionModeButton.addListener (this);
 transitionModeButton.setClickingTogglesState (true);
 transitionModeButton.setToggleState (false, juce::dontSendNotification);
 addAndMakeVisible (transitionModeButton);
 
 transitionModeLabel.setText ("Mode", juce::dontSendNotification);
 transitionModeLabel.setJustificationType (juce::Justification::centred);
 transitionModeLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00d9ff));
 transitionModeLabel.setFont (juce::Font (14.0f, juce::Font::bold)); // Was 12, now 14 - BIGGER!
 addAndMakeVisible (transitionModeLabel);
 
 DBG("Transition mode button added and configured!");
 
 // About Button - Top-left corner - Light purple pill
 aboutButton.setButtonText("About");
 aboutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff9d4edd)); // Light purple
 aboutButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffb537f2)); // Brighter when clicked
 aboutButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
 aboutButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
 aboutButton.setLookAndFeel(modern3DLAF.get());
 aboutButton.addListener(this);
 addAndMakeVisible(aboutButton);
 
 DBG("About button added!");
 
 // Drag visualizer hidden - not needed in UI
 dragVisualizer.setVisible(false);
 
 DBG("DragVisualizer added to editor!");
 DBG(" Parent is DragAndDropContainer: " + juce::String(dynamic_cast<juce::DragAndDropContainer*>(this) != nullptr ? "YES" : "NO"));
 
 // ============================================
 // TREMOLO UI CONTROLS - LFO-driven Gain Modulation
 // ============================================
 
 // Tremolo Enable Button - Green when ON, Purple when OFF
 tremoloEnableButton.setButtonText("TREMOLO OFF");
 tremoloEnableButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff006e)); // Hot pink when OFF
 tremoloEnableButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00ff88)); // Green when ON
 tremoloEnableButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
 tremoloEnableButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
 tremoloEnableButton.setLookAndFeel(modern3DLAF.get()); // Apply modern 3D look!
 tremoloEnableButton.setClickingTogglesState(true);
 tremoloEnableButton.setToggleState(audioProcessor.getTremoloEnabled(), juce::dontSendNotification);
 if (audioProcessor.getTremoloEnabled())
 {
 tremoloEnableButton.setButtonText("TREMOLO ON");
 tremoloExpanded = true;
 }
 tremoloEnableButton.addListener(this);
 addAndMakeVisible(tremoloEnableButton);
 
 tremoloLabel.setText("Tremolo", juce::dontSendNotification);
 tremoloLabel.setJustificationType(juce::Justification::centredLeft);
 tremoloLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ff88));
 tremoloLabel.setFont(juce::Font(16.0f, juce::Font::bold));
 addAndMakeVisible(tremoloLabel);

 // Hide expand button - expand/collapse is handled by the ON/OFF button
 tremoloExpandButton.setVisible(false);
 
 // Tremolo Depth Slider - Rotary Knob - Cyan theme
 tremoloDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
 tremoloDepthSlider.setRange(0.0, 1.0, 0.001);
 tremoloDepthSlider.setValue(audioProcessor.getTremoloDepth());
 tremoloDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
 tremoloDepthSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d9ff));
 tremoloDepthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00d9ff));
 tremoloDepthSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a1a3f));
 tremoloDepthSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
 tremoloDepthSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
 tremoloDepthSlider.setMouseDragSensitivity(200);
 tremoloDepthSlider.setVelocityBasedMode(true);
 tremoloDepthSlider.setVelocityModeParameters(1.0, 1, 0.05, false);
 tremoloDepthSlider.setLookAndFeel(modern3DLAF.get()); // Apply modern 3D look!
 tremoloDepthSlider.addListener(this);
 addAndMakeVisible(tremoloDepthSlider);
 
 tremoloDepthLabel.setText("Depth", juce::dontSendNotification);
 tremoloDepthLabel.setJustificationType(juce::Justification::centred);
 tremoloDepthLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d9ff));
 tremoloDepthLabel.setFont(juce::Font(15.0f, juce::Font::bold)); // Was 14, now 15 - BIGGER!
 addAndMakeVisible(tremoloDepthLabel);
 
 // Tremolo Rate Slider - Rotary Knob - Orange theme
 tremoloRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
 tremoloRateSlider.setRange(0.1, 20.0, 0.01);
 tremoloRateSlider.setValue(audioProcessor.getTremoloRate());
 tremoloRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
 tremoloRateSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffa500));
 tremoloRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffffa500));
 tremoloRateSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a1a3f));
 tremoloRateSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
 tremoloRateSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
 tremoloRateSlider.setMouseDragSensitivity(200);
 tremoloRateSlider.setVelocityBasedMode(true);
 tremoloRateSlider.setVelocityModeParameters(1.0, 1, 0.05, false);
 tremoloRateSlider.setTextValueSuffix(" Hz");
 tremoloRateSlider.setLookAndFeel(modern3DLAF.get()); // Apply modern 3D look!
 tremoloRateSlider.addListener(this);
 // Enabled ONLY if tremolo is ON and sync/ramp are BOTH OFF
 tremoloRateSlider.setEnabled(audioProcessor.getTremoloEnabled() && 
 !audioProcessor.getTremoloSyncEnabled() && 
 !audioProcessor.getTremoloRateRampEnabled());
 addAndMakeVisible(tremoloRateSlider);
 
 tremoloRateLabel.setText("Rate", juce::dontSendNotification);
 tremoloRateLabel.setJustificationType(juce::Justification::centred);
 tremoloRateLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffa500));
 tremoloRateLabel.setFont(juce::Font(15.0f, juce::Font::bold)); // Was 14, now 15 - BIGGER!
 addAndMakeVisible(tremoloRateLabel);
 
 // Tremolo Waveform ComboBox - Purple theme
 tremoloWaveformCombo.addItem("Sine", 1);
 tremoloWaveformCombo.addItem("Triangle", 2);
 tremoloWaveformCombo.addItem("Square", 3);
 tremoloWaveformCombo.setSelectedId(audioProcessor.getTremoloWaveform() + 1, juce::dontSendNotification);
 tremoloWaveformCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a1a3f));
 tremoloWaveformCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
 tremoloWaveformCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff9d4edd));
 tremoloWaveformCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff9d4edd));
 tremoloWaveformCombo.addListener(this);
 addAndMakeVisible(tremoloWaveformCombo);
 
 tremoloWaveformLabel.setText("Waveform", juce::dontSendNotification);
 tremoloWaveformLabel.setJustificationType(juce::Justification::centredLeft);
 tremoloWaveformLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9d4edd));
 tremoloWaveformLabel.setFont(juce::Font(15.0f, juce::Font::bold)); // Was 14, now 15 - BIGGER!
 addAndMakeVisible(tremoloWaveformLabel);
 
 // Tremolo Sync Toggle - Hot Pink theme
 tremoloSyncToggle.setButtonText("Host Sync");
 tremoloSyncToggle.setToggleState(audioProcessor.getTremoloSyncEnabled(), juce::dontSendNotification);
 tremoloSyncToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffff006e));
 tremoloSyncToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffff006e));
 tremoloSyncToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff666666));
 tremoloSyncToggle.addListener(this);
 addAndMakeVisible(tremoloSyncToggle);
 
 // Tremolo Sync Division ComboBox - Hot Pink theme
 tremoloSyncDivisionCombo.addItem("2 Bar", 1);
 tremoloSyncDivisionCombo.addItem("1 Bar", 2);
 tremoloSyncDivisionCombo.addItem("1/1", 3);
 tremoloSyncDivisionCombo.addItem("1/2", 4);
 tremoloSyncDivisionCombo.addItem("1/4", 5);
 tremoloSyncDivisionCombo.addItem("1/8", 6);
 tremoloSyncDivisionCombo.addItem("1/16", 7);
 tremoloSyncDivisionCombo.addItem("1/32", 8);
 tremoloSyncDivisionCombo.addItem("1/64", 9);
 tremoloSyncDivisionCombo.setSelectedId(audioProcessor.getTremoloSyncDivision() + 1, juce::dontSendNotification);
 tremoloSyncDivisionCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a1a3f));
 tremoloSyncDivisionCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
 tremoloSyncDivisionCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffff006e));
 tremoloSyncDivisionCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffff006e));
 tremoloSyncDivisionCombo.setEnabled(audioProcessor.getTremoloSyncEnabled()); // Only enabled if synced
 tremoloSyncDivisionCombo.addListener(this);
 addAndMakeVisible(tremoloSyncDivisionCombo);
 
 tremoloSyncDivisionLabel.setText("Division", juce::dontSendNotification);
 tremoloSyncDivisionLabel.setJustificationType(juce::Justification::centredLeft);
 tremoloSyncDivisionLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff006e));
 tremoloSyncDivisionLabel.setFont(juce::Font(15.0f, juce::Font::bold)); // Was 14, now 15 - BIGGER!
 addAndMakeVisible(tremoloSyncDivisionLabel);
 
 // Tremolo Rate Ramp Toggle - Green/Cyan theme - WORKS WITH OR WITHOUT SYNC! 
 tremoloRateRampToggle.setButtonText("Rate Ramp");
 tremoloRateRampToggle.setToggleState(audioProcessor.getTremoloRateRampEnabled(), juce::dontSendNotification);
 tremoloRateRampToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xff00d9ff)); // Cyan
 tremoloRateRampToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff00ff88)); // Green when ON
 tremoloRateRampToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff666666));
 tremoloRateRampToggle.addListener(this);
 addAndMakeVisible(tremoloRateRampToggle);
 
 tremoloRateRampLabel.setText("Rate Ramp", juce::dontSendNotification);
 tremoloRateRampLabel.setJustificationType(juce::Justification::centredLeft);
 tremoloRateRampLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d9ff));
 tremoloRateRampLabel.setFont(juce::Font(15.0f, juce::Font::bold));
 addAndMakeVisible(tremoloRateRampLabel);
 
 // Tremolo Start Division ComboBox - Cyan theme
 tremoloStartDivisionCombo.addItem("2 Bar", 1);
 tremoloStartDivisionCombo.addItem("1 Bar", 2);
 tremoloStartDivisionCombo.addItem("1/1", 3);
 tremoloStartDivisionCombo.addItem("1/2", 4);
 tremoloStartDivisionCombo.addItem("1/4", 5);
 tremoloStartDivisionCombo.addItem("1/8", 6);
 tremoloStartDivisionCombo.addItem("1/16", 7);
 tremoloStartDivisionCombo.addItem("1/32", 8);
 tremoloStartDivisionCombo.addItem("1/64", 9);
 tremoloStartDivisionCombo.setSelectedId(audioProcessor.getTremoloStartDivision() + 1, juce::dontSendNotification);
 tremoloStartDivisionCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a1a3f));
 tremoloStartDivisionCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
 tremoloStartDivisionCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff00d9ff));
 tremoloStartDivisionCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff00d9ff));
 tremoloStartDivisionCombo.setEnabled(audioProcessor.getTremoloRateRampEnabled());
 tremoloStartDivisionCombo.addListener(this);
 addAndMakeVisible(tremoloStartDivisionCombo);
 
 tremoloStartDivisionLabel.setText("Start", juce::dontSendNotification);
 tremoloStartDivisionLabel.setJustificationType(juce::Justification::centredLeft);
 tremoloStartDivisionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d9ff));
 tremoloStartDivisionLabel.setFont(juce::Font(15.0f, juce::Font::bold));
 addAndMakeVisible(tremoloStartDivisionLabel);
 
 // Tremolo End Division ComboBox - Orange theme
 tremoloEndDivisionCombo.addItem("2 Bar", 1);
 tremoloEndDivisionCombo.addItem("1 Bar", 2);
 tremoloEndDivisionCombo.addItem("1/1", 3);
 tremoloEndDivisionCombo.addItem("1/2", 4);
 tremoloEndDivisionCombo.addItem("1/4", 5);
 tremoloEndDivisionCombo.addItem("1/8", 6);
 tremoloEndDivisionCombo.addItem("1/16", 7);
 tremoloEndDivisionCombo.addItem("1/32", 8);
 tremoloEndDivisionCombo.addItem("1/64", 9);
 tremoloEndDivisionCombo.setSelectedId(audioProcessor.getTremoloEndDivision() + 1, juce::dontSendNotification);
 tremoloEndDivisionCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a1a3f));
 tremoloEndDivisionCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
 tremoloEndDivisionCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffffa500));
 tremoloEndDivisionCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffffa500));
 tremoloEndDivisionCombo.setEnabled(audioProcessor.getTremoloRateRampEnabled());
 tremoloEndDivisionCombo.addListener(this);
 addAndMakeVisible(tremoloEndDivisionCombo);
 
 tremoloEndDivisionLabel.setText("End", juce::dontSendNotification);
 tremoloEndDivisionLabel.setJustificationType(juce::Justification::centredLeft);
 tremoloEndDivisionLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffa500));
 tremoloEndDivisionLabel.setFont(juce::Font(15.0f, juce::Font::bold));
 addAndMakeVisible(tremoloEndDivisionLabel);
 
 DBG("Tremolo UI controls initialized!");
 DBG("Tremolo Rate Ramp controls added!");
 
 // Add live waveform display
 addAndMakeVisible(*waveformDisplay);

 DBG("Live waveform display added!");
 
 // Start timer for animations AND throttled processing (30ms = 33fps for smooth animations)
 startTimer (30);
 
 // DRAG-TO-DAW INFO:
 // The Editor inherits from DragAndDropContainer (see header)
 // User can DRAG the DragVisualizer directly into FL Studio/Ableton/Desktop!
 // - DragVisualizer handles all drag logic internally
 // - No separate EXPORT button needed - just drag!
 DBG("Drag-to-DAW enabled! Drag the button to export.");
}


ReverseReverbAudioProcessorEditor::~ReverseReverbAudioProcessorEditor()
{
 // Stop timer
 stopTimer();
 
 // Clean up 3D LookAndFeel (set to nullptr for all components)
 reverbSizeSlider.setLookAndFeel(nullptr);
 dryWetSlider.setLookAndFeel(nullptr);
 tailDivisionSlider.setLookAndFeel(nullptr);
 bpmSlider.setLookAndFeel(nullptr);
 stereoWidthSlider.setLookAndFeel(nullptr);
 lowCutSlider.setLookAndFeel(nullptr);
 playButton.setLookAndFeel(nullptr);
 transitionModeButton.setLookAndFeel(nullptr);
 aboutButton.setLookAndFeel(nullptr);
 
 // Tremolo controls
 tremoloEnableButton.setLookAndFeel(nullptr);
 tremoloExpandButton.setLookAndFeel(nullptr);
 tremoloDepthSlider.setLookAndFeel(nullptr);
 tremoloRateSlider.setLookAndFeel(nullptr);
 
 // Delete 3D components
 waveformDisplay.reset();
 modern3DLAF.reset();
}

void ReverseReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
 // Try to draw background image first
 if (backgroundImage.isValid())
 {
 g.drawImage(backgroundImage, getLocalBounds().toFloat(),
 juce::RectanglePlacement::fillDestination);
 }
 else
 {
 // Dark background base
 g.fillAll(DesignColours::background);

 // Subtle gradient - slightly lighter towards bottom with teal hint
 juce::ColourGradient gradient(
 DesignColours::background, 0, 0,
 DesignColours::backgroundLight, 0, (float)getHeight(),
 false);
 g.setGradientFill(gradient);
 g.fillRect(getLocalBounds());

 // Circuit board pattern in the lower portion (below waveform)
 auto circuitArea = getLocalBounds();
 circuitArea.removeFromTop(waveformArea.getBottom());
 drawCircuitBoardPattern(g, circuitArea);
 }

 float sf = getWidth() / 700.0f;

 // Draw logo banner area
 if (logoImage.isValid())
 {
 // Draw the user's logo/background image in the banner area
 g.drawImage(logoImage, logoArea.toFloat(),
  juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
 }
 else
 {
 // Placeholder: subtle gradient banner with animated title
 juce::ColourGradient bannerGrad(
  juce::Colour(0xff0f0a1e), 0, (float)logoArea.getY(),
  DesignColours::background, 0, (float)logoArea.getBottom(), false);
 bannerGrad.addColour(0.5, juce::Colour(0xff150d28));
 g.setGradientFill(bannerGrad);
 g.fillRect(logoArea);

 // Subtle bottom edge glow
 float edgeGlow = 0.3f + 0.15f * std::sin(animationPhase * 1.2f);
 auto edgeColour = DesignColours::purple.withAlpha(edgeGlow * 0.4f);
 g.setColour(edgeColour);
 g.fillRect(logoArea.getX(), logoArea.getBottom() - 1, logoArea.getWidth(), 1);
 }

 // Draw tremolo strip border
 if (tremoloBoxBounds.getWidth() > 0 && tremoloBoxBounds.getHeight() > 0)
 {
 float shimmer = juce::jlimit(0.0f, 1.0f, 0.25f + 0.15f * std::sin(animationPhase * 2.0f));
 g.setColour(DesignColours::hotPink.withAlpha(shimmer));
 g.drawRoundedRectangle(tremoloBoxBounds.toFloat(), 8.0f, 1.5f);
 }

 // Tremolo arrow removed (expand/collapse handled by ON/OFF button text)
}

void ReverseReverbAudioProcessorEditor::drawCircuitBoardPattern(
 juce::Graphics& g, juce::Rectangle<int> area)
{
 if (area.getWidth() <= 0 || area.getHeight() <= 0)
 return;

 juce::Random rng(42); // deterministic so pattern is stable

 auto colour = DesignColours::circuitTeal.withAlpha(0.12f);

 // Horizontal PCB traces with right-angle bends
 for (int i = 0; i < 18; ++i)
 {
 float y = area.getY() + rng.nextFloat() * area.getHeight();
 float startX = area.getX() + rng.nextFloat() * area.getWidth() * 0.3f;
 float endX = startX + rng.nextFloat() * area.getWidth() * 0.5f;
 endX = juce::jmin(endX, (float)area.getRight());

 juce::Path trace;
 trace.startNewSubPath(startX, y);
 float midX = (startX + endX) * 0.5f;
 float bendY = y + (rng.nextFloat() - 0.5f) * 40.0f;
 bendY = juce::jlimit((float)area.getY(), (float)area.getBottom(), bendY);
 trace.lineTo(midX, y);
 trace.lineTo(midX, bendY);
 trace.lineTo(endX, bendY);

 g.setColour(colour);
 g.strokePath(trace, juce::PathStrokeType(1.0f));

 // Solder pads at endpoints
 g.fillEllipse(startX - 2.0f, y - 2.0f, 4.0f, 4.0f);
 g.fillEllipse(endX - 2.0f, bendY - 2.0f, 4.0f, 4.0f);
 }

 // Vertical traces
 for (int i = 0; i < 12; ++i)
 {
 float x = area.getX() + rng.nextFloat() * area.getWidth();
 float startY = area.getY() + rng.nextFloat() * area.getHeight() * 0.3f;
 float endY = startY + rng.nextFloat() * area.getHeight() * 0.6f;
 endY = juce::jmin(endY, (float)area.getBottom());

 juce::Path trace;
 trace.startNewSubPath(x, startY);
 float midY = (startY + endY) * 0.5f;
 float bendX = x + (rng.nextFloat() - 0.5f) * 30.0f;
 bendX = juce::jlimit((float)area.getX(), (float)area.getRight(), bendX);
 trace.lineTo(x, midY);
 trace.lineTo(bendX, midY);
 trace.lineTo(bendX, endY);

 g.setColour(colour);
 g.strokePath(trace, juce::PathStrokeType(1.0f));

 g.fillEllipse(x - 2.0f, startY - 2.0f, 4.0f, 4.0f);
 g.fillEllipse(bendX - 2.0f, endY - 2.0f, 4.0f, 4.0f);
 }

 // IC package outlines (small rectangles with pin dots)
 auto padColour = DesignColours::circuitTeal.withAlpha(0.08f);
 for (int i = 0; i < 5; ++i)
 {
 float rx = area.getX() + rng.nextFloat() * (area.getWidth() - 40);
 float ry = area.getY() + rng.nextFloat() * (area.getHeight() - 25);
 float rw = 20.0f + rng.nextFloat() * 15.0f;
 float rh = 10.0f + rng.nextFloat() * 10.0f;

 g.setColour(padColour);
 g.drawRoundedRectangle(rx, ry, rw, rh, 2.0f, 0.8f);

 // Pin dots
 int numPins = (int)(rw / 6.0f);
 for (int pin = 0; pin < numPins; pin++)
 {
 g.fillEllipse(rx + pin * 6.0f + 2.0f, ry - 3.0f, 2.0f, 2.0f);
 g.fillEllipse(rx + pin * 6.0f + 2.0f, ry + rh + 1.0f, 2.0f, 2.0f);
 }
 }
}

void ReverseReverbAudioProcessorEditor::resized()
{
 auto area = getLocalBounds();
 
 // Calculate scale factor based on width only (height changes with tremolo expand)
 float scaleFactor = getWidth() / 700.0f;
 
 // Scale margins and spacing proportionally
 auto margin = juce::roundToInt(10 * scaleFactor);
 auto spacing = juce::roundToInt(8 * scaleFactor);
 
 // Logo banner area at the very top
 auto logoHeight = juce::roundToInt(90 * scaleFactor);
 logoArea = area.removeFromTop(logoHeight);

 // Options button - top-left corner overlaying logo area
 auto optionW = juce::roundToInt(75 * scaleFactor);
 auto optionH = juce::roundToInt(26 * scaleFactor);
 aboutButton.setBounds(margin, juce::roundToInt(5 * scaleFactor), optionW, optionH);

 // Live waveform display takes the full top area
 waveformArea = area.removeFromTop(juce::roundToInt(210 * scaleFactor)).reduced(margin);
 waveformDisplay->setBounds(waveformArea);
 
 statusLabel.setBounds(area.removeFromTop(juce::roundToInt(16 * scaleFactor)).reduced(margin));
 statusLabel.setFont(juce::Font(11.0f * scaleFactor));
 
 area.removeFromTop(spacing);

 // ==== MAIN CONTROLS AREA ====
 auto workArea = area.reduced(margin);

 // Row 1: Play (half) + Mode (half) side by side - tall pill buttons
 auto btnRow = workArea.removeFromTop(juce::roundToInt(42 * scaleFactor));
 auto btnHalf = (btnRow.getWidth() - spacing) / 2;
 playButton.setBounds(btnRow.removeFromLeft(btnHalf));
 btnRow.removeFromLeft(spacing);
 transitionModeLabel.setVisible(false);
 transitionModeButton.setBounds(btnRow);

 workArea.removeFromTop(spacing);

 // Row 2: Left side = Stereo + LowCut faders, Right side = 3 Knobs
 auto controlsArea = workArea.removeFromTop(juce::roundToInt(130 * scaleFactor));
 auto leftControls = controlsArea.removeFromLeft(controlsArea.getWidth() / 2 - spacing / 2);
 controlsArea.removeFromLeft(spacing);
 auto rightControls = controlsArea;

 // -- Left: Stereo Width fader
 auto stereoArea = leftControls.removeFromTop(juce::roundToInt(40 * scaleFactor));
 auto stereoLabelWidth = juce::roundToInt(85 * scaleFactor);
 stereoWidthLabel.setBounds(stereoArea.removeFromLeft(stereoLabelWidth));
 stereoWidthLabel.setFont(juce::Font(13.0f * scaleFactor, juce::Font::bold));
 stereoWidthSlider.setBounds(stereoArea.reduced(juce::roundToInt(10 * scaleFactor), juce::roundToInt(4 * scaleFactor)));

 leftControls.removeFromTop(juce::roundToInt(4 * scaleFactor));

 // -- Left: Low Cut fader
 auto lowCutArea = leftControls.removeFromTop(juce::roundToInt(40 * scaleFactor));
 auto lowCutLabelWidth = juce::roundToInt(85 * scaleFactor);
 lowCutLabel.setBounds(lowCutArea.removeFromLeft(lowCutLabelWidth));
 lowCutLabel.setFont(juce::Font(13.0f * scaleFactor, juce::Font::bold));
 lowCutSlider.setBounds(lowCutArea.reduced(juce::roundToInt(10 * scaleFactor), juce::roundToInt(4 * scaleFactor)));

 // -- Right: 3 Knobs
 auto labelHeight = juce::roundToInt(16 * scaleFactor);
 auto knobsArea = rightControls;
 auto knobWidth = (knobsArea.getWidth() - spacing * 2) / 3;

 // Reverb Size
 auto sizeArea = knobsArea.removeFromLeft(knobWidth);
 reverbSizeLabel.setBounds(sizeArea.removeFromTop(labelHeight));
 reverbSizeLabel.setFont(juce::Font(13.0f * scaleFactor, juce::Font::bold));
 reverbSizeSlider.setBounds(sizeArea.reduced(0, juce::roundToInt(4 * scaleFactor)));

 knobsArea.removeFromLeft(spacing);

 // Tail Length knob (replaces Feedback knob)
 auto tailArea = knobsArea.removeFromLeft(knobWidth);
 tailDivisionLabel.setBounds(tailArea.removeFromTop(labelHeight));
 tailDivisionLabel.setFont(juce::Font(13.0f * scaleFactor, juce::Font::bold));
 tailDivisionSlider.setBounds(tailArea.reduced(0, juce::roundToInt(4 * scaleFactor)));

 knobsArea.removeFromLeft(spacing);

 // Gain
 dryWetLabel.setBounds(knobsArea.removeFromTop(labelHeight));
 dryWetLabel.setFont(juce::Font(13.0f * scaleFactor, juce::Font::bold));
 dryWetSlider.setBounds(knobsArea.reduced(0, juce::roundToInt(4 * scaleFactor)));

 workArea.removeFromTop(spacing);

 // BPM row (standalone: slider, VST3: auto label)
 auto bpmRowH = juce::roundToInt(22 * scaleFactor);
 auto bpmRow = workArea.removeFromTop(bpmRowH);
 if (audioProcessor.isStandalone())
 {
     auto bpmLblW = juce::roundToInt(35 * scaleFactor);
     bpmDisplayLabel.setBounds(bpmRow.removeFromLeft(bpmLblW));
     bpmDisplayLabel.setFont(juce::Font(10.0f * scaleFactor, juce::Font::bold));
     bpmSlider.setBounds(bpmRow.reduced(0, juce::roundToInt(1 * scaleFactor)));
     bpmSlider.setVisible(true);
 }
 else
 {
     bpmDisplayLabel.setBounds(bpmRow);
     bpmDisplayLabel.setFont(juce::Font(10.0f * scaleFactor));
     bpmSlider.setVisible(false);
 }

 workArea.removeFromTop(spacing);

 // Drop zone is now integrated into the waveform display area
 dropZone = waveformArea;
 dropLabel.setVisible(false);
 // DragVisualizer will be positioned after tremolo section

 // ==== TREMOLO STRIP (Full-width, collapsible) ====
 auto tremoloHeaderHeight = juce::roundToInt(38 * scaleFactor);
 auto tremoloExpandedHeight = juce::roundToInt(175 * scaleFactor);
 auto tremoloTotalHeight = tremoloHeaderHeight + (tremoloExpanded ? tremoloExpandedHeight : 0);
 auto tremoloStripArea = workArea.removeFromTop(tremoloTotalHeight);

 tremoloBoxBounds = tremoloStripArea;
 auto tremContent = tremoloStripArea.reduced(juce::roundToInt(8 * scaleFactor), juce::roundToInt(4 * scaleFactor));

 // Header row: ON/OFF button full width
 auto headerRow = tremContent.removeFromTop(juce::roundToInt(30 * scaleFactor));
 tremoloLabel.setVisible(false);
 tremoloEnableButton.setBounds(headerRow);

 // Tremolo controls - only show if expanded
 bool showTremControls = tremoloExpanded;
 tremoloDepthLabel.setVisible(showTremControls);
 tremoloDepthSlider.setVisible(showTremControls);
 tremoloRateLabel.setVisible(showTremControls);
 tremoloRateSlider.setVisible(showTremControls);
 tremoloWaveformLabel.setVisible(showTremControls);
 tremoloWaveformCombo.setVisible(showTremControls);
 tremoloSyncToggle.setVisible(showTremControls);
 tremoloSyncDivisionLabel.setVisible(showTremControls);
 tremoloSyncDivisionCombo.setVisible(showTremControls);
 tremoloRateRampToggle.setVisible(showTremControls);
 tremoloStartDivisionLabel.setVisible(showTremControls);
 tremoloStartDivisionCombo.setVisible(showTremControls);
 tremoloEndDivisionLabel.setVisible(showTremControls);
 tremoloEndDivisionCombo.setVisible(showTremControls);

 if (showTremControls)
 {
 tremContent.removeFromTop(juce::roundToInt(5 * scaleFactor));

 // Row 1: Depth knob | Rate knob | Waveform combo + Sync
 auto row1 = tremContent.removeFromTop(juce::roundToInt(110 * scaleFactor));
 auto knobSize = juce::roundToInt(110 * scaleFactor);

 // Depth knob
 auto depthCol = row1.removeFromLeft(knobSize);
 tremoloDepthLabel.setBounds(depthCol.removeFromTop(juce::roundToInt(15 * scaleFactor)));
 tremoloDepthLabel.setFont(juce::Font(13.0f * scaleFactor, juce::Font::bold));
 tremoloDepthSlider.setBounds(depthCol);

 row1.removeFromLeft(spacing);

 // Rate knob
 auto rateCol = row1.removeFromLeft(knobSize);
 tremoloRateLabel.setBounds(rateCol.removeFromTop(juce::roundToInt(15 * scaleFactor)));
 tremoloRateLabel.setFont(juce::Font(13.0f * scaleFactor, juce::Font::bold));
 tremoloRateSlider.setBounds(rateCol);

 row1.removeFromLeft(spacing * 2);

 // Right side: Waveform combo + Sync + Division (stacked)
 auto rcArea = row1;
 auto rcRowH = juce::roundToInt(28 * scaleFactor);

 // Waveform combo
 auto wfRow = rcArea.removeFromTop(rcRowH);
 auto waveLblW = juce::roundToInt(55 * scaleFactor);
 tremoloWaveformLabel.setBounds(wfRow.removeFromLeft(waveLblW));
 tremoloWaveformLabel.setFont(juce::Font(9.5f * scaleFactor, juce::Font::bold));
 tremoloWaveformCombo.setBounds(wfRow.reduced(0, juce::roundToInt(2 * scaleFactor)));

 rcArea.removeFromTop(juce::roundToInt(3 * scaleFactor));

 // Sync toggle + Division
 auto syncRow = rcArea.removeFromTop(rcRowH);
 auto syncToggleW = juce::roundToInt(90 * scaleFactor);
 tremoloSyncToggle.setBounds(syncRow.removeFromLeft(syncToggleW));
 auto divLblW = juce::roundToInt(45 * scaleFactor);
 tremoloSyncDivisionLabel.setBounds(syncRow.removeFromLeft(divLblW));
 tremoloSyncDivisionLabel.setFont(juce::Font(9.5f * scaleFactor, juce::Font::bold));
 tremoloSyncDivisionCombo.setBounds(syncRow.reduced(0, juce::roundToInt(2 * scaleFactor)));

 rcArea.removeFromTop(juce::roundToInt(3 * scaleFactor));

 // Rate Ramp + Start/End in the remaining space
 auto rampRow = rcArea.removeFromTop(rcRowH);
 auto rampToggleW = juce::roundToInt(110 * scaleFactor);
 tremoloRateRampToggle.setBounds(rampRow.removeFromLeft(rampToggleW));
 rampRow.removeFromLeft(spacing);
 auto divColWidth = (rampRow.getWidth() - spacing) / 2;

 // Start
 auto startCol = rampRow.removeFromLeft(divColWidth);
 auto startLblW = juce::roundToInt(35 * scaleFactor);
 tremoloStartDivisionLabel.setBounds(startCol.removeFromLeft(startLblW));
 tremoloStartDivisionLabel.setFont(juce::Font(9.0f * scaleFactor, juce::Font::bold));
 tremoloStartDivisionCombo.setBounds(startCol);

 rampRow.removeFromLeft(spacing);

 // End
 auto endLblW = juce::roundToInt(28 * scaleFactor);
 tremoloEndDivisionLabel.setBounds(rampRow.removeFromLeft(endLblW));
 tremoloEndDivisionLabel.setFont(juce::Font(9.0f * scaleFactor, juce::Font::bold));
 tremoloEndDivisionCombo.setBounds(rampRow);
 }


 // Mark waveform for update when resizing
 waveformNeedsUpdate = true;
}

bool ReverseReverbAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
 // CRITICAL: Ignore files from our own temp directory (drag OUT operations)
 for (auto file : files)
 {
 // Check if this is our own exported file (starts with "Reverse Reverb -" or "ReverseReverb_")
 juce::File checkFile(file);
 auto fileName = checkFile.getFileName();
 
 if ((fileName.startsWith("Reverse Reverb -") || fileName.startsWith("ReverseReverb_")) && 
 file.contains(juce::File::getSpecialLocation(juce::File::tempDirectory).getFullPathName()))
 {
 DBG("Ignoring our own exported file: "+ fileName);
 return false; // Don't accept our own exports!
 }
 
 // Accept external audio files
 if (file.endsWith(".wav") || file.endsWith(".mp3") || 
 file.endsWith(".aiff") || file.endsWith(".flac") ||
 file.endsWith(".WAV") || file.endsWith(".MP3") || 
 file.endsWith(".AIFF") || file.endsWith(".FLAC"))
 {
 DBG("Accepting external file: "+ file);
 isDraggingOver = true;
 waveformDisplay->setDraggingOver(true);
 repaint();
 return true;
 }
 }
 return false;
}

void ReverseReverbAudioProcessorEditor::fileDragExit (const juce::StringArray&)
{
 isDraggingOver = false;
 waveformDisplay->setDraggingOver(false);
 repaint();
}

void ReverseReverbAudioProcessorEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
 isDraggingOver = false;
 waveformDisplay->setDraggingOver(false);
 repaint();
 
 if (files.size() > 0)
 {
 juce::File audioFile (files[0]);
 
 // Check file duration before loading
 std::unique_ptr<juce::AudioFormatReader> reader(
 audioProcessor.formatManager.createReaderFor(audioFile));
 
 if (reader != nullptr)
 {
 auto duration = reader->lengthInSamples / reader->sampleRate;
 
 if (duration > 8.0)
 {
 juce::AlertWindow::showMessageBoxAsync(
 juce::AlertWindow::WarningIcon,
 "File Too Long",
 "Please load a sample shorter than 8 seconds.\n\nThis file is " + 
 juce::String(duration, 1) + " seconds.",
 "OK"
 );
 updateStatus("Error: File too long (" + juce::String(duration, 1) + "s)");
 return;
 }
 }
 
 audioProcessor.loadAudioFile (audioFile);
 updateStatus ("Loaded & Processed: " + audioFile.getFileName());
 // Update waveform display
 updateWaveformWithTremolo();
 waveformNeedsUpdate = true;
 repaint();
 }
}

void ReverseReverbAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
 if (slider == &reverbSizeSlider)
 {
 audioProcessor.setReverbSize ((float)slider->getValue());
 // Schedule processing for reverb size changes
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus ("Updating...");
 }
 }
 else if (slider == &dryWetSlider)
 {
 audioProcessor.setDryWet ((float)slider->getValue());
 waveformDisplay->setGain((float)slider->getValue());
 return; // No need to schedule processing for gain
 }
 else if (slider == &tailDivisionSlider)
 {
 audioProcessor.setTailDivision((int)slider->getValue());
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus("Tail: " + tailDivisionSlider.getTextFromValue(slider->getValue()));
 }
 }
 else if (slider == &bpmSlider)
 {
 audioProcessor.setManualBpm((float)slider->getValue());
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus("BPM: " + juce::String((int)slider->getValue()));
 }
 }
 else if (slider == &stereoWidthSlider)
 {
 audioProcessor.setStereoWidth ((float)slider->getValue());
 
 // Update label to show current mode
 float value = slider->getValue();
 if (value < 0.4f)
 stereoWidthLabel.setText ("Stereo Width (Narrow)", juce::dontSendNotification);
 else if (value < 0.6f)
 stereoWidthLabel.setText ("Stereo Width (Normal)", juce::dontSendNotification);
 else
 stereoWidthLabel.setText ("Stereo Width (Wide)", juce::dontSendNotification);
 
 // Schedule processing for stereo width changes
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus ("Updating...");
 }
 }
 else if (slider == &lowCutSlider)
 {
 audioProcessor.setLowCutFreq ((float)slider->getValue());
 
 // Schedule processing for low cut changes
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus ("Updating...");
 }
 }
 else if (slider == &tremoloDepthSlider)
 {
 audioProcessor.setTremoloDepth((float)slider->getValue());
 // Update waveform to show tremolo effect
 updateWaveformWithTremolo();
 }
 else if (slider == &tremoloRateSlider)
 {
 audioProcessor.setTremoloRate((float)slider->getValue());
 // Update waveform to show tremolo effect
 updateWaveformWithTremolo();
 }
}

void ReverseReverbAudioProcessorEditor::mouseMove (const juce::MouseEvent& event)
{
 auto pos = event.getPosition();
 
 // Check if hovering over waveform area
 if (waveformArea.contains (pos) && !audioProcessor.isSampleLoaded())
 {
 setMouseCursor (juce::MouseCursor::PointingHandCursor);
 return;
 }

 // Check if hovering over waveform area for fade handles
 if (waveformArea.contains (pos) && audioProcessor.isSampleLoaded())
 {
 auto waveArea = waveformArea.reduced(5);
 auto relativeX = pos.x - waveArea.getX();
 auto width = waveArea.getWidth();
 
 // Calculate fade handle positions (the LINE position, not center)
 auto fadeInHandleX = fadeInAmount * width;
 auto fadeOutHandleX = width - (fadeOutAmount * width);
 
 // Larger hit area for easier grabbing (20 pixels instead of 10)
 const float hitArea = 20.0f;
 
 // Check if near fade in handle (at the END of fade in zone - the line)
 if (fadeInAmount > 0.01f && std::abs(relativeX - fadeInHandleX) < hitArea)
 {
 setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
 return;
 }
 
 // Check if near fade out handle (at the START of fade out zone - the line)
 if (fadeOutAmount > 0.01f && std::abs(relativeX - fadeOutHandleX) < hitArea)
 {
 setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
 return;
 }
 
 // Check if in left 15% (for creating new fade in)
 if (fadeInAmount < 0.01f && relativeX < width * 0.15f)
 {
 setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
 return;
 }
 
 // Check if in right 15% (for creating new fade out)
 if (fadeOutAmount < 0.01f && relativeX > width * 0.85f)
 {
 setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
 return;
 }

 // Not near any fade handle - show drag cursor for drag-to-DAW
 setMouseCursor (juce::MouseCursor::DraggingHandCursor);
 return;
 }

 setMouseCursor (juce::MouseCursor::NormalCursor);
}

void ReverseReverbAudioProcessorEditor::mouseDown (const juce::MouseEvent& event)
{
 auto pos = event.getPosition();
 
 // Check if click is inside waveform area for fade adjustment
 if (waveformArea.contains (pos) && audioProcessor.isSampleLoaded())
 {
 auto waveArea = waveformArea.reduced(5);
 auto relativeX = pos.x - waveArea.getX();
 auto width = waveArea.getWidth();
 
 // Calculate fade handle positions (the LINE position, not center)
 auto fadeInHandleX = fadeInAmount * width;
 auto fadeOutHandleX = width - (fadeOutAmount * width);
 
 // Larger hit area for easier grabbing (20 pixels instead of 10)
 const float hitArea = 20.0f;
 
 // Check if clicking on fade in handle (at the END of fade in zone - the line)
 if (fadeInAmount > 0.01f && std::abs(relativeX - fadeInHandleX) < hitArea)
 {
 isDraggingFadeIn = true;
 return;
 }
 
 // Check if clicking on fade out handle (at the START of fade out zone - the line)
 if (fadeOutAmount > 0.01f && std::abs(relativeX - fadeOutHandleX) < hitArea)
 {
 isDraggingFadeOut = true;
 return;
 }
 
 // Check if clicking near left edge to create fade in
 if (fadeInAmount < 0.01f && relativeX < width * 0.15f)
 {
 isDraggingFadeIn = true;
 fadeInAmount = juce::jmax(0.0f, (float)(relativeX / width));
 audioProcessor.setFadeIn(fadeInAmount);
 repaint();
 return;
 }
 
 // Check if clicking near right edge to create fade out
 if (fadeOutAmount < 0.01f && relativeX > width * 0.85f)
 {
 isDraggingFadeOut = true;
 auto fadeOutPos = (float)((width - relativeX) / width);
 fadeOutAmount = juce::jmax(0.0f, fadeOutPos);
 audioProcessor.setFadeOut(fadeOutAmount);
 repaint();
 return;
 }
 }
 
 // Click on waveform area (not on fade handle)
 // If sample loaded: wait to see if this becomes a drag (export) or just a click (file browser)
 // If no sample: open file browser immediately
 if (waveformArea.contains (pos))
 {
 if (audioProcessor.isSampleLoaded())
 {
 // Defer action - might be drag-to-DAW or click-to-browse
 waveformClickPending = true;
 isDraggingToDAW = false;
 return;
 }

 // No sample loaded - open file browser immediately
 openFileBrowser();
 }
}

void ReverseReverbAudioProcessorEditor::openFileBrowser()
{
 auto chooser = std::make_shared<juce::FileChooser>(
 "Select an audio file...",
 juce::File::getSpecialLocation(juce::File::userHomeDirectory),
 "*.wav;*.mp3;*.aiff;*.flac");

 auto flags = juce::FileBrowserComponent::openMode |
 juce::FileBrowserComponent::canSelectFiles;

 chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc)
 {
 auto audioFile = fc.getResult();

 if (audioFile == juce::File{})
 return;

 // Check file duration before loading
 std::unique_ptr<juce::AudioFormatReader> reader(
 audioProcessor.formatManager.createReaderFor(audioFile));

 if (reader != nullptr)
 {
 auto duration = reader->lengthInSamples / reader->sampleRate;

 if (duration > 8.0)
 {
 juce::AlertWindow::showMessageBoxAsync(
 juce::AlertWindow::WarningIcon,
 "File Too Long",
 "Please load a sample shorter than 8 seconds.\n\nThis file is " +
 juce::String(duration, 1) + " seconds.",
 "OK"
 );
 updateStatus("Error: File too long (" + juce::String(duration, 1) + "s)");
 return;
 }
 }

 audioProcessor.loadAudioFile(audioFile);
 updateStatus("Loaded & Processed: " + audioFile.getFileName());
 updateWaveformWithTremolo();
 waveformNeedsUpdate = true;
 repaint();
 });
}

void ReverseReverbAudioProcessorEditor::mouseDrag (const juce::MouseEvent& event)
{
 // Handle fade in/out dragging
 if (isDraggingFadeIn || isDraggingFadeOut)
 {
 auto waveArea = waveformArea.reduced(5);
 auto relativeX = event.position.x - waveArea.getX();
 auto width = waveArea.getWidth();

 if (isDraggingFadeIn)
 {
 fadeInAmount = juce::jlimit(0.0f, 1.0f, (float)(relativeX / width));
 audioProcessor.setFadeIn(fadeInAmount);
 waveformDisplay->repaint();
 repaint();
 }
 else if (isDraggingFadeOut)
 {
 auto fadeOutPos = (float)((width - relativeX) / width);
 fadeOutAmount = juce::jlimit(0.0f, 1.0f, fadeOutPos);
 audioProcessor.setFadeOut(fadeOutAmount);
 waveformDisplay->repaint();
 repaint();
 }
 return;
 }

 // Handle drag-to-DAW from waveform area
 if (waveformClickPending && !isDraggingToDAW && audioProcessor.isSampleLoaded())
 {
 if (event.getDistanceFromDragStart() > 5)
 {
 isDraggingToDAW = true;
 waveformClickPending = false;
 performDragToDAW();
 }
 }
}

void ReverseReverbAudioProcessorEditor::performDragToDAW()
{
 if (!audioProcessor.isSampleLoaded())
 return;

 // Create temp file
 auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);

 // Build file name: "Reverse Reverb - [Original Name]"
 auto originalName = audioProcessor.getLoadedFileName();
 juce::String exportFileName;

 if (originalName.isNotEmpty())
 exportFileName = "Reverse Reverb - " + originalName + ".wav";
 else
 {
 auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
 exportFileName = "ReverseReverb_" + timestamp + ".wav";
 }

 auto tempFile = tempDir.getChildFile(exportFileName);

 if (audioProcessor.exportProcessedAudio(tempFile))
 {
 if (tempFile.existsAsFile())
 {
 juce::StringArray files;
 files.add(tempFile.getFullPathName());
 performExternalDragDropOfFiles(files, true);
 updateStatus("Drag to DAW!");
 }
 }
 else
 {
 updateStatus("Export failed");
 }
}

void ReverseReverbAudioProcessorEditor::mouseUp (const juce::MouseEvent& event)
{
 // Handle fade in/out mouse up
 if (isDraggingFadeIn || isDraggingFadeOut)
 {
 isDraggingFadeIn = false;
 isDraggingFadeOut = false;

 updateStatus("Fade updated");
 repaint();
 }

 // If waveform click was pending and no drag happened, open file browser
 if (waveformClickPending)
 {
 waveformClickPending = false;
 openFileBrowser();
 }

 isDraggingToDAW = false;
}

void ReverseReverbAudioProcessorEditor::mouseDoubleClick (const juce::MouseEvent& event)
{
 auto pos = event.getPosition();
 
 // Check if double-clicking on reverb size knob
 if (reverbSizeSlider.getBounds().contains(pos))
 {
 reverbSizeSlider.setValue(defaultReverbSize); // Default
 audioProcessor.setReverbSize(defaultReverbSize);
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus("Reset Room Size");
 }
 return;
 }
 
 // Check if double-clicking on gain knob
 if (dryWetSlider.getBounds().contains(pos))
 {
 dryWetSlider.setValue(defaultDryWet);
 audioProcessor.setDryWet(defaultDryWet);
 updateStatus("Reset Gain to 0dB");
 repaint(); // Update waveform display
 return;
 }
 
 // Check if double-clicking on tail length knob
 if (tailDivisionSlider.getBounds().contains(pos))
 {
 tailDivisionSlider.setValue(defaultTailDivision);
 audioProcessor.setTailDivision(defaultTailDivision);
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus("Reset Tail Length");
 }
 return;
 }
 
 // Check if double-clicking on stereo width fader
 if (stereoWidthSlider.getBounds().contains(pos))
 {
 stereoWidthSlider.setValue(defaultStereoWidth);
 audioProcessor.setStereoWidth(defaultStereoWidth);
 stereoWidthLabel.setText("Stereo Width (Normal)", juce::dontSendNotification);
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus("Reset Stereo Width");
 }
 return;
 }
 
 // Check if double-clicking on low cut fader
 if (lowCutSlider.getBounds().contains(pos))
 {
 lowCutSlider.setValue(defaultLowCut);
 audioProcessor.setLowCutFreq(defaultLowCut);
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus("Reset Low Cut");
 }
 return;
 }
}

void ReverseReverbAudioProcessorEditor::buttonClicked (juce::Button* button)
{
 if (button == &playButton)
 {
 if (audioProcessor.isSampleLoaded())
 {
 audioProcessor.triggerSample();
 updateStatus ("Playing processed sample...");
 playButton.setButtonText ("PLAYING...");
 
 // Reset button text after a delay
 juce::Timer::callAfterDelay(100, [this]() 
 {
 playButton.setButtonText ("PLAY");
 });
 }
 else
 {
 updateStatus ("Please load a sample first");
 }
 }
 else if (button == &aboutButton)
 {
 // Show beautiful animated About window
 DBG("About button clicked!");
 
 auto* aboutWindow = new AboutWindow();
 addAndMakeVisible(aboutWindow);
 
 // Center the about window with animation
 auto centerX = (getWidth() - aboutWindow->getWidth()) / 2;
 auto centerY = (getHeight() - aboutWindow->getHeight()) / 2;
 aboutWindow->setBounds(centerX, centerY, aboutWindow->getWidth(), aboutWindow->getHeight());
 
 // Bring to front and make modal
 aboutWindow->setAlwaysOnTop(true);
 aboutWindow->toFront(true);
 
 // Fade in animation
 aboutWindow->setAlpha(0.0f);
 juce::Desktop::getInstance().getAnimator().fadeIn(aboutWindow, 300);
 
 DBG("About window shown!");
 }
 else if (button == &transitionModeButton)
 {
 bool isTransitionMode = transitionModeButton.getToggleState();
 
 DBG("Transition Mode button clicked!");
 DBG(" Toggle state: " + juce::String(isTransitionMode ? "ON (TRANSITION)" : "OFF (REVERSE ONLY)"));
 DBG(" Sample loaded: " + juce::String(audioProcessor.isSampleLoaded() ? "YES" : "NO"));
 
 audioProcessor.setTransitionMode(isTransitionMode);
 
 if (isTransitionMode)
 {
 transitionModeButton.setButtonText("TRANSITION");
 }
 else
 {
 transitionModeButton.setButtonText("REVERSE ONLY");
 }
 
 if (audioProcessor.isSampleLoaded())
 {
 processingScheduled = true;
 updateStatus("Updating mode...");
 DBG("Processing scheduled!");
 }
 else
 {
 // Update status even if no sample is loaded
 if (isTransitionMode)
 updateStatus("Transition Mode (load sample)");
 else
 updateStatus("Reverse Only (load sample)");
 
 DBG("No sample loaded - mode saved but not processed yet");
 }
 }
 else if (button == &tremoloEnableButton)
 {
 bool isEnabled = tremoloEnableButton.getToggleState();
 audioProcessor.setTremoloEnabled(isEnabled);

 // Update button text
 tremoloEnableButton.setButtonText(isEnabled ? "TREMOLO ON" : "TREMOLO OFF");

 // Expand/collapse tremolo section - resize window so controls don't move
 tremoloExpanded = isEnabled;
 {
 float sf = getWidth() / 700.0f;
 int expandedH = juce::roundToInt(175 * sf);
 int newHeight = isEnabled ? getHeight() + expandedH : getHeight() - expandedH;
 newHeight = juce::jlimit(580, 1020, newHeight);
 setSize(getWidth(), newHeight);
 }
 
 // Enable/disable rate slider based on sync AND ramp state
 // Disabled if: synced OR ramp is ON
 tremoloRateSlider.setEnabled(isEnabled && !audioProcessor.getTremoloSyncEnabled() && !audioProcessor.getTremoloRateRampEnabled());
 
 // Enable/disable all tremolo controls
 tremoloDepthSlider.setEnabled(isEnabled);
 tremoloWaveformCombo.setEnabled(isEnabled);
 tremoloSyncToggle.setEnabled(isEnabled);
 tremoloSyncDivisionCombo.setEnabled(isEnabled && audioProcessor.getTremoloSyncEnabled() && !audioProcessor.getTremoloRateRampEnabled());
 
 // Rate ramp controls - NOW WORK WITHOUT SYNC! 
 tremoloRateRampToggle.setEnabled(isEnabled); // Always enabled when tremolo is ON!
 
 // Start/End divisions - enabled when ramp is ON (works with or without sync)
 tremoloStartDivisionCombo.setEnabled(isEnabled && audioProcessor.getTremoloRateRampEnabled());
 tremoloEndDivisionCombo.setEnabled(isEnabled && audioProcessor.getTremoloRateRampEnabled());
 
 updateStatus(isEnabled ? "Tremolo Enabled" : "Tremolo Disabled");
 updateWaveformWithTremolo();

 DBG("Tremolo "+ juce::String(isEnabled ? "ENABLED": "DISABLED"));
 }
 else if (button == &tremoloSyncToggle)
 {
 bool isSynced = tremoloSyncToggle.getToggleState();
 audioProcessor.setTremoloSyncEnabled(isSynced);
 
 // Rate Ramp and Sync can now coexist
 // Sync provides BPM source, Rate Ramp uses it for division-based ramping
 
 // Enable/disable rate slider and division combos
 tremoloRateSlider.setEnabled(!isSynced && !audioProcessor.getTremoloRateRampEnabled());
 tremoloSyncDivisionCombo.setEnabled(isSynced && !audioProcessor.getTremoloRateRampEnabled());
 
 // Update rate ramp controls - work with or without sync
 tremoloRateRampToggle.setEnabled(true); // Always available!
 tremoloStartDivisionCombo.setEnabled(audioProcessor.getTremoloRateRampEnabled());
 tremoloEndDivisionCombo.setEnabled(audioProcessor.getTremoloRateRampEnabled());
 
 updateStatus(isSynced ? "Tremolo Synced to Host" : "Tremolo Manual Rate");
 
 DBG("Tremolo Sync "+ juce::String(isSynced ? "ENABLED": "DISABLED"));
 }
 else if (button == &tremoloRateRampToggle)
 {
 bool isRampEnabled = tremoloRateRampToggle.getToggleState();
 audioProcessor.setTremoloRateRampEnabled(isRampEnabled);
 
 DBG("Rate Ramp Toggle clicked! State: "+ juce::String(isRampEnabled ? "ON": "OFF"));
 DBG(" Sync Enabled: " + juce::String(audioProcessor.getTremoloSyncEnabled() ? "YES" : "NO"));
 DBG(" Tremolo Enabled: " + juce::String(audioProcessor.getTremoloEnabled() ? "YES" : "NO"));
 DBG(" Start Division: " + juce::String(audioProcessor.getTremoloStartDivision()));
 DBG(" End Division: " + juce::String(audioProcessor.getTremoloEndDivision()));
 
 // Rate Ramp now works both with and without sync
 // No exclusive mode needed anymore
 
 // Enable/disable controls based on ramp state
 if (isRampEnabled)
 {
 // Ramp is ON: Disable manual rate slider AND sync controls, enable start/end
 tremoloRateSlider.setEnabled(false);
 tremoloSyncDivisionCombo.setEnabled(false);
 tremoloStartDivisionCombo.setEnabled(true);
 tremoloEndDivisionCombo.setEnabled(true);
 updateStatus("Rate Ramp ON");
 updateWaveformWithTremolo();
 }
 else
 {
 // Ramp is OFF: Enable normal controls based on sync state
 bool isSynced = audioProcessor.getTremoloSyncEnabled();
 tremoloRateSlider.setEnabled(!isSynced);
 tremoloSyncDivisionCombo.setEnabled(isSynced);
 tremoloStartDivisionCombo.setEnabled(false);
 tremoloEndDivisionCombo.setEnabled(false);
 updateStatus("Rate Ramp Disabled");
 updateWaveformWithTremolo();
 }
 
 DBG("Tremolo Rate Ramp "+ juce::String(isRampEnabled ? "ENABLED": "DISABLED"));
 }
}

void ReverseReverbAudioProcessorEditor::updateStatus(const juce::String& message)
{
 statusLabel.setText (message, juce::dontSendNotification);
}

void ReverseReverbAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
 if (comboBox == &tremoloWaveformCombo)
 {
 int selectedId = tremoloWaveformCombo.getSelectedId();
 if (selectedId > 0)
 {
 audioProcessor.setTremoloWaveform(selectedId - 1); // Convert from 1-indexed to 0-indexed
 
 juce::String waveformName;
 switch (selectedId - 1)
 {
 case 0: waveformName = "Sine"; break;
 case 1: waveformName = "Triangle"; break;
 case 2: waveformName = "Square"; break;
 default: waveformName = "Unknown"; break;
 }
 
 updateStatus("Tremolo: " + waveformName + " wave");
 updateWaveformWithTremolo();
 DBG("Tremolo waveform changed to: "+ waveformName);
 }
 }
 else if (comboBox == &tremoloSyncDivisionCombo)
 {
 int selectedId = tremoloSyncDivisionCombo.getSelectedId();
 if (selectedId > 0)
 {
 audioProcessor.setTremoloSyncDivision(selectedId - 1);

 juce::String divisionName = tremoloSyncDivisionCombo.getText();
 updateStatus("Tremolo: " + divisionName + " sync");
 updateWaveformWithTremolo();
 DBG("Tremolo sync division changed to: "+ divisionName);
 }
 }
 else if (comboBox == &tremoloStartDivisionCombo)
 {
 int selectedId = tremoloStartDivisionCombo.getSelectedId();
 if (selectedId > 0)
 {
 audioProcessor.setTremoloStartDivision(selectedId - 1);

 juce::String startDiv = tremoloStartDivisionCombo.getText();
 juce::String endDiv = tremoloEndDivisionCombo.getText();
 updateStatus("Rate Ramp: "+ startDiv + "-> "+ endDiv);
 updateWaveformWithTremolo();
 DBG("Tremolo start division changed to: "+ startDiv);
 }
 }
 else if (comboBox == &tremoloEndDivisionCombo)
 {
 int selectedId = tremoloEndDivisionCombo.getSelectedId();
 if (selectedId > 0)
 {
 audioProcessor.setTremoloEndDivision(selectedId - 1);

 juce::String startDiv = tremoloStartDivisionCombo.getText();
 juce::String endDiv = tremoloEndDivisionCombo.getText();
 updateStatus("Rate Ramp: "+ startDiv + "-> "+ endDiv);
 updateWaveformWithTremolo();
 DBG("Tremolo end division changed to: "+ endDiv);
 }
 }
}

void ReverseReverbAudioProcessorEditor::timerCallback()
{
 // Update animation phase for synchronized animations across all components
 animationPhase += 0.05f;
 if (animationPhase > juce::MathConstants<float>::twoPi)
 animationPhase -= juce::MathConstants<float>::twoPi;
 
 // Update 3D LookAndFeel and child components with the same phase for synchronized animations
 modern3DLAF->setAnimationPhase(animationPhase);
 if (waveformDisplay != nullptr)
     waveformDisplay->setAnimationPhase(animationPhase);
 
 // Trigger repaint for animated components
 reverbSizeSlider.repaint();
 dryWetSlider.repaint();
 tailDivisionSlider.repaint();
 playButton.repaint();

 // Update BPM display in VST3 mode
 if (!audioProcessor.isStandalone())
 {
     double currentBpm = audioProcessor.getEffectiveBpm();
     bpmDisplayLabel.setText("BPM: " + juce::String((int)currentBpm), juce::dontSendNotification);
 }
 transitionModeButton.repaint();
 aboutButton.repaint();
 
 // Tremolo controls
 tremoloEnableButton.repaint();
 tremoloDepthSlider.repaint();
 tremoloRateSlider.repaint();

 // Update waveform playback position indicator
 waveformDisplay->setPlaybackProgress(audioProcessor.getPlaybackProgress());

 // THROTTLED PROCESSING: Only check every ~150ms (5 frames @ 30ms)
 processingThrottleCounter++;
 if (processingThrottleCounter >= 5)
 {
 processingThrottleCounter = 0;
 
 // Only process if scheduled and not already processing
 if (processingScheduled && !isCurrentlyProcessing.load())
 {
 DBG("Timer: Processing scheduled! Starting background processing...");
 
 processingScheduled = false;
 isCurrentlyProcessing.store(true);
 
 // Process in background thread
 juce::Thread::launch([this]()
 {
 DBG("Background thread: Calling processReverseReverb()...");
 audioProcessor.processReverseReverb();
 DBG("Background thread: processReverseReverb() completed!");
 
 // Update UI on message thread
 juce::MessageManager::callAsync([this]()
 {
 isCurrentlyProcessing.store(false);
 waveformNeedsUpdate = true;
 updateWaveformWithTremolo();
 updateStatus ("Updated");
 repaint();
 DBG("UI updated after processing!");
 });
 });
 }
 }
}

void ReverseReverbAudioProcessorEditor::updateWaveformWithTremolo()
{
 if (!audioProcessor.isSampleLoaded())
 {
 waveformDisplay->setAudioBuffer(nullptr);
 return;
 }

 // Generate display buffer with tremolo modulation applied
 audioProcessor.getDisplayBufferWithTremolo(tremoloDisplayBuffer);
 waveformDisplay->setAudioBuffer(&tremoloDisplayBuffer);
 waveformDisplay->setGain(audioProcessor.getDryWet());
}

void ReverseReverbAudioProcessorEditor::paintWaveform(juce::Graphics& g)
{
 // Safety check: ensure waveformArea is valid
 if (waveformArea.getWidth() <= 0 || waveformArea.getHeight() <= 0)
 return;
 
 // Draw waveform background
 g.setColour (juce::Colours::black.withAlpha(0.5f));
 g.fillRoundedRectangle (waveformArea.toFloat(), 5.0f);
 
 // Draw border
 g.setColour (juce::Colours::white.withAlpha(0.3f));
 g.drawRoundedRectangle (waveformArea.toFloat(), 5.0f, 1.0f);
 
 // Get processed sample from processor
 if (!audioProcessor.isSampleLoaded())
 {
 // Draw "No sample loaded" text
 g.setColour (juce::Colours::grey);
 g.setFont (12.0f);
 g.drawText ("No waveform to display", waveformArea, juce::Justification::centred);
 waveformNeedsUpdate = true; // Mark for update when sample loads
 return;
 }
 
 // Get the processed buffer
 auto* processedBuffer = audioProcessor.getProcessedBuffer();
 
 if (processedBuffer == nullptr || processedBuffer->getNumSamples() == 0)
 return;
 
 auto numSamples = processedBuffer->getNumSamples();
 auto numChannels = processedBuffer->getNumChannels();
 
 if (numChannels == 0 || numSamples == 0)
 return;
 
 // Calculate waveform drawing area
 auto waveArea = waveformArea.reduced(5);
 auto width = waveArea.getWidth();
 auto height = waveArea.getHeight();
 
 // Safety check: make sure we have valid dimensions
 if (width <= 0 || height <= 0)
 return;
 
 auto centerY = waveArea.getCentreY();
 
 // Draw center line
 g.setColour (juce::Colours::white.withAlpha(0.2f));
 g.drawHorizontalLine (centerY, (float)waveArea.getX(), (float)waveArea.getRight());
 
 // OPTIMIZATION: Only recalculate waveform path if needed
 if (waveformNeedsUpdate)
 {
 cachedWaveformPath.clear();
 
 // Calculate samples per pixel
 auto samplesPerPixel = numSamples / width;
 
 if (samplesPerPixel < 1)
 samplesPerPixel = 1;
 
 // Don't apply gain here - we'll scale the path when drawing for live updates
 bool firstPoint = true;
 
 for (int x = 0; x < width; ++x)
 {
 auto sampleStart = x * samplesPerPixel;
 auto sampleEnd = juce::jmin(sampleStart + samplesPerPixel, numSamples);
 
 // Find min and max in this range
 float minVal = 0.0f;
 float maxVal = 0.0f;
 
 for (int sample = sampleStart; sample < sampleEnd; ++sample)
 {
 float sampleValue = 0.0f;
 
 // Average all channels
 for (int channel = 0; channel < numChannels; ++channel)
 {
 sampleValue += processedBuffer->getSample(channel, sample);
 }
 sampleValue /= numChannels;
 
 // Don't apply gain here - stored at 1.0x
 minVal = juce::jmin(minVal, sampleValue);
 maxVal = juce::jmax(maxVal, sampleValue);
 }
 
 // Scale to pixel coordinates (without gain)
 auto pixelX = (float)(waveArea.getX() + x);
 auto pixelYMax = centerY - (maxVal * height * 0.4f);
 auto pixelYMin = centerY - (minVal * height * 0.4f);
 
 // Draw vertical line for this pixel
 if (firstPoint)
 {
 cachedWaveformPath.startNewSubPath(pixelX, pixelYMax);
 firstPoint = false;
 }
 
 cachedWaveformPath.lineTo(pixelX, pixelYMax);
 cachedWaveformPath.lineTo(pixelX, pixelYMin);
 }
 
 waveformNeedsUpdate = false;
 }
 
 // Apply gain transform to the cached path for LIVE updates
 float currentGain = audioProcessor.getDryWet();
 juce::AffineTransform gainTransform = juce::AffineTransform::scale(1.0f, currentGain, 0.0f, centerY);
 juce::Path transformedPath = cachedWaveformPath;
 transformedPath.applyTransform(gainTransform);
 
 // Draw the waveform SOLID - NO GRADIENTS! PURE HOT PINK! 
 // We'll draw it in 3 parts: fade in zone, normal zone, fade out zone
 
 // Calculate fade zone boundaries in pixels
 auto fadeInPixels = (int)(width * fadeInAmount);
 auto fadeOutPixels = (int)(width * fadeOutAmount);
 auto fadeOutStartPixel = width - fadeOutPixels;
 
 // Set up clipping and draw each zone separately
 g.saveState();
 
 // 1 FADE IN ZONE - SOLID with fade from transparent to full HOT PINK!
 if (fadeInPixels > 0)
 {
 g.reduceClipRegion(juce::Rectangle<int>(waveArea.getX(), waveArea.getY(), fadeInPixels, waveArea.getHeight()));
 
 // Gradient from transparent to SOLID HOT PINK
 juce::ColourGradient fadeInGradient(
 juce::Colour(0xffff006e).withAlpha(0.0f), // Transparent at start
 (float)waveArea.getX(), centerY,
 juce::Colour(0xffff006e), // FULL SOLID HOT PINK at end!
 (float)(waveArea.getX() + fadeInPixels), centerY,
 false
 );
 g.setGradientFill(fadeInGradient);
 g.fillPath(transformedPath);
 }
 
 g.restoreState();
 g.saveState();
 
 // 2 NORMAL ZONE - SOLID HOT PINK! NO GRADIENTS! 
 auto normalZoneStart = waveArea.getX() + fadeInPixels;
 auto normalZoneWidth = fadeOutStartPixel - fadeInPixels;
 
 if (normalZoneWidth > 0)
 {
 g.reduceClipRegion(juce::Rectangle<int>(normalZoneStart, waveArea.getY(), normalZoneWidth, waveArea.getHeight()));
 
 // PURE SOLID HOT PINK - NO TRANSPARENCY!
 g.setColour(juce::Colour(0xffff006e)); // 100% SOLID HOT PINK! 
 g.fillPath(transformedPath);
 }
 
 g.restoreState();
 g.saveState();
 
 // 3 FADE OUT ZONE - SOLID fading to transparent
 if (fadeOutPixels > 0)
 {
 g.reduceClipRegion(juce::Rectangle<int>(waveArea.getX() + fadeOutStartPixel, waveArea.getY(), fadeOutPixels, waveArea.getHeight()));
 
 // Gradient from SOLID HOT PINK to transparent
 juce::ColourGradient fadeOutGradient(
 juce::Colour(0xffff006e), // FULL SOLID HOT PINK at start!
 (float)(waveArea.getX() + fadeOutStartPixel), centerY,
 juce::Colour(0xffff006e).withAlpha(0.0f), // Transparent at end
 (float)waveArea.getRight(), centerY,
 false
 );
 g.setGradientFill(fadeOutGradient);
 g.fillPath(transformedPath);
 }
 
 g.restoreState();
 
 // Always draw fade in zone (even if 0) - Purple theme
 auto fadeInWidth = width * juce::jmax(0.0f, fadeInAmount);
 juce::Rectangle<float> fadeInRect(waveArea.getX(), waveArea.getY(), 
 juce::jmax(1.0f, fadeInWidth), waveArea.getHeight());
 
 if (fadeInAmount > 0.01f)
 {
 g.setColour (juce::Colour (0xff7209b7).withAlpha(0.3f)); // Deep purple
 g.fillRect (fadeInRect);
 
 g.setColour (juce::Colour (0xffb537f2).withAlpha(0.9f)); // Vibrant purple line
 g.drawLine (fadeInRect.getRight(), fadeInRect.getY(), 
 fadeInRect.getRight(), fadeInRect.getBottom(), 2.0f);
 
 g.setColour (juce::Colours::white);
 g.setFont (10.0f);
 g.drawText ("FADE IN", fadeInRect.reduced(2), juce::Justification::centredTop);
 }
 else
 {
 // Draw hint at left edge
 g.setColour (juce::Colour (0xff7209b7).withAlpha(0.3f));
 g.fillRect ((float)waveArea.getX(), (float)waveArea.getY(), 2.0f, (float)waveArea.getHeight());
 }
 
 // Always draw fade out zone (even if 0) - Hot pink theme
 auto fadeOutWidth = width * juce::jmax(0.0f, fadeOutAmount);
 auto fadeOutX = waveArea.getRight() - juce::jmax(1.0f, fadeOutWidth);
 juce::Rectangle<float> fadeOutRect(fadeOutX, waveArea.getY(), 
 juce::jmax(1.0f, fadeOutWidth), waveArea.getHeight());
 
 if (fadeOutAmount > 0.01f)
 {
 g.setColour (juce::Colour (0xffff006e).withAlpha(0.3f)); // Hot pink
 g.fillRect (fadeOutRect);
 
 g.setColour (juce::Colour (0xffff006e).withAlpha(0.9f)); // Hot pink line
 g.drawLine (fadeOutRect.getX(), fadeOutRect.getY(), 
 fadeOutRect.getX(), fadeOutRect.getBottom(), 2.0f);
 
 g.setColour (juce::Colours::white);
 g.setFont (10.0f);
 g.drawText ("FADE OUT", fadeOutRect.reduced(2), juce::Justification::centredTop);
 }
 else
 {
 // Draw hint at right edge
 g.setColour (juce::Colour (0xffff006e).withAlpha(0.3f));
 g.fillRect ((float)waveArea.getRight() - 2.0f, (float)waveArea.getY(), 2.0f, (float)waveArea.getHeight());
 }
}

// DragVisualizer implementation
void DragVisualizer::mouseDrag(const juce::MouseEvent& event)
{
 DBG("DragVisualizer::mouseDrag called! Distance: "+ juce::String((int)event.getDistanceFromDragStart()));
 
 if (!processor.isSampleLoaded())
 {
 DBG("DragVisualizer: No sample loaded, ignoring drag");
 return;
 }
 
 // CRITICAL: Only start drag ONCE per drag session
 if (isDragging)
 {
 DBG("Already dragging, skipping...");
 return;
 }
 
 if (event.getDistanceFromDragStart() > 5)
 {
 isDragging = true; // Mark as dragging to prevent multiple calls
 
 DBG("DragVisualizer: Drag started! Distance: "+ juce::String((int)event.getDistanceFromDragStart()));
 
 if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
 {
 DBG("Found DragAndDropContainer: "+ juce::String::toHexString((juce::pointer_sized_int)container));
 
 // Create temp file
 auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
 DBG("Temp directory: "+ tempDir.getFullPathName());
 
 // Build file name: "Reverse Reverb - [Original Name]"
 auto originalName = processor.getLoadedFileName();
 juce::String exportFileName;
 
 if (originalName.isNotEmpty())
 {
 exportFileName = "Reverse Reverb - " + originalName + ".wav";
 }
 else
 {
 // Fallback if no file name
 auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
 exportFileName = "ReverseReverb_" + timestamp + ".wav";
 }
 
 auto tempFile = tempDir.getChildFile(exportFileName);
 
 DBG("DragVisualizer: Creating temp file: "+ tempFile.getFullPathName());
 
 if (processor.exportProcessedAudio(tempFile))
 {
 if (tempFile.existsAsFile())
 {
 DBG("File created successfully!");
 DBG("File size: "+ juce::String(tempFile.getSize()) + "bytes");
 DBG("Full path: "+ tempFile.getFullPathName());
 
 juce::StringArray files;
 files.add(tempFile.getFullPathName());
 
 DBG("Calling performExternalDragDropOfFiles with "+ juce::String(files.size()) + "file(s)...");
 DBG(" File 0: " + files[0]);
 
 container->performExternalDragDropOfFiles(files, true);
 
 DBG("performExternalDragDropOfFiles returned!");
 
 if (auto* editor = dynamic_cast<ReverseReverbAudioProcessorEditor*>(container))
 {
 editor->updateStatus("Drag to DAW!");
 }
 }
 else
 {
 DBG("File was exported but doesn't exist!");
 }
 }
 else
 {
 DBG("exportProcessedAudio returned false");
 if (auto* editor = dynamic_cast<ReverseReverbAudioProcessorEditor*>(container))
 {
 editor->updateStatus("Export failed");
 }
 }
 }
 else
 {
 DBG("No DragAndDropContainer found!");
 DBG(" Parent: " + juce::String::toHexString((juce::pointer_sized_int)getParentComponent()));
 }
 }
 else
 {
 DBG("Distance too small, waiting... ("+ juce::String((int)event.getDistanceFromDragStart()) + "pixels)");
 }
}

