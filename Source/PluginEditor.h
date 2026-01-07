/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NoctaveAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NoctaveAudioProcessorEditor (NoctaveAudioProcessor&);
    ~NoctaveAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NoctaveAudioProcessor& audioProcessor;

    // Vampire-themed colors
    juce::Colour vampireBlack = juce::Colour::fromFloatRGBA (0.05f, 0.02f, 0.05f, 1.0f);
    juce::Colour vampireDark = juce::Colour::fromFloatRGBA (0.15f, 0.08f, 0.12f, 1.0f);
    juce::Colour vampireRed = juce::Colour::fromFloatRGBA (0.8f, 0.1f, 0.1f, 1.0f);
    juce::Colour vampireCrimson = juce::Colour::fromFloatRGBA (0.6f, 0.05f, 0.1f, 1.0f);
    juce::Colour vampireGray = juce::Colour::fromFloatRGBA (0.3f, 0.25f, 0.3f, 1.0f);
    juce::Colour vampireText = juce::Colour::fromFloatRGBA (0.9f, 0.85f, 0.9f, 1.0f);

    // Controls
    juce::Slider pitchShiftSlider;
    juce::Slider mixSlider;
    juce::Slider feedbackSlider;
    juce::Slider harmonizerSlider;
    
    juce::Label pitchShiftLabel;
    juce::Label mixLabel;
    juce::Label feedbackLabel;
    juce::Label harmonizerLabel;
    juce::Label titleLabel;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchShiftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> harmonizerAttachment;
    
    // Nosferatu image
    juce::Image nosferatuImage;
    
    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& labelText);
    void drawGothicFrame (juce::Graphics& g, juce::Rectangle<int> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoctaveAudioProcessorEditor)
};

