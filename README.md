# Noctave - Vampire-Themed Octave Pitch Shifter

A dark and atmospheric VST plugin that provides smooth pitch shifting similar to the DigiTech Whammy pedal.

## Features

- **Pitch Shifting**: -2 to +2 octaves (-24 to +24 semitones) with smooth, artifact-free shifting
- **Mix Control**: Blend between dry and wet signal (0-100%)
- **Feedback Control**: Add regeneration for more dramatic effects (0-50%)
- **Dark Vampire Theme**: Gothic-inspired UI with Nosferatu imagery

## Building

1. Open `Noctave.jucer` in Projucer
2. Export to your preferred IDE (Xcode, Visual Studio, etc.)
3. Build the project

**Note**: The project references JUCE modules from `../NebulaEQ/JUCE/modules`. Make sure NebulaEQ is in the same parent directory, or update the module paths in the .jucer file.

## Adding the Nosferatu Image

To display the Nosferatu image in the plugin:

1. Place your Nosferatu image file at: `Noctave/Resources/nosferatu.png`
2. In Projucer, ensure the image is added as a resource:
   - The image should appear in the Resources group
   - Make sure it's marked as a resource (not compiled)
3. Re-export the project and rebuild

Alternatively, you can embed the image in BinaryData:
1. In Projucer, add the image to the BinaryData section
2. Update `PluginEditor.cpp` to use `BinaryData::nosferatu_png` instead of file loading

## Parameters

- **Pitch Shift**: Controls the pitch shift amount in semitones (-24 to +24)
- **Mix**: Controls the blend between original and pitch-shifted signal (0-100%)
- **Feedback**: Adds regeneration to the pitch-shifted signal (0-50%)

## Technical Details

The pitch shifter uses a delay-based algorithm with linear interpolation for smooth pitch changes. The implementation is optimized for real-time performance and provides low latency operation.

