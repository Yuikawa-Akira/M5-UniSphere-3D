/*
 * [M5-UniSphere-3D] : 
 * * Copyright (c) 2026 Yuikawa-Akira
 * This software includes code from : https://github.com/K-Yama2010/3D_Sphere_Pong_Wars
 * Copyright (c) 2025 K-Yama2010
 * * Released under the MIT License.
 */

#include <M5Unified.h>

// =================================================================
// 設定 (Configuration)
// =================================================================
const int GRID_SIZE = 32;            // 球の分割数。上げるとテクスチャが滑らかになりますが重くなります
const float SPHERE_RADIUS = 100.0f;  // 球の半径

// 画面表示用スプライトのサイズ
const int CANVAS_WIDTH = 240;
const int CANVAS_HEIGHT = 240;

// テクスチャ用スプライト（キャンバス）のサイズ (2:1の比率が球体に貼りやすいです)
const int TEX_WIDTH = 64;
const int TEX_HEIGHT = 32;

// カメラ設定
const float CAMERA_Z = SPHERE_RADIUS * 3.0f;
const float FOV = 220.0f;

// FPS表示用
uint32_t lastTime = 0;
float fps = 0;

// =================================================================
// 3D演算用の構造体とヘルパー関数
// =================================================================
struct Vec3 {
  float x, y, z;
  Vec3(float x = 0, float y = 0, float z = 0)
    : x(x), y(y), z(z) {}
  float magnitude() const {
    return sqrt(x * x + y * y + z * z);
  }
  void normalize() {
    float mag = magnitude();
    if (mag > 0) {
      x /= mag;
      y /= mag;
      z /= mag;
    }
  }
  Vec3 operator+(const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
  }
  Vec3 operator-(const Vec3& v) const {
    return Vec3(x - v.x, y - v.y, z - v.z);
  }
  Vec3 operator*(float s) const {
    return Vec3(x * s, y * s, z * s);
  }
};
struct Vec2 {
  float x, y;
};

struct Quad {
  int vertex_indices[4];
  Vec3 center;
  int tex_index;
};

Vec3 cross(const Vec3& a, const Vec3& b) {
  return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
float dot(const Vec3& a, const Vec3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

uint16_t swap16(uint16_t value) {
  return (value << 8) | (value >> 8);
}

// =================================================================
// グローバル変数
// =================================================================
M5Canvas canvas(&M5.Display);  // 画面描画用
M5Canvas texCanvas;            // テクスチャ用（任意のキャンバス）

std::vector<Vec3> vertices;
std::vector<Quad> quads;
float rotationY = 0.0f;
float rotationX = 0.0f;

// =================================================================
// 描画用のワークエリア
// =================================================================
std::vector<Vec2> projected_vertices;
std::vector<Vec3> rotated_vertices;
struct ZOrder {
  float z;
  int index;
};
std::vector<ZOrder> z_sorted_quads;

void reserveMemory() {
  size_t v_size = vertices.size();
  size_t q_size = quads.size();
  projected_vertices.resize(v_size);
  rotated_vertices.resize(v_size);
  z_sorted_quads.resize(q_size);
}

// =================================================================
// テクスチャキャンバスの生成
// =================================================================
void createTextureCanvas() {
  // 色深度を16ビットに指定（ポインタで直接アクセスするため）
  texCanvas.setColorDepth(16);
  texCanvas.createSprite(TEX_WIDTH, TEX_HEIGHT);

  // 背景を市松模様にする
  for (int y = 0; y < TEX_HEIGHT; y += 2) {
    for (int x = 0; x < TEX_WIDTH; x += 2) {
      uint16_t color = ((x / 2 + y / 2) % 2 == 0) ? TFT_DARKGREEN : TFT_GREEN;
      texCanvas.fillRect(x, y, 2, 2, color);
    }
  }

  // 文字を描画
  texCanvas.setTextDatum(middle_center);
  texCanvas.setTextSize(1);
  texCanvas.setTextColor(TFT_WHITE);
  texCanvas.drawString("Hello!", TEX_WIDTH / 2, TEX_HEIGHT / 2);
}

// =================================================================
// UVスフィア生成と色情報の事前準備
// =================================================================
void createUVSphere() {
  vertices.clear();
  quads.clear();

  int latBands = GRID_SIZE;
  int lonBands = GRID_SIZE * 2;

  for (int lat = 0; lat <= latBands; lat++) {
    float theta = lat * PI / latBands;
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);

    for (int lon = 0; lon <= lonBands; lon++) {
      float phi = lon * TWO_PI / lonBands;
      float sinPhi = sin(phi);
      float cosPhi = cos(phi);

      Vec3 v;
      v.x = SPHERE_RADIUS * cosPhi * sinTheta;
      v.y = SPHERE_RADIUS * cosTheta;
      v.z = SPHERE_RADIUS * sinPhi * sinTheta;
      vertices.push_back(v);
    }
  }

  for (int lat = 0; lat < latBands; lat++) {
    for (int lon = 0; lon < lonBands; lon++) {
      int first = (lat * (lonBands + 1)) + lon;
      int second = first + lonBands + 1;

      Quad q;
      q.vertex_indices[0] = first;
      q.vertex_indices[1] = first + 1;
      q.vertex_indices[2] = second + 1;
      q.vertex_indices[3] = second;

      q.center = (vertices[q.vertex_indices[0]] + vertices[q.vertex_indices[1]] + vertices[q.vertex_indices[2]] + vertices[q.vertex_indices[3]]) * 0.25f;

      float u = (lon + 0.5f) / (float)lonBands;
      float v = (latBands - lat - 0.5f) / (float)latBands;

      int tx = constrain((int)(u * TEX_WIDTH), 0, TEX_WIDTH - 1);
      int ty = constrain((int)(v * TEX_HEIGHT), 0, TEX_HEIGHT - 1);

      q.tex_index = ty * TEX_WIDTH + tx;

      quads.push_back(q);
    }
  }
}

// =================================================================
// 球体描画関数（メモリ効率化＆高速化版）
// =================================================================
void drawSphere() {
  canvas.fillSprite(TFT_BLACK);

  float sX = sin(rotationX);
  float cX = cos(rotationX);
  float sY = sin(rotationY);
  float cY = cos(rotationY);

  // 1. 頂点の3D回転と2D投影
  for (size_t i = 0; i < vertices.size(); ++i) {
    const Vec3& v = vertices[i];

    float y1 = v.y * cX - v.z * sX;
    float z1 = v.y * sX + v.z * cX;

    Vec3 rv;
    rv.x = v.x * cY - z1 * sY;
    rv.y = y1;
    rv.z = v.x * sY + z1 * cY + CAMERA_Z;

    // 直接配列を上書き
    rotated_vertices[i] = rv;

    if (rv.z <= 0.1f) {
      projected_vertices[i] = { -9999, -9999 };
    } else {
      float scale = FOV / rv.z;
      projected_vertices[i] = { rv.x * scale + CANVAS_WIDTH / 2,
                                rv.y * scale + CANVAS_HEIGHT / 2 };
    }
  }

  // 2. カリング（見えない面の除外）とZソートの準備
  int visible_quad_count = 0;
  for (size_t i = 0; i < quads.size(); ++i) {
    const Quad& q = quads[i];
    const Vec3& r0 = rotated_vertices[q.vertex_indices[0]];
    const Vec3& r1 = rotated_vertices[q.vertex_indices[1]];
    const Vec3& r2 = rotated_vertices[q.vertex_indices[2]];

    Vec3 edge1 = r1 - r0;
    Vec3 edge2 = r2 - r0;
    Vec3 normal = cross(edge1, edge2);
    if (dot(normal, r0 * -1.0f) < 0) continue;  // 裏を向いている面はここで終了

    // 見える面だけ中心Zを計算し、リストに追加
    Vec3 c = q.center;
    float y1 = c.y * cX - c.z * sX;
    float z1 = c.y * sX + c.z * cX;
    float center_z = c.x * sY + z1 * cY + CAMERA_Z;

    z_sorted_quads[visible_quad_count].z = center_z;
    z_sorted_quads[visible_quad_count].index = i;
    visible_quad_count++;
  }

  std::sort(z_sorted_quads.begin(), z_sorted_quads.begin() + visible_quad_count, [](const ZOrder& a, const ZOrder& b) {
    return a.z > b.z;
  });

  uint16_t* texBuffer = (uint16_t*)texCanvas.getBuffer();

  // 3. 描画処理
  for (int k = 0; k < visible_quad_count; ++k) {
    int i = z_sorted_quads[k].index;
    const Quad& q = quads[i];

    Vec2 p[4];
    bool visible = true;
    for (int j = 0; j < 4; ++j) {
      p[j] = projected_vertices[q.vertex_indices[j]];
      if (p[j].x == -9999) {
        visible = false;
        break;
      }
    }
    if (!visible) continue;

    uint16_t color = swap16(texBuffer[q.tex_index]);

    // 三角形描画
    canvas.fillTriangle(p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, color);
    canvas.fillTriangle(p[0].x, p[0].y, p[2].x, p[2].y, p[3].x, p[3].y, color);
  }
}

// =================================================================
// セットアップ
// =================================================================
void setup(void) {
  auto cfg = M5.config();
  M5.begin(cfg);
  //Serial.begin(115200);

  //canvas.setColorDepth(m5gfx::color_depth_t::grayscale_8bit);  // 8bit Grayscale 256 level.
  canvas.createSprite(CANVAS_WIDTH, CANVAS_HEIGHT);

  createTextureCanvas();
  createUVSphere();
  reserveMemory();
}

// =================================================================
// メインループ
// =================================================================
void loop(void) {
  M5.update();

  // FPSの計算
  uint32_t now = millis();
  uint32_t dt = now - lastTime;
  if (dt > 0) {
    fps = fps * 0.9f + (1000.0f / dt) * 0.1f;
  }
  lastTime = now;

  // 1. 回転の更新
  //rotationX += 0.015f;
  rotationY -= 0.03f;
  if (rotationY > TWO_PI) rotationY -= TWO_PI;
  if (rotationX > TWO_PI) rotationX -= TWO_PI;

  // 2. 描画関数の呼び出し
  drawSphere();

  // FPSの表示
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextDatum(top_right);
  canvas.setFont(&fonts::Font0);
  canvas.setCursor(CANVAS_WIDTH - 5, 5);
  canvas.printf("FPS: %.1f", fps);

  // ディスプレイにプッシュ
  canvas.pushSprite((M5.Display.width() - CANVAS_WIDTH) / 2, (M5.Display.height() - CANVAS_HEIGHT) / 2);

  yield();
}
