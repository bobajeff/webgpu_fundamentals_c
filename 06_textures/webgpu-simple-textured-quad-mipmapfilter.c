#include "create_surface.h"
#include "framework.h"
#include "webgpu.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <cglm/cglm.h>

// create a struct to hold the values for the uniforms
typedef struct Uniforms {
  mat4 matrix;
} Uniforms;

char lerp(char a, char b, int t) { return a + (b - a) * t; };
void mix(char *a, char *b, int t, char *return_array) {
  int i;
  for (i = 0; i < 4; ++i) {
    return_array[i] = lerp(a[i], b[i], t);
  }
};

void bilinearFilter(char *tl, char *tr, char *bl, char *br, int t1, int t2,
                    char *return_array) {
  char t[4];
  char b[4];
  mix(tl, tr, t1, t);
  mix(bl, br, t1, b);
  mix(t, b, t2, return_array);
};

typedef struct mip_data {
  char *data;
  int data_size;
  int width;
  int height;
} mip_data;

typedef struct mips_array_data {
  mip_data *mips;
  int mips_size;
  int mips_length;
} mips_array_data;

mip_data createNextMipLevelRgba8Unorm(mip_data *mip) {
  char *src = mip->data;
  int srcWidth = mip->width;
  int srcHeight = mip->height;
  // compute the size of the next mip
  int tmp = srcWidth / 2 | 0;
  int dstWidth = (1 > tmp) ? 1 : tmp;
  tmp = srcHeight / 2 | 0;
  int dstHeight = (1 > tmp) ? 1 : tmp;
  int data_size = dstWidth * dstHeight * 4 * sizeof(char);
  char *dst = (char *)malloc(data_size);

  int y, x;
  for (y = 0; y < dstHeight; ++y) {
    for (x = 0; x < dstWidth; ++x) {
      // compute texcoord of the center of the destination texel
      int u = (x + 0.5) / dstWidth;
      int v = (y + 0.5) / dstHeight;

      // compute the same texcoord in the source - 0.5 a pixel
      int au = (u * srcWidth - 0.5);
      int av = (v * srcHeight - 0.5);

      // compute the src top left texel coord (not texcoord)
      int tx = au | 0;
      int ty = av | 0;

      // compute the mix amounts between pixels
      int t1 = au % 1;
      int t2 = av % 1;

      // get the 4 pixels
      char *tl = &src[(ty * srcWidth * tx) * 4];
      char *tr = &src[(ty * srcWidth * (tx + 1)) * 4];
      char *bl = &src[((ty + 1) * srcWidth * tx) * 4];
      char *br = &src[((ty + 1) * srcWidth * (tx + 1)) * 4];

      // copy the "sampled" result into the dest.
      char dstOffset = (y * dstWidth + x) * 4;
      // char filtered_things[4];
      bilinearFilter(tl, tr, bl, br, t1, t2, &dst[dstOffset]);
    }
  }
  return (mip_data){.data = dst,
                    .width = dstWidth,
                    .height = dstHeight,
                    .data_size = data_size};
};

mips_array_data generateMips(char *src, int srcWidth, int srcHeight,
                             int src_size) {
  int mips_length = 1; // start with a length of 1 since we'll use the
                       // srcWidth/srcHeight in base level
  int tmp_width = srcWidth;
  int tmp_height = srcHeight;
  while (tmp_width > 1 || tmp_height > 1) {
    int tmp = tmp_width / 2 | 0;
    tmp_width = (1 > tmp) ? 1 : tmp;
    tmp = tmp_height / 2 | 0;
    tmp_height = (1 > tmp) ? 1 : tmp;
    mips_length++;
  }
  int mips_size = mips_length * sizeof(mip_data);
  mip_data *mips = (mip_data *)malloc(mips_size);

  // populate with first mip level (base level)
  int index = 0;
  mip_data mip = {.data = src,
                  .width = srcWidth,
                  .height = srcHeight,
                  .data_size = src_size};

  mips[index].data = mip.data;
  mips[index].width = mip.width;
  mips[index].height = mip.height;
  mips[index].data_size = mip.data_size;
  index++;

  while (index < mips_length) {
    mip = createNextMipLevelRgba8Unorm(&mip);
    mips[index].data = mip.data;
    mips[index].width = mip.width;
    mips[index].height = mip.height;
    mips[index].data_size = mip.data_size;
    index++;
  }

  return (mips_array_data){
      .mips = mips, .mips_size = mips_size, .mips_length = mips_length};
};

void fill_rect(char * data, char * color, int x, int y, int width, int height, int total_width){
      int x_start = x;
      int y_start = y;
      int x_end = x + width;
      int y_end = y + height;
      int span = 4;
      int pixel_index;

  for (y = y_start; y < y_end; ++y) {
      for (x = x_start; x < x_end; ++x) {
            pixel_index = (x + (y * total_width)) * span;
            
            data[pixel_index] = color[0]; //r
            data[pixel_index + 1] = color[1]; //g
            data[pixel_index + 2] = color[2]; //b
            data[pixel_index + 3] = (char)255; //a
      }
    }
};

typedef struct special_struct{
  WGPUBindGroup bindGroups[2];
  WGPUBuffer uniformBuffer;
} special_struct;

int main(int argc, char *argv[]) {
  initializeLog();

  WGPUInstance instance =
      wgpuCreateInstance(&(WGPUInstanceDescriptor){.nextInChain = NULL});

  WGPUAdapter adapter;
  wgpuInstanceRequestAdapter(instance, NULL, request_adapter_callback,
                             (void *)&adapter);

  WGPUDevice device;
  wgpuAdapterRequestDevice(adapter, NULL, request_device_callback,
                           (void *)&device);

  wgpuDeviceSetUncapturedErrorCallback(device, handle_uncaptured_error, NULL);
  wgpuDeviceSetDeviceLostCallback(device, handle_device_lost, NULL);

  // Create GLFW Window and use as WebGPU surface
  if (!glfwInit()) {
    printf("Cannot initialize glfw");
    return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(640, 480, "wgpu with glfw", NULL, NULL);


  if (!window) {
    printf("Cannot create window");
    return 1;
  }
  WGPUSurface surface = create_surface(instance, window);
  WGPUTextureFormat presentationFormat =
      wgpuSurfaceGetPreferredFormat(surface, adapter);

  WGPUShaderModuleDescriptor shaderSource =
      load_wgsl(RESOURCE_DIR "/06_textures/webgpu-simple-textured-quad-mipmapfilter.wgsl");
    shaderSource.label = "our hardcoded textured quad shaders";
  WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &shaderSource);

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(
      device,
      &(WGPURenderPipelineDescriptor){
          .label = "hardcoded textured quad pipeline",
          .vertex =
              (WGPUVertexState){
                  .module = module,
                  .entryPoint = "vs",
                  .bufferCount = 0,
                  .buffers = NULL,
              },
          .primitive =
              (WGPUPrimitiveState){
                  .topology = WGPUPrimitiveTopology_TriangleList,
                  .stripIndexFormat = WGPUIndexFormat_Undefined,
                  .frontFace = WGPUFrontFace_CCW,
                  .cullMode = WGPUCullMode_None},
          .multisample =
              (WGPUMultisampleState){
                  .count = 1,
                  .mask = ~0,
                  .alphaToCoverageEnabled = false,
              },
          .fragment =
              &(WGPUFragmentState){
                  .module = module,
                  .entryPoint = "fs",
                  .targetCount = 1,
                  .targets =
                      &(WGPUColorTargetState){
                          .format = presentationFormat,
                          .blend =
                              &(WGPUBlendState){
                                  .color =
                                      (WGPUBlendComponent){
                                          .srcFactor = WGPUBlendFactor_One,
                                          .dstFactor = WGPUBlendFactor_Zero,
                                          .operation = WGPUBlendOperation_Add,
                                      },
                                  .alpha =
                                      (WGPUBlendComponent){
                                          .srcFactor = WGPUBlendFactor_One,
                                          .dstFactor = WGPUBlendFactor_Zero,
                                          .operation = WGPUBlendOperation_Add,
                                      }},
                          .writeMask = WGPUColorWriteMask_All,
                      },
              },
          .depthStencil = NULL,
      });

  WGPUSwapChainDescriptor config = (WGPUSwapChainDescriptor){
      .nextInChain =
          (const WGPUChainedStruct *)&(WGPUSwapChainDescriptorExtras){
              .chain =
                  (WGPUChainedStruct){
                      .next = NULL,
                      .sType = (WGPUSType)WGPUSType_SwapChainDescriptorExtras,
                  },
              .alphaMode = WGPUCompositeAlphaMode_Auto,
              .viewFormatCount = 0,
              .viewFormats = NULL,
          },
      .usage = WGPUTextureUsage_RenderAttachment,
      .format = presentationFormat,
      .width = 0,
      .height = 0,
      .presentMode = WGPUPresentMode_Fifo,
  };

  glfwGetWindowSize(window, (int *)&config.width, (int *)&config.height);

  WGPUSwapChain swapChain = wgpuDeviceCreateSwapChain(device, surface, &config);

  #define NUM_OF_TEXTURES 2
  mips_array_data mips_arrays[NUM_OF_TEXTURES];

  // createBlendedMipmap()
  char w[4] = {255, 255, 255, 255};
  char r[4] = {255,   0,   0, 255};
  char b[4] = {  0,  28, 116, 255};
  char y[4] = {255, 231,   0, 255};
  char g[4] = { 58, 181,  75, 255};
  char a[4] = { 38, 123, 167, 255};
  char _data[16 * 16 * 4] = {
        w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], a[0],a[1],a[2],a[3], a[0],a[1],a[2],a[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], a[0],a[1],a[2],a[3], a[0],a[1],a[2],a[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], a[0],a[1],a[2],a[3], a[0],a[1],a[2],a[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], a[0],a[1],a[2],a[3], a[0],a[1],a[2],a[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], a[0],a[1],a[2],a[3], a[0],a[1],a[2],a[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], a[0],a[1],a[2],a[3], a[0],a[1],a[2],a[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], a[0],a[1],a[2],a[3], a[0],a[1],a[2],a[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], a[0],a[1],a[2],a[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3],
        b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], b[0],b[1],b[2],b[3], g[0],g[1],g[2],g[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], g[0],g[1],g[2],g[3], g[0],g[1],g[2],g[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], g[0],g[1],g[2],g[3], g[0],g[1],g[2],g[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], g[0],g[1],g[2],g[3], g[0],g[1],g[2],g[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], g[0],g[1],g[2],g[3], g[0],g[1],g[2],g[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], g[0],g[1],g[2],g[3], g[0],g[1],g[2],g[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], g[0],g[1],g[2],g[3], g[0],g[1],g[2],g[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3], w[0],w[1],w[2],w[3],
        w[0],w[1],w[2],w[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], g[0],g[1],g[2],g[3], g[0],g[1],g[2],g[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], r[0],r[1],r[2],r[3], w[0],w[1],w[2],w[3],
  };
  mips_arrays[0] = generateMips(_data, 16, 16, sizeof(_data));

  // createCheckedMipmap()
  #define LEVELS 7
  int mips_length = LEVELS;
  int mips_size = mips_length * sizeof(mip_data);
  mip_data *mips = (mip_data *)malloc(mips_size);

  int sizes[LEVELS] = {
        64, 32, 16, 8, 4, 2, 1
  };
  char colors[LEVELS][3] = {
        {128,0,255},
        {0,255,0},
        {255,0,0},
        {255,255,0},
        {0,0,255},
        {0,255,255},
        {255,0,255},
  };
  char white[3] = {255, 255, 255};
  char black[3] = {0, 0, 0};
  int i;
  for (i = 0; i < LEVELS; i++){
        int size = sizes[i];
        char * color = colors[i];
        int data_size = size * size * 4 * sizeof(char);
        char *data = (char *)malloc(data_size);
        char * bg = (i & 1) ? black : white;
        fill_rect(data, bg, 0, 0, size, size, size);
        fill_rect(data, color, 0, 0, size / 2, size / 2, size);
        fill_rect(data, color, size / 2, size / 2, size / 2, size / 2, size);     
        mips[i].data = data;
        mips[i].data_size = data_size;
        mips[i].height = size;
        mips[i].width = size;
  };
  mips_arrays[1] = (mips_array_data){.mips = mips, .mips_size = mips_size, .mips_length = mips_length};

  char * labels[NUM_OF_TEXTURES] =  {"blended","checker"};

  // createTextureWithMips()
  WGPUTexture textures[2];
  WGPUQueue queue = wgpuDeviceGetQueue(device);
  for (i = 0; i < NUM_OF_TEXTURES; ++i) {
    mip_data *mips = mips_arrays[i].mips;
    textures[i] = wgpuDeviceCreateTexture(
        device,
        &(WGPUTextureDescriptor){
            .nextInChain = NULL,
            .label = labels[i],
            .size =
                (WGPUExtent3D){
                    .depthOrArrayLayers =
                        1, //** mystery_setting ** - needs this value to work
                    .width = mips[0].width,
                    .height = mips[0].height},
            .format = WGPUTextureFormat_RGBA8Unorm,
            .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
            .dimension = WGPUTextureDimension_2D,
            .mipLevelCount = mips_arrays[i].mips_length,
            .sampleCount = 1,
            .viewFormatCount = 0,
            .viewFormats = NULL});

    int j;
    for (j = 0; j < mips_arrays[i].mips_length; ++j) {
      int datasize = mips[j].data_size;
      char *data = mips[j].data;
      int width = mips[j].width;
      int height = mips[j].height;
      wgpuQueueWriteTexture(
          queue,
          &(WGPUImageCopyTexture){.texture = textures[i],
                                  .nextInChain = NULL,
                                  .aspect = WGPUTextureAspect_All,
                                  .mipLevel = j,
                                  .origin =
                                      (WGPUOrigin3D){.x = 0, .y = 0, .z = 0}},
          data, datasize,
          &(WGPUTextureDataLayout){
              .bytesPerRow = width * 4,
              .nextInChain = NULL,
              .offset = 0,
              .rowsPerImage = height,
          },
          &(WGPUExtent3D){
              .depthOrArrayLayers =
                  1, //** mystery_setting ** - needs this value to work
              .width = width,
              .height = height});
      // free all the malloced data (can't free mips[0].data because it points
      // textureData which isn't malloced)
      if (j > 0) {
        // free(data);
      }
    }
  // free(mips);
  }

  special_struct objectInfos[8];
  
  for (i = 0; i < 8; ++i) {
    WGPUSampler sampler = wgpuDeviceCreateSampler(device, &(WGPUSamplerDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .addressModeU = WGPUAddressMode_Repeat,
        .addressModeV = WGPUAddressMode_Repeat,
        .addressModeW = WGPUAddressMode_Repeat,
        .magFilter = (i & 1) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest,
        .minFilter = (i & 2) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest,
        .mipmapFilter = (i & 4) ? WGPUMipmapFilterMode_Linear : WGPUMipmapFilterMode_Nearest,
        .lodMinClamp = 0.0,
        .lodMaxClamp = 0.0,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 0,
    });

    // create a buffer for the uniform values
    const uint64_t uniformBufferSize = 16 * 4; // matrix is 16 32bit floats (4bytes each)
    objectInfos[i].uniformBuffer = wgpuDeviceCreateBuffer(
        device, &(WGPUBufferDescriptor){.label = "uniforms for quad",
                                        .size = uniformBufferSize,
                                        .usage = WGPUBufferUsage_Uniform |
                                                 WGPUBufferUsage_CopyDst,
                                        .mappedAtCreation = false});

    int j;
    for (j = 0; j < NUM_OF_TEXTURES; ++j) {
      objectInfos[i].bindGroups[j] = wgpuDeviceCreateBindGroup(device, &(WGPUBindGroupDescriptor){
          .nextInChain = NULL,
          .label = NULL,
          .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
          .entryCount = 3,
          .entries = (WGPUBindGroupEntry[3]){
              {
                  .nextInChain = NULL,
                  .binding = 0,
                  .buffer = NULL,
                  .offset = 0,
                  .size = 0,
                  .sampler = sampler,
                  .textureView = NULL
              },
              {
                  .nextInChain = NULL,
                  .binding = 1,
                  .buffer = NULL,
                  .offset = 0,
                  .size = 0,
                  .sampler = NULL,
                  .textureView = wgpuTextureCreateView(textures[j], NULL)
              },
              {
                  .nextInChain = NULL,
                  .binding = 2,
                  .buffer = objectInfos[i].uniformBuffer,
                  .offset = 0,
                  .size = uniformBufferSize,
                  .sampler = NULL,
                  .textureView = NULL
              },
          }
      });
    }

  }


  
  WGPUSupportedLimits limits;
  bool gotlimits = wgpuDeviceGetLimits(device, &limits);


  int texNdx = 0;
  int count = 0;
  
  
  while (!glfwWindowShouldClose(window)) {
    // change texture after 250 loops (to avoiding implementing click handling)
    if (count < 250) 
    {
        count++;
    }
    else {
        count = 0;
        texNdx = (texNdx + 1) % NUM_OF_TEXTURES;
    }
    
    
    // compute a scale that will draw our 0 to 1 clip space quad
    // 2x2 pixels in the canvas.


    int window_width, window_height;

    glfwGetWindowSize(window, (int *)&window_width, (int *)&window_height);

    window_width = (window_width < limits.limits.maxTextureDimension2D)
                       ? window_width
                       : limits.limits.maxTextureDimension2D;
    window_height = (window_height < limits.limits.maxTextureDimension2D)
                        ? window_height
                        : limits.limits.maxTextureDimension2D;

    window_width = (window_width > 1) ? window_width : 1;
    window_height = (window_height > 1) ? window_height : 1;

    float fov = 60 * M_PI / 180;  // 60 degrees in radians
    int aspect = window_width / window_height;
    float zNear  = 1;
    float zFar   = 2000;
    mat4 projectionMatrix;
    mat4 cameraMatrix;
    mat4 viewMatrix;
    mat4 viewProjectionMatrix;
    glm_perspective(fov, aspect, zNear, zFar, projectionMatrix);

    vec3 cameraPosition = {0, 0, 2};
    vec3 up = {0, 1, 0};
    vec3 target = {0, 0, 0};
    glm_lookat(cameraPosition, target, up, cameraMatrix);
    glm_mat4_inv(cameraMatrix, viewMatrix);
    glm_mat4_mul(projectionMatrix, viewMatrix, viewProjectionMatrix);

    WGPUTextureView view = NULL;

    for (int attempt = 0; attempt < 2; attempt++) {
      uint32_t prevWidth = config.width;
      uint32_t prevHeight = config.height;
      glfwGetWindowSize(window, (int *)&config.width, (int *)&config.height);

      if (prevWidth != config.width || prevHeight != config.height) {
        swapChain = wgpuDeviceCreateSwapChain(device, surface, &config);
      }
      // Get the current texture from the swapChain to use for rendering to by the render pass
      view = wgpuSwapChainGetCurrentTextureView(swapChain);
      if (attempt == 0 && !view) {
        printf("wgpuSwapChainGetCurrentTextureView() failed; trying to create "
               "a new swap chain...\n");
        config.width = 0;
        config.height = 0;
        continue;
      }

      break;
    }

    if (!view) {
      printf("Cannot acquire next swap chain texture\n");
      return 1;
    }

    // make a command encoder to start encoding commands
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(
        device, &(WGPUCommandEncoderDescriptor){.label = "render quad encoder"});

    WGPURenderPassDescriptor renderPassDescriptor = {
        .colorAttachments =
            &(WGPURenderPassColorAttachment){
                .view = view, //texture from SwapChain
                .resolveTarget = NULL,
                .loadOp = WGPULoadOp_Clear,
                .storeOp = WGPUStoreOp_Store,
                .clearValue =
                    (WGPUColor){
                        .r = 0.3,
                        .g = 0.3,
                        .b = 0.3,
                        .a = 1.0,
                    },
            },
        .colorAttachmentCount = 1,
        .depthStencilAttachment = NULL,
    };

    // make a render pass encoder to encode render specific commands
    WGPURenderPassEncoder pass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescriptor);
    wgpuRenderPassEncoderSetPipeline(pass, pipeline);
    for (i = 0; i < 8; ++i) {
      WGPUBindGroup bindGroup = objectInfos[i].bindGroups[texNdx];

      float xSpacing = 1.2;
      float ySpacing = 0.7;
      float zDepth = 50;

      float x = i % 4 - 1.5;
      float y = i < 4 ? 1 : -1;

      Uniforms uniformValues;
      glm_translate_to(viewProjectionMatrix, (float[3]){x * xSpacing, y * ySpacing, -zDepth * 0.5}, uniformValues.matrix);
      glm_rotate_x(uniformValues.matrix, 0.5 * M_PI, uniformValues.matrix);
      glm_scale_to(uniformValues.matrix, (float[3]){1, zDepth * 2, 1}, uniformValues.matrix);
      glm_translate_to(uniformValues.matrix, (float[3]){-0.5, -0.5, 0}, uniformValues.matrix);
      
      // copy the values from CPU to the GPU
      wgpuQueueWriteBuffer(queue, objectInfos[i].uniformBuffer, 0, &uniformValues, sizeof(uniformValues));


      wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, 0);
      wgpuRenderPassEncoderDraw(pass, 6, 1, 0,
                                0); // call our vertex shader 6 times.
    }
    wgpuRenderPassEncoderEnd(pass);

    wgpuTextureViewDrop(view);

    WGPUQueue queue = wgpuDeviceGetQueue(device);
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(
        encoder, &(WGPUCommandBufferDescriptor){.label = NULL});
    wgpuQueueSubmit(queue, 1, &commandBuffer);
    wgpuSwapChainPresent(swapChain);

    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}