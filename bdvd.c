#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "raylib.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(*arr))

int main(void) {
  // Main variables
  int win_w = 400, win_h = 300;
  int dvd_w, dvd_h;
  Texture2D dvd_tex;
  Color dvd_colors[] = {WHITE, GRAY,  BROWN,   ORANGE, PINK,
                        RED,   GREEN, SKYBLUE, BLUE,   VIOLET};

  // Raylib setup
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
  Image dvd_img = LoadImageFromMemory(".png", dvd_png_img, dvd_png_img_sz);
  dvd_tex = LoadTextureFromImage(dvd_img);
  free(dvd_png_img);

  // Modify image color
  ImageColorInvert(&dvd_img);
  ImageColorTint(&dvd_img,
                 dvd_colors[GetRandomValue(0, LENGTH(dvd_colors) - 1)]);
  UpdateTexture(dvd_tex, LoadImageColors(dvd_img));

  while (!WindowShouldClose()) {
    win_w = GetScreenWidth();
    win_h = GetScreenHeight();

    BeginDrawing();
    ClearBackground(BLUE);
    DrawTexture(dvd_tex, win_w / 2, win_h / 2, WHITE);
    DrawFPS(5, 5);
    EndDrawing();
  }

  UnloadTexture(dvd_tex);
  UnloadImage(dvd_img);
  CloseWindow();
  return 0;
}
