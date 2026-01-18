# Bat Signal Watchface

An animated Batman-themed watchface for Pebble smartwatches featuring the iconic Bat Signal sweeping across Gotham's night sky.

## Features

- **Animated Bat Signal** - A searchlight beam sweeps back and forth across the sky
- **Iconic Batman Logo** - The classic bat symbol appears with a dramatic glow when illuminated by the beam
- **Gotham City Skyline** - Dark silhouette of buildings with glowing windows
- **Twinkling Stars** - Animated stars in the night sky
- **Time & Date Display** - Clear, readable time with day and date
- **Battery Indicator** - Color-coded battery level (green/orange/red)
- **Multi-Platform Support** - Works on all Pebble watches (Aplite, Basalt, Chalk, Diorite)

## Screenshots

### Color Display (Basalt/Diorite)
The full-color version features a deep blue night sky, yellow searchlight beam, and glowing bat symbol.

### B&W Display (Aplite)
The black and white version maintains the dramatic contrast with a striped beam effect.

### Round Display (Chalk)
Optimized layout for the round Pebble Time Round.

## Installation

### From PBW File
1. Transfer `batman-watchface.pbw` to your phone
2. Open with the Pebble app
3. Install to your watch

### From Source
```bash
cd batman-watchface
pebble build
pebble install --emulator basalt  # or --phone for real device
```

## Technical Details

- **Animation Rate**: 20 FPS (50ms interval), drops to 10 FPS on low battery
- **Memory Usage**: ~4.5KB RAM
- **Supported Platforms**: Aplite, Basalt, Chalk, Diorite

## Credits

Created with Claude Code

## License

MIT License
