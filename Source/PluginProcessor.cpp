#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

ReverseReverbAudioProcessor::ReverseReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
 : AudioProcessor (BusesProperties()
 #if ! JucePlugin_IsMidiEffect
 #if ! JucePlugin_IsSynth
 .withInput ("Input", juce::AudioChannelSet::stereo(), true)
 #endif
 .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
 #endif
 )
#endif
{
 // Register audio formats (WAV, AIFF, MP3, FLAC, etc.)
 formatManager.registerBasicFormats();
 
 // Setup reverb parameters
 reverbParams.roomSize = reverbSize;
 reverbParams.damping = 0.5f;
 reverbParams.wetLevel = reverbMix;
 reverbParams.dryLevel = 0.0f;
 reverbParams.width = 1.0f;
 reverbParams.freezeMode = 0.0f;
 
 reverb.setParameters(reverbParams);
}

ReverseReverbAudioProcessor::~ReverseReverbAudioProcessor()
{
}

const juce::String ReverseReverbAudioProcessor::getName() const
{
 return JucePlugin_Name;
}

bool ReverseReverbAudioProcessor::acceptsMidi() const
{
 #if JucePlugin_WantsMidiInput
 return true;
 #else
 return false;
 #endif
}

bool ReverseReverbAudioProcessor::producesMidi() const
{
 #if JucePlugin_ProducesMidiOutput
 return true;
 #else
 return false;
 #endif
}

bool ReverseReverbAudioProcessor::isMidiEffect() const
{
 #if JucePlugin_IsMidiEffect
 return true;
 #else
 return false;
 #endif
}

double ReverseReverbAudioProcessor::getTailLengthSeconds() const
{
 return 0.0;
}

int ReverseReverbAudioProcessor::getNumPrograms()
{
 return 1;
}

int ReverseReverbAudioProcessor::getCurrentProgram()
{
 return 0;
}

void ReverseReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ReverseReverbAudioProcessor::getProgramName (int index)
{
 return {};
}

void ReverseReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void ReverseReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
 currentSampleRate = sampleRate;

 // Reset reverb completely
 reverb.reset();
 reverb.setSampleRate(sampleRate);

 // Set initial reverb parameters
 reverbParams.roomSize = reverbSize;
 reverbParams.damping = 0.5f;
 reverbParams.wetLevel = reverbMix;
 reverbParams.dryLevel = 0.0f;
 reverbParams.width = 1.0f;
 reverbParams.freezeMode = 0.0f;
 reverb.setParameters(reverbParams);

 // Initialize delay buffers for stereo width effect (max 2000 samples delay)
 int maxDelaySamples = 2000;
 delayBufferLeft.setSize(1, maxDelaySamples, false, true, false);
 delayBufferRight.setSize(1, maxDelaySamples, false, true, false);
 delayBufferLeft.clear();
 delayBufferRight.clear();
 delayWritePosition = 0;

 // Reset filter state
 lowCutPrevInputL = 0.0f;
 lowCutPrevInputR = 0.0f;
 lowCutPrevOutputL = 0.0f;
 lowCutPrevOutputR = 0.0f;

 // Reset tremolo state
 tremoloPhase = 0.0f;
 tremoloSampleCounter = 0;
}

void ReverseReverbAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReverseReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
 #if JucePlugin_IsMidiEffect
 juce::ignoreUnused (layouts);
 return true;
 #else
 if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
 && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
 return false;

 #if ! JucePlugin_IsSynth
 if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
 return false;
 #endif

 return true;
 #endif
}
#endif

void ReverseReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
 juce::ScopedNoDenormals noDenormals;
 auto totalNumInputChannels = getTotalNumInputChannels();
 auto totalNumOutputChannels = getTotalNumOutputChannels();

 // Clear unused channels
 for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
 buffer.clear (i, 0, buffer.getNumSamples());

 // Clear the buffer first to prevent noise
 for (int i = 0; i < totalNumOutputChannels; ++i)
 buffer.clear(i, 0, buffer.getNumSamples());

 // Check for MIDI note triggers
 for (const auto metadata : midiMessages)
 {
 auto message = metadata.getMessage();
 if (message.isNoteOn())
 {
 triggerSample();
 }
 else if (message.isNoteOff())
 {
 isPlaying = false;
 }
 }

 // Playback processed sample if triggered
 // Capture atomic state once per block for consistency (avoid reading mid-change)
 bool currentlyPlaying = isPlaying.load();
 int processedNumSamples = processedSample.getNumSamples();
 int processedNumChannels = processedSample.getNumChannels();

 if (currentlyPlaying && processedNumSamples > 0 && processedNumChannels > 0)
 {
 auto numChannels = juce::jmin(buffer.getNumChannels(), processedNumChannels);
 auto numSamples = buffer.getNumSamples();
 auto totalSamples = processedNumSamples;
 
 for (int i = 0; i < numSamples; ++i)
 {
 // : 
 if (currentPlaybackPosition >= 0 && currentPlaybackPosition < totalSamples && isPlaying.load())
 {
 // Calculate fade gain for this position
 float fadeGain = 1.0f;
 float samplePosition = (float)currentPlaybackPosition / totalSamples;
 
 // Apply fade in with cubic curve for sharper fade
 if (fadeIn > 0.0f && samplePosition < fadeIn)
 {
 float fadePos = samplePosition / fadeIn;
 fadeGain *= fadePos * fadePos * fadePos; // Cubic curve
 }
 
 // Apply fade out with cubic curve for sharper fade
 if (fadeOut > 0.0f && samplePosition > (1.0f - fadeOut))
 {
 float fadeOutPos = (1.0f - samplePosition) / fadeOut;
 fadeGain *= fadeOutPos * fadeOutPos * fadeOutPos; // Cubic curve
 }
 
 for (int channel = 0; channel < numChannels; ++channel)
 {
 auto* outputData = buffer.getWritePointer(channel);
 auto* sampleData = processedSample.getReadPointer(channel);
 
 float sample = sampleData[currentPlaybackPosition];
 
 // Remove denormals more aggressively
 if (std::abs(sample) < 1e-10f)
 sample = 0.0f;
 
 // Apply fade and output gain
 float outputSample = sample * fadeGain * dryWet;
 if (outputSample > 0.99f)
 outputSample = 0.99f;
 else if (outputSample < -0.99f)
 outputSample = -0.99f;
 
 outputData[i] = outputSample;
 }
 
 currentPlaybackPosition++;
 }
 else
 {
 isPlaying = false;
 
 // Reset tremolo sample counter when playback stops
 tremoloSampleCounter = 0;
 
 for (int channel = 0; channel < numChannels; ++channel)
 {
 buffer.getWritePointer(channel)[i] = 0.0f;
 }
 }
 }
 
 // Apply Tremolo at the end of the signal chain (if enabled)
 if (tremoloEnabled && isPlaying.load())
 {
 // Debug at start of tremolo application
 static bool firstTime = true;
 if (firstTime && tremoloRateRampEnabled && tremoloSyncEnabled)
 {
 DBG(" TREMOLO LOOP STARTED with Rate Ramp!");
 DBG(" Num samples in buffer: " << numSamples);
 DBG(" Initial counter: " << tremoloSampleCounter);
 firstTime = false;
 }
 
 // : 
 int samplesToProcess = numSamples; // Process all samples in the buffer

 for (int i = 0; i < samplesToProcess; ++i)
 {
 // Safety: skip if playback stopped mid-block
 if (!isPlaying.load())
 break;
 
 float tremoloGain = calculateTremoloLFO();
 
 for (int channel = 0; channel < numChannels; ++channel)
 {
 auto* channelData = buffer.getWritePointer(channel);
 channelData[i] *= tremoloGain;
 }
 
 // Increment sample counter for Rate Ramp tracking AFTER applying tremolo
 // Works with or without sync enabled
 if (tremoloRateRampEnabled)
 {
 tremoloSampleCounter++;

 // Reset counter when we reach the end of the sample (prevent overflow)
 int sampleLength = isSampleLoaded() ? processedSample.getNumSamples() : 0;
 if (sampleLength > 0 && tremoloSampleCounter >= sampleLength)
 {
 DBG(" Counter reached end! Resetting from " << tremoloSampleCounter << " to 0");
 tremoloSampleCounter = 0;
 }
 else if (sampleLength <= 0)
 {
 tremoloSampleCounter = 0; // Safety reset
 }
 }
 }
 }
 else if (!isPlaying)
 {
 // Reset flag when playback stops
 static bool firstTime_reset = true;
 firstTime_reset = true;
 }
 }

}

void ReverseReverbAudioProcessor::loadAudioFile(const juce::File& file)
{
 if (!file.existsAsFile())
 return;
 
 std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
 
 if (reader == nullptr)
 return;

 // Validate sample rate to prevent division by zero
 if (reader->sampleRate <= 0.0)
 {
 DBG("Invalid sample rate: " << reader->sampleRate);
 return;
 }

 auto duration = reader->lengthInSamples / reader->sampleRate;

 // Limit to 8 seconds as requested
 if (duration > 8.0 || duration <= 0.0)
 {
 DBG("File too long or invalid: " << duration << " seconds");
 return;
 }
 
 // Save the original file name (without extension) for export naming
 loadedFileName = file.getFileNameWithoutExtension();
 DBG(" Loaded file name: " + loadedFileName);
 
 // Stop playback before loading new sample
 isPlaying = false;
 currentPlaybackPosition = 0;
 
 // Reset tremolo sample counter
 tremoloSampleCounter = 0;
 
 // Load the sample
 auto numChannels = juce::jmin((int)reader->numChannels, 2); // Max stereo
 auto numSamples = (int)reader->lengthInSamples;
 
 if (numSamples <= 0 || numChannels <= 0)
 return;
 
 originalSample.setSize(numChannels, numSamples, false, true, false);
 
 if (!reader->read(&originalSample, 0, numSamples, 0, true, true))
 {
 DBG("Failed to read audio file");
 originalSample.setSize(0, 0);
 loadedFileName = ""; // Clear file name on failure
 return;
 }
 
 // Automatically process when loaded
 processReverseReverb();
}

void ReverseReverbAudioProcessor::processReverseReverb()
{
 if (originalSample.getNumSamples() == 0 || originalSample.getNumChannels() == 0)
 return;

 // Save playback state so we can resume after reprocessing
 bool wasPlaying = isPlaying.load();
 int savedPlaybackPos = currentPlaybackPosition;
 int oldSampleLength = processedSample.getNumSamples();

 // Stop playback during processing - MUST happen before any buffer operations
 isPlaying = false;
 currentPlaybackPosition = 0;

 // Reset tremolo sample counter
 tremoloSampleCounter = 0;

 // Small delay to ensure audio thread has completed current block
 // This prevents race condition where processBlock reads buffers mid-resize
 juce::Thread::sleep(10);

 try
 {
 // Step 1: Create a copy and normalize input to prevent clipping
 reverbBuffer.makeCopyOf(originalSample);
 
 if (reverbBuffer.getNumSamples() == 0)
 return;
 
 // OPTIMIZATION: Use getMagnitude() for faster peak finding
 float maxLevel = 0.0f;
 for (int channel = 0; channel < reverbBuffer.getNumChannels(); ++channel)
 {
 maxLevel = juce::jmax(maxLevel, reverbBuffer.getMagnitude(channel, 0, reverbBuffer.getNumSamples()));
 }
 
 // Scale down to 50% to prevent reverb clipping
 if (maxLevel > 0.001f)
 {
 float scaleFactor = 0.5f / maxLevel;
 reverbBuffer.applyGain(scaleFactor); // Apply to all channels at once
 }
 
 // Step 2: Reset and configure reverb with beat-synced tail length
 reverb.reset();
 reverb.setSampleRate(currentSampleRate);
 
 // Calculate tail duration from BPM + division
 float tailDuration = getTailDurationSeconds();
 float normalizedFeedback = juce::jlimit(0.0f, 1.0f, tailDuration / 10.0f);

 // Adjust room size based on tail duration - longer tail = larger room
 float adjustedRoomSize = juce::jlimit(0.0f, 1.0f, reverbSize + (normalizedFeedback * 0.3f));

 reverbParams.roomSize = adjustedRoomSize;
 reverbParams.damping = juce::jlimit(0.1f, 0.9f, 0.5f - (normalizedFeedback * 0.3f));
 reverbParams.wetLevel = juce::jlimit(0.0f, 1.0f, reverbMix) * 0.7f;
 reverbParams.dryLevel = 0.0f;

 // Adjust stereo width in reverb parameters
 reverbParams.width = stereoWidth;
 reverbParams.freezeMode = 0.0f;
 reverb.setParameters(reverbParams);

 DBG("Processing with tail duration: " << tailDuration << "s (BPM: " << getEffectiveBpm() << ")");
 DBG("Adjusted room size: " << adjustedRoomSize);
 DBG("Damping: " << reverbParams.damping);
 DBG("Stereo width: " << stereoWidth);

 // Step 3: Add silence at the end based on tail duration to let reverb tail ring out
 int extraSamples = (int)(tailDuration * currentSampleRate);

 DBG("Adding extra samples: " << extraSamples << " (" << tailDuration << " seconds)");
 
 if (extraSamples > 0)
 {
 juce::AudioBuffer<float> extendedBuffer(reverbBuffer.getNumChannels(), 
 reverbBuffer.getNumSamples() + extraSamples);
 extendedBuffer.clear();
 
 // Copy original samples
 for (int channel = 0; channel < reverbBuffer.getNumChannels(); ++channel)
 {
 extendedBuffer.copyFrom(channel, 0, reverbBuffer, channel, 0, reverbBuffer.getNumSamples());
 }
 
 reverbBuffer.setSize(extendedBuffer.getNumChannels(), extendedBuffer.getNumSamples(), false, false, true);
 reverbBuffer.makeCopyOf(extendedBuffer);
 
 DBG("Extended buffer size: " << reverbBuffer.getNumSamples() << " samples");
 }
 
 // Step 4: Apply reverb in smaller chunks to prevent distortion
 const int chunkSize = 512;
 
 if (reverbBuffer.getNumChannels() == 1)
 {
 // Mono - convert to stereo
 juce::AudioBuffer<float> stereoBuffer(2, reverbBuffer.getNumSamples());
 stereoBuffer.clear();
 stereoBuffer.copyFrom(0, 0, reverbBuffer, 0, 0, reverbBuffer.getNumSamples());
 stereoBuffer.copyFrom(1, 0, reverbBuffer, 0, 0, reverbBuffer.getNumSamples());
 
 // Process in chunks
 for (int pos = 0; pos < stereoBuffer.getNumSamples(); pos += chunkSize)
 {
 int samplesToProcess = juce::jmin(chunkSize, stereoBuffer.getNumSamples() - pos);
 reverb.processStereo(stereoBuffer.getWritePointer(0) + pos, 
 stereoBuffer.getWritePointer(1) + pos, 
 samplesToProcess);
 }
 
 reverbBuffer.setSize(2, stereoBuffer.getNumSamples(), false, false, true);
 reverbBuffer.makeCopyOf(stereoBuffer);
 }
 else
 {
 // Stereo - process in chunks
 for (int pos = 0; pos < reverbBuffer.getNumSamples(); pos += chunkSize)
 {
 int samplesToProcess = juce::jmin(chunkSize, reverbBuffer.getNumSamples() - pos);
 reverb.processStereo(reverbBuffer.getWritePointer(0) + pos, 
 reverbBuffer.getWritePointer(1) + pos, 
 samplesToProcess);
 }
 }
 
 // Step 4.5: Apply additional stereo width using Haas effect (delay-based)
 if (stereoWidth != 0.5f && reverbBuffer.getNumChannels() >= 2)
 {
 // Calculate delay time based on width (0-20ms range)
 // 0.5 = no delay (normal), 0.0 = mono, 1.0 = max width
 float delayMs = 0.0f;
 
 if (stereoWidth < 0.5f)
 {
 // Narrowing: move towards mono
 float monoAmount = 1.0f - (stereoWidth * 2.0f);
 
 for (int i = 0; i < reverbBuffer.getNumSamples(); ++i)
 {
 float left = reverbBuffer.getSample(0, i);
 float right = reverbBuffer.getSample(1, i);
 float mono = (left + right) * 0.5f;
 
 // Blend towards mono
 left = left * (1.0f - monoAmount) + mono * monoAmount;
 right = right * (1.0f - monoAmount) + mono * monoAmount;
 
 reverbBuffer.setSample(0, i, left);
 reverbBuffer.setSample(1, i, right);
 }
 }
 else if (stereoWidth > 0.5f)
 {
 // Widening: apply Haas effect
 delayMs = (stereoWidth - 0.5f) * 40.0f; // 0-20ms
 int delaySamples = (int)(delayMs * 0.001f * currentSampleRate);
 
 if (delaySamples > 0 && delaySamples < 2000) // Max 2000 samples
 {
 juce::AudioBuffer<float> wideBuffer(2, reverbBuffer.getNumSamples());
 wideBuffer.makeCopyOf(reverbBuffer);
 
 // Delay the right channel slightly for width
 for (int i = delaySamples; i < wideBuffer.getNumSamples(); ++i)
 {
 float delayedSample = reverbBuffer.getSample(1, i - delaySamples);
 wideBuffer.setSample(1, i, delayedSample);
 }
 
 // Also add some subtle cross-feed for more natural sound
 float crossfeed = 0.15f;
 for (int i = 0; i < wideBuffer.getNumSamples(); ++i)
 {
 float left = wideBuffer.getSample(0, i);
 float right = wideBuffer.getSample(1, i);
 
 wideBuffer.setSample(0, i, left - (right * crossfeed));
 wideBuffer.setSample(1, i, right - (left * crossfeed));
 }
 
 reverbBuffer.makeCopyOf(wideBuffer);
 }
 }
 }
 
 // Step 5: Clean up the reverb output
 // OPTIMIZATION: Simplified cleanup - removed DC offset calculation
 for (int channel = 0; channel < reverbBuffer.getNumChannels(); ++channel)
 {
 auto* data = reverbBuffer.getWritePointer(channel);
 
 // Apply cleanup with soft clipping and denormal removal
 for (int i = 0; i < reverbBuffer.getNumSamples(); ++i)
 {
 float sample = data[i];
 
 // Soft clip to prevent harsh distortion
 if (sample > 0.95f)
 sample = 0.95f + 0.05f * std::tanh((sample - 0.95f) / 0.05f);
 else if (sample < -0.95f)
 sample = -0.95f + 0.05f * std::tanh((sample + 0.95f) / 0.05f);
 
 // Remove denormals
 if (std::abs(sample) < 1e-10f)
 sample = 0.0f;
 
 data[i] = sample;
 }
 }
 
 // Step 6: Reverse the audio OR Create transition
 if (transitionMode)
 {
 // TRANSITION MODE: Reversed reverb -> Original sample WITH reverb
 // This creates a smooth transition effect where the original also has reverb!
 
 // First, reverse the reverb buffer
 for (int channel = 0; channel < reverbBuffer.getNumChannels(); ++channel)
 {
 auto* data = reverbBuffer.getWritePointer(channel);
 std::reverse(data, data + reverbBuffer.getNumSamples());
 }
 
 // Now create a FORWARD reverb for the original sample
 juce::AudioBuffer<float> originalWithReverb;
 originalWithReverb.makeCopyOf(originalSample);
 
 // Reset reverb for forward processing
 reverb.reset();
 reverb.setSampleRate(currentSampleRate);
 
 // IDENTICAL reverb settings - create SYMMETRY!
 // Use THE SAME settings as the reversed reverb for visual balance
 juce::Reverb::Parameters forwardReverbParams;
 forwardReverbParams.roomSize = adjustedRoomSize; // SAME as reversed!
 forwardReverbParams.damping = reverbParams.damping; // SAME damping!
 forwardReverbParams.wetLevel = reverbParams.wetLevel; // SAME wet level!
 forwardReverbParams.dryLevel = 0.0f; // 0% dry - PURE reverb like the reversed!
 forwardReverbParams.width = stereoWidth; // SAME
 forwardReverbParams.freezeMode = 0.0f;
 reverb.setParameters(forwardReverbParams);
 
 // Add extra silence for tail (capped at 4s for transition mode)
 float forwardTailDuration = juce::jmin(tailDuration, 4.0f);
 int extraSamplesForward = (int)(forwardTailDuration * currentSampleRate);
 
 if (extraSamplesForward > 0)
 {
 // Extend original sample with silence to match reversed reverb tail
 juce::AudioBuffer<float> extendedOriginal(originalWithReverb.getNumChannels(), 
 originalWithReverb.getNumSamples() + extraSamplesForward);
 extendedOriginal.clear();
 
 // Copy original to beginning
 for (int channel = 0; channel < originalWithReverb.getNumChannels(); ++channel)
 {
 extendedOriginal.copyFrom(channel, 0, originalWithReverb, channel, 0, originalWithReverb.getNumSamples());
 }
 
 // Replace originalWithReverb with extended version
 originalWithReverb = std::move(extendedOriginal);
 }
 
 DBG(" SYMMETRICAL reverb settings:");
 DBG(" Room size: " + juce::String(adjustedRoomSize, 3));
 DBG(" Damping: " + juce::String(reverbParams.damping, 3));
 DBG(" Wet level: " + juce::String(reverbParams.wetLevel, 3));
 DBG(" Extra samples: " + juce::String(extraSamplesForward));
 
 // Process original sample with reverb
 if (originalWithReverb.getNumChannels() == 1)
 {
 reverb.processMono(originalWithReverb.getWritePointer(0), originalWithReverb.getNumSamples());
 }
 else if (originalWithReverb.getNumChannels() >= 2)
 {
 reverb.processStereo(originalWithReverb.getWritePointer(0), 
 originalWithReverb.getWritePointer(1), 
 originalWithReverb.getNumSamples());
 }
 
 // Get lengths for blending calculation
 int reversedReverbLength = reverbBuffer.getNumSamples();
 int originalLength = originalWithReverb.getNumSamples();
 
 // NOW CREATE A SMOOTH OVERLAP/BLEND - NO GAP!
 // Instead of putting original at the END, we OVERLAP them!
 
 // Overlap length - how much the two buffers overlap (50% of original or reverb, whichever is shorter)
 int overlapLength = juce::jmin(originalLength / 2, reversedReverbLength / 2);
 
 // Total length is LESS than sum because of overlap!
 int totalLength = reversedReverbLength + originalLength - overlapLength;
 
 // Start position for original sample (overlaps with end of reversed reverb)
 int originalStartPos = reversedReverbLength - overlapLength;
 
 // Create output buffer
 processedSample.setSize(reverbBuffer.getNumChannels(), 
 totalLength, 
 false, false, true);
 processedSample.clear();
 
 // 1 Copy reversed reverb to the beginning (full length)
 for (int channel = 0; channel < processedSample.getNumChannels(); ++channel)
 {
 processedSample.copyFrom(channel, 0, reverbBuffer, channel, 0, reversedReverbLength);
 }
 
 // 2 Blend original sample WITH REVERB starting BEFORE reverb ends
 for (int channel = 0; channel < juce::jmin(processedSample.getNumChannels(), originalWithReverb.getNumChannels()); ++channel)
 {
 auto* destData = processedSample.getWritePointer(channel);
 auto* srcData = originalWithReverb.getReadPointer(channel);
 
 for (int i = 0; i < originalLength; ++i)
 {
 int destPos = originalStartPos + i;
 if (destPos >= 0 && destPos < totalLength)
 {
 // Calculate blend factor based on position in overlap
 float blend = 1.0f; // Default: full original volume
 
 if (i < overlapLength)
 {
 // We're in the overlap zone - crossfade!
 float fadePos = (float)i / overlapLength;
 
 // Smooth S-curve for better blending
 fadePos = fadePos * fadePos * (3.0f - 2.0f * fadePos); // Smoothstep
 
 // Original fades IN, reverb already there will fade OUT naturally
 blend = fadePos;
 }
 
 // Mix the samples
 destData[destPos] = destData[destPos] * (1.0f - blend) + srcData[i] * blend;
 }
 }
 }
 
 DBG(" TRANSITION MODE: Reversed reverb -> Original WITH reverb");
 DBG(" Overlap length: " + juce::String(overlapLength) + " samples (" + juce::String(overlapLength / currentSampleRate, 2) + "s)");
 DBG(" Total length: " + juce::String(totalLength) + " samples");
 }
 else
 {
 // REVERSE ONLY MODE: Standard reverse reverb
 processedSample.setSize(reverbBuffer.getNumChannels(), 
 reverbBuffer.getNumSamples(), 
 false, false, true);
 processedSample.makeCopyOf(reverbBuffer);
 
 // Reverse the entire buffer
 for (int channel = 0; channel < processedSample.getNumChannels(); ++channel)
 {
 auto* data = processedSample.getWritePointer(channel);
 std::reverse(data, data + processedSample.getNumSamples());
 }
 
 DBG(" REVERSE ONLY MODE: Reversed reverb only (length: " + juce::String(processedSample.getNumSamples()) + " samples)");
 }
 
 // Step 6.5: Apply Low Cut Filter (simple 1-pole high-pass)
 if (lowCutFreq > 20.0f && currentSampleRate > 0.0)
 {
 // Calculate filter coefficient
 float RC = 1.0f / (juce::MathConstants<float>::twoPi * lowCutFreq);
 float dt = 1.0f / static_cast<float>(currentSampleRate);
 float alpha = RC / (RC + dt);
 
 // Reset filter state
 lowCutPrevInputL = 0.0f;
 lowCutPrevInputR = 0.0f;
 lowCutPrevOutputL = 0.0f;
 lowCutPrevOutputR = 0.0f;
 
 // Apply filter to left channel
 if (processedSample.getNumChannels() >= 1)
 {
 auto* dataL = processedSample.getWritePointer(0);
 for (int i = 0; i < processedSample.getNumSamples(); ++i)
 {
 float input = dataL[i];
 float output = alpha * (lowCutPrevOutputL + input - lowCutPrevInputL);
 lowCutPrevInputL = input;
 lowCutPrevOutputL = output;
 dataL[i] = output;
 }
 }
 
 // Apply filter to right channel
 if (processedSample.getNumChannels() >= 2)
 {
 auto* dataR = processedSample.getWritePointer(1);
 for (int i = 0; i < processedSample.getNumSamples(); ++i)
 {
 float input = dataR[i];
 float output = alpha * (lowCutPrevOutputR + input - lowCutPrevInputR);
 lowCutPrevInputR = input;
 lowCutPrevOutputR = output;
 dataR[i] = output;
 }
 }
 
 DBG("Applied Low Cut filter at " << lowCutFreq << " Hz");
 }
 
 // Step 7: Final gentle normalization to -3dB
 // OPTIMIZATION: Use getMagnitude() instead of manual loop
 maxLevel = processedSample.getMagnitude(0, processedSample.getNumSamples());
 
 // Normalize to -3dB (0.707) instead of 0dB to prevent clipping
 if (maxLevel > 0.001f)
 {
 float targetLevel = 0.707f; // -3dB
 float scaleFactor = targetLevel / maxLevel;
 processedSample.applyGain(scaleFactor); // Apply to all channels at once
 }
 
 // Note: Fades are NOT applied here - they're applied during playback/export only
 // This keeps the processed buffer "clean" for fade adjustments
 
 // Restore playback if it was playing before reprocessing
 if (wasPlaying && processedSample.getNumSamples() > 0)
 {
 int newSampleLength = processedSample.getNumSamples();
 if (oldSampleLength > 0 && savedPlaybackPos > 0)
 {
 // Scale position proportionally if sample length changed
 float progress = (float)savedPlaybackPos / (float)oldSampleLength;
 currentPlaybackPosition = juce::jlimit(0, newSampleLength - 1,
 (int)(progress * newSampleLength));
 }
 else
 {
 currentPlaybackPosition = 0;
 }
 // Restore tremolo counter proportionally too
 tremoloSampleCounter = currentPlaybackPosition;
 isPlaying = true;
 DBG("Playback restored at position: " + juce::String(currentPlaybackPosition));
 }
 else
 {
 currentPlaybackPosition = 0;
 tremoloSampleCounter = 0;
 }
 }
 catch (const std::bad_alloc& e)
 {
 DBG("Memory allocation failed in processReverseReverb: " << e.what());
 processedSample.setSize(0, 0);
 reverbBuffer.setSize(0, 0);
 isPlaying = false;
 currentPlaybackPosition = 0;
 tremoloSampleCounter = 0;
 }
 catch (const std::exception& e)
 {
 DBG("Exception in processReverseReverb: " << e.what());
 processedSample.setSize(0, 0);
 reverbBuffer.setSize(0, 0);
 isPlaying = false;
 currentPlaybackPosition = 0;
 tremoloSampleCounter = 0;
 }
 catch (...)
 {
 DBG("Unknown exception in processReverseReverb");
 processedSample.setSize(0, 0);
 reverbBuffer.setSize(0, 0);
 isPlaying = false;
 currentPlaybackPosition = 0;
 tremoloSampleCounter = 0;
 }
}

void ReverseReverbAudioProcessor::triggerSample()
{
 if (processedSample.getNumSamples() > 0 && processedSample.getNumChannels() > 0)
 {
 currentPlaybackPosition = 0;
 isPlaying = true;
 
 // Reset tremolo sample counter for Rate Ramp
 tremoloSampleCounter = 0;
 
 DBG(" ");
 DBG(" SAMPLE TRIGGERED!");
 DBG(" Sample length: " << processedSample.getNumSamples() << " samples");
 DBG(" Tremolo enabled: " << (tremoloEnabled ? "YES" : "NO"));
 DBG(" Sync enabled: " << (tremoloSyncEnabled ? "YES" : "NO"));
 DBG(" Rate Ramp enabled: " << (tremoloRateRampEnabled ? "YES" : "NO"));
 if (tremoloRateRampEnabled)
 {
 DBG(" Start division: " << tremoloStartDivision);
 DBG(" End division: " << tremoloEndDivision);
 }
 DBG(" Counter reset to: " << tremoloSampleCounter);
 DBG(" ");
 }
}

bool ReverseReverbAudioProcessor::exportProcessedAudio(const juce::File& file)
{
 DBG(" exportProcessedAudio called!");
 DBG(" File path: " + file.getFullPathName());
 DBG(" Num samples: " + juce::String(processedSample.getNumSamples()));
 DBG(" Num channels: " + juce::String(processedSample.getNumChannels()));
 
 if (processedSample.getNumSamples() == 0 || processedSample.getNumChannels() == 0)
 {
 DBG(" No processed sample to export!");
 return false;
 }
 
 // Create a copy with fades applied
 juce::AudioBuffer<float> exportBuffer;
 exportBuffer.makeCopyOf(processedSample);
 
 // Apply fades to the export buffer
 if (fadeIn > 0.0f || fadeOut > 0.0f)
 {
 auto totalSamples = exportBuffer.getNumSamples();
 
 for (int channel = 0; channel < exportBuffer.getNumChannels(); ++channel)
 {
 auto* data = exportBuffer.getWritePointer(channel);
 
 for (int i = 0; i < totalSamples; ++i)
 {
 float fadeGain = 1.0f;
 float samplePosition = (float)i / totalSamples;
 
 // Apply fade in with cubic curve for sharper fade
 if (fadeIn > 0.0f && samplePosition < fadeIn)
 {
 float fadePos = samplePosition / fadeIn;
 fadeGain *= fadePos * fadePos * fadePos; // Cubic curve
 }
 
 // Apply fade out with cubic curve for sharper fade
 if (fadeOut > 0.0f && samplePosition > (1.0f - fadeOut))
 {
 float fadeOutPos = (1.0f - samplePosition) / fadeOut;
 fadeGain *= fadeOutPos * fadeOutPos * fadeOutPos; // Cubic curve
 }
 
 data[i] *= fadeGain;
 }
 }
 }
 
 // Delete existing file if it exists
 if (file.exists())
 {
 DBG(" Deleting existing file...");
 file.deleteFile();
 }
 
 // Create WAV file
 DBG(" Creating output stream...");
 std::unique_ptr<juce::FileOutputStream> outputStream(file.createOutputStream());
 
 if (outputStream == nullptr)
 {
 DBG(" Failed to create output stream!");
 return false;
 }
 
 // Create WAV writer
 DBG(" Creating WAV writer...");
 juce::WavAudioFormat wavFormat;
 std::unique_ptr<juce::AudioFormatWriter> writer(
 wavFormat.createWriterFor(outputStream.get(),
 currentSampleRate,
 exportBuffer.getNumChannels(),
 24, // 24-bit
 {},
 0));
 
 if (writer == nullptr)
 {
 DBG(" Failed to create WAV writer!");
 // outputStream still owns the stream, it will be cleaned up automatically
 return false;
 }

 // Writer successfully created - it now owns the stream, so release our ownership
 outputStream.release();
 
 // Write the export buffer with fades applied
 DBG(" Writing audio data to file...");
 DBG(" Samples to write: " + juce::String(exportBuffer.getNumSamples()));
 bool writeSuccess = writer->writeFromAudioSampleBuffer(exportBuffer, 0, exportBuffer.getNumSamples());
 
 if (!writeSuccess)
 {
 DBG(" Failed to write audio data!");
 return false;
 }
 
 DBG(" Export successful!");
 DBG(" File size: " + juce::String(file.getSize()) + " bytes");
 
 return true;
}

bool ReverseReverbAudioProcessor::hasEditor() const
{
 return true;
}

juce::AudioProcessorEditor* ReverseReverbAudioProcessor::createEditor()
{
 return new ReverseReverbAudioProcessorEditor (*this);
}

void ReverseReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
 // Save parameters
 std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("ReverseReverbSettings"));
 xml->setAttribute("reverbSize", reverbSize);
 xml->setAttribute("reverbMix", reverbMix);
 xml->setAttribute("dryWet", dryWet);
 xml->setAttribute("tailDivision", tailDivision);
 xml->setAttribute("manualBpm", (double)manualBpm);
 xml->setAttribute("stereoWidth", stereoWidth);
 copyXmlToBinary(*xml, destData);
}

void ReverseReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
 // Load parameters
 std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
 
 if (xmlState.get() != nullptr)
 {
 if (xmlState->hasTagName("ReverseReverbSettings"))
 {
 stereoWidth = (float)xmlState->getDoubleAttribute("stereoWidth", 0.5);
 reverbMix = 1.0f; // Always 100%, ignore saved value
 // Room Size, Tail Division, Gain: load from state
 reverbSize = 1.0f;
 tailDivision = xmlState->getIntAttribute("tailDivision", 3);
 manualBpm = (float)xmlState->getDoubleAttribute("manualBpm", 120.0);
 dryWet = 1.0f;
 }
 }
}

// Get effective BPM: from host (VST3) or manual setting (standalone)
double ReverseReverbAudioProcessor::getEffectiveBpm() const
{
 if (wrapperType == wrapperType_Standalone)
     return (double)manualBpm;

 auto* ph = const_cast<ReverseReverbAudioProcessor*>(this)->getPlayHead();
 if (ph != nullptr)
 {
     auto positionInfo = ph->getPosition();
     if (positionInfo.hasValue())
     {
         auto bpm = positionInfo->getBpm();
         if (bpm.hasValue() && *bpm > 0.0)
             return *bpm;
     }
 }
 return (double)manualBpm; // fallback
}

// Convert tail division + BPM to duration in seconds
float ReverseReverbAudioProcessor::getTailDurationSeconds() const
{
 double bpm = getEffectiveBpm();
 if (bpm <= 0.0) bpm = 120.0;
 float beats = divisionBeats[juce::jlimit(0, 8, tailDivision)];
 return (float)((60.0 / bpm) * (double)beats);
}

// Calculate Tremolo LFO value (returns gain multiplier between 0.0 and 1.0)
float ReverseReverbAudioProcessor::calculateTremoloLFO()
{
 float lfoValue = 0.0f;

 // Calculate the LFO frequency based on mode
 float currentFrequency = tremoloRate;

 // Division multiplier table (shared between sync and non-sync modes)
 static const float divisionMultipliers[] = {
 0.125f, // 0: 2 Bar
 0.25f, // 1: 1 Bar
 1.0f, // 2: 1/1
 2.0f, // 3: 1/2
 4.0f, // 4: 1/4
 8.0f, // 5: 1/8
 16.0f, // 6: 1/16
 32.0f, // 7: 1/32
 64.0f // 8: 1/64
 };

 // Rate Ramp (works with or without sync!)
 if (tremoloRateRampEnabled && isSampleLoaded() && this->isPlaying.load())
 {
 // Calculate progress through the sample (0.0 to 1.0)
 float progress = static_cast<float>(tremoloSampleCounter) /
 static_cast<float>(processedSample.getNumSamples());
 progress = juce::jlimit(0.0f, 1.0f, progress);

 // Get start and end division multipliers
 float startMultiplier = divisionMultipliers[juce::jlimit(0, 8, tremoloStartDivision)];
 float endMultiplier = divisionMultipliers[juce::jlimit(0, 8, tremoloEndDivision)];

 // Interpolate between start and end
 float divisionMultiplier = startMultiplier + (endMultiplier - startMultiplier) * progress;

 // Get BPM (from host or fallback 120)
 double effectiveBpm = 120.0;
 auto playHead = getPlayHead();
 if (playHead != nullptr)
 {
 auto positionInfo = playHead->getPosition();
 if (positionInfo.hasValue())
 {
 auto bpm = positionInfo->getBpm();
 if (bpm.hasValue() && *bpm > 0.0)
 effectiveBpm = *bpm;
 }
 }

 // Calculate frequency: BPM * (subdivision / 60)
 currentFrequency = static_cast<float>((effectiveBpm / 60.0) * divisionMultiplier);

 // Debug output every 10000 samples
 static int debugCounter = 0;
 if (++debugCounter >= 10000)
 {
 DBG(" RATE RAMP ACTIVE! Progress: " << juce::String(progress, 3)
 << " Freq: " << juce::String(currentFrequency, 2) << " Hz"
 << " BPM: " << effectiveBpm);
 debugCounter = 0;
 }
 }
 else if (tremoloSyncEnabled)
 {
 // Host Sync mode (without rate ramp)
 auto playHead = getPlayHead();
 if (playHead != nullptr)
 {
 auto positionInfo = playHead->getPosition();

 if (positionInfo.hasValue())
 {
 auto bpm = positionInfo->getBpm();
 auto ppqPosition = positionInfo->getPpqPosition();
 auto isPlayingHost = positionInfo->getIsPlaying();

 double effectiveBpm = (bpm.hasValue() && *bpm > 0.0) ? *bpm : 120.0;
 bool useHostTransport = isPlayingHost && bpm.hasValue();

 if (effectiveBpm > 0.0)
 {
 float divisionMultiplier = divisionMultipliers[juce::jlimit(0, 8, tremoloSyncDivision)];
 currentFrequency = static_cast<float>((effectiveBpm / 60.0) * divisionMultiplier);

 // Reset phase on transport for tight sync
 if (useHostTransport && ppqPosition.hasValue())
 {
 double currentPpq = *ppqPosition;
 if (currentPpq != lastPosInfo)
 {
 double phaseOffset = std::fmod(currentPpq * divisionMultiplier, 1.0);
 tremoloPhase = static_cast<float>(phaseOffset * juce::MathConstants<double>::twoPi);
 lastPosInfo = currentPpq;
 }
 }
 }
 }
 }
 }
 // else: use tremoloRate (Hz) as-is (free-running mode)

 // Calculate LFO waveform
 switch (tremoloWaveform)
 {
 case 0: // Sine
 lfoValue = std::sin(tremoloPhase);
 break;

 case 1: // Triangle
 lfoValue = (2.0f / juce::MathConstants<float>::pi) * std::asin(std::sin(tremoloPhase));
 break;

 case 2: // Square
 lfoValue = (tremoloPhase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
 break;

 default:
 lfoValue = std::sin(tremoloPhase);
 break;
 }

 // Advance phase
 float phaseIncrement = (juce::MathConstants<float>::twoPi * currentFrequency) / static_cast<float>(currentSampleRate);
 tremoloPhase += phaseIncrement;

 // Wrap phase
 if (tremoloPhase >= juce::MathConstants<float>::twoPi)
 tremoloPhase = std::fmod(tremoloPhase, juce::MathConstants<float>::twoPi);

 // Convert LFO from [-1, 1] to gain multiplier [1-depth, 1]
 float gainMultiplier = 1.0f - (tremoloDepth * 0.5f * (1.0f - lfoValue));

 return gainMultiplier;
}

void ReverseReverbAudioProcessor::getDisplayBufferWithTremolo(juce::AudioBuffer<float>& displayBuffer) const
{
 if (processedSample.getNumSamples() == 0 || !tremoloEnabled)
 {
 displayBuffer.makeCopyOf(processedSample);
 return;
 }

 displayBuffer.makeCopyOf(processedSample);
 int numSamples = displayBuffer.getNumSamples();
 int numChannels = displayBuffer.getNumChannels();

 // Division multiplier table
 static const float divisionMultipliers[] = {
 0.125f, 0.25f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f
 };

 // Simulate the LFO across the entire buffer for visualization
 float phase = 0.0f;
 float sampleRate = (float)currentSampleRate;
 if (sampleRate <= 0.0f) sampleRate = 44100.0f;

 for (int i = 0; i < numSamples; ++i)
 {
 // Determine frequency at this sample position
 float freq = tremoloRate;

 if (tremoloRateRampEnabled)
 {
 // Rate Ramp: interpolate based on position in sample
 float progress = (float)i / (float)numSamples;
 float startMult = divisionMultipliers[juce::jlimit(0, 8, tremoloStartDivision)];
 float endMult = divisionMultipliers[juce::jlimit(0, 8, tremoloEndDivision)];
 float divMult = startMult + (endMult - startMult) * progress;
 // Use 120 BPM as default for visualization
 freq = (120.0f / 60.0f) * divMult;
 }
 else if (tremoloSyncEnabled)
 {
 float divMult = divisionMultipliers[juce::jlimit(0, 8, tremoloSyncDivision)];
 freq = (120.0f / 60.0f) * divMult;
 }

 // Calculate LFO value
 float lfoValue = 0.0f;
 switch (tremoloWaveform)
 {
 case 0: lfoValue = std::sin(phase); break;
 case 1: lfoValue = (2.0f / juce::MathConstants<float>::pi) * std::asin(std::sin(phase)); break;
 case 2: lfoValue = (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f; break;
 default: lfoValue = std::sin(phase); break;
 }

 // Convert to gain
 float gain = 1.0f - (tremoloDepth * 0.5f * (1.0f - lfoValue));

 // Apply to all channels
 for (int ch = 0; ch < numChannels; ++ch)
 {
 displayBuffer.setSample(ch, i, displayBuffer.getSample(ch, i) * gain);
 }

 // Advance phase
 phase += (juce::MathConstants<float>::twoPi * freq) / sampleRate;
 if (phase >= juce::MathConstants<float>::twoPi)
 phase = std::fmod(phase, juce::MathConstants<float>::twoPi);
 }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
 return new ReverseReverbAudioProcessor();
}
