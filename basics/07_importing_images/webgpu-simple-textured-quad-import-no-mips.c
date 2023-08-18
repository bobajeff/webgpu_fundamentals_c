#include "create_surface.h"
#include "framework.h"
#include "webgpu.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
      load_wgsl(RESOURCE_DIR "shader.wgsl");
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

  stbi_set_flip_vertically_on_load(1);

  int kTextureWidth, kTextureHeight, raw_image_channels;
  unsigned char *textureData = stbi_load(RESOURCE_DIR "f-texture.png", &kTextureWidth, &kTextureHeight, &raw_image_channels, 4);
  int textureDataSize = kTextureWidth * kTextureHeight * 4;

  WGPUQueue queue = wgpuDeviceGetQueue(device);
  
  WGPUTexture texture = wgpuDeviceCreateTexture(device, &(WGPUTextureDescriptor){
    .nextInChain = NULL,
    .label = "yellow F on red",
    .size = (WGPUExtent3D){
        .depthOrArrayLayers = 1, //** mystery_setting ** - needs this value to work
        .width = kTextureWidth,
        .height = kTextureHeight
    },
    .format = WGPUTextureFormat_RGBA8Unorm,
    .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
    .dimension = WGPUTextureDimension_2D,
    .mipLevelCount = 1, //** mystery_setting ** - needs this value to work
    .sampleCount = 1,
    .viewFormatCount = 0,
    .viewFormats = NULL
  });
  
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

  stbi_image_free(textureData);

  WGPUBindGroup bindGroups[8];
  int i;
  for (i = 0; i < 8; ++i) {
    WGPUSampler sampler = wgpuDeviceCreateSampler(device, &(WGPUSamplerDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .addressModeU = (i & 1) ? WGPUAddressMode_Repeat : WGPUAddressMode_ClampToEdge,
        .addressModeV = (i & 2) ? WGPUAddressMode_Repeat : WGPUAddressMode_ClampToEdge,
        .addressModeW = WGPUAddressMode_Repeat,
        .magFilter = (i & 4) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest,
        .minFilter = WGPUFilterMode_Nearest,
        .mipmapFilter = WGPUMipmapFilterMode_Nearest,
        .lodMinClamp = 0.0,
        .lodMaxClamp = 0.0,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 1, //** mystery_setting ** - needs this value to work
    });

    bindGroups[i] = wgpuDeviceCreateBindGroup(device, &(WGPUBindGroupDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
        .entryCount = 2,
        .entries = (WGPUBindGroupEntry[2]){
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
            }
        }
    });

  }
  int ndx = 0;
  int count = 0;

  while (!glfwWindowShouldClose(window)) {
    // change setting after 250 loops (because making a GUI is not as easy outside of the web)
    if (count < 250) 
    {
        count++;
    }
    else {
        count = 0;
        if (ndx < 8 - 1){
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
        printf("magFilter: %s\n\n", magFilter);
    }


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