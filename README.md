# M5-UniSphere-3D

Efficient 3D UV Sphere rendering with texture mapping for M5Stack devices, powered by `M5Unified`.

![License](https://img.shields.io/github/license/Yuikawa-Akira/M5-UniSphere-3D)
![Platform](https://img.shields.io/badge/Platform-Arduino/M5Stack-orange)

## Overview

**M5-UniSphere-3D** generates a UV Sphere and maps a dynamic `M5Canvas` texture onto its surface. 

This project demonstrates 3D coordinate transformation, Z-sorting, and back-face culling on resource-constrained microcontrollers.

## Features

* **UV Sphere Generation:** Procedural generation of sphere vertices and quads.
* **Texture Mapping:** Maps an `M5Canvas` (texture) onto the 3D sphere surface.
* **Real-time Rendering:** Includes rotation, projection, and optimized drawing loops.
* **Performance Optimization:** * Back-face culling to skip hidden polygons.
* **Hardware Agnostic:** Uses `M5Unified` to support various M5Stack hardware (CoreS3, Core2, Basic, etc.).

## Demo


## Requirements

### Hardware
* **M5Stack Series:** CoreS3, Core2, M5Stack Fire, Basic, etc.

### Software/Libraries
* [M5Unified](https://github.com/m5stack/M5Unified)

## Installation

1.  Open the [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/).
2.  Install the `M5Unified` library via the Library Manager.
3.  Copy the code from `M5-UniSphere-3D.ino` into your project.
4.  Select your M5Stack board and upload.

## Configuration

You can easily adjust the rendering quality and performance in the source code:

```cpp
const int GRID_SIZE = 32;       // Higher = Smoother sphere, lower = Higher FPS
const float SPHERE_RADIUS = 100.0f; 
const int TEX_WIDTH = 64;       // Texture resolution
const int TEX_HEIGHT = 32;
```

## Credits
### This software includes code and logic derived from:

* [3D Sphere Pong Wars](https://github.com/K-Yama2010/3D_Sphere_Pong_Wars/tree/main) by K-Yama2010
    * Copyright (c) 2025 K-Yama2010
