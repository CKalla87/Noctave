/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class NoctaveAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    NoctaveAudioProcessor();
    ~NoctaveAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter management
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    // Pitch shifter parameters
    std::atomic<float>* pitchShiftParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* feedbackParam = nullptr;
    std::atomic<float>* harmonizerParam = nullptr;

private:
    //==============================================================================
    // Pitch shifter implementation using delay-based approach
    class PitchShifter
    {
    public:
        PitchShifter();
        void prepare (double sampleRate, int maxBlockSize);
        void reset();
        void processBlock (juce::AudioBuffer<float>& buffer, float pitchShiftSemitones, float mix, float feedback);
        
    private:
        static constexpr int maxDelaySamples = 44100; // 1 second at 44.1kHz
        
        struct Voice
        {
            juce::AudioBuffer<float> delayBuffer;
            float writePosition = 0.0f;
            float readPosition = 0.0f;
        };
        
        Voice voices[1]; // Single voice for pitch shifting
        double currentSampleRate = 44100.0;
        float smoothedPitchShift = 0.0f;
    };
    
    PitchShifter pitchShifters[2]; // One per channel (stereo)
    PitchShifter harmonizers[2]; // One per channel for harmonizer
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoctaveAudioProcessor)
};

