/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NoctaveAudioProcessorEditor::NoctaveAudioProcessorEditor (NoctaveAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set editor size
    setSize (800, 600);

    // Setup sliders
    setupSlider (pitchShiftSlider, pitchShiftLabel, "Pitch Shift");
    setupSlider (mixSlider, mixLabel, "Mix");
    setupSlider (feedbackSlider, feedbackLabel, "Feedback");
    setupSlider (harmonizerSlider, harmonizerLabel, "Harmonizer");

    // Title label
    titleLabel.setText ("NOCTAVE", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (56.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, vampireRed);
    addAndMakeVisible (&titleLabel);

    // Try to load Nosferatu image from resources
    // Projucer should generate BinaryData when the resource is marked in .jucer file
    // First try BinaryData (most reliable for plugins)
    #ifdef BinaryData_nosferatu_png
    nosferatuImage = juce::ImageCache::getFromMemory (BinaryData::nosferatu_png, BinaryData::nosferatu_pngSize);
    #endif
    
    // If BinaryData didn't work, try loading from file paths
    if (! nosferatuImage.isValid())
    {
        // Try from bundle resources (macOS plugins)
        auto bundleFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                            .getParentDirectory()
                            .getChildFile ("Resources")
                            .getChildFile ("nosferatu.png");
        
        if (bundleFile.existsAsFile())
        {
            nosferatuImage = juce::ImageFileFormat::loadFrom (bundleFile);
        }
        else
        {
            // Try from source Resources folder (for development/standalone)
            auto sourceFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                                .getParentDirectory()
                                .getParentDirectory()
                                .getChildFile ("Resources")
                                .getChildFile ("nosferatu.png");
            
            if (sourceFile.existsAsFile())
            {
                nosferatuImage = juce::ImageFileFormat::loadFrom (sourceFile);
            }
            else
            {
                // Try absolute path from project root (development)
                auto projectFile = juce::File ("/Users/christopherkalla/Software Projects/Noctave/Resources/nosferatu.png");
                if (projectFile.existsAsFile())
                {
                    nosferatuImage = juce::ImageFileFormat::loadFrom (projectFile);
                }
            }
        }
    }
}

NoctaveAudioProcessorEditor::~NoctaveAudioProcessorEditor()
{
}

void NoctaveAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& labelText)
{
    // Configure slider
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 25);
    slider.setPopupDisplayEnabled (true, false, this);
    slider.setColour (juce::Slider::rotarySliderFillColourId, vampireRed);
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, vampireDark);
    slider.setColour (juce::Slider::thumbColourId, vampireCrimson);
    slider.setColour (juce::Slider::textBoxTextColourId, vampireText);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, vampireBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId, vampireGray);
    addAndMakeVisible (&slider);

    // Configure label
    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (18.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, vampireText);
    addAndMakeVisible (&label);

    // Attach to parameters
    if (labelText == "Pitch Shift")
    {
        pitchShiftSlider.setTextValueSuffix (" st");
        pitchShiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "PITCH_SHIFT", pitchShiftSlider);
    }
    else if (labelText == "Mix")
    {
        mixSlider.setTextValueSuffix ("%");
        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "MIX", mixSlider);
    }
    else if (labelText == "Feedback")
    {
        feedbackSlider.setTextValueSuffix ("%");
        feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "FEEDBACK", feedbackSlider);
    }
    else if (labelText == "Harmonizer")
    {
        harmonizerSlider.setTextValueSuffix (" st");
        harmonizerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "HARMONIZER", harmonizerSlider);
    }
}

//==============================================================================
void NoctaveAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark vampire-themed gradient background
    juce::ColourGradient gradient (vampireBlack, 0, 0,
                                   vampireDark, 0, (float) getHeight(),
                                   false);
    gradient.addColour (0.3, vampireCrimson.withAlpha (0.1f));
    gradient.addColour (0.7, vampireDark);
    g.setGradientFill (gradient);
    g.fillAll();

    // Draw gothic frame
    drawGothicFrame (g, getLocalBounds());

    // Draw Nosferatu image if available
    if (nosferatuImage.isValid())
    {
        juce::Rectangle<int> imageArea (getWidth() - 280, 100, 250, 400);
        
        // Draw with dark overlay for atmosphere
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRect (imageArea);
        
        // Draw image with slight transparency for eerie effect
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.drawImageWithin (nosferatuImage, 
                          imageArea.getX(), imageArea.getY(),
                          imageArea.getWidth(), imageArea.getHeight(),
                          juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        
        // Add red glow around image
        g.setColour (vampireRed.withAlpha (0.2f));
        for (int i = 1; i <= 5; ++i)
        {
            g.drawRect (imageArea.expanded (i), 1);
        }
    }
    else
    {
        // Draw placeholder if image not found
        juce::Rectangle<int> placeholderArea (getWidth() - 280, 100, 250, 400);
        g.setColour (vampireGray.withAlpha (0.3f));
        g.fillRect (placeholderArea);
        g.setColour (vampireText.withAlpha (0.5f));
        g.setFont (14.0f);
        g.drawText ("Nosferatu Image\n(Add to Resources/)", placeholderArea,
                   juce::Justification::centred, false);
    }

    // Draw decorative gothic elements
    g.setColour (vampireRed.withAlpha (0.3f));
    
    // Draw vertical lines on sides
    g.drawLine (20.0f, 0.0f, 20.0f, (float) getHeight(), 2.0f);
    g.drawLine ((float) getWidth() - 20.0f, 0.0f, (float) getWidth() - 20.0f, (float) getHeight(), 2.0f);
    
    // Draw horizontal decorative lines
    g.drawLine (0.0f, 80.0f, (float) getWidth(), 80.0f, 1.0f);
    g.drawLine (0.0f, (float) getHeight() - 20.0f, (float) getWidth(), (float) getHeight() - 20.0f, 1.0f);

    // Draw subtitle
    g.setFont (juce::Font (14.0f, juce::Font::italic));
    g.setColour (vampireGray);
    g.drawText ("Vampire-Themed Octave Pitch Shifter", getWidth() / 2 - 200, 90, 400, 20,
               juce::Justification::centred, false);
}

void NoctaveAudioProcessorEditor::drawGothicFrame (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Draw ornate gothic-style frame
    g.setColour (vampireRed.withAlpha (0.4f));
    
    // Corner decorations
    const int cornerSize = 30;
    
    // Top-left corner
    g.drawLine (0.0f, 0.0f, (float) cornerSize, 0.0f, 2.0f);
    g.drawLine (0.0f, 0.0f, 0.0f, (float) cornerSize, 2.0f);
    
    // Top-right corner
    g.drawLine ((float) bounds.getWidth() - cornerSize, 0.0f, (float) bounds.getWidth(), 0.0f, 2.0f);
    g.drawLine ((float) bounds.getWidth(), 0.0f, (float) bounds.getWidth(), (float) cornerSize, 2.0f);
    
    // Bottom-left corner
    g.drawLine (0.0f, (float) bounds.getHeight() - cornerSize, 0.0f, (float) bounds.getHeight(), 2.0f);
    g.drawLine (0.0f, (float) bounds.getHeight(), (float) cornerSize, (float) bounds.getHeight(), 2.0f);
    
    // Bottom-right corner
    g.drawLine ((float) bounds.getWidth() - cornerSize, (float) bounds.getHeight(), 
                (float) bounds.getWidth(), (float) bounds.getHeight(), 2.0f);
    g.drawLine ((float) bounds.getWidth(), (float) bounds.getHeight() - cornerSize, 
                (float) bounds.getWidth(), (float) bounds.getHeight(), 2.0f);
}

void NoctaveAudioProcessorEditor::resized()
{
    const int sliderSize = 120;
    const int labelHeight = 30;
    const int spacing = 40;
    const int startY = 150;
    const int leftMargin = 50;
    
    // Title - centered horizontally across the full width
    const int titleWidth = getWidth() - 300; // Account for image on right side
    const int titleX = (getWidth() - titleWidth) / 2; // Center the title in the available space
    titleLabel.setBounds (titleX, 20, titleWidth, 60);

    // Pitch Shift slider (main control)
    pitchShiftSlider.setBounds (leftMargin, startY, sliderSize, sliderSize);
    pitchShiftLabel.setBounds (leftMargin, startY + sliderSize + 5, sliderSize, labelHeight);

    // Mix slider
    mixSlider.setBounds (leftMargin + sliderSize + spacing, startY, sliderSize, sliderSize);
    mixLabel.setBounds (leftMargin + sliderSize + spacing, startY + sliderSize + 5, sliderSize, labelHeight);

    // Feedback slider
    feedbackSlider.setBounds (leftMargin + 2 * (sliderSize + spacing), startY, sliderSize, sliderSize);
    feedbackLabel.setBounds (leftMargin + 2 * (sliderSize + spacing), startY + sliderSize + 5, sliderSize, labelHeight);

    // Harmonizer slider - placed on second row to avoid overlapping with image
    const int secondRowY = startY + sliderSize + labelHeight + 40;
    harmonizerSlider.setBounds (leftMargin, secondRowY, sliderSize, sliderSize);
    harmonizerLabel.setBounds (leftMargin, secondRowY + sliderSize + 5, sliderSize, labelHeight);
}

