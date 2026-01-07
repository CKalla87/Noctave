/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// PitchShifter Implementation
//==============================================================================

NoctaveAudioProcessor::PitchShifter::PitchShifter()
{
    voices[0].delayBuffer.setSize (1, maxDelaySamples);
    voices[0].delayBuffer.clear();
}

void NoctaveAudioProcessor::PitchShifter::prepare (double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    voices[0].delayBuffer.setSize (1, maxDelaySamples);
    voices[0].delayBuffer.clear();
    voices[0].writePosition = maxDelaySamples * 0.5f; // Start at middle of buffer
    voices[0].readPosition = maxDelaySamples * 0.5f;
    
    smoothedPitchShift = 0.0f;
}

void NoctaveAudioProcessor::PitchShifter::reset()
{
    voices[0].delayBuffer.clear();
    voices[0].writePosition = maxDelaySamples * 0.5f;
    voices[0].readPosition = maxDelaySamples * 0.5f;
}


void NoctaveAudioProcessor::PitchShifter::processBlock (juce::AudioBuffer<float>& buffer, 
                                                         float pitchShiftSemitones, 
                                                         float mix, 
                                                         float feedback)
{
    if (buffer.getNumSamples() == 0)
        return;
    
    // Smooth pitch shift parameter to avoid clicks
    const float smoothingFactor = 0.995f;
    smoothedPitchShift = smoothedPitchShift * smoothingFactor + pitchShiftSemitones * (1.0f - smoothingFactor);
    
    // Convert semitones to pitch ratio
    float pitchRatio = std::pow (2.0f, smoothedPitchShift / 12.0f);
    
    // Clamp feedback to prevent runaway accumulation
    feedback = juce::jlimit (0.0f, 0.5f, feedback);
    
    // Process each sample
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float input = buffer.getSample (0, sample);
        
        // Protect against hot input signals that could cause clipping
        // More aggressive input limiting to prevent downstream issues
        input = juce::jlimit (-0.9f, 0.9f, input);
        
        // Use the first voice for processing
        auto& voice = voices[0];
        
        // Calculate read position based on pitch ratio
        // When pitchRatio > 1 (shift up), read moves faster than write (read decrements more)
        // When pitchRatio < 1 (shift down), read moves slower than write (read decrements less)
        // Read position moves backwards relative to write
        voice.readPosition -= pitchRatio;
        
        // Wrap read position
        while (voice.readPosition < 0.0f)
            voice.readPosition += maxDelaySamples;
        while (voice.readPosition >= maxDelaySamples)
            voice.readPosition -= maxDelaySamples;
        
        // Linear interpolation for smooth reading
        int readPosInt = static_cast<int> (voice.readPosition);
        float frac = voice.readPosition - readPosInt;
        int readPosNext = (readPosInt + 1) % maxDelaySamples;
        
        float sample1 = voice.delayBuffer.getSample (0, readPosInt);
        float sample2 = voice.delayBuffer.getSample (0, readPosNext);
        float delayed = sample1 + frac * (sample2 - sample1);
        
        // Apply soft clipping to delayed signal to prevent harsh clipping
        // More aggressive limiting to prevent hot signals from pitch shifter
        float output = juce::jlimit (-0.85f, 0.85f, delayed);
        
        // Apply feedback with proper scaling to prevent accumulation
        // Calculate feedback contribution with stronger attenuation to prevent runaway
        // Use exponential decay to prevent feedback from building up indefinitely
        float feedbackContribution = output * feedback * 0.75f; // Even stronger attenuation for stability
        
        // Write input + feedback to delay buffer, with aggressive limiting to prevent clipping
        // This ensures the delay buffer never contains values that would cause clipping
        float delayInput = juce::jlimit (-0.85f, 0.85f, input + feedbackContribution);
        int writePosInt = static_cast<int> (voice.writePosition);
        voice.delayBuffer.setSample (0, writePosInt, delayInput);
        
        // Update write position (always increments by 1)
        voice.writePosition += 1.0f;
        if (voice.writePosition >= maxDelaySamples)
            voice.writePosition -= maxDelaySamples;
        
        // Mix dry and wet with proper gain staging and headroom
        // Reduce gain more aggressively when mix is high to prevent clipping at 100% wet
        float mixReduction = 1.0f - (mix * 0.15f); // Reduce up to 15% when mix is 100%
        float wetGain = mix * 0.85f * mixReduction;  // More reduction for headroom
        float dryGain = (1.0f - mix) * 0.9f;  // Slight reduction for headroom
        float finalOutput = input * dryGain + output * wetGain;
        
        // Apply aggressive soft clipping to final output to prevent hard clipping
        // Use a smooth tanh-based soft clipper for natural-sounding limiting
        const float threshold = 0.8f;  // Lower threshold for more protection
        if (std::abs (finalOutput) > threshold)
        {
            // Soft clip using tanh approximation for smooth limiting
            float sign = finalOutput > 0.0f ? 1.0f : -1.0f;
            float absValue = std::abs (finalOutput);
            float excess = absValue - threshold;
            // Smooth transition: compress excess above threshold more aggressively
            finalOutput = sign * (threshold + (1.0f - threshold) * std::tanh (excess * 6.0f));
        }
        
        // Final hard limit as safety (should rarely be needed with soft clipping)
        finalOutput = juce::jlimit (-0.9f, 0.9f, finalOutput);
        
        buffer.setSample (0, sample, finalOutput);
    }
}

//==============================================================================
// AudioProcessor Implementation
//==============================================================================

NoctaveAudioProcessor::NoctaveAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Get parameter pointers
    pitchShiftParam = apvts.getRawParameterValue("PITCH_SHIFT");
    mixParam = apvts.getRawParameterValue("MIX");
    feedbackParam = apvts.getRawParameterValue("FEEDBACK");
    harmonizerParam = apvts.getRawParameterValue("HARMONIZER");
}

NoctaveAudioProcessor::~NoctaveAudioProcessor()
{
}

//==============================================================================
const juce::String NoctaveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoctaveAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoctaveAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoctaveAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NoctaveAudioProcessor::getTailLengthSeconds() const
{
    return 1.0; // 1 second tail for delay buffer
}

int NoctaveAudioProcessor::getNumPrograms()
{
    return 1;
}

int NoctaveAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoctaveAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoctaveAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoctaveAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NoctaveAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    for (int channel = 0; channel < 2; ++channel)
    {
        pitchShifters[channel].prepare (sampleRate, samplesPerBlock);
        harmonizers[channel].prepare (sampleRate, samplesPerBlock);
    }
}

void NoctaveAudioProcessor::releaseResources()
{
    for (int channel = 0; channel < 2; ++channel)
    {
        pitchShifters[channel].reset();
        harmonizers[channel].reset();
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoctaveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NoctaveAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get parameter values
    float pitchShift = pitchShiftParam->load();
    float mix = mixParam->load();
    float feedback = feedbackParam->load();
    float harmonizerInterval = harmonizerParam->load();

    // Process each channel
    for (int channel = 0; channel < totalNumInputChannels && channel < 2; ++channel)
    {
        // Create a single-channel buffer for processing
        juce::AudioBuffer<float> singleChannelBuffer (1, buffer.getNumSamples());
        singleChannelBuffer.copyFrom (0, 0, buffer, channel, 0, buffer.getNumSamples());
        
        // Store original input for harmonizer
        juce::AudioBuffer<float> originalBuffer (1, buffer.getNumSamples());
        originalBuffer.copyFrom (0, 0, buffer, channel, 0, buffer.getNumSamples());
        
        // Process the channel with pitch shifter
        pitchShifters[channel].processBlock (singleChannelBuffer, pitchShift, mix, feedback);
        
        // Process harmonizer if interval is not zero
        if (std::abs (harmonizerInterval) > 0.1f)
        {
            juce::AudioBuffer<float> harmonizerBuffer (1, buffer.getNumSamples());
            harmonizerBuffer.copyFrom (0, 0, originalBuffer, 0, 0, buffer.getNumSamples());
            
            // Process harmonizer with 100% wet mix and no feedback
            harmonizers[channel].processBlock (harmonizerBuffer, harmonizerInterval, 1.0f, 0.0f);
            
            // Mix harmonizer with main output with proper gain staging
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float mainSample = singleChannelBuffer.getSample (0, sample);
                float harmonySample = harmonizerBuffer.getSample (0, sample);
                
                // Limit both signals before mixing to prevent clipping (more aggressive)
                mainSample = juce::jlimit (-0.85f, 0.85f, mainSample);
                harmonySample = juce::jlimit (-0.85f, 0.85f, harmonySample);
                
                // Mix: 60% main, 40% harmony with reduced gain for headroom
                // Additional reduction when mix is high to prevent clipping
                float mixLevel = mixParam->load();
                float mixScale = 1.0f - (mixLevel * 0.1f); // Reduce up to 10% more when mix is high
                float mixed = (mainSample * 0.6f + harmonySample * 0.4f) * mixScale;
                
                // Apply aggressive soft limiting to prevent clipping
                if (std::abs (mixed) > 0.8f)
                {
                    float sign = mixed > 0.0f ? 1.0f : -1.0f;
                    float absValue = std::abs (mixed);
                    float excess = absValue - 0.8f;
                    mixed = sign * (0.8f + (1.0f - 0.8f) * std::tanh (excess * 6.0f));
                }
                
                // Final hard limit (more conservative)
                mixed = juce::jlimit (-0.9f, 0.9f, mixed);
                singleChannelBuffer.setSample (0, sample, mixed);
            }
        }
        
        // Copy processed audio back to main buffer
        buffer.copyFrom (channel, 0, singleChannelBuffer, 0, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool NoctaveAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NoctaveAudioProcessor::createEditor()
{
    return new NoctaveAudioProcessorEditor (*this);
}

//==============================================================================
void NoctaveAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void NoctaveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// Parameter layout creation
juce::AudioProcessorValueTreeState::ParameterLayout NoctaveAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Pitch Shift: -24 to +24 semitones (-2 to +2 octaves, like DigiTech Whammy)
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID ("PITCH_SHIFT", 1), "Pitch Shift",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
        0.0f, "semitones"
    ));

    // Mix: 0 to 100% wet
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID ("MIX", 1), "Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        1.0f, "%"
    ));

    // Feedback: 0 to 50% for regeneration
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID ("FEEDBACK", 1), "Feedback",
        juce::NormalisableRange<float> (0.0f, 0.5f, 0.01f),
        0.0f, "%"
    ));

    // Harmonizer: -12 to +12 semitones for harmony intervals
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID ("HARMONIZER", 1), "Harmonizer",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 1.0f),
        0.0f, "semitones"
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoctaveAudioProcessor();
}

