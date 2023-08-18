#include "create_surface.h"
#include "framework.h"
#include "webgpu.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



// A random number between [min and max)
// With 1 argument it will be [0 to min)
// With no arguments it will be [0 to 1)
float rand_(float min, float max) {
  if (min == 0) {
    min = 0;
    max = 1;
  } else if (max == 0) {
    max = min;
    min = 0;
  }
  float scale = rand() / (float)RAND_MAX;
  return min + scale * (max - min);
};
 
// create a struct to hold the values for the uniforms
typedef struct OurStruct {
    float color[4];
    float scale[2];
	float offset[2];
} OurStruct;

typedef struct objectInfo {
    float scale;
    WGPUBuffer uniformBuffer;
    OurStruct uniformValues;
    WGPUBindGroup bindGroup;
} objectInfo;

int main(int argc, char *argv[]) {
  srand(time(NULL)); //seed random number generator

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
      load_wgsl(RESOURCE_DIR "webgpu-simple-triangle-uniforms-multiple.wgsl");
  WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &shaderSource);

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(
      device,
      &(WGPURenderPipelineDescriptor){
          .label = "our hardcoded red triangle pipeline",
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

  // create a buffer for the uniform values
  const uint64_t uniformBufferSize =
      4 * 4 + // color is 4 32bit floats (4bytes each)
      2 * 4 + // scale is 2 32bit floats (4bytes each)
      2 * 4;  // offset is 2 32bit floats (4bytes each)

  const int kNumObjects = 100;
  objectInfo objectInfos[kNumObjects];

  int i;
  for (i = 0; i < kNumObjects; ++i) {

    objectInfos[i].uniformBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
        .label = "uniforms for obj: $i", // Note: C can't do string interpolation as easily as other languages. So I avoid that and just quote the variable '$i' instead.
        .size = uniformBufferSize,
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .mappedAtCreation = false
        }
    );
    // make instance of OurStruct with the values for the uniforms
        // set the color
    objectInfos[i].uniformValues.color[0] = rand_(0, 0);
    objectInfos[i].uniformValues.color[1] = rand_(0, 0);
    objectInfos[i].uniformValues.color[2] = rand_(0, 0);
    objectInfos[i].uniformValues.color[3] = 1.0;
        // set the offset
    objectInfos[i].uniformValues.offset[0] = rand_(-0.9, 0.9);
    objectInfos[i].uniformValues.offset[1] = rand_(-0.9, 0.9);
    

    objectInfos[i].bindGroup = wgpuDeviceCreateBindGroup(
        device, &(WGPUBindGroupDescriptor){
                    .label = "bind group for obj: $i",
                    .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
                    .entries = &(WGPUBindGroupEntry){.binding = 0,
                                                     .buffer = objectInfos[i].uniformBuffer,
                                                     .size = uniformBufferSize,
                                                     .offset = 0

                    },
                    .entryCount = 1});
    objectInfos[i].scale = rand_(0.2, 0.5);
  }




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
  WGPUQueue queue = wgpuDeviceGetQueue(device);
  while (!glfwWindowShouldClose(window)) {
    WGPUTextureView view = NULL;

    for (int attempt = 0; attempt < 2; attempt++) {
      uint32_t prevWidth = config.width;
      uint32_t prevHeight = config.height;
      glfwGetWindowSize(window, (int *)&config.width, (int *)&config.height);


      if (prevWidth != config.width || prevHeight != config.height) {
        swapChain = wgpuDeviceCreateSwapChain(device, surface, &config);
      }
      // Get the current texture from the swapChain to use for rendering to by
      // the render pass
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
        device, &(WGPUCommandEncoderDescriptor){.label = "our encoder"});

    WGPURenderPassDescriptor renderPassDescriptor = {
        .colorAttachments =
            &(WGPURenderPassColorAttachment){
                .view = view, // texture from SwapChain
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
    for (i = 0; i < kNumObjects; ++i) {
        // set the scale of the uniform values
        const uint32_t aspect = config.width / config.height;
        objectInfos[i].uniformValues.scale[0] = objectInfos[i].scale / aspect;
        objectInfos[i].uniformValues.scale[1] = objectInfos[i].scale;
        wgpuQueueWriteBuffer(queue, objectInfos[i].uniformBuffer, 0, &objectInfos[i].uniformValues, sizeof(objectInfos[i].uniformValues));
        
        wgpuRenderPassEncoderSetBindGroup(pass, 0, objectInfos[i].bindGroup, 0, NULL);
        wgpuRenderPassEncoderDraw(pass, 3, 1, 0,
                                0); // call our vertex shader 3 times.
    }
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