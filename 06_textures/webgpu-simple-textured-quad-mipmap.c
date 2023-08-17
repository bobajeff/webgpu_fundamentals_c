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

// create a struct to hold the values for the uniforms
typedef struct Uniforms {
  float scale[2];
	float offset[2];
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
      load_wgsl(RESOURCE_DIR "webgpu-simple-textured-quad-mipmap.wgsl");
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

  #define kTextureWidth 5
  #define kTextureHeight 7
  char _[4] = {255,   0,   0, 255};  // red
  char y[4] = {255, 255,   0, 255};  // yellow
  char b[4] = {  0,   0, 255, 255};  // blue
  char textureData[4 * kTextureWidth * kTextureHeight] = {
    _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3],
    _[0],_[1],_[2],_[3], y[0],y[1],y[2],y[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3],
    _[0],_[1],_[2],_[3], y[0],y[1],y[2],y[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3],
    _[0],_[1],_[2],_[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3],
    _[0],_[1],_[2],_[3], y[0],y[1],y[2],y[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3],
    _[0],_[1],_[2],_[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], y[0],y[1],y[2],y[3], _[0],_[1],_[2],_[3],
    b[0],b[1],b[2],b[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3], _[0],_[1],_[2],_[3],
  };

  mips_array_data generated_mips_data = generateMips(textureData, kTextureWidth, kTextureHeight, 4 * kTextureWidth * kTextureHeight * sizeof(char));
  mip_data * mips = generated_mips_data.mips;

  WGPUTexture texture = wgpuDeviceCreateTexture(device, &(WGPUTextureDescriptor){
    .nextInChain = NULL,
    .label = "yellow F on red",
    .size = (WGPUExtent3D){
        .depthOrArrayLayers = 1, //** mystery_setting ** - needs this value to work
        .width = mips[0].width,
        .height = mips[0].height
    },
    .format = WGPUTextureFormat_RGBA8Unorm,
    .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
    .dimension = WGPUTextureDimension_2D,
    .mipLevelCount = generated_mips_data.mips_length,
    .sampleCount = 1,
    .viewFormatCount = 0,
    .viewFormats = NULL
  });
  WGPUQueue queue = wgpuDeviceGetQueue(device);
  int i;
  for (i = 0; i < generated_mips_data.mips_length; ++i) {
    int datasize = mips[i].data_size;
    char * data = mips[i].data;
    int width = mips[i].width;
    int height = mips[i].height;
    wgpuQueueWriteTexture(queue, &(WGPUImageCopyTexture){
      .texture = texture,
      .nextInChain = NULL,
      .aspect = WGPUTextureAspect_All,
      .mipLevel = i,
      .origin = (WGPUOrigin3D){
          .x = 0,
          .y = 0,
          .z = 0
      }
    }, data, datasize, &(WGPUTextureDataLayout){
      .bytesPerRow = width * 4,
      .nextInChain = NULL,
      .offset = 0,
      .rowsPerImage = kTextureHeight,
    }, &(WGPUExtent3D){
      .depthOrArrayLayers = 1, //** mystery_setting ** - needs this value to work
      .width = width,
      .height = height
    });
    // free all the malloced data (can't free mips[0].data because it points textureData which isn't malloced)
    if (i > 0){
      free(data);
    }

  }
  free(mips);

    // create a buffer for the uniform values
  const uint64_t uniformBufferSize =
    2 * 4 + // scale is 2 32bit floats (4bytes each)
    2 * 4;  // offset is 2 32bit floats (4bytes each)
  const WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
        .label = "uniforms for quad",
        .size = uniformBufferSize,
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .mappedAtCreation = false
        }
    );

  // make instance of Uniforms with the values for the uniforms
  Uniforms uniformValues;

  WGPUBindGroup bindGroups[16];
  
  for (i = 0; i < 16; ++i) {
    WGPUSampler sampler = wgpuDeviceCreateSampler(device, &(WGPUSamplerDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .addressModeU = (i & 1) ? WGPUAddressMode_Repeat : WGPUAddressMode_ClampToEdge,
        .addressModeV = (i & 2) ? WGPUAddressMode_Repeat : WGPUAddressMode_ClampToEdge,
        .addressModeW = WGPUAddressMode_Repeat,
        .magFilter = (i & 4) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest,
        .minFilter = (i & 8) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest,
        .mipmapFilter = WGPUMipmapFilterMode_Nearest,
        .lodMinClamp = 0.0,
        .lodMaxClamp = 0.0,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 0,
    });

    bindGroups[i] = wgpuDeviceCreateBindGroup(device, &(WGPUBindGroupDescriptor){
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
                .textureView = wgpuTextureCreateView(texture, &(WGPUTextureViewDescriptor){
                    .nextInChain = NULL,
                    .label = NULL,
                    .format = WGPUTextureFormat_Undefined,
                    .dimension = WGPUTextureViewDimension_Undefined,
                    .baseMipLevel = 0,
                    .mipLevelCount = 1, //** mystery_setting ** - needs this value to work
                    .baseArrayLayer = 0,
                    .arrayLayerCount = 1, //** mystery_setting ** - needs this value to work
                    .aspect = WGPUTextureAspect_All,
                })
            },
            {
                .nextInChain = NULL,
                .binding = 2,
                .buffer = uniformBuffer,
                .offset = 0,
                .size = uniformBufferSize,
                .sampler = NULL,
                .textureView = NULL
            },
        }
    });

  }
  int ndx = 0;
  int count = 0;

  struct timeval start, stop;
  WGPUSupportedLimits limits;
  bool gotlimits = wgpuDeviceGetLimits(device, &limits);
  gettimeofday(&start, NULL);
  gettimeofday(&stop, NULL);

  while (!glfwWindowShouldClose(window)) {
    
    
    float time =  (stop.tv_sec - start.tv_sec)*1000 + (stop.tv_usec - start.tv_usec)/1000.0;
    time *= 0.001;
    // change setting after 250 loops (because making a GUI is not as easy outside of the web)
    if (count < 250) 
    {
        count++;
    }
    else {
        count = 0;
        if (ndx < 16 - 1){
            ndx++;
        }
        else {
            ndx = 0;
        }
        const char * addressModeU = (ndx & 1) ? "WGPUAddressMode_Repeat" : "WGPUAddressMode_ClampToEdge";
        printf("addressModeU: %s\n", addressModeU);
        const char * addressModeV = (ndx & 2) ? "WGPUAddressMode_Repeat" : "WGPUAddressMode_ClampToEdge";
        printf("addressModeV: %s\n", addressModeV);
        const char * magFilter = (ndx & 4) ? "WGPUFilterMode_Linear" : "WGPUFilterMode_Nearest";
        printf("magFilter: %s\n", magFilter);
        const char * minFilter = (ndx & 8) ? "WGPUFilterMode_Linear" : "WGPUFilterMode_Nearest";
        printf("minFilter: %s\n\n", minFilter);
    }
    
    // compute a scale that will draw our 0 to 1 clip space quad
    // 2x2 pixels in the canvas.


    int window_width, window_height;

    glfwGetWindowSize(window, (int *)&window_width, (int *)&window_height);

    window_width = window_width / 64 | 0;
    window_height = window_height / 64 | 0;

    window_width = (window_width < limits.limits.maxTextureDimension2D)
                       ? window_width
                       : limits.limits.maxTextureDimension2D;
    window_height = (window_height < limits.limits.maxTextureDimension2D)
                        ? window_height
                        : limits.limits.maxTextureDimension2D;

    window_width = (window_width > 1) ? window_width : 1;
    window_height = (window_height > 1) ? window_height : 1;

    float scale_setting = 0.5; // set arbitrary scale (avoiding making a GUI)

    float scaleX = 4.0 / window_width * scale_setting;
    float scaleY = 4.0 / window_height * scale_setting;

    uniformValues.scale[0] = scaleX;
    uniformValues.scale[1] = scaleY;

    uniformValues.offset[0] = sinf(time * 0.25) * 0.9;
    uniformValues.offset[1] = -0.8;

    // copy the values from CPU to the GPU
    wgpuQueueWriteBuffer(queue, uniformBuffer, 0, &uniformValues, sizeof(uniformValues));

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
    wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroups[ndx], 0, 0);
    wgpuRenderPassEncoderDraw(pass, 6, 1, 0,
                              0); // call our vertex shader 6 times.
    wgpuRenderPassEncoderEnd(pass);
    wgpuTextureViewDrop(view);

    WGPUQueue queue = wgpuDeviceGetQueue(device);
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(
        encoder, &(WGPUCommandBufferDescriptor){.label = NULL});
    wgpuQueueSubmit(queue, 1, &commandBuffer);
    wgpuSwapChainPresent(swapChain);

    glfwPollEvents();
    gettimeofday(&stop, NULL);
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}