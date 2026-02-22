import { useRef, useState, useEffect } from 'react';

interface CircularPitchControlProps {
  label: string;
  value: number;
  min: number;
  max: number;
  onChange: (value: number) => void;
}

export function CircularPitchControl({ label, value, min, max, onChange }: CircularPitchControlProps) {
  const [isDragging, setIsDragging] = useState(false);
  const startYRef = useRef(0);
  const startValueRef = useRef(0);

  const normalizedValue = ((value - min) / (max - min)) * 300 - 150;

  useEffect(() => {
    const handleMouseMove = (e: MouseEvent) => {
      if (!isDragging) return;
      
      const delta = startYRef.current - e.clientY;
      const range = max - min;
      const newValue = Math.max(min, Math.min(max, startValueRef.current + (delta / 100) * range));
      const stepped = Math.round(newValue);
      onChange(stepped);
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

  const displayValue = value > 0 ? `+${value}` : value.toString();

  return (
    <div className="flex flex-col items-center gap-3">
      <div 
        className="text-xs uppercase tracking-widest"
        style={{ 
          color: '#666',
          fontFamily: 'monospace',
          fontWeight: '600',
          letterSpacing: '0.15em'
        }}
      >
        {label}
      </div>
      <div className="relative w-48 h-48">
        <svg
          className="w-full h-full cursor-pointer select-none"
          viewBox="0 0 200 200"
          onMouseDown={handleMouseDown}
        >
          <defs>
            <filter id={`glow-${label}`}>
              <feGaussianBlur stdDeviation="2" result="coloredBlur"/>
              <feMerge>
                <feMergeNode in="coloredBlur"/>
                <feMergeNode in="SourceGraphic"/>
              </feMerge>
            </filter>
          </defs>
          
          {/* Outer ring */}
          <circle
            cx="100"
            cy="100"
            r="90"
            fill="none"
            stroke="#1a1a1a"
            strokeWidth="2"
          />
          
          {/* Inner circle background */}
          <circle
            cx="100"
            cy="100"
            r="70"
            fill="radial-gradient(circle, #0a0a0a, #000000)"
            stroke="#2a2a2a"
            strokeWidth="1"
          />
          
          {/* Progress arc notches */}
          {[...Array(61)].map((_, i) => {
            const angle = (i * 300 / 60) - 150;
            const isActive = angle <= normalizedValue;
            const rad = (angle * Math.PI) / 180;
            const x1 = 100 + Math.cos(rad) * 75;
            const y1 = 100 + Math.sin(rad) * 75;
            const x2 = 100 + Math.cos(rad) * 85;
            const y2 = 100 + Math.sin(rad) * 85;
            
            // Make every 5th notch larger
            const isMarker = i % 5 === 0;
            const innerRadius = isMarker ? 72 : 75;
            const x1Final = 100 + Math.cos(rad) * innerRadius;
            const y1Final = 100 + Math.sin(rad) * innerRadius;
            
            return (
              <line
                key={i}
                x1={x1Final}
                y1={y1Final}
                x2={x2}
                y2={y2}
                stroke={isActive ? '#8b0000' : '#2a2a2a'}
                strokeWidth={isMarker ? 2 : 1}
                strokeLinecap="round"
                opacity={isActive ? 1 : 0.3}
              />
            );
          })}
          
          {/* Pointer/indicator */}
          <g transform={`rotate(${normalizedValue} 100 100)`}>
            <line
              x1="100"
              y1="100"
              x2="100"
              y2="40"
              stroke="#8b0000"
              strokeWidth="3"
              strokeLinecap="round"
              filter={`url(#glow-${label})`}
            />
            <polygon
              points="100,35 95,45 105,45"
              fill="#8b0000"
              filter={`url(#glow-${label})`}
            />
          </g>
          
          {/* Center dot */}
          <circle
            cx="100"
            cy="100"
            r="8"
            fill="#1a1a1a"
            stroke="#8b0000"
            strokeWidth="2"
          />
          
          {/* Value display */}
          <text
            x="100"
            y="110"
            textAnchor="middle"
            style={{
              fontSize: '48px',
              fontFamily: 'monospace',
              fontWeight: 'bold',
              fill: '#999',
              userSelect: 'none'
            }}
          >
            {displayValue}
          </text>
        </svg>
      </div>
    </div>
  );
}
