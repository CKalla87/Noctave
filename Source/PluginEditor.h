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
class NoctaveAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     private juce::AudioProcessorValueTreeState::Listener
{
public:
    NoctaveAudioProcessorEditor (NoctaveAudioProcessor&);
    ~NoctaveAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void paintOverChildren (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent& e) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct SteppedRotarySlider : public juce::Slider
    {
        SteppedRotarySlider (float pixelsForFullRange, bool snapToInt);

        void mouseDown (const juce::MouseEvent& e) override;
        void mouseDrag (const juce::MouseEvent& e) override;

    private:
        int startDragY = 0;
        double startValue = 0.0;
        float pixelsForFullRange = 100.0f;
        bool snapToInt = false;
    };

    struct TriangleToggleButton : public juce::ToggleButton
    {
        void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    };

    struct TriangleButton : public juce::Button
    {
        TriangleButton();
        void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    };

    struct PowerIndicator : public juce::Component
    {
        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent&) override;

        void setOn (bool shouldBeOn);
        bool isOn() const;

        std::function<void(bool)> onPowerChange;

    private:
        bool on = true;
    };

    struct DetuneDisplay : public juce::Component
    {
        enum class DetuneMode
        {
            Align,
            Left,
            Right
        };

        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;

        void setDetuneMode (DetuneMode newMode);
        DetuneMode getDetuneMode() const;
        void setDetuneValue (float newValue);
        std::function<void(DetuneMode)> onModeChange;

    private:
        DetuneMode detuneMode = DetuneMode::Right;
        float detuneValue = 8.5f;
        juce::Rectangle<float> alignBounds;
        juce::Rectangle<float> detuneLeftBounds;
        juce::Rectangle<float> detuneRightBounds;
    };

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NoctaveAudioProcessor& audioProcessor;

    // Controls
    SteppedRotarySlider pitchASlider { 100.0f, true };
    SteppedRotarySlider pitchBSlider { 100.0f, true };
    SteppedRotarySlider widthSlider { 150.0f, false };
    SteppedRotarySlider lowEqSlider { 150.0f, false };
    SteppedRotarySlider harmonizerSlider { 150.0f, false };
    SteppedRotarySlider highEqSlider { 150.0f, false };
    SteppedRotarySlider mixSlider { 150.0f, false };

    TriangleToggleButton crushButton;
    TriangleToggleButton gurnButton;
    TriangleButton leftCornerButton;
    TriangleButton rightCornerButton;
    PowerIndicator powerIndicator;
    DetuneDisplay detuneDisplay;

    std::unique_ptr<juce::LookAndFeel_V4> bigKnobLookAndFeel;
    std::unique_ptr<juce::LookAndFeel_V4> smallKnobLookAndFeel;
    std::unique_ptr<SliderAttachment> pitchAAttachment;
    std::unique_ptr<SliderAttachment> pitchBAttachment;
    std::unique_ptr<SliderAttachment> harmonizerAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<ButtonAttachment> crushAttachment;
    std::unique_ptr<ButtonAttachment> gurnAttachment;
    std::unique_ptr<ButtonAttachment> leftCornerAttachment;
    std::unique_ptr<ButtonAttachment> rightCornerAttachment;

    juce::Image backgroundImage;

    void parameterChanged (const juce::String& parameterID, float newValue) override;

    juce::Rectangle<int> pitchALabelBounds;
    juce::Rectangle<int> pitchBLabelBounds;
    juce::Rectangle<int> widthLabelBounds;
    juce::Rectangle<int> lowEqLabelBounds;
    juce::Rectangle<int> harmonizerLabelBounds;
    juce::Rectangle<int> highEqLabelBounds;
    juce::Rectangle<int> mixLabelBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoctaveAudioProcessorEditor)
};

