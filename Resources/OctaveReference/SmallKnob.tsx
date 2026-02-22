import { useRef, useState, useEffect } from 'react';

interface SmallKnobProps {
  label: string;
  value: number;
  min: number;
  max: number;
  onChange: (value: number) => void;
}

export function SmallKnob({ label, value, min, max, onChange }: SmallKnobProps) {
  const [isDragging, setIsDragging] = useState(false);
  const startYRef = useRef(0);
  const startValueRef = useRef(0);

  const normalizedValue = ((value - min) / (max - min)) * 300 - 150;

  useEffect(() => {
    const handleMouseMove = (e: MouseEvent) => {
      if (!isDragging) return;
      
      const delta = startYRef.current - e.clientY;
      const range = max - min;
      const newValue = Math.max(min, Math.min(max, startValueRef.current + (delta / 150) * range));
      onChange(newValue);
    };

    const handleMouseUp = () => {
      setIsDragging(false);
    };

    if (isDragging) {
      document.addEventListener('mousemove', handleMouseMove);
      document.addEventListener('mouseup', handleMouseUp);
    }

    return () => {
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, [isDragging, min, max, onChange]);

  const handleMouseDown = (e: React.MouseEvent) => {
    setIsDragging(true);
    startYRef.current = e.clientY;
    startValueRef.current = value;
  };

  return (
    <div className="flex flex-col items-center gap-2">
      <div className="relative w-20 h-20">
        <svg
          className="w-full h-full cursor-pointer select-none"
          viewBox="0 0 100 100"
          onMouseDown={handleMouseDown}
        >
          <defs>
            <radialGradient id={`smallKnob-${label}`}>
              <stop offset="0%" stopColor="#1a1a1a" />
              <stop offset="100%" stopColor="#0a0a0a" />
            </radialGradient>
          </defs>
          
          {/* Outer ring */}
          <circle
            cx="50"
            cy="50"
            r="45"
            fill="none"
            stroke="#2a2a2a"
            strokeWidth="1"
          />
          
          {/* Knob body */}
          <circle
            cx="50"
            cy="50"
            r="35"
            fill={`url(#smallKnob-${label})`}
            stroke="#2a2a2a"
            strokeWidth="1"
          />
          
          {/* Notches */}
          {[...Array(21)].map((_, i) => {
            const angle = (i * 300 / 20) - 150;
            const isActive = angle <= normalizedValue;
            const rad = (angle * Math.PI) / 180;
            const x1 = 50 + Math.cos(rad) * 28;
            const y1 = 50 + Math.sin(rad) * 28;
            const x2 = 50 + Math.cos(rad) * 32;
            const y2 = 50 + Math.sin(rad) * 32;
            
            return (
              <line
                key={i}
                x1={x1}
                y1={y1}
                x2={x2}
                y2={y2}
                stroke={isActive ? '#8b0000' : '#2a2a2a'}
                strokeWidth="1.5"
                strokeLinecap="round"
              />
            );
          })}
          
          {/* Pointer */}
          <g transform={`rotate(${normalizedValue} 50 50)`}>
            <line
              x1="50"
              y1="50"
              x2="50"
              y2="20"
              stroke="#8b0000"
              strokeWidth="2"
              strokeLinecap="round"
            />
          </g>
        </svg>
      </div>
      
      <div 
        className="text-[10px] uppercase tracking-wider text-center"
        style={{ 
          color: '#666',
          fontFamily: 'monospace',
          fontWeight: '600'
        }}
      >
        {label}
      </div>
    </div>
  );
}
