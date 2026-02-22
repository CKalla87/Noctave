import { useState } from 'react';
import { CircularPitchControl } from './components/CircularPitchControl';
import { SmallKnob } from './components/SmallKnob';
import backgroundImage from 'figma:asset/068418aaba4f10fc82093bd7760e030159edda61.png';

interface NoctaveSettings {
  pitchA: number;
  pitchB: number;
  width: number;
  lowEq: number;
  highEq: number;
  mix: number;
  detune: number;
  harmonizer: number;
}

export default function App() {
  const [isPowerOn, setIsPowerOn] = useState(true);
  const [crushActive, setCrushActive] = useState(false);
  const [gurnActive, setGurnActive] = useState(false);
  const [alignMode, setAlignMode] = useState(false);
  
  const [settings, setSettings] = useState<NoctaveSettings>({
    pitchA: 5,
    pitchB: -7,
    width: 50,
    lowEq: 30,
    highEq: 70,
    mix: 65,
    detune: 8.5,
    harmonizer: 50,
  });

  const updateSetting = <K extends keyof NoctaveSettings>(
    key: K,
    value: NoctaveSettings[K]
  ) => {
    setSettings(prev => ({ ...prev, [key]: value }));
  };

  return (
    <div 
      className="min-h-screen flex items-center justify-center p-8"
      style={{
        background: '#000000',
      }}
    >
      <div className="w-full max-w-5xl">
        <div 
          className="relative rounded-lg overflow-hidden"
          style={{
            backgroundImage: `url(${backgroundImage})`,
            backgroundSize: 'cover',
            backgroundPosition: 'center',
            boxShadow: '0 30px 80px rgba(0,0,0,0.9)',
            aspectRatio: '16/10',
          }}
        >
          {/* Title section */}
          <div className="absolute top-22 left-0 right-0 flex flex-col items-center">
            <h1 
              className="text-sm uppercase tracking-widest"
              style={{
                color: '#666',
                fontFamily: 'monospace',
              }}
            >
              PITCH SHIFTER
            </h1>
          </div>

          {/* Main control area - Pitch controls */}
          <div className="absolute" style={{ top: '140px', left: '0', right: '0' }}>
            <div className="flex items-center justify-between px-20">
              {/* Left: Pitch A */}
              <div className="flex-shrink-0">
                <CircularPitchControl
                  label="PITCH A"
                  value={settings.pitchA}
                  min={-12}
                  max={12}
                  onChange={(v) => updateSetting('pitchA', v)}
                />
              </div>

              {/* Right: Pitch B */}
              <div className="flex-shrink-0">
                <CircularPitchControl
                  label="PITCH B"
                  value={settings.pitchB}
                  min={-12}
                  max={12}
                  onChange={(v) => updateSetting('pitchB', v)}
                />
              </div>
            </div>
          </div>

          {/* Small knobs section */}
          <div className="absolute" style={{ bottom: '165px', left: '0', right: '0' }}>
            <div className="flex items-center justify-between px-20">
              {/* Left side knobs */}
              <div className="flex items-center gap-8" style={{ marginLeft: '20px' }}>
                <SmallKnob
                  label="WIDTH"
                  value={settings.width}
                  min={0}
                  max={100}
                  onChange={(v) => updateSetting('width', v)}
                />
                <SmallKnob
                  label="LOW EQ"
                  value={settings.lowEq}
                  min={0}
                  max={100}
                  onChange={(v) => updateSetting('lowEq', v)}
                />
              </div>
              {/* Right side knobs */}
              <div className="flex items-center gap-8" style={{ marginRight: '20px' }}>
                <SmallKnob
                  label="HIGH EQ"
                  value={settings.highEq}
                  min={0}
                  max={100}
                  onChange={(v) => updateSetting('highEq', v)}
                />
                <SmallKnob
                  label="MIX"
                  value={settings.mix}
                  min={0}
                  max={100}
                  onChange={(v) => updateSetting('mix', v)}
                />
              </div>
            </div>
          </div>

          {/* Effect buttons and harmonizer knob */}
          <div className="absolute" style={{ bottom: '105px', left: '0', right: '0' }}>
            <div className="relative">
              {/* Left side - CRUSH button centered between WIDTH and LOW EQ */}
              <div className="absolute" style={{ left: '130px' }}>
                <button
                  onClick={() => setCrushActive(!crushActive)}
                  className="relative group"
                  style={{
                    width: '45px',
                    height: '45px',
                  }}
                >
                  <div 
                    className="absolute inset-0 rounded-full transition-all"
                    style={{
                      background: crushActive 
                        ? 'linear-gradient(180deg, #8b0000, #5a0000)'
                        : 'linear-gradient(180deg, #2a2a2a, #1a1a1a)',
                      border: '2px solid #3a3a3a',
                      boxShadow: crushActive
                        ? '0 0 20px rgba(139,0,0,0.6), inset 0 2px 4px rgba(0,0,0,0.5)'
                        : 'inset 0 2px 4px rgba(0,0,0,0.8)',
                    }}
                  />
                  <div className="absolute inset-0 flex items-center justify-center">
                    <svg width="18" height="14" viewBox="0 0 18 14">
                      <path d="M 9 0 L 0 14 L 18 14 Z" fill={crushActive ? '#ff0000' : '#4a4a4a'} />
                    </svg>
                  </div>
                </button>
              </div>

              {/* Center - HARMONIZER knob */}
              <div className="absolute left-1/2 -translate-x-1/2" style={{ bottom: '20px' }}>
                <SmallKnob
                  label="HARMONIZER"
                  value={settings.harmonizer}
                  min={0}
                  max={100}
                  onChange={(v) => updateSetting('harmonizer', v)}
                />
              </div>

              {/* Right side - GURN button centered between HIGH EQ and MIX */}
              <div className="absolute" style={{ right: '130px' }}>
                <button
                  onClick={() => setGurnActive(!gurnActive)}
                  className="relative group"
                  style={{
                    width: '45px',
                    height: '45px',
                  }}
                >
                  <div 
                    className="absolute inset-0 rounded-full transition-all"
                    style={{
                      background: gurnActive 
                        ? 'linear-gradient(180deg, #8b0000, #5a0000)'
                        : 'linear-gradient(180deg, #2a2a2a, #1a1a1a)',
                      border: '2px solid #3a3a3a',
                      boxShadow: gurnActive
                        ? '0 0 20px rgba(139,0,0,0.6), inset 0 2px 4px rgba(0,0,0,0.5)'
                        : 'inset 0 2px 4px rgba(0,0,0,0.8)',
                    }}
                  />
                  <div className="absolute inset-0 flex items-center justify-center">
                    <svg width="18" height="14" viewBox="0 0 18 14">
                      <path d="M 9 0 L 0 14 L 18 14 Z" fill={gurnActive ? '#ff0000' : '#4a4a4a'} />
                    </svg>
                  </div>
                </button>
              </div>
            </div>
          </div>

          {/* Bottom controls - centered ALIGN/DETUNE section */}
          <div className="absolute bottom-8 left-1/2 -translate-x-1/2">
            <div className="flex items-center gap-8">
              {/* ALIGN toggle */}
              <button
                onClick={() => setAlignMode(!alignMode)}
                className="flex flex-col items-center gap-1"
              >
                <div
                  className="w-5 h-5 rounded-full border-2 transition-all"
                  style={{
                    borderColor: '#3a3a3a',
                    background: alignMode ? '#8b0000' : 'transparent',
                  }}
                />
                <span 
                  className="text-[10px] uppercase tracking-wider"
                  style={{ color: '#666', fontFamily: 'monospace' }}
                >
                  ALIGN
                </span>
              </button>

              {/* DETUNE display */}
              <div className="flex flex-col items-center gap-1">
                <div className="flex items-center gap-2">
                  <div
                    className="w-5 h-5 rounded-full border-2"
                    style={{
                      borderColor: '#3a3a3a',
                      background: !alignMode ? '#8b0000' : 'transparent',
                    }}
                  />
                  <div
                    className="w-5 h-5 rounded-full border-2"
                    style={{
                      borderColor: '#3a3a3a',
                      background: 'transparent',
                    }}
                  />
                </div>
                <span 
                  className="text-[10px] uppercase tracking-wider"
                  style={{ color: '#666', fontFamily: 'monospace' }}
                >
                  DETUNE
                </span>
              </div>

              {/* Detune value */}
              <div 
                className="text-4xl font-mono font-bold"
                style={{
                  color: '#8b0000',
                  textShadow: '0 0 10px rgba(139,0,0,0.5)'
                }}
              >
                +{settings.detune.toFixed(1)}
              </div>
            </div>
          </div>

          {/* Power indicator */}
          <div className="absolute bottom-4 left-32 flex items-center gap-2">
            <div
              className="w-3 h-3 rounded-full"
              style={{
                background: isPowerOn ? '#8b0000' : '#1a0000',
                boxShadow: isPowerOn ? '0 0 10px #8b0000' : 'none',
                border: '1px solid #3a3a3a',
              }}
            />
            <button
              onClick={() => setIsPowerOn(!isPowerOn)}
              className="text-[10px] uppercase tracking-wider"
              style={{ color: '#999', fontFamily: 'monospace' }}
            >
              PWR
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}
