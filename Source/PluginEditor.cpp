/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
    constexpr int kEditorWidth = 1200;
    constexpr int kEditorHeight = 800;
    constexpr int kLayoutWidth = 1536;
    constexpr int kLayoutHeight = 1024;

    const juce::Colour kActiveRed (0xff8b0000);
    const juce::Colour kEyeGlowRed (0xffcc2222);
    const juce::Colour kInactiveTick (0xff2a2a2a);

    // Vampire eye positions (red overlay centered on white eyeballs)
    constexpr int kLeftEyeX = 768;
    constexpr int kRightEyeX = 836;
    constexpr int kLeftEyeY = 326;
    constexpr int kRightEyeY = 330;
    constexpr int kEyeRadius = 12;
    constexpr int kGlowRadius = 42;
    const juce::Colour kOuterRingDark (0xff1a1a1a);
    const juce::Colour kLabelGrey (0xff666666);
    const juce::Colour kValueGrey (0xff999999);

    constexpr float kStartAngle = juce::MathConstants<float>::pi * (-150.0f / 180.0f);
    constexpr float kEndAngle = juce::MathConstants<float>::pi * (150.0f / 180.0f);

    juce::Font makeMonoFont (float height, juce::Font::FontStyleFlags style)
    {
        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, style));
    }

    juce::Image loadBackgroundImage()
    {
        juce::Image image;

        #ifdef BinaryData_noctave_background_png
        image = juce::ImageCache::getFromMemory (BinaryData::noctave_background_png, BinaryData::noctave_background_pngSize);
        #endif

        if (! image.isValid())
        {
            auto executableFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
            auto bundleFile = executableFile.getParentDirectory()
                                          .getParentDirectory()
                                          .getChildFile ("Resources")
                                          .getChildFile ("noctave_background.png");

            if (bundleFile.existsAsFile())
            {
                image = juce::ImageFileFormat::loadFrom (bundleFile);
            }
            else
            {
                auto sourceFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                    .getParentDirectory()
                    .getParentDirectory()
                    .getChildFile ("Resources")
                    .getChildFile ("noctave_background.png");

                if (sourceFile.existsAsFile())
                {
                    image = juce::ImageFileFormat::loadFrom (sourceFile);
                }
                else
                {
                    auto projectFile = juce::File ("/Users/christopherkalla/Software Projects/Noctave/Resources/noctave_background.png");
                    if (projectFile.existsAsFile())
                        image = juce::ImageFileFormat::loadFrom (projectFile);
                }
            }
        }

        return image;
    }

    void drawTriangle (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour)
    {
        juce::Path triangle;
        auto center = bounds.getCentre();
        float width = bounds.getWidth() * 0.4f;
        float height = bounds.getHeight() * 0.32f;

        triangle.addTriangle (center.x, center.y - height,
                              center.x - width, center.y + height,
                              center.x + width, center.y + height);
        g.setColour (colour);
        g.fillPath (triangle);
    }

    struct BigKnobLookAndFeel : public juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                               juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                  static_cast<float> (width), static_cast<float> (height));
            auto size = std::min (bounds.getWidth(), bounds.getHeight());
            auto centre = bounds.getCentre();
            float radius = size * 0.5f;

            float outerRingRadius = radius * 0.9f;
            float innerRadius = radius * 0.7f;
            float tickOuterRadius = radius * 0.85f;
            float tickInnerRadius = radius * 0.75f;
            float tickInnerMarkerRadius = radius * 0.72f;

            auto currentAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            g.setColour (kOuterRingDark);
            g.drawEllipse (centre.x - outerRingRadius, centre.y - outerRingRadius,
                           outerRingRadius * 2.0f, outerRingRadius * 2.0f, 2.0f);

            juce::ColourGradient gradient (juce::Colour (0xff0a0a0a), centre.x, centre.y,
                                           juce::Colour (0xff000000), centre.x, centre.y + innerRadius, true);
            g.setGradientFill (gradient);
            g.fillEllipse (centre.x - innerRadius, centre.y - innerRadius,
                           innerRadius * 2.0f, innerRadius * 2.0f);

            g.setColour (kInactiveTick);
            g.drawEllipse (centre.x - innerRadius, centre.y - innerRadius,
                           innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

            for (int i = 0; i <= 60; ++i)
            {
                float t = static_cast<float> (i) / 60.0f;
                float angle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
                bool isMarker = (i % 5 == 0);
                float inner = isMarker ? tickInnerMarkerRadius : tickInnerRadius;

                juce::Point<float> start (centre.x + std::cos (angle) * inner,
                                          centre.y + std::sin (angle) * inner);
                juce::Point<float> end (centre.x + std::cos (angle) * tickOuterRadius,
                                        centre.y + std::sin (angle) * tickOuterRadius);

                bool isActive = angle <= currentAngle;
                auto tickColour = isActive ? kActiveRed : kInactiveTick.withAlpha (0.3f);
                g.setColour (tickColour);
                g.drawLine ({ start, end }, isMarker ? 2.0f : 1.0f);
            }

            float pointerLength = outerRingRadius * 0.666f;
            float pointerThickness = std::max (2.0f, size * 0.015f);
            juce::Point<float> pointerEnd (centre.x + std::cos (currentAngle) * pointerLength,
                                           centre.y + std::sin (currentAngle) * pointerLength);

            g.setColour (kActiveRed);
            g.drawLine (centre.x, centre.y, pointerEnd.x, pointerEnd.y, pointerThickness);

            float tipLength = pointerLength + outerRingRadius * 0.055f;
            float baseHalf = outerRingRadius * 0.06f;
            juce::Point<float> tip (centre.x + std::cos (currentAngle) * tipLength,
                                    centre.y + std::sin (currentAngle) * tipLength);
            juce::Point<float> left (centre.x + std::cos (currentAngle + juce::MathConstants<float>::pi / 2.0f) * baseHalf,
                                     centre.y + std::sin (currentAngle + juce::MathConstants<float>::pi / 2.0f) * baseHalf);
            juce::Point<float> right (centre.x + std::cos (currentAngle - juce::MathConstants<float>::pi / 2.0f) * baseHalf,
                                      centre.y + std::sin (currentAngle - juce::MathConstants<float>::pi / 2.0f) * baseHalf);

            juce::Path triangle;
            triangle.addTriangle (tip, left, right);
            g.fillPath (triangle);

            float dotRadius = radius * 0.08f;
            g.setColour (kOuterRingDark);
            g.fillEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
            g.setColour (kActiveRed);
            g.drawEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f, 2.0f);

            auto value = slider.getValue();
            juce::String displayValue = value > 0.0 ? "+" + juce::String (static_cast<int> (std::round (value)))
                                                    : juce::String (static_cast<int> (std::round (value)));

            juce::Font font = makeMonoFont (size * 0.24f, juce::Font::bold);
            g.setFont (font);
            g.setColour (kValueGrey);

            auto textBounds = bounds.withY (centre.y - size * 0.05f).withHeight (size * 0.3f);
            g.drawText (displayValue, textBounds, juce::Justification::centred, false);
        }
    };

    struct SmallKnobLookAndFeel : public juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                               juce::Slider&) override
        {
            auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                  static_cast<float> (width), static_cast<float> (height));
            auto size = std::min (bounds.getWidth(), bounds.getHeight());
            auto centre = bounds.getCentre();
            float radius = size * 0.5f;

            float outerRingRadius = radius * 0.9f;
            float innerRadius = radius * 0.7f;
            float tickOuterRadius = radius * 0.64f;
            float tickInnerRadius = radius * 0.56f;

            auto currentAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            g.setColour (kInactiveTick);
            g.drawEllipse (centre.x - outerRingRadius, centre.y - outerRingRadius,
                           outerRingRadius * 2.0f, outerRingRadius * 2.0f, 1.0f);

            juce::ColourGradient gradient (kOuterRingDark, centre.x, centre.y,
                                           juce::Colour (0xff0a0a0a), centre.x, centre.y + innerRadius, true);
            g.setGradientFill (gradient);
            g.fillEllipse (centre.x - innerRadius, centre.y - innerRadius,
                           innerRadius * 2.0f, innerRadius * 2.0f);

            g.setColour (kInactiveTick);
            g.drawEllipse (centre.x - innerRadius, centre.y - innerRadius,
                           innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

            for (int i = 0; i <= 20; ++i)
            {
                float t = static_cast<float> (i) / 20.0f;
                float angle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);

                juce::Point<float> start (centre.x + std::cos (angle) * tickInnerRadius,
                                          centre.y + std::sin (angle) * tickInnerRadius);
                juce::Point<float> end (centre.x + std::cos (angle) * tickOuterRadius,
                                        centre.y + std::sin (angle) * tickOuterRadius);

                bool isActive = angle <= currentAngle;
                g.setColour (isActive ? kActiveRed : kInactiveTick);
                g.drawLine ({ start, end }, 1.5f);
            }

            float pointerLength = radius * 0.6f;
            juce::Point<float> pointerEnd (centre.x + std::cos (currentAngle) * pointerLength,
                                           centre.y + std::sin (currentAngle) * pointerLength);
            g.setColour (kActiveRed);
            g.drawLine (centre.x, centre.y, pointerEnd.x, pointerEnd.y, 2.0f);
        }
    };
}

NoctaveAudioProcessorEditor::SteppedRotarySlider::SteppedRotarySlider (float pixels, bool snap)
    : pixelsForFullRange (pixels), snapToInt (snap)
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
}

void NoctaveAudioProcessorEditor::SteppedRotarySlider::mouseDown (const juce::MouseEvent& e)
{
    startDragY = e.y;
    startValue = getValue();
}

void NoctaveAudioProcessorEditor::SteppedRotarySlider::mouseDrag (const juce::MouseEvent& e)
{
    auto range = getRange().getLength();
    if (range <= 0.0)
        return;

    auto delta = static_cast<float> (startDragY - e.y);
    auto newValue = startValue + (delta / pixelsForFullRange) * range;
    if (snapToInt)
        newValue = std::round (newValue);

    newValue = juce::jlimit (getRange().getStart(), getRange().getEnd(), newValue);
    setValue (newValue, juce::sendNotificationSync);
}

void NoctaveAudioProcessorEditor::TriangleToggleButton::paintButton (juce::Graphics& g, bool, bool)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    bool isOn = getToggleState();

    juce::ColourGradient gradient (isOn ? kActiveRed : kInactiveTick, bounds.getCentreX(), bounds.getY(),
                                   isOn ? juce::Colour (0xff5a0000) : kOuterRingDark, bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillEllipse (bounds);

    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawEllipse (bounds, 2.0f);

    drawTriangle (g, bounds, isOn ? kActiveRed : juce::Colour (0xff4a4a4a));
}

NoctaveAudioProcessorEditor::TriangleButton::TriangleButton()
    : juce::Button ("triangle")
{
    setClickingTogglesState (true);
}

void NoctaveAudioProcessorEditor::TriangleButton::paintButton (juce::Graphics& g, bool, bool)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    bool isOn = getToggleState();
    juce::ColourGradient gradient (isOn ? kActiveRed : kInactiveTick, bounds.getCentreX(), bounds.getY(),
                                   isOn ? juce::Colour (0xff5a0000) : kOuterRingDark, bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillEllipse (bounds);

    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawEllipse (bounds, 2.0f);

    drawTriangle (g, bounds, isOn ? kActiveRed : juce::Colour (0xff4a4a4a));
}

void NoctaveAudioProcessorEditor::PowerIndicator::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto dotBounds = juce::Rectangle<float> (bounds.getX(), bounds.getCentreY() - 5.0f, 10.0f, 10.0f);

    g.setColour (on ? kActiveRed : juce::Colour (0xff1a0000));
    g.fillEllipse (dotBounds);
    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawEllipse (dotBounds, 1.0f);

    juce::Font font = makeMonoFont (12.0f, juce::Font::plain);
    g.setFont (font);
    g.setColour (kValueGrey);
    g.drawText ("PWR", dotBounds.getRight() + 6.0f, bounds.getY(), 40.0f, bounds.getHeight(),
                juce::Justification::centredLeft, false);
}

void NoctaveAudioProcessorEditor::PowerIndicator::mouseDown (const juce::MouseEvent&)
{
    on = ! on;
    if (onPowerChange)
        onPowerChange (on);
    repaint();
}

void NoctaveAudioProcessorEditor::PowerIndicator::setOn (bool shouldBeOn)
{
    on = shouldBeOn;
    repaint();
}

bool NoctaveAudioProcessorEditor::PowerIndicator::isOn() const
{
    return on;
}

void NoctaveAudioProcessorEditor::DetuneDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float indicatorSize = 20.0f;
    float labelHeight = 14.0f;

    auto alignCenter = juce::Point<float> (bounds.getX() + 40.0f, bounds.getCentreY() - 10.0f);
    auto detuneLeftCenter = juce::Point<float> (bounds.getX() + 120.0f, bounds.getCentreY() - 10.0f);
    auto detuneRightCenter = juce::Point<float> (bounds.getX() + 150.0f, bounds.getCentreY() - 10.0f);

    alignBounds = juce::Rectangle<float> (alignCenter.x - indicatorSize * 0.5f,
                                          alignCenter.y - indicatorSize * 0.5f,
                                          indicatorSize, indicatorSize);
    detuneLeftBounds = juce::Rectangle<float> (detuneLeftCenter.x - indicatorSize * 0.5f,
                                               detuneLeftCenter.y - indicatorSize * 0.5f,
                                               indicatorSize, indicatorSize);
    detuneRightBounds = juce::Rectangle<float> (detuneRightCenter.x - indicatorSize * 0.5f,
                                                detuneRightCenter.y - indicatorSize * 0.5f,
                                                indicatorSize, indicatorSize);

    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawEllipse (alignBounds, 2.0f);
    g.drawEllipse (detuneLeftBounds, 2.0f);
    g.drawEllipse (detuneRightBounds, 2.0f);

    g.setColour (kActiveRed);
    if (detuneMode == DetuneMode::Align)
        g.fillEllipse (alignBounds.reduced (3.0f));
    else if (detuneMode == DetuneMode::Left)
        g.fillEllipse (detuneLeftBounds.reduced (3.0f));
    else
        g.fillEllipse (detuneRightBounds.reduced (3.0f));

    juce::Font labelFont = makeMonoFont (12.0f, juce::Font::plain);
    g.setFont (labelFont);
    g.setColour (kLabelGrey);
    g.drawText ("ALIGN", alignBounds.withY (alignBounds.getBottom() + 4.0f).withHeight (labelHeight),
                juce::Justification::centred, false);
    g.drawText ("DETUNE", detuneLeftBounds.withX (detuneLeftBounds.getX() - 5.0f)
                .withWidth (60.0f)
                .withY (detuneLeftBounds.getBottom() + 4.0f)
                .withHeight (labelHeight),
                juce::Justification::centred, false);

    juce::Font valueFont = makeMonoFont (44.0f, juce::Font::bold);
    g.setFont (valueFont);

    auto valueBounds = bounds.withX (bounds.getX() + 210.0f).withWidth (160.0f);
    float displayValue = detuneValue;
    if (detuneMode == DetuneMode::Align)
        displayValue = 0.0f;
    else if (detuneMode == DetuneMode::Left)
        displayValue = -std::abs (detuneValue);
    else
        displayValue = std::abs (detuneValue);

    juce::String valueText = (displayValue >= 0.0f ? "+" : "") + juce::String (displayValue, 1);

    g.setColour (kActiveRed.withAlpha (0.4f));
    g.drawText (valueText, valueBounds.translated (0.0f, 1.0f), juce::Justification::centredLeft, false);
    g.setColour (kActiveRed);
    g.drawText (valueText, valueBounds, juce::Justification::centredLeft, false);
}

void NoctaveAudioProcessorEditor::DetuneDisplay::mouseDown (const juce::MouseEvent& e)
{
    auto pos = e.position;
    if (alignBounds.contains (pos))
    {
        setDetuneMode (DetuneMode::Align);
        if (onModeChange)
            onModeChange (detuneMode);
        return;
    }

    if (detuneLeftBounds.contains (pos))
    {
        setDetuneMode (DetuneMode::Left);
        if (onModeChange)
            onModeChange (detuneMode);
        return;
    }

    if (detuneRightBounds.contains (pos))
    {
        setDetuneMode (DetuneMode::Right);
        if (onModeChange)
            onModeChange (detuneMode);
    }
}

void NoctaveAudioProcessorEditor::DetuneDisplay::setDetuneMode (DetuneMode newMode)
{
    detuneMode = newMode;
    repaint();
}

NoctaveAudioProcessorEditor::DetuneDisplay::DetuneMode
NoctaveAudioProcessorEditor::DetuneDisplay::getDetuneMode() const
{
    return detuneMode;
}

void NoctaveAudioProcessorEditor::DetuneDisplay::setDetuneValue (float newValue)
{
    detuneValue = std::abs (newValue);
    repaint();
}

//==============================================================================
NoctaveAudioProcessorEditor::NoctaveAudioProcessorEditor (NoctaveAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setResizable (false, false);
    setOpaque (false);

    bigKnobLookAndFeel = std::make_unique<BigKnobLookAndFeel>();
    smallKnobLookAndFeel = std::make_unique<SmallKnobLookAndFeel>();

    auto configureBigKnob = [this] (SteppedRotarySlider& slider)
    {
        slider.setLookAndFeel (bigKnobLookAndFeel.get());
        slider.setRange (-12.0, 12.0, 1.0);
        slider.setRotaryParameters (kStartAngle, kEndAngle, true);
        addAndMakeVisible (slider);
    };

    auto configureSmallKnob = [this] (SteppedRotarySlider& slider)
    {
        slider.setLookAndFeel (smallKnobLookAndFeel.get());
        slider.setRange (0.0, 100.0, 0.1);
        slider.setRotaryParameters (kStartAngle, kEndAngle, true);
        addAndMakeVisible (slider);
    };

    configureBigKnob (pitchASlider);
    configureBigKnob (pitchBSlider);

    configureSmallKnob (widthSlider);
    configureSmallKnob (lowEqSlider);
    configureSmallKnob (harmonizerSlider);
    configureSmallKnob (highEqSlider);
    configureSmallKnob (mixSlider);

    harmonizerSlider.setRange (-12.0, 12.0, 0.1);
    mixSlider.setRange (0.0, 1.0, 0.01);

    pitchASlider.setValue (5.0);
    pitchBSlider.setValue (-7.0);
    widthSlider.setValue (50.0);
    lowEqSlider.setValue (30.0);
    highEqSlider.setValue (70.0);
    mixSlider.setValue (0.65);
    harmonizerSlider.setValue (8.5);

    addAndMakeVisible (crushButton);
    addAndMakeVisible (gurnButton);
    addAndMakeVisible (leftCornerButton);
    addAndMakeVisible (rightCornerButton);
    addAndMakeVisible (powerIndicator);
    auto* powerParam = audioProcessor.apvts.getParameter ("POWER");
    powerIndicator.setOn (powerParam != nullptr && powerParam->getValue() >= 0.5f);
    powerIndicator.onPowerChange = [this] (bool poweredOn)
    {
        if (auto* p = audioProcessor.apvts.getParameter ("POWER"))
            p->setValueNotifyingHost (poweredOn ? 1.0f : 0.0f);
        repaint();
    };
    audioProcessor.apvts.addParameterListener ("POWER", this);
    addAndMakeVisible (detuneDisplay);
    detuneDisplay.setDetuneValue (harmonizerSlider.getValue());

    detuneDisplay.onModeChange = [this] (DetuneDisplay::DetuneMode mode)
    {
        float amount = std::abs (harmonizerSlider.getValue());
        if (amount < 0.1f)
            amount = 8.5f;

        if (mode == DetuneDisplay::DetuneMode::Align)
            harmonizerSlider.setValue (0.0, juce::sendNotificationSync);
        else if (mode == DetuneDisplay::DetuneMode::Left)
            harmonizerSlider.setValue (-amount, juce::sendNotificationSync);
        else
            harmonizerSlider.setValue (amount, juce::sendNotificationSync);
    };

    harmonizerSlider.onValueChange = [this]
    {
        detuneDisplay.setDetuneValue (harmonizerSlider.getValue());
    };

    backgroundImage = loadBackgroundImage();
    if (backgroundImage.isValid())
        setSize (backgroundImage.getWidth(), backgroundImage.getHeight());
    else
        setSize (kEditorWidth, kEditorHeight);

    pitchAAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "PITCH_SHIFT", pitchASlider);
    harmonizerAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "HARMONIZER", harmonizerSlider);
    mixAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "MIX", mixSlider);
    crushAttachment = std::make_unique<ButtonAttachment> (audioProcessor.apvts, "CRUSH", crushButton);
    gurnAttachment = std::make_unique<ButtonAttachment> (audioProcessor.apvts, "GURN", gurnButton);
    leftCornerAttachment = std::make_unique<ButtonAttachment> (audioProcessor.apvts, "LEFT_CORNER", leftCornerButton);
    rightCornerAttachment = std::make_unique<ButtonAttachment> (audioProcessor.apvts, "RIGHT_CORNER", rightCornerButton);
}

NoctaveAudioProcessorEditor::~NoctaveAudioProcessorEditor()
{
    audioProcessor.apvts.removeParameterListener ("POWER", this);
    pitchASlider.setLookAndFeel (nullptr);
    pitchBSlider.setLookAndFeel (nullptr);
    widthSlider.setLookAndFeel (nullptr);
    lowEqSlider.setLookAndFeel (nullptr);
    harmonizerSlider.setLookAndFeel (nullptr);
    highEqSlider.setLookAndFeel (nullptr);
    mixSlider.setLookAndFeel (nullptr);
}

void NoctaveAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == "POWER")
    {
        powerIndicator.setOn (newValue >= 0.5f);
        repaint();
    }
}

//==============================================================================
void NoctaveAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (backgroundImage.isValid())
    {
        g.drawImageAt (backgroundImage, 0, 0);
    }

    juce::Font labelFont = makeMonoFont (14.0f, juce::Font::plain);
    labelFont.setExtraKerningFactor (0.15f);
    g.setFont (labelFont);
    g.setColour (kLabelGrey);
    g.drawText ("PITCH A", pitchALabelBounds, juce::Justification::centred, false);
    g.drawText ("PITCH B", pitchBLabelBounds, juce::Justification::centred, false);

    juce::Font smallFont = makeMonoFont (12.0f, juce::Font::plain);
    smallFont.setExtraKerningFactor (0.12f);
    g.setFont (smallFont);
    g.drawText ("WIDTH", widthLabelBounds, juce::Justification::centred, false);
    g.drawText ("LOW EQ", lowEqLabelBounds, juce::Justification::centred, false);
    g.drawText ("HARMONIZER", harmonizerLabelBounds, juce::Justification::centred, false);
    g.drawText ("HIGH EQ", highEqLabelBounds, juce::Justification::centred, false);
    g.drawText ("MIX", mixLabelBounds, juce::Justification::centred, false);
}

void NoctaveAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    if (! powerIndicator.isOn())
        return;

    const float scaleX = static_cast<float> (getWidth()) / static_cast<float> (kLayoutWidth);
    const float scaleY = static_cast<float> (getHeight()) / static_cast<float> (kLayoutHeight);
    const float scale = std::min (scaleX, scaleY);

    auto scaled = [scale] (int value) { return static_cast<float> (value) * scale; };

    const float leftEyeX = scaled (kLeftEyeX);
    const float rightEyeX = scaled (kRightEyeX);
    const float leftEyeY = scaled (kLeftEyeY);
    const float rightEyeY = scaled (kRightEyeY);
    const float eyeRadius = scaled (kEyeRadius);
    const float glowRadius = scaled (kGlowRadius);

    auto drawGlowingEye = [&g] (float centreX, float centreY, float glowRad, float eyeRad)
    {
        juce::ColourGradient glow (kEyeGlowRed.withAlpha (0.85f), centreX, centreY,
                                   juce::Colours::transparentBlack, centreX + glowRad, centreY, true);
        g.setGradientFill (glow);
        g.fillEllipse (centreX - glowRad, centreY - glowRad, glowRad * 2.0f, glowRad * 2.0f);

        g.setColour (kEyeGlowRed.withAlpha (0.95f));
        g.fillEllipse (centreX - eyeRad, centreY - eyeRad, eyeRad * 2.0f, eyeRad * 2.0f);

        g.setColour (kActiveRed);
        g.fillEllipse (centreX - eyeRad * 0.6f, centreY - eyeRad * 0.6f,
                       eyeRad * 1.2f, eyeRad * 1.2f);
    };

    drawGlowingEye (leftEyeX, leftEyeY, glowRadius, eyeRadius);
    drawGlowingEye (rightEyeX, rightEyeY, glowRadius, eyeRadius);
}

void NoctaveAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    DBG ("x=" << e.x << " y=" << e.y);
}

void NoctaveAudioProcessorEditor::resized()
{
    const float scaleX = static_cast<float> (getWidth()) / static_cast<float> (kLayoutWidth);
    const float scaleY = static_cast<float> (getHeight()) / static_cast<float> (kLayoutHeight);
    const float scale = std::min (scaleX, scaleY);

    auto scaled = [scale] (int value)
    {
        return juce::roundToInt (value * scale);
    };

    const int bigKnobSize = scaled (320);
    const int smallKnobSize = scaled (100);
    const int buttonSize = scaled (40);
    const int cornerButtonSize = scaled (60);

    auto placeCentered = [] (juce::Component& component, int centerX, int centerY, int size)
    {
        component.setBounds (centerX - size / 2, centerY - size / 2, size, size);
    };

    placeCentered (pitchASlider, scaled (285), scaled (501), bigKnobSize);
    placeCentered (pitchBSlider, scaled (1251), scaled (517), bigKnobSize);

    placeCentered (widthSlider, scaled (207), scaled (726), smallKnobSize);
    placeCentered (lowEqSlider, scaled (363), scaled (726), smallKnobSize);
    placeCentered (harmonizerSlider, scaled (774), scaled (760), smallKnobSize);
    placeCentered (highEqSlider, scaled (1174), scaled (726), smallKnobSize);
    placeCentered (mixSlider, scaled (1330), scaled (726), smallKnobSize);

    constexpr int buttonOffsetX = 40;
    // Crush button centered between WIDTH (207) and LOW EQ (363)
    placeCentered (crushButton, scaled (285), scaled (822), buttonSize);
    placeCentered (gurnButton, scaled (1252), scaled (822), buttonSize);  // Centered between HIGH EQ (1174) and MIX (1330)
    // Left: crush (small) at 285, big at 243 — big is 42 units left of small
    placeCentered (leftCornerButton, scaled (243), scaled (890), cornerButtonSize);
    // Right: mirror layout — gurn (small) at 1252, big 42 units to the right (opposite side)
    placeCentered (rightCornerButton, scaled (1294), scaled (890), cornerButtonSize);

    // PWR centered below leftCornerButton (big triangle), with small gap
    constexpr int powerIndicatorWidth = 120;
    constexpr int powerIndicatorHeight = 30;
    constexpr int leftCornerCenterX = 243;
    constexpr int leftCornerBottomY = 890 + 30;  // center Y + half of cornerButtonSize
    constexpr int gapBelowButton = 5;
    powerIndicator.setBounds (scaled (leftCornerCenterX - powerIndicatorWidth / 2),
                              scaled (leftCornerBottomY + gapBelowButton),
                              scaled (powerIndicatorWidth),
                              scaled (powerIndicatorHeight));

    detuneDisplay.setBounds (scaled (565), scaled (860), scaled (420), scaled (90));

    auto labelAbove = [] (juce::Rectangle<int> knobBounds, int height, int offset)
    {
        return juce::Rectangle<int> (knobBounds.getX(), knobBounds.getY() - offset - height,
                                     knobBounds.getWidth(), height);
    };

    auto labelBelow = [] (juce::Rectangle<int> knobBounds, int height, int offset)
    {
        return juce::Rectangle<int> (knobBounds.getX(), knobBounds.getBottom() + offset,
                                     knobBounds.getWidth(), height);
    };

    pitchALabelBounds = labelAbove (pitchASlider.getBounds(), scaled (20), scaled (12));
    pitchBLabelBounds = labelAbove (pitchBSlider.getBounds(), scaled (20), scaled (12));

    widthLabelBounds = labelBelow (widthSlider.getBounds(), scaled (16), scaled (6));
    lowEqLabelBounds = labelBelow (lowEqSlider.getBounds(), scaled (16), scaled (6));
    harmonizerLabelBounds = labelBelow (harmonizerSlider.getBounds(), scaled (16), scaled (6))
        .withX (harmonizerSlider.getX() - scaled (20))
        .withWidth (harmonizerSlider.getWidth() + scaled (40));
    highEqLabelBounds = labelBelow (highEqSlider.getBounds(), scaled (16), scaled (6));
    mixLabelBounds = labelBelow (mixSlider.getBounds(), scaled (16), scaled (6));
}

//==============================================================================
