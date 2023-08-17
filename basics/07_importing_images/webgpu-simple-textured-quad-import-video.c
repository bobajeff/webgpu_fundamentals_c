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
#include "video.h"


// create a struct to hold the values for the uniforms
typedef struct Uniforms {
  mat4 matrix;
} Uniforms;

int numMipLevels(int *sizes, int sizes_length) {
  int maxSize = 0;
  int i;
  for (i = 0; i < sizes_length; i++) {
    maxSize = (maxSize > sizes[i]) ? maxSize : sizes[i];
  }
  return 1 + (int)log2(maxSize) | 0;
};

WGPUSampler _sampler = NULL;
WGPURenderPipeline _pipeline = NULL;
WGPUTextureFormat _texture_format = WGPUTextureFormat_RGBA8Unorm;

void init_mip_level_generator_pipeline(WGPUDevice device){
  WGPUShaderModuleDescriptor shaderSource =
      load_wgsl(RESOURCE_DIR "textured_quad_shaders_for_mip_level_generation.wgsl");
    shaderSource.label = "our hardcoded textured quad shaders";
  WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &shaderSource);

    _sampler = wgpuDeviceCreateSampler(device, &(WGPUSamplerDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .addressModeU = WGPUAddressMode_Repeat,
        .addressModeV = WGPUAddressMode_Repeat,
        .addressModeW = WGPUAddressMode_Repeat,
        .magFilter = WGPUFilterMode_Nearest,
        .minFilter = WGPUFilterMode_Linear,
        .mipmapFilter = WGPUMipmapFilterMode_Nearest,
        .lodMinClamp = 0.0,
        .lodMaxClamp = 0.0,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 0,
    });
    _pipeline = wgpuDeviceCreateRenderPipeline(
        device,
        &(WGPURenderPipelineDescriptor){
            .label = "mip level generator pipeline",
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
                            .format = _texture_format,
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
}

void generateMips(WGPUDevice device, WGPUTexture texture, int texture_width, int texture_height){

  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &(WGPUCommandEncoderDescriptor){.label = "mip gen encoder"});

  int width = texture_width;
  int height = texture_height;
  int baseMipLevel = 0;

  while (width > 1 || height > 1) {
    int tmp = width / 2 | 0;
    width = (1 > tmp) ? 1 : tmp;
    tmp = height / 2 | 0;
    height = (1 > tmp) ? 1 : tmp;

    WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(
        device,
        &(WGPUBindGroupDescriptor){
            .nextInChain = NULL,
            .label = NULL,
            .layout = wgpuRenderPipelineGetBindGroupLayout(_pipeline, 0),
            .entryCount = 2,
            .entries = (WGPUBindGroupEntry[2]){
                {.nextInChain = NULL,
                 .binding = 0,
                 .buffer = NULL,
                 .offset = 0,
                 .size = 0,
                 .sampler = _sampler,
                 .textureView = NULL},
                {.nextInChain = NULL,
                 .binding = 1,
                 .buffer = NULL,
                 .offset = 0,
                 .size = 0,
                 .sampler = NULL,
                 .textureView = wgpuTextureCreateView(
                     texture,
                     &(WGPUTextureViewDescriptor){
                         .nextInChain = NULL,
                         .label = NULL,
                         .format = WGPUTextureFormat_Undefined,
                         .dimension = WGPUTextureViewDimension_Undefined,
                         .baseMipLevel = baseMipLevel,
                         .mipLevelCount = 1, //** mystery_setting ** - needs
                                             //this value to work
                         .baseArrayLayer = 0,
                         .arrayLayerCount = 1, //** mystery_setting ** - needs
                                               //this value to work
                         .aspect = WGPUTextureAspect_All,
                     })},
            }});

    ++baseMipLevel;

    WGPURenderPassDescriptor renderPassDescriptor = {
        .label = "our basic canvas renderPass",
        .colorAttachments =
            &(WGPURenderPassColorAttachment){
                .view = wgpuTextureCreateView(
                    texture,
                    &(WGPUTextureViewDescriptor){
                        .nextInChain = NULL,
                        .label = NULL,
                        .format = WGPUTextureFormat_Undefined,
                        .dimension = WGPUTextureViewDimension_Undefined,
                        .baseMipLevel = baseMipLevel,
                        .mipLevelCount = 1, //** mystery_setting ** - needs this
                                            //value to work
                        .baseArrayLayer = 0,
                        .arrayLayerCount = 1, //** mystery_setting ** - needs
                                              //this value to work
                        .aspect = WGPUTextureAspect_All,
                    }),
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

    WGPURenderPassEncoder pass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescriptor);
    wgpuRenderPassEncoderSetPipeline(pass, _pipeline);
    wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, 0);
    wgpuRenderPassEncoderDraw(pass, 6, 1, 0, 0);
    wgpuRenderPassEncoderEnd(pass);
  }

  WGPUQueue queue = wgpuDeviceGetQueue(device);
  WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, &(WGPUCommandBufferDescriptor){.label = NULL});
  wgpuQueueSubmit(queue, 1, &commandBuffer);
};

void copySourceToTexture(WGPUDevice device, WGPUTexture texture, video_data * vdata){
  pthread_mutex_lock(&vdata->lock);
  uint32_t channels = 4u;
  uint32_t depthOrArrayLayers = 1;
  if (vdata->filt_frame && vdata->filt_frame->data[0]){

    void * textureData = vdata->filt_frame->data[0];
    
    int kTextureWidth = vdata->dec_ctx->width, kTextureHeight = vdata->dec_ctx->height;
    int textureDataSize = kTextureWidth * kTextureHeight * depthOrArrayLayers * channels * sizeof(uint8_t);
    int texture_mipLevelCount = numMipLevels((int[2]){kTextureWidth, kTextureHeight}, 2);
    
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    wgpuQueueWriteTexture(queue, &(WGPUImageCopyTexture){
        .texture = texture,
        .nextInChain = NULL,
        .aspect = WGPUTextureAspect_All,
        .mipLevel = 0,
        .origin = (WGPUOrigin3D){
            .x = 0,
            .y = 0,
            .z = 0
        }
    }, textureData, textureDataSize, &(WGPUTextureDataLayout){
        .bytesPerRow = kTextureWidth * 4,
        .nextInChain = NULL,
        .offset = 0,
        .rowsPerImage = kTextureHeight,
    }, &(WGPUExtent3D){
        .depthOrArrayLayers = 1, //** mystery_setting ** - needs this value to work
        .width = kTextureWidth,
        .height = kTextureHeight
    });

    if (texture_mipLevelCount > 1) {
        generateMips(device, texture, kTextureWidth, kTextureHeight);
    }
  }
  pthread_mutex_unlock(&vdata->lock);
}

WGPUTexture createTextureFromSource(WGPUDevice device, video_data * vdata) {
  pthread_mutex_lock(&vdata->lock);
  uint32_t channels = 4u;
  uint32_t depthOrArrayLayers = 1;

  int kTextureWidth = vdata->dec_ctx->width,
      kTextureHeight = vdata->dec_ctx->height;
  int textureDataSize = kTextureWidth * kTextureHeight * depthOrArrayLayers *
                        channels * sizeof(uint8_t);
  int texture_mipLevelCount =
      numMipLevels((int[2]){kTextureWidth, kTextureHeight}, 2);

  WGPUTexture texture = wgpuDeviceCreateTexture(
      device,
      &(WGPUTextureDescriptor){
          .nextInChain = NULL,
          .label = "yellow F on red",
          .size =
              (WGPUExtent3D){
                  .depthOrArrayLayers =
                      1, //** mystery_setting ** - needs this value to work
                  .width = kTextureWidth,
                  .height = kTextureHeight},
          .format = WGPUTextureFormat_RGBA8Unorm,
          .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst |
                   WGPUTextureUsage_RenderAttachment,
          .dimension = WGPUTextureDimension_2D,
          .mipLevelCount = texture_mipLevelCount,
          .sampleCount = 1,
          .viewFormatCount = 0,
          .viewFormats = NULL});

  pthread_mutex_unlock(&vdata->lock);

  return texture;
}

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
      load_wgsl(RESOURCE_DIR "webgpu-simple-textured-quad-import.wgsl");
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

  init_mip_level_generator_pipeline(device);

  video_data vdata = {
      .flipY = 1,
      .loop = 1,
      .src = RESOURCE_DIR "Golden_retriever_swimming_the_doggy_paddle.webm",
      .output_format = AV_PIX_FMT_RGBA};
  playvideo(&vdata);
  WGPUTexture texture = createTextureFromSource(device, &vdata);
  
  #define NUM_OF_TEXTURES 1

  WGPUBindGroup bindGroups[8][NUM_OF_TEXTURES];
  WGPUBuffer uniformBuffers[8];
  int i;  
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
    uniformBuffers[i] = wgpuDeviceCreateBuffer(
        device, &(WGPUBufferDescriptor){.label = "uniforms for quad",
                                        .size = uniformBufferSize,
                                        .usage = WGPUBufferUsage_Uniform |
                                                 WGPUBufferUsage_CopyDst,
                                        .mappedAtCreation = false});

    int j;
    for (j = 0; j < NUM_OF_TEXTURES; ++j) {
      bindGroups[i][j] = wgpuDeviceCreateBindGroup(device, &(WGPUBindGroupDescriptor){
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
                  .textureView = wgpuTextureCreateView(texture, NULL)
              },
              {
                  .nextInChain = NULL,
                  .binding = 2,
                  .buffer = uniformBuffers[i],
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
  WGPUQueue queue = wgpuDeviceGetQueue(device);


  int texNdx = 0;
  int count = 0;
  
  
  while (!glfwWindowShouldClose(window)) {
    copySourceToTexture(device, texture, &vdata);
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
      WGPUBindGroup bindGroup = bindGroups[i][texNdx];

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
      wgpuQueueWriteBuffer(queue, uniformBuffers[i], 0, &uniformValues, sizeof(uniformValues));


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