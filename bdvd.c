#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "raylib.h"
#include "raymath.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(*arr))

int main(void) {
  // Main variables
  int win_w = 640, win_h = 480,
      color_idx = 0; // color_idx is based on dvd_colors[] indexes
  double delta;
  bool dvd_collided;
  float dvd_w, dvd_h;
  Vector2 dvd_pos, dvd_vel;
  Image dvd_img;
  Texture2D dvd_tex;
  Color dvd_colors[] = {WHITE, GRAY,  BROWN,   ORANGE, PINK,
                        RED,   GREEN, SKYBLUE, BLUE,   VIOLET};

  // Setup raylib
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(win_w, win_h, "Bouncing DVD");

  // Load SVG image
  const char *path = "dvd.svg";
  NSVGimage *img = nsvgParseFromFile(path, "px", 96);
  if (!img)
    TraceLog(LOG_FATAL, "nsvgParseFromFile() failed to parse '%s'", path);
  dvd_w = img->width;
  dvd_h = img->height;

  // Rasterize SVG
  size_t dvd_img_comp = 4;
  size_t dvd_img_stride = dvd_w * dvd_img_comp;
  size_t dvd_raw_img_sz = dvd_w * dvd_h * dvd_img_comp;
  unsigned char *dvd_raw_img;
  NSVGrasterizer *rast = nsvgCreateRasterizer();
  if (!rast) {
    nsvgDelete(img);
    TraceLog(LOG_FATAL, "nsvgCreateRasterizer() failed create rasterizer");
  }
  dvd_raw_img = malloc(dvd_raw_img_sz);
  if (!dvd_raw_img) {
    nsvgDelete(img);
    TraceLog(LOG_FATAL, "malloc() failed to allocate image buffer");
  }
  nsvgRasterize(rast, img, 0, 0, 1, dvd_raw_img, dvd_w, dvd_h, dvd_img_stride);
  nsvgDeleteRasterizer(rast);
  nsvgDelete(img);

  // Convert SVG to PNG
  int dvd_png_img_sz;
  unsigned char *dvd_png_img = stbi_write_png_to_mem(
      dvd_raw_img, dvd_img_stride, dvd_w, dvd_h, dvd_img_comp, &dvd_png_img_sz);
  free(dvd_raw_img);
  if (!dvd_png_img)
    TraceLog(LOG_FATAL, "stbi_write_png_to_mem() failed to write png");

  // Load PNG to raylib
  Image dvd_img_orig = LoadImageFromMemory(".png", dvd_png_img, dvd_png_img_sz);
  ImageColorInvert(&dvd_img_orig);
  dvd_img = ImageCopy(dvd_img_orig);
  dvd_tex = LoadTextureFromImage(dvd_img);
  free(dvd_png_img);

  // Set DVD logo starting position and velocity
  dvd_pos = (Vector2){dvd_w, win_h - dvd_h * 1.5f};
  dvd_vel = (Vector2){160.f, -160.f};

  while (!WindowShouldClose()) {
    win_w = GetScreenWidth();
    win_h = GetScreenHeight();
    delta = GetFrameTime();
    dvd_collided = false;

    /// TODO: Add a little bit of randomness to the velocity

    // Move DVD logo
    dvd_pos = Vector2Add(dvd_pos, Vector2Scale(dvd_vel, delta));

    // Handle vertical boundary collision
    if (dvd_pos.x + dvd_w > win_w) {
      dvd_pos.x = win_w - dvd_w + 1;
      dvd_vel.x *= -1;
      dvd_collided = true;
    } else if (dvd_pos.x < 0) {
      dvd_pos.x = 0;
      dvd_vel.x *= -1;
      dvd_collided = true;
    }

    // Handle horizontal boundary collision
    if (dvd_pos.y + dvd_h > win_h) {
      dvd_pos.y = win_h - dvd_h + 1;
      dvd_vel.y *= -1;
      dvd_collided = true;
    } else if (dvd_pos.y < 0) {
      dvd_pos.y = 0;
      dvd_vel.y *= -1;
      dvd_collided = true;
    }

    // Modify image color if DVD collided
    if (dvd_collided) {
      int random_color_idx;
      do {
        random_color_idx = GetRandomValue(0, LENGTH(dvd_colors) - 1);
      } while (random_color_idx == color_idx);
      color_idx = random_color_idx;

      UnloadImage(dvd_img);
      dvd_img = ImageCopy(dvd_img_orig);
      ImageColorTint(&dvd_img, dvd_colors[color_idx]);
      UpdateTexture(dvd_tex, LoadImageColors(dvd_img));
    }

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTextureV(dvd_tex, dvd_pos, WHITE);
    DrawFPS(5, 5);
    EndDrawing();
  }

  UnloadTexture(dvd_tex);
  UnloadImage(dvd_img_orig);
  CloseWindow();
  return 0;
}
