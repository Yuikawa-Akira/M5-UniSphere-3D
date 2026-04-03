# M5-UniSphere-3D

`M5Unified`を活用し、M5Stack 向けにテクスチャマッピング対応の3D UV スフィアを描画します。

![License](https://img.shields.io/github/license/Yuikawa-Akira/M5-UniSphere-3D)
![Platform](https://img.shields.io/badge/Platform-Arduino/M5Stack-orange)

## Overview

**M5-UniSphere-3D** はUV スフィアを生成し、その表面に動的な `M5Canvas` テクスチャをマッピングします。

リソースの限られたマイコン上で、3D座標変換、Zソート、およびバックフェースカリングを実装するデモンストレーションが可能です。

## Features

* **UV Sphere Generation:** 頂点と面（Quad）を動的に生成
* **Texture Mapping:** `M5Canvas` 上の描画内容を3D球体の表面にマッピング
* **Real-time Rendering:** 高速な回転・投影・描画ループ
* **Performance Optimization:** * 背面にあるポリゴンの描画をスキップ
* **Hardware Agnostic:** `M5Unified` により、CoreS3, Core2, Basic などの多様なM5ハードウェアに対応
  
## Demo


## Requirements

### Hardware
* **M5Stack Series:** CoreS3, Core2, M5Stack Fire, Basic 等

### Software/Libraries
* [M5Unified](https://github.com/m5stack/M5Unified)

## Installation

1.  [Arduino IDE](https://www.arduino.cc/en/software) または [PlatformIO](https://platformio.org/) を開きます
2.  ライブラリマネージャーから `M5Unified` をインストールします
3.  `M5-UniSphere-3D.ino` のコードをプロジェクトにコピーします
4.  お使いのM5Stackボードを選択してアップロードしてください

## Configuration

ソースコード内の以下の定数を変更することで、品質とパフォーマンスを調整できます

```cpp
const int GRID_SIZE = 32;       // 値を上げると滑らかになりますが、FPSが低下します
const float SPHERE_RADIUS = 100.0f; 
const int TEX_WIDTH = 64;       // テクスチャの解像度
const int TEX_HEIGHT = 32;
```

## Credits
### 以下のコードおよびロジックが含まれています

* [3D Sphere Pong Wars](https://github.com/K-Yama2010/3D_Sphere_Pong_Wars/tree/main) by K-Yama2010
    * Copyright (c) 2025 K-Yama2010
