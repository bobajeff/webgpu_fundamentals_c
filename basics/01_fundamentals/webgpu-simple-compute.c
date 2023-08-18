#include "create_surface.h"
#include "framework.h"
#include "webgpu.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const float input[3] = {1, 3, 5};

WGPUBuffer resultBuffer;

void callback(WGPUBufferMapAsyncStatus status, void *user_data) {
  if (status != WGPUBufferMapAsyncStatus_Success)
    return;

  // Print the results
  float *result =
      (float *)wgpuBufferGetMappedRange(resultBuffer, 0, sizeof(float) * 3);

  printf("input = [");
  for (int i = 0; i < 3; i++) {
    if (i > 0) {
      printf(",");
    }
    printf("%f", input[i]);
  }
  printf("]\n");
  printf("result = [");
  for (int i = 0; i < 3; i++) {
    if (i > 0) {
      printf(",");
    }
    printf("%f", result[i]);
  }
  printf("]\n");

  wgpuBufferUnmap(resultBuffer);
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

  WGPUShaderModuleDescriptor shaderSource =
      load_wgsl(RESOURCE_DIR "webgpu-simple-compute.wgsl");
  WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &shaderSource);

  WGPUComputePipeline pipeline = wgpuDeviceCreateComputePipeline(
      device, &(WGPUComputePipelineDescriptor){
                  .label = "doubling compute pipeline",
                  .compute = (WGPUProgrammableStageDescriptor){
                      .module = module, .entryPoint = "computeSomething"}});

  WGPUQueue queue = wgpuDeviceGetQueue(device);

  // create a buffer on the GPU to hold our computation
  // input and output
  WGPUBuffer workBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
                  .nextInChain = NULL,
                  .label = "work buffer",
                  .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc |
                           WGPUBufferUsage_CopyDst,
                  .size = sizeof(input),
                  .mappedAtCreation = false,
              });
  // Copy our input data to that buffer
  wgpuQueueWriteBuffer(queue, workBuffer, 0, input, sizeof(input));

  // create a buffer on the GPU to get a copy of the results
  resultBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
                  .nextInChain = NULL,
                  .label = "result buffer",
                  .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
                  .size = sizeof(input),
                  .mappedAtCreation = false,
              });

  WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(
      device, &(WGPUBindGroupDescriptor){
                  .nextInChain = NULL,
                  .layout = wgpuComputePipelineGetBindGroupLayout(pipeline, 0),
                  .entries = &(WGPUBindGroupEntry){.nextInChain = NULL,
                                                   .binding = 0,
                                                   .buffer = workBuffer,
                                                   .offset = 0,
                                                   .size = sizeof(input)},
                  .entryCount = 1, //**Important!**
              });

  // make a command encoder to start encoding commands
  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(
      device, &(WGPUCommandEncoderDescriptor){.label = "our encoder"});

  WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(
      encoder, &(WGPUComputePassDescriptor){
                   .label = "doubling compute pass",
               });
  wgpuComputePassEncoderSetPipeline(pass, pipeline);
  wgpuComputePassEncoderSetBindGroup(pass, 0, bindGroup, 0, NULL);
  wgpuComputePassEncoderDispatchWorkgroups(pass, 3, 1, 1);
  wgpuComputePassEncoderEnd(pass);

  // Encode a command to copy the results to a mappable buffer.
  wgpuCommandEncoderCopyBufferToBuffer(encoder, workBuffer, 0, resultBuffer, 0,
                                       sizeof(input));
  // Finish encoding and submit the commands
  WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(
      encoder, &(WGPUCommandBufferDescriptor){.label = NULL});
  wgpuQueueSubmit(queue, 1, &commandBuffer);

  // Read the results
  wgpuBufferMapAsync(resultBuffer, WGPUMapMode_Read, 0, sizeof(input), callback,
                     NULL);

  while (!glfwWindowShouldClose(window)) {
    wgpuQueueSubmit(queue, 0, NULL);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}