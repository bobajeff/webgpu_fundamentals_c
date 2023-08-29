#include "create_surface.h"
#include "framework.h"
#include "webgpu.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>



// create a struct to hold the values for the uniforms
typedef struct Uniforms {
  float color[4];
  float resolution[2];
  float padding[2];
  float matrix[12];
} Uniforms;

void multiply_3x3_mat(float * a, float * b, float * dest) {
  float a00 = a[0 * 3 + 0];
  float a01 = a[0 * 3 + 1];
  float a02 = a[0 * 3 + 2];
  float a10 = a[1 * 3 + 0];
  float a11 = a[1 * 3 + 1];
  float a12 = a[1 * 3 + 2];
  float a20 = a[2 * 3 + 0];
  float a21 = a[2 * 3 + 1];
  float a22 = a[2 * 3 + 2];
  float b00 = b[0 * 3 + 0];
  float b01 = b[0 * 3 + 1];
  float b02 = b[0 * 3 + 2];
  float b10 = b[1 * 3 + 0];
  float b11 = b[1 * 3 + 1];
  float b12 = b[1 * 3 + 2];
  float b20 = b[2 * 3 + 0];
  float b21 = b[2 * 3 + 1];
  float b22 = b[2 * 3 + 2];

  dest[0] = b00 * a00 + b01 * a10 + b02 * a20;
  dest[1] = b00 * a01 + b01 * a11 + b02 * a21;
  dest[2] = b00 * a02 + b01 * a12 + b02 * a22;
  dest[3] = b10 * a00 + b11 * a10 + b12 * a20;
  dest[4] = b10 * a01 + b11 * a11 + b12 * a21;
  dest[5] = b10 * a02 + b11 * a12 + b12 * a22;
  dest[6] = b20 * a00 + b21 * a10 + b22 * a20;
  dest[7] = b20 * a01 + b21 * a11 + b22 * a21;
  dest[8] = b20 * a02 + b21 * a12 + b22 * a22;
}

void identity_3x3_mat(float * dest) {
    dest[0] = 1.0, dest[1] = 0.0, dest[2] = 0.0, 
    dest[3] = 0.0, dest[4] = 1.0, dest[5] = 0.0, 
    dest[6] = 0.0, dest[7] = 0.0, dest[8] = 1.0;
}

void translation_3x3_mat(float tx, float ty, float * dest) {
    dest[0] = 1.0, dest[1] = 0.0, dest[2] = 0.0, 
    dest[3] = 0.0, dest[4] = 1.0, dest[5] = 0.0, 
    dest[6] = tx,  dest[7] = ty,  dest[8] = 1.0;
}

void rotation_3x3_mat(float angleInRadians, float * dest) {
    float c = cosf(angleInRadians);
    float s = sinf(angleInRadians);
    dest[0] = c,   dest[1] = s,   dest[2] = 0.0, 
    dest[3] = -s,  dest[4] = c,   dest[5] = 0.0, 
    dest[6] = 0.0, dest[7] = 0.0, dest[8] = 1.0;
}

void scaling_3x3_mat(float sx, float sy, float * dest) {
    dest[0] = sx,  dest[1] = 0.0, dest[2] = 0.0, 
    dest[3] = 0.0, dest[4] = sy,  dest[5] = 0.0, 
    dest[6] = 0.0, dest[7] = 0.0, dest[8] = 1.0;
}


int main(int argc, char *argv[]) {
  srand(time(NULL)); // seed random number generator

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

  WGPUShaderModuleDescriptor shaderSource = load_wgsl(
      RESOURCE_DIR "shader.wgsl");
  WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &shaderSource);

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(
      device,
      &(WGPURenderPipelineDescriptor){
          .label = "just 2d position",
          .vertex =
              (WGPUVertexState){
                  .module = module,
                  .entryPoint = "vs",
                  .bufferCount = 1,
                  .buffers =
                      (WGPUVertexBufferLayout[]){
                          {.arrayStride = 2 * 4, // (2) floats, 4 bytes each
                           .attributes =
                               (WGPUVertexAttribute[]){
                                   // position
                                   {.shaderLocation = 0,
                                    .offset = 0,
                                    .format = WGPUVertexFormat_Float32x2},
                               },
                           .attributeCount = 1,
                           .stepMode = WGPUVertexStepMode_Vertex},
                         },},
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

  #define numObjects 5
  WGPUBuffer _uniformBuffer[numObjects];
  Uniforms _uniformValues[numObjects];
  WGPUBindGroup _bindGroup[numObjects];
  int i;
  for (i = 0; i < numObjects; ++i) {
    // color, resolution, padding, matrix
    const uint64_t uniformBufferSize = (4 + 2 + 2 + 12) * 4;
    _uniformBuffer[i] = wgpuDeviceCreateBuffer(
        device, &(WGPUBufferDescriptor){.label = "uniforms",
                                        .size = uniformBufferSize,
                                        .usage = WGPUBufferUsage_Uniform |
                                                WGPUBufferUsage_CopyDst,
                                        .mappedAtCreation = false});

    _uniformValues[i].color[0] = (float)rand()/(float)(RAND_MAX/1.0);
    _uniformValues[i].color[1] = (float)rand()/(float)(RAND_MAX/1.0);
    _uniformValues[i].color[2] = (float)rand()/(float)(RAND_MAX/1.0);
    _uniformValues[i].color[3] = 1;

    _bindGroup[i] = wgpuDeviceCreateBindGroup(
        device, &(WGPUBindGroupDescriptor){
                    .label = "bind group for object",
                    .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
                    .entries =
                        (WGPUBindGroupEntry[]){
                            {.binding = 0,
                             .buffer = _uniformBuffer[i],
                             .size = uniformBufferSize,
                             .offset = 0},
                        },
                    .entryCount = 1});
  }


  // createFVertices()
  float vertexData[] = {
    // left column
    0, 0,
    30, 0,
    0, 150,
    30, 150,

    // top rung
    30, 0,
    100, 0,
    30, 30,
    100, 30,

    // middle rung
    30, 60,
    70, 60,
    30, 90,
    70, 90,
  };
  int vertexDataSize = sizeof(vertexData);
  u_int32_t indexData[] = {
    0,  1,  2,    2,  1,  3,  // left column
    4,  5,  6,    6,  5,  7,  // top run
    8,  9, 10,   10,  9, 11,  // middle run
  };
  int indexDataSize = sizeof(indexData);
  int numVertices = sizeof(indexData) / sizeof(u_int32_t);

  const WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){.label = "vertex buffer vertices",
                                      .size = vertexDataSize,
                                      .usage = WGPUBufferUsage_Vertex |
                                               WGPUBufferUsage_CopyDst,
                                      .mappedAtCreation = false});
  wgpuQueueWriteBuffer(queue, vertexBuffer, 0, vertexData,

                       vertexDataSize);
  const WGPUBuffer indexBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){.label = "index buffer",
                                      .size = indexDataSize,
                                      .usage = WGPUBufferUsage_Index |
                                               WGPUBufferUsage_CopyDst,
                                      .mappedAtCreation = false});
  wgpuQueueWriteBuffer(queue, indexBuffer, 0, indexData,
                       indexDataSize);

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

  WGPUSupportedLimits limits = {};
  bool gotlimits = wgpuDeviceGetLimits(device, &limits);
  
  #define NUM_OF_PRESETS 10
  float translation[NUM_OF_PRESETS][2] = {
    {150.0,100.0},
    {200.0,300.0},
    {500.0,40.0},
    {700.0,1000.0},
    {300.0,200.0},
    {40.0,500.0},
    {1000.0,700.0},
    {80.0,20.0},
    {10.0,250.0},
    {1000.0,1000.0},
  };

  float rotation_degree = 0;
  int preset = 0;
  int count = 0;

  float scale[2] = {1.0,1.0};
  int scale_forward = 1;
  int scale_axis = 0;
  
  while (!glfwWindowShouldClose(window)) {
    // change translation preset after 250 loops (because making a GUI is not as easy outside of the web)
    if (count < 250) 
    {
        count++;
    }
    else {
        scale_axis = scale_axis == 0 ? 1 : 0; //switch scale axis every 250 loop
        count = 0;
        // if (preset < NUM_OF_PRESETS + 1){
        //     preset++;
        // }
        // else {
        //     preset = 0;
        // }
        // printf("translation: %f,%f\n", translation[preset][0],translation[preset][1]);
    }
    // if (rotation_degree < 90){
    //     rotation_degree += 0.1;
    // } else {
    //     rotation_degree = 0;
    // }
    // printf("rotation: %f°\n", rotation_degree);

    if (scale_forward){
        if (scale[scale_axis] < 5){
            scale[scale_axis] += 0.1;
        } else {
            scale_forward = 0;
        }
    } else {
        if (scale[scale_axis] > -5){
            scale[scale_axis] -= 0.1;
        } else {
            scale_forward = 1;
        }
    }
    printf("scale: %f,%f\n", scale[0],scale[1]);
    


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
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertexBuffer, 0,
                                         vertexDataSize);
    wgpuRenderPassEncoderSetIndexBuffer(pass, indexBuffer, WGPUIndexFormat_Uint32, 0, indexDataSize);
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

    float translationMatrix[9];
    translation_3x3_mat(translation[preset][0], translation[preset][1], translationMatrix);
    float rotationMatrix[9];
    rotation_3x3_mat(rotation_degree, rotationMatrix);
    float scaleMatrix[9];
    scaling_3x3_mat(scale[0], scale[1], scaleMatrix);
    
    // Starting Matrix.
    float matrix[9];
    identity_3x3_mat(matrix);

    for (i = 0; i < numObjects; ++i) {
        
        float matrix_reloaded[9];
        multiply_3x3_mat(matrix, translationMatrix, matrix_reloaded);
        float matrix_revolutions[9];
        multiply_3x3_mat(matrix_reloaded, rotationMatrix, matrix_revolutions);
        float matrix_resurrections[9];
        multiply_3x3_mat(matrix_revolutions, scaleMatrix, matrix_resurrections);

        _uniformValues[i].resolution[0] = window_width;
        _uniformValues[i].resolution[1] = window_height;
        _uniformValues[i].matrix[0] = matrix_resurrections[0];
        _uniformValues[i].matrix[1] = matrix_resurrections[1];
        _uniformValues[i].matrix[2] = matrix_resurrections[2];
        _uniformValues[i].matrix[3] = 0; //padding
        _uniformValues[i].matrix[4] = matrix_resurrections[3];
        _uniformValues[i].matrix[5] = matrix_resurrections[4];
        _uniformValues[i].matrix[6] = matrix_resurrections[5];
        _uniformValues[i].matrix[7] = 0; //padding
        _uniformValues[i].matrix[8] = matrix_resurrections[6];
        _uniformValues[i].matrix[9] = matrix_resurrections[7];
        _uniformValues[i].matrix[10] = matrix_resurrections[8];
        _uniformValues[i].matrix[11] = 0; //padding

        matrix[0] = matrix_resurrections[0];
        matrix[1] = matrix_resurrections[1];
        matrix[2] = matrix_resurrections[2];
        matrix[3] = matrix_resurrections[3];
        matrix[4] = matrix_resurrections[4];
        matrix[5] = matrix_resurrections[5];
        matrix[6] = matrix_resurrections[6];
        matrix[7] = matrix_resurrections[7];
        matrix[8] = matrix_resurrections[8];

        // upload the uniform values to the uniform buffer
        wgpuQueueWriteBuffer(queue, _uniformBuffer[i], 0, &_uniformValues[i],
                            sizeof(_uniformValues[i]));

        wgpuRenderPassEncoderSetBindGroup(pass, 0, _bindGroup[i], 0, NULL);

        wgpuRenderPassEncoderDrawIndexed(pass, numVertices, 1, 0, 0, 0);
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