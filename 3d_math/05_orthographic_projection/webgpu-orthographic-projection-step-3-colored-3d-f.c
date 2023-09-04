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
  float matrix[16];
} Uniforms;

void multiply_4x4_mat(float * a, float * b, float * dest) {
    float b00 = b[0 * 4 + 0];
    float b01 = b[0 * 4 + 1];
    float b02 = b[0 * 4 + 2];
    float b03 = b[0 * 4 + 3];
    float b10 = b[1 * 4 + 0];
    float b11 = b[1 * 4 + 1];
    float b12 = b[1 * 4 + 2];
    float b13 = b[1 * 4 + 3];
    float b20 = b[2 * 4 + 0];
    float b21 = b[2 * 4 + 1];
    float b22 = b[2 * 4 + 2];
    float b23 = b[2 * 4 + 3];
    float b30 = b[3 * 4 + 0];
    float b31 = b[3 * 4 + 1];
    float b32 = b[3 * 4 + 2];
    float b33 = b[3 * 4 + 3];
    float a00 = a[0 * 4 + 0];
    float a01 = a[0 * 4 + 1];
    float a02 = a[0 * 4 + 2];
    float a03 = a[0 * 4 + 3];
    float a10 = a[1 * 4 + 0];
    float a11 = a[1 * 4 + 1];
    float a12 = a[1 * 4 + 2];
    float a13 = a[1 * 4 + 3];
    float a20 = a[2 * 4 + 0];
    float a21 = a[2 * 4 + 1];
    float a22 = a[2 * 4 + 2];
    float a23 = a[2 * 4 + 3];
    float a30 = a[3 * 4 + 0];
    float a31 = a[3 * 4 + 1];
    float a32 = a[3 * 4 + 2];
    float a33 = a[3 * 4 + 3];

    dest[0] = b00 * a00 + b01 * a10 + b02 * a20 + b03 * a30;
    dest[1] = b00 * a01 + b01 * a11 + b02 * a21 + b03 * a31;
    dest[2] = b00 * a02 + b01 * a12 + b02 * a22 + b03 * a32;
    dest[3] = b00 * a03 + b01 * a13 + b02 * a23 + b03 * a33;

    dest[4] = b10 * a00 + b11 * a10 + b12 * a20 + b13 * a30;
    dest[5] = b10 * a01 + b11 * a11 + b12 * a21 + b13 * a31;
    dest[6] = b10 * a02 + b11 * a12 + b12 * a22 + b13 * a32;
    dest[7] = b10 * a03 + b11 * a13 + b12 * a23 + b13 * a33;

    dest[8] = b20 * a00 + b21 * a10 + b22 * a20 + b23 * a30;
    dest[9] = b20 * a01 + b21 * a11 + b22 * a21 + b23 * a31;
    dest[10] = b20 * a02 + b21 * a12 + b22 * a22 + b23 * a32;
    dest[11] = b20 * a03 + b21 * a13 + b22 * a23 + b23 * a33;

    dest[12] = b30 * a00 + b31 * a10 + b32 * a20 + b33 * a30;
    dest[13] = b30 * a01 + b31 * a11 + b32 * a21 + b33 * a31;
    dest[14] = b30 * a02 + b31 * a12 + b32 * a22 + b33 * a32;
    dest[15] = b30 * a03 + b31 * a13 + b32 * a23 + b33 * a33;
}

void projection_4x4_mat(int width, int height, int depth, float * dest) {
    // Note: This matrix flips the Y axis so that 0 is at the top.
    dest[0]  = 2.0 / width,  dest[1]  = 0.0,            dest[2]   = 0.0,          dest[3]  = 0.0,
    dest[4]  = 0.0,          dest[5]  = -2.0 / height,  dest[6]   = 0.0,          dest[7]  = 0.0,
    dest[8]  = 0.0,          dest[9]  = 0.0,            dest[10]  = 0.5 / depth,  dest[11] = 0,0,
    dest[12] = -1.0,         dest[13] = 1.0,            dest[14]  = 0.5,          dest[15] = 1.0;
}

void identity_4x4_mat(float * dest) {
    dest[0]  = 1.0,  dest[1]  = 0.0,  dest[2]  = 0.0,   dest[3]  = 0.0,
    dest[4]  = 0.0,  dest[5]  = 1.0,  dest[6]  = 0.0,   dest[7]  = 0.0,
    dest[8]  = 0.0,  dest[9]  = 0.0,  dest[10] = 1.0,   dest[11] = 0.0,
    dest[12] = 0.0,  dest[13] = 0.0,  dest[14] = 0.0,   dest[15] = 1.0;
}

void translation_4x4_mat(float tx, float ty, float tz, float * dest) {
    dest[0]  = 1.0,  dest[1]  = 0.0,  dest[2]  = 0.0,  dest[3]  = 0.0,
    dest[4]  = 0.0,  dest[5]  = 1.0,  dest[6]  = 0.0,  dest[7]  = 0.0,
    dest[8]  = 0.0,  dest[9]  = 0.0,  dest[10] = 1.0,  dest[11] = 0.0,
    dest[12] = tx,   dest[13] = ty,   dest[14] = tz,   dest[15] = 1.0;
}

void rotationX(float angleInRadians, float * dest) {
    float c = cosf(angleInRadians);
    float s = sinf(angleInRadians);
    dest[0]  = 1.0,  dest[1]  = 0.0,  dest[2]  = 0.0,  dest[3]  = 0.0,
    dest[4]  = 0.0,  dest[5]  = c,    dest[6]  = s,    dest[7]  = 0.0,
    dest[8]  = 0.0,  dest[9]  = -s,   dest[10] = c,    dest[11] = 0.0,
    dest[12] = 0.0,  dest[13] = 0.0,  dest[14] = 0.0,  dest[15] = 1.0;
}

void rotationY(float angleInRadians, float * dest) {
    float c = cosf(angleInRadians);
    float s = sinf(angleInRadians);
    dest[0]  = c,    dest[1]    = 0.0,    dest[2]  = -s,    dest[3]    = 0.0,
    dest[4]  = 0.0,  dest[5]    = 1.0,    dest[6]  = 0.0,   dest[7]    = 0.0,
    dest[8]  = s,    dest[9]    = 0.0,    dest[10] = c,     dest[11]   = 0.0,
    dest[12] = 0.0,  dest[13]   = 0.0,    dest[14] = 0.0,   dest[15]   = 1.0;
}

void rotationZ(float angleInRadians, float * dest) {
    float c = cosf(angleInRadians);
    float s = sinf(angleInRadians);
    dest[0]  = c,      dest[1]  = s,     dest[2]  = 0.0,   dest[3]  = 0.0,
    dest[4]  = -s,     dest[5]  = c,     dest[6]  = 0.0,   dest[7]  = 0.0,
    dest[8]  = 0.0,    dest[9]  = 0.0,   dest[10] = 1.0,   dest[11] = 0.0,
    dest[12] = 0.0,    dest[13] = 0.0,   dest[14] = 0.0,   dest[15] = 1.0;
}

void scaling_4x4_mat(float sx, float sy, float sz, float * dest) {
    dest[0]  = sx;     dest[1]  = 0.0;    dest[2]  = 0.0;     dest[3]  = 0.0;
    dest[4]  = 0.0;    dest[5]  = sy;     dest[6]  = 0.0;     dest[7]  = 0.0;
    dest[8]  = 0.0;    dest[9]  = 0.0;    dest[10] = sz;      dest[11] = 0.0;
    dest[12] = 0.0;    dest[13] = 0.0;    dest[14] = 0.0;     dest[15] = 1.0;
}

void translate_4x4_mat(float * m, float tx, float ty, float tz, float * dst) {
    float temp_matrix[16];
    translation_4x4_mat(tx, ty, tz, temp_matrix);
    multiply_4x4_mat(m, temp_matrix, dst);
}

void rotateX(float * m, float angleInRadians, float * dst) {
    float temp_matrix[16];
    rotationX(angleInRadians, temp_matrix);
    multiply_4x4_mat(m, temp_matrix, dst);
}

void rotateY(float * m, float angleInRadians, float * dst) {
    float temp_matrix[16];
    rotationY(angleInRadians, temp_matrix);
    multiply_4x4_mat(m, temp_matrix, dst);
}

void rotateZ(float * m, float angleInRadians, float * dst) {
    float temp_matrix[16];
    rotationZ(angleInRadians, temp_matrix);
    multiply_4x4_mat(m, temp_matrix, dst);
}

void scale_4x4_mat(float * m, float sx, float sy, float sz, float * dst) {
    float temp_matrix[16];
    scaling_4x4_mat(sx, sy, sz, temp_matrix);
    multiply_4x4_mat(m, temp_matrix, dst);
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
      RESOURCE_DIR "webgpu-orthographic-projection-step-3-colored-3d-f.wgsl");
  WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &shaderSource);

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(
      device,
      &(WGPURenderPipelineDescriptor){
          .label = "2 attributes",
          .vertex =
              (WGPUVertexState){
                  .module = module,
                  .entryPoint = "vs",
                  .bufferCount = 1,
                  .buffers =
                      (WGPUVertexBufferLayout[]){
                          {.arrayStride = (4) * 4, // (3) floats 4 bytes each + one 4 byte color
                           .attributes =
                               (WGPUVertexAttribute[]){
                                   // position
                                   {.shaderLocation = 0,
                                    .offset = 0,
                                    .format = WGPUVertexFormat_Float32x3},
                                   {.shaderLocation = 1,
                                    .offset = 12,
                                    .format = WGPUVertexFormat_Unorm8x4},
                               },
                           .attributeCount = 2,
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

  Uniforms uniformValues = {};

  // matrix
  const uint64_t uniformBufferSize = (16) * 4;
  const WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){.label = "uniforms",
                                      .size = uniformBufferSize,
                                      .usage = WGPUBufferUsage_Uniform |
                                               WGPUBufferUsage_CopyDst,
                                      .mappedAtCreation = false});

  // createFVertices()
  float positions[] = {
    // left column
    0, 0, 0,
    30, 0, 0,
    0, 150, 0,
    30, 150, 0,

    // top rung
    30, 0, 0,
    100, 0, 0,
    30, 30, 0,
    100, 30, 0,

    // middle rung
    30, 60, 0,
    70, 60, 0,
    30, 90, 0,
    70, 90, 0,

    // left column back
    0, 0, 30,
    30, 0, 30,
    0, 150, 30,
    30, 150, 30,

    // top rung back
    30, 0, 30,
    100, 0, 30,
    30, 30, 30,
    100, 30, 30,

    // middle rung back
    30, 60, 30,
    70, 60, 30,
    30, 90, 30,
    70, 90, 30,
  };

  u_int32_t indices[] = {
    // front
    0,  1,  2,    2,  1,  3,  // left column
    4,  5,  6,    6,  5,  7,  // top rung
    8,  9, 10,   10,  9, 11,  // middle rung

    // back
    12,  13,  14,   14, 13, 15,  // left column back
    16,  17,  18,   18, 17, 19,  // top rung back
    20,  21,  22,   22, 21, 23,  // middle rung back

    0, 5, 12,   12, 5, 17,   // top
    5, 7, 17,   17, 7, 19,   // top rung right
    6, 7, 18,   18, 7, 19,   // top rung bottom
    6, 8, 18,   18, 8, 20,   // between top and middle rung
    8, 9, 20,   20, 9, 21,   // middle rung top
    9, 11, 21,  21, 11, 23,  // middle rung right
    10, 11, 22, 22, 11, 23,  // middle rung bottom
    10, 3, 22,  22, 3, 15,   // stem right
    2, 3, 14,   14, 3, 15,   // bottom
    0, 2, 12,   12, 2, 14,   // left
  };

  char quadColors[] = {
      200,  70, 120,  // left column front
      200,  70, 120,  // top rung front
      200,  70, 120,  // middle rung front
       80,  70, 200,  // left column back
       80,  70, 200,  // top rung back
       80,  70, 200,  // middle rung back
       70, 200, 210,  // top
      160, 160, 220,  // left side
       90, 130, 110,  // bottom
      200, 200,  70,  // top rung right
      210, 100,  70,  // under top rung
      210, 160,  70,  // between top rung and middle
       70, 180, 210,  // top of middle rung
      100,  70, 210,  // right of middle rung
       76, 210, 100,  // bottom of middle rung.
      140, 210,  80,  // right of bottom
  };

  int indices_length = sizeof(indices) / sizeof(u_int32_t);
  int numVertices = indices_length;
  int vertexDataSize = indices_length * 4 * sizeof(float);
  float *vertexData = (float *)malloc(vertexDataSize);
  char * colorData = (char *)vertexData;
  int i;
  for (i = 0; i < indices_length; i++){
      int positionNdx = indices[i] * 3;
      vertexData[i * 4] = positions[positionNdx];
      vertexData[i * 4 + 1] = positions[positionNdx + 1];
      vertexData[i * 4 + 2] = positions[positionNdx + 2];
  
      int quadNdx = (i / 6 | 0) * 3;
      colorData[i * 16 + 12] = quadColors[quadNdx];
      colorData[i * 16 + 12 + 1] = quadColors[quadNdx + 1];
      colorData[i * 16 + 12 + 2] = quadColors[quadNdx + 2];
      colorData[i * 16 + 12 + 3] = (char)225;
  }

  const WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){.label = "vertex buffer vertices",
                                      .size = vertexDataSize,
                                      .usage = WGPUBufferUsage_Vertex |
                                               WGPUBufferUsage_CopyDst,
                                      .mappedAtCreation = false});
  wgpuQueueWriteBuffer(queue, vertexBuffer, 0, vertexData,

                       vertexDataSize);

  WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(
      device,
      &(WGPUBindGroupDescriptor){
          .label = "bind group for object",
          .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
          .entries = (WGPUBindGroupEntry[]){{.binding = 0,
                                              .buffer = uniformBuffer,
                                              .size = uniformBufferSize,
                                              .offset = 0},},
          .entryCount = 1});

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
  float translation[NUM_OF_PRESETS][3] = {
    {150.0,100.0, 0},
    {200.0,300.0, 0},
    {500.0,40.0, 0},
    {700.0,1000.0, 0},
    {300.0,200.0, 0},
    {40.0,500.0, 0},
    {1000.0,700.0, 0},
    {80.0,20.0, 0},
    {10.0,250.0, 0},
    {1000.0,1000.0, 0},
  };

  float rotation[3] = {45,100,0};
  int rotation_axis = 0;
  int preset = 0;
  int count = 0;

  float scale[3] = {1.0,1.0,1.0};
  int scale_forward = 1;
  int scale_axis = 0;
  
  while (!glfwWindowShouldClose(window)) {
    // change translation preset after 250 loops (because making a GUI is not as easy outside of the web)
    if (count < 250) 
    {
        count++;
    }
    else {
        scale_axis = scale_axis < 2 ? scale_axis + 1 : 0; //switch scale axis every 250 loop
        rotation_axis = rotation_axis < 2 ? rotation_axis + 1 : 0; //switch scale axis every 250 loop
        count = 0;
        if (preset < NUM_OF_PRESETS + 1){
            preset++;
        }
        else {
            preset = 0;
        }
    }
    printf("translation: %f,%f,%f\n", translation[preset][0],translation[preset][1], translation[preset][2]);
    if (rotation[rotation_axis] < 360){
        rotation[rotation_axis] += 0.1;
    } else {
        rotation[rotation_axis] = -360;
    }
    printf("rotation: %f,%f,%f°\n", rotation[0], rotation[1], rotation[2]);

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
    printf("scale: %f,%f,%f\n", scale[0],scale[1], scale[2]);
    


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

    float tmp_matrix[16], tmp_matrix_2[16];
    projection_4x4_mat(window_width, window_height, 400, tmp_matrix);
    translate_4x4_mat(tmp_matrix, translation[preset][0], translation[preset][1], translation[preset][2], tmp_matrix_2);
    float tmp_matrix_3[16];
    rotateX(tmp_matrix_2, rotation[0], tmp_matrix);
    rotateY(tmp_matrix, rotation[1], tmp_matrix_2);
    rotateZ(tmp_matrix_2, rotation[2], tmp_matrix);
    scale_4x4_mat(tmp_matrix, scale[0], scale[1], scale[2], uniformValues.matrix);

    // upload the uniform values to the uniform buffer
    wgpuQueueWriteBuffer(queue, uniformBuffer, 0, &uniformValues,
                         sizeof(uniformValues));

    wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, NULL);
    wgpuRenderPassEncoderDraw(pass, numVertices, 1, 0, 0);
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