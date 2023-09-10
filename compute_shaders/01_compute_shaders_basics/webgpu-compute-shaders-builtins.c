#include "create_surface.h"
#include "framework.h"
#include "webgpu.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// multiply all elements of an array
int arrayProd(int * array, int array_length){
  int i;
  int product = 1;
  for (i = 0; i < array_length; i++){
    product = product * array[i];
  }
  return product;
};

WGPUBuffer workgroupReadBuffer;
WGPUBuffer localReadBuffer;
WGPUBuffer globalReadBuffer;
int numWorkgroups;
int numResults;
int size;
int numThreadsPerWorkgroup;

bool got_workgroup = false;
bool got_local = false;
bool got_global = false;

void read_the_results(){
    // only run when all buffers are gotten
  if (got_workgroup && got_local && got_global){
    int *workgroup = (int *)wgpuBufferGetMappedRange(workgroupReadBuffer, 0, size);
    int *local = (int *)wgpuBufferGetMappedRange(localReadBuffer, 0, size);
    int *global = (int *)wgpuBufferGetMappedRange(globalReadBuffer, 0, size);
    int i;
    for (i = 0; i < numResults; ++i) {
      if (i % numThreadsPerWorkgroup == 0) {
        printf(
            "---------------------------------------\n"
            "global                 local     global   dispatch: %i\n"
            "invoc.    workgroup    invoc.    invoc.\n"
            "index     id           id        id\n"
            "---------------------------------------\n", i / numThreadsPerWorkgroup
        );
        printf("%i:", i);
        printf("\t%i, %i, %i", workgroup[i],workgroup[i + 1],workgroup[i + 2]);
        printf("     %i, %i, %i", local[i],local[i + 1],local[i + 2]);
        printf("     %i, %i, %i\n", global[i],global[i + 1],global[i + 2]);
      }
    }

    wgpuBufferUnmap(workgroupReadBuffer);
    wgpuBufferUnmap(localReadBuffer);
    wgpuBufferUnmap(globalReadBuffer);
  }
}

void wg_callback(WGPUBufferMapAsyncStatus status, void *user_data) {
  if (status != WGPUBufferMapAsyncStatus_Success)
    return;
  got_workgroup = true;
  read_the_results();
};

void lc_callback(WGPUBufferMapAsyncStatus status, void *user_data) {
  if (status != WGPUBufferMapAsyncStatus_Success)
    return;
  got_local = true;
  read_the_results();
};

void gb_callback(WGPUBufferMapAsyncStatus status, void *user_data) {
  if (status != WGPUBufferMapAsyncStatus_Success)
    return;
  got_global = true;
  read_the_results();
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

  int dispatchCount[] = {4, 3, 2};
  int dispatchCount_length = sizeof(dispatchCount) / sizeof(dispatchCount[0]);
  int workgroupSize[] = {2, 3, 4};
  int workgroupSize_length = sizeof(workgroupSize) / sizeof(workgroupSize[0]);

  numThreadsPerWorkgroup = arrayProd(workgroupSize, workgroupSize_length);

  WGPUShaderModuleDescriptor shaderSource =
      load_wgsl(RESOURCE_DIR "shader.wgsl");
  WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &shaderSource);

  WGPUComputePipeline pipeline = wgpuDeviceCreateComputePipeline(
      device, &(WGPUComputePipelineDescriptor){
                  .label = "compute pipeline",
                  .compute = (WGPUProgrammableStageDescriptor){
                      .module = module, .entryPoint = "computeSomething"}});

  WGPUQueue queue = wgpuDeviceGetQueue(device);

  numWorkgroups = arrayProd(dispatchCount, dispatchCount_length);
  numResults = numWorkgroups * numThreadsPerWorkgroup;
  size = numResults * 4 * 4;  // vec3f * u32

  WGPUBuffer workgroupBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
                  .nextInChain = NULL,
                  .label = NULL,
                  .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc |
                           WGPUBufferUsage_CopyDst,
                  .size = size,
                  .mappedAtCreation = false,
              });

  WGPUBuffer localBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
                  .nextInChain = NULL,
                  .label = NULL,
                  .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc |
                           WGPUBufferUsage_CopyDst,
                  .size = size,
                  .mappedAtCreation = false,
              });

  WGPUBuffer globalBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
                  .nextInChain = NULL,
                  .label = NULL,
                  .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc |
                           WGPUBufferUsage_CopyDst,
                  .size = size,
                  .mappedAtCreation = false,
              });

  workgroupReadBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
                  .nextInChain = NULL,
                  .label = NULL,
                  .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
                  .size = size,
                  .mappedAtCreation = false,
              });

  localReadBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
                  .nextInChain = NULL,
                  .label = NULL,
                  .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
                  .size = size,
                  .mappedAtCreation = false,
              });

  globalReadBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){
                  .nextInChain = NULL,
                  .label = NULL,
                  .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
                  .size = size,
                  .mappedAtCreation = false,
              });

  WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(
      device, &(WGPUBindGroupDescriptor){
                  .nextInChain = NULL,
                  .layout = wgpuComputePipelineGetBindGroupLayout(pipeline, 0),
                  .entries = (WGPUBindGroupEntry[]){
                                                    {.nextInChain = NULL,
                                                   .binding = 0,
                                                   .buffer = workgroupBuffer,
                                                   .offset = 0,
                                                   .size = size},
                                                    {.nextInChain = NULL,
                                                   .binding = 1,
                                                   .buffer = localBuffer,
                                                   .offset = 0,
                                                   .size = size},
                                                    {.nextInChain = NULL,
                                                   .binding = 2,
                                                   .buffer = globalBuffer,
                                                   .offset = 0,
                                                   .size = size},
                                                   },
                  .entryCount = 3, //**Important!**
              });

  // Encode commands to do the computation
  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(
      device, &(WGPUCommandEncoderDescriptor){.label = "compute builtin encoder"});

  WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(
      encoder, &(WGPUComputePassDescriptor){
                   .label = "compute builtin pass",
               });
  wgpuComputePassEncoderSetPipeline(pass, pipeline);
  wgpuComputePassEncoderSetBindGroup(pass, 0, bindGroup, 0, NULL);
  wgpuComputePassEncoderDispatchWorkgroups(pass, dispatchCount[0], dispatchCount[1], dispatchCount[2]);
  wgpuComputePassEncoderEnd(pass);

  wgpuCommandEncoderCopyBufferToBuffer(encoder, workgroupBuffer, 0, workgroupReadBuffer, 0, size);
  wgpuCommandEncoderCopyBufferToBuffer(encoder, localBuffer, 0, localReadBuffer, 0, size);
  wgpuCommandEncoderCopyBufferToBuffer(encoder, globalBuffer, 0, globalReadBuffer, 0, size);

  // Finish encoding and submit the commands
  WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(
      encoder, &(WGPUCommandBufferDescriptor){.label = NULL});
  wgpuQueueSubmit(queue, 1, &commandBuffer);

  // Read the results
  wgpuBufferMapAsync(workgroupReadBuffer, WGPUMapMode_Read, 0, size, wg_callback, NULL);
  wgpuBufferMapAsync(localReadBuffer, WGPUMapMode_Read, 0, size, lc_callback, NULL);
  wgpuBufferMapAsync(globalReadBuffer, WGPUMapMode_Read, 0, size, gb_callback, NULL);

  while (!glfwWindowShouldClose(window)) {
    wgpuQueueSubmit(queue, 0, NULL);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}