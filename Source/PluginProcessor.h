#pragma once

#include <JuceHeader.h>

class ReverseReverbAudioProcessor : public juce::AudioProcessor
{
public:
 ReverseReverbAudioProcessor();
 ~ReverseReverbAudioProcessor() override;

 void prepareToPlay (double sampleRate, int samplesPerBlock) override;
 void releaseResources() override;

 bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

 void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override;

 const juce::String getName() const override;

 bool acceptsMidi() const override;
 bool producesMidi() const override;
 bool isMidiEffect() const override;
 double getTailLengthSeconds() const override;

 int getNumPrograms() override;
 int getCurrentProgram() override;
 void setCurrentProgram (int index) override;
 const juce::String getProgramName (int index) override;
 void changeProgramName (int index, const juce::String& newName) override;

 void getStateInformation (juce::MemoryBlock& destData) override;
 void setStateInformation (const void* data, int sizeInBytes) override;

 // Custom methods for our plugin
 void loadAudioFile(const juce::File& file);
 void processReverseReverb();
 void triggerSample();
 bool isSampleLoaded() const { return processedSample.getNumSamples() > 0; }
 bool exportProcessedAudio(const juce::File& file);
 const juce::AudioBuffer<float>* getProcessedBuffer() const { return &processedSample; }

 // Generate a display buffer with tremolo modulation applied (for waveform visualization)
 void getDisplayBufferWithTremolo(juce::AudioBuffer<float>& displayBuffer) const;

 // Playback state getters
 bool getIsPlaying() const { return isPlaying.load(); }
 float getPlaybackProgress() const
 {
  if (!isPlaying.load() || processedSample.getNumSamples() == 0)
   return 0.0f;
  return (float)currentPlaybackPosition / (float)processedSample.getNumSamples();
 }

 // Parameter getters
 float getReverbSize() const { return reverbSize; }
 float getReverbMix() const { return reverbMix; }
 float getDryWet() const { return dryWet; }
 int getTailDivision() const { return tailDivision; }
 float getManualBpm() const { return manualBpm; }
 bool isStandalone() const { return wrapperType == wrapperType_Standalone; }
 double getEffectiveBpm() const;
 float getTailDurationSeconds() const;
 float getStereoWidth() const { return stereoWidth; }
 float getLowCutFreq() const { return lowCutFreq; }
 bool getTransitionMode() const { return transitionMode; }
 juce::String getLoadedFileName() const { return loadedFileName; } // Get original file name
 
 // Tremolo getters
 bool getTremoloEnabled() const { return tremoloEnabled; }
 float getTremoloDepth() const { return tremoloDepth; }
 float getTremoloRate() const { return tremoloRate; }
 int getTremoloWaveform() const { return tremoloWaveform; } // 0=Sine, 1=Triangle, 2=Square
 bool getTremoloSyncEnabled() const { return tremoloSyncEnabled; }
 int getTremoloSyncDivision() const { return tremoloSyncDivision; } // 0=1/4, 1=1/8, 2=1/16, etc.
 
 // Tremolo Rate Ramp getters
 bool getTremoloRateRampEnabled() const { return tremoloRateRampEnabled; }
 int getTremoloStartDivision() const { return tremoloStartDivision; }
 int getTremoloEndDivision() const { return tremoloEndDivision; }
 
 // Parameter setters
 void setReverbSize(float value) { reverbSize = value; }
 void setReverbMix(float value) { reverbMix = value; }
 void setDryWet(float value) { dryWet = value; }
 void setTailDivision(int value) { tailDivision = juce::jlimit(0, 8, value); }
 void setManualBpm(float value) { manualBpm = juce::jlimit(20.0f, 300.0f, value); }
 void setFadeIn(float value) { fadeIn = value; }
 void setFadeOut(float value) { fadeOut = value; }
 void setStereoWidth(float value) { stereoWidth = value; }
 void setLowCutFreq(float value) { lowCutFreq = value; }
 void setTransitionMode(bool value) { transitionMode = value; }
 
 // Tremolo setters
 void setTremoloEnabled(bool value) { tremoloEnabled = value; }
 void setTremoloDepth(float value) { tremoloDepth = juce::jlimit(0.0f, 1.0f, value); }
 void setTremoloRate(float value) { tremoloRate = juce::jlimit(0.1f, 20.0f, value); }
 void setTremoloWaveform(int value) { tremoloWaveform = juce::jlimit(0, 2, value); }
 void setTremoloSyncEnabled(bool value) { tremoloSyncEnabled = value; }
 void setTremoloSyncDivision(int value) { tremoloSyncDivision = juce::jlimit(0, 8, value); } // Changed from 0-6 to 0-8
 
 // Tremolo Rate Ramp setters
 void setTremoloRateRampEnabled(bool value) { tremoloRateRampEnabled = value; }
 void setTremoloStartDivision(int value) { tremoloStartDivision = juce::jlimit(0, 8, value); } // Changed from 0-6 to 0-8
 void setTremoloEndDivision(int value) { tremoloEndDivision = juce::jlimit(0, 8, value); } // Changed from 0-6 to 0-8
 
 // Public access to format manager for file validation
 juce::AudioFormatManager formatManager;

private:
 // Audio buffers
 juce::AudioBuffer<float> originalSample;
 juce::AudioBuffer<float> processedSample;
 juce::AudioBuffer<float> reverbBuffer;
 
 // Playback state
 int currentPlaybackPosition = 0;
 std::atomic<bool> isPlaying { false };
 
 // Reverb engine
 juce::Reverb reverb;
 juce::Reverb::Parameters reverbParams;
 
 // Parameters
 float reverbSize = 1.0f;
 float reverbMix = 1.0f; // Always 100%
 float dryWet = 1.0f;
 int tailDivision = 3;        // 0=8Bar,1=4Bar,2=2Bar,3=1Bar,4=1/2,5=1/4,6=1/8,7=1/16,8=1/32
 float manualBpm = 120.0f;    // Manual BPM for standalone mode

 // Beats per division (assumes 4/4 time)
 static constexpr float divisionBeats[9] = {
     32.0f,   // 0: 8 Bar
     16.0f,   // 1: 4 Bar
     8.0f,    // 2: 2 Bar
     4.0f,    // 3: 1 Bar
     2.0f,    // 4: 1/2
     1.0f,    // 5: 1/4
     0.5f,    // 6: 1/8
     0.25f,   // 7: 1/16
     0.125f   // 8: 1/32
 };
 float fadeIn = 0.0f; // 0.0 to 0.5 (percentage)
 float fadeOut = 0.0f; // 0.0 to 0.5 (percentage)
 float stereoWidth = 0.5f; // 0.0 = mono, 0.5 = normal, 1.0 = max width
 float lowCutFreq = 20.0f; // 20Hz to 500Hz
 bool transitionMode = false;
 
 // Tremolo parameters
 bool tremoloEnabled = false;
 float tremoloDepth = 0.5f; // 0.0 to 1.0 (0% to 100% modulation)
 float tremoloRate = 4.0f; // 0.1 to 20.0 Hz
 int tremoloWaveform = 0; // 0=Sine, 1=Triangle, 2=Square
 bool tremoloSyncEnabled = false;
 int tremoloSyncDivision = 2; // 0=2Bar, 1=1Bar, 2=1/1, 3=1/2, 4=1/4, 5=1/8, 6=1/16, 7=1/32, 8=1/64
 
 // Tremolo Rate Ramp parameters (for sync mode)
 bool tremoloRateRampEnabled = false; // Enable rate ramping from start to end division
 int tremoloStartDivision = 5; // Start division (default 1/8)
 int tremoloEndDivision = 7; // End division (default 1/32)
 
 // Tremolo state
 float tremoloPhase = 0.0f;
 double lastPosInfo = -1.0;
 int tremoloSampleCounter = 0; // Track samples processed for Rate Ramp
 
 // Low cut filter coefficients
 float lowCutPrevInputL = 0.0f;
 float lowCutPrevInputR = 0.0f;
 float lowCutPrevOutputL = 0.0f;
 float lowCutPrevOutputR = 0.0f;
 
 // Stereo delay buffers for width effect
 juce::AudioBuffer<float> delayBufferLeft;
 juce::AudioBuffer<float> delayBufferRight;
 int delayWritePosition = 0;
 
 // Sample rate
 double currentSampleRate = 44100.0;
 
 // Track original loaded file name for export naming
 juce::String loadedFileName = "";
 
 // Tremolo LFO calculation
 float calculateTremoloLFO();
 
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverseReverbAudioProcessor)
};
