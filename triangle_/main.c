#include "framework.h"
#include "create_surface.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "webgpu.h"

int main(int argc, char *argv[]) {
  initializeLog();

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

  WGPUInstance instance = wgpuCreateInstance(&(WGPUInstanceDescriptor){.nextInChain = NULL});

  WGPUSurface surface = create_surface(instance, window);

  WGPUAdapter adapter;
  wgpuInstanceRequestAdapter(instance, NULL, request_adapter_callback,
                             (void *)&adapter);

  printAdapterFeatures(adapter);
  printSurfaceCapabilities(surface, adapter);

  WGPUDevice device;
  wgpuAdapterRequestDevice(adapter, NULL, request_device_callback,
                           (void *)&device);

  wgpuDeviceSetUncapturedErrorCallback(device, handle_uncaptured_error, NULL);

  WGPUShaderModuleDescriptor shaderSource =
      load_wgsl(RESOURCE_DIR "shader.wgsl");
  WGPUShaderModule shader = wgpuDeviceCreateShaderModule(device, &shaderSource);

  WGPUTextureFormat swapChainFormat =
      wgpuSurfaceGetPreferredFormat(surface, adapter);

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(
      device,
      &(WGPURenderPipelineDescriptor){
          .label = "Render pipeline",
          .vertex =
              (WGPUVertexState){
                  .module = shader,
                  .entryPoint = "vs_main",
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
                  .module = shader,
                  .entryPoint = "fs_main",
                  .targetCount = 1,
                  .targets =
                      &(WGPUColorTargetState){
                          .format = swapChainFormat,
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
      .format = swapChainFormat,
      .width = 0,
      .height = 0,
      .presentMode = WGPUPresentMode_Fifo,
  };

  glfwGetWindowSize(window, (int *)&config.width, (int *)&config.height);

  WGPUSwapChain swapChain = wgpuDeviceCreateSwapChain(device, surface, &config);

  while (!glfwWindowShouldClose(window)) {
    WGPUTextureView nextTexture = NULL;

    for (int attempt = 0; attempt < 2; attempt++) {
      uint32_t prevWidth = config.width;
      uint32_t prevHeight = config.height;
      glfwGetWindowSize(window, (int *)&config.width, (int *)&config.height);

      if (prevWidth != config.width || prevHeight != config.height) {
        swapChain = wgpuDeviceCreateSwapChain(device, surface, &config);
      }

      nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
      if (attempt == 0 && !nextTexture) {
        printf("wgpuSwapChainGetCurrentTextureView() failed; trying to create "
               "a new swap chain...\n");
        config.width = 0;
        config.height = 0;
        continue;
      }

      break;
    }

    if (!nextTexture) {
      printf("Cannot acquire next swap chain texture\n");
      return 1;
    }

    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(
        device, &(WGPUCommandEncoderDescriptor){.label = "Command Encoder"});

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(
        encoder, &(WGPURenderPassDescriptor){
                     .colorAttachments =
                         &(WGPURenderPassColorAttachment){
                             .view = nextTexture,
                             .resolveTarget = NULL,
                             .loadOp = WGPULoadOp_Clear,
                             .storeOp = WGPUStoreOp_Store,
                             .clearValue =
                                 (WGPUColor){
                                     .r = 0.0,
                                     .g = 1.0,
                                     .b = 0.0,
                                     .a = 1.0,
                                 },
                         },
                     .colorAttachmentCount = 1,
                     .depthStencilAttachment = NULL,
                 });

    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
    wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
    wgpuRenderPassEncoderEnd(renderPass);

    WGPUQueue queue = wgpuDeviceGetQueue(device);
    WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(
        encoder, &(WGPUCommandBufferDescriptor){.label = NULL});
    wgpuQueueSubmit(queue, 1, &cmdBuffer);
    wgpuSwapChainPresent(swapChain);

    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}