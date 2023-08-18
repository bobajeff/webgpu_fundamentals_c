#include "create_surface.h"
#include "framework.h"
#include "webgpu.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>



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

typedef struct circleVertices {
    float * vertexData; // will need to dynamically make an array using malloc()
    float numVertices;
    int vertexDataSize; // store the size (not length) of the array 
} circleVertices;

typedef struct circleVerticesConfig {
    float radius;
    int numSubdivisions ;
    float innerRadius;
    float startAngle;
    float endAngle;
} circleVerticesConfig;

const circleVerticesConfig DEFAULT_CIRCLE_VERTICES_CONFIG = {
    .radius = 1,
    .numSubdivisions = 25,
    .innerRadius = 0,
    .startAngle = 0,
    .endAngle = M_PI * 2
};

circleVertices createCircleVertices(circleVerticesConfig config){
    // 2 triangles per subdivision, 3 verts per tri, 2 values (xy) each.
    const int numVertices = config.numSubdivisions * 3 * 2;
    const int vertexDataSize = config.numSubdivisions * 2 * 3 * 2 * sizeof(float);
    //malloc is used so I can create array at runtime and free it when I'm done with it
    float * vertexData = (float*)malloc(vertexDataSize);  
    const circleVertices return_data = {
        .numVertices = numVertices,
        .vertexData = vertexData,
        .vertexDataSize = vertexDataSize
    };

    int offset = 0;

    // 2 vertices per subdivision
    //
    // 0--1 4
    // | / /|
    // |/ / |
    // 2 3--5
    int i;
    for (i = 0; i < config.numSubdivisions; ++i) {
      const float angle1 = config.startAngle + (i + 0) * (config.endAngle - config.startAngle) / config.numSubdivisions;
      const float angle2 = config.startAngle + (i + 1) * (config.endAngle - config.startAngle) / config.numSubdivisions;
      
      const float c1 = cosf(angle1);
      const float s1 = sinf(angle1);
      const float c2 = cosf(angle2);
      const float s2 = sinf(angle2);

    // first triangle
      vertexData[offset++] = c1 * config.radius; // x
      vertexData[offset++] = s1 * config.radius; // y
      vertexData[offset++] = c2 * config.radius; // x
      vertexData[offset++] = s2 * config.radius; // y
      vertexData[offset++] = c1 * config.innerRadius; // x
      vertexData[offset++] = s1 * config.innerRadius; // y

    // second triangle
      vertexData[offset++] = c1 * config.innerRadius; // x
      vertexData[offset++] = s1 * config.innerRadius; // y
      vertexData[offset++] = c2 * config.radius; // x
      vertexData[offset++] = s2 * config.radius; // y
      vertexData[offset++] = c2 * config.innerRadius; // x
      vertexData[offset++] = s2 * config.innerRadius; // y
    }
    return return_data;
}

// create a struct to hold the values for the uniforms
typedef struct OurStruct {
  float color[4];
  float offset[2];
  float padding[2];
} OurStruct;

typedef struct OtherStruct {
    float scale[2];
} OtherStruct;

typedef struct objectInfo {
    float scale;
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

WGPUQueue queue = wgpuDeviceGetQueue(device);

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
      load_wgsl(RESOURCE_DIR "webgpu-storage-buffer-vertices.wgsl");
  WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &shaderSource);

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(
      device,
      &(WGPURenderPipelineDescriptor){
          .label = "storage buffer vertices",
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

  const int kNumObjects = 100;
  objectInfo objectInfos[kNumObjects];

  // create 2 storage buffers
  const uint64_t staticUnitSize =
      4 * 4 + // color is 4 32bit floats (4bytes each)
      2 * 4 + // offset is 2 32bit floats (4bytes each)
      2 * 4;  // padding
  const uint64_t changingUnitSize =
      2 * 4;  // scale is 2 32bit floats (4bytes each)
  const uint64_t staticStorageBufferSize = staticUnitSize * kNumObjects;
  const uint64_t changingStorageBufferSize = changingUnitSize * kNumObjects;

  const WGPUBuffer staticStorageBuffer = wgpuDeviceCreateBuffer(
      device,
      &(WGPUBufferDescriptor){
          .label = "static storage for objects",
          .size = staticStorageBufferSize,
          .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst,
          .mappedAtCreation = false
          }
    );

  const WGPUBuffer changingStorageBuffer = wgpuDeviceCreateBuffer(
      device,
      &(WGPUBufferDescriptor){
          .label = "static storage for objects",
          .size = changingStorageBufferSize,
          .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst,
          .mappedAtCreation = false
          }
    );

  OurStruct staticStorageValues[kNumObjects];
  int i;
  for (i = 0; i < kNumObjects; ++i) {

    // These are only set once so set them now
        // set the color
    staticStorageValues[i].color[0] = rand_(0, 0);
    staticStorageValues[i].color[1] = rand_(0, 0);
    staticStorageValues[i].color[2] = rand_(0, 0);
    staticStorageValues[i].color[3] = 1.0;
        // set the offset
    staticStorageValues[i].offset[0] = rand_(-0.9, 0.9);
    staticStorageValues[i].offset[1] = rand_(-0.9, 0.9);

    objectInfos[i].scale = rand_(0.2, 0.5);
  }
  wgpuQueueWriteBuffer(queue, staticStorageBuffer, 0, staticStorageValues,
                       sizeof(staticStorageValues));

  // a array we can use to update the changingStorageBuffer
  OtherStruct storageValues[kNumObjects];

  // setup a storage buffer with vertex data
  circleVerticesConfig circle_config = DEFAULT_CIRCLE_VERTICES_CONFIG;
  circle_config.radius = 0.5;
  circle_config.innerRadius = 0.25;
  circleVertices vertexValues = createCircleVertices(circle_config);
  const WGPUBuffer vertexStorageBuffer = wgpuDeviceCreateBuffer(
      device,
      &(WGPUBufferDescriptor){
          .label = "storage buffer vertices",
          .size = vertexValues.vertexDataSize,
          .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst,
          .mappedAtCreation = false
          }
    );
  wgpuQueueWriteBuffer(queue, vertexStorageBuffer, 0, vertexValues.vertexData,
                       vertexValues.vertexDataSize);
  free(vertexValues.vertexData); //free the malloced data after it's sent to the gpu

  WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(
      device,
      &(WGPUBindGroupDescriptor){
          .label = "bind group for objects",
          .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
          .entries = (WGPUBindGroupEntry[3]){{.binding = 0,
                                              .buffer = staticStorageBuffer,
                                              .size = staticStorageBufferSize,
                                              .offset = 0},
                                             {.binding = 1,
                                              .buffer = changingStorageBuffer,
                                              .size = changingStorageBufferSize,
                                              .offset = 0},
                                             {.binding = 2,
                                              .buffer = vertexStorageBuffer,
                                              .size = vertexValues.vertexDataSize,
                                              .offset = 0}},
          .entryCount = 3});

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
    // set the scales for each object
    for (i = 0; i < kNumObjects; ++i) {
        // set the scale of the uniform values
        const uint32_t aspect = config.width / config.height;
        storageValues[i].scale[0] = objectInfos[i].scale / aspect;
        storageValues[i].scale[1] = objectInfos[i].scale;
    }
    // upload all scales at once
    wgpuQueueWriteBuffer(queue, changingStorageBuffer, 0,storageValues, sizeof(storageValues));
    
    wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, NULL);
    wgpuRenderPassEncoderDraw(pass, vertexValues.numVertices, kNumObjects, 0,
                            0);
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