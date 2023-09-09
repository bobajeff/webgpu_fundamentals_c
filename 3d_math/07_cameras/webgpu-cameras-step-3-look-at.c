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

float degToRad(int d){ return d * M_PI / 180;};

void cross_vec3(float * a, float * b, float * dest) {

    float t0 = a[1] * b[2] - a[2] * b[1];
    float t1 = a[2] * b[0] - a[0] * b[2];
    float t2 = a[0] * b[1] - a[1] * b[0];

    dest[0] = t0;
    dest[1] = t1;
    dest[2] = t2;
};

void subtract_vec3(float * a, float * b, float * dest) {

    dest[0] = a[0] - b[0];
    dest[1] = a[1] - b[1];
    dest[2] = a[2] - b[2];

}

void normalize_vec3(float * v, float * dest) {

    double length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    // make sure we don't divide by 0.
    if (length > 0.00001) {
      dest[0] = v[0] / length;
      dest[1] = v[1] / length;
      dest[2] = v[2] / length;
    } else {
      dest[0] = 0;
      dest[1] = 0;
      dest[2] = 0;
    }

}

void cameraAim_vec3(float * eye, float * target, float * up, float * dest) {

    float tmp_vector[3];
    float zAxis[3];
    subtract_vec3(eye, target, tmp_vector);
    normalize_vec3(tmp_vector, zAxis);
    float xAxis[3];
    cross_vec3(up, zAxis, tmp_vector);
    normalize_vec3(tmp_vector, xAxis);
    float yAxis[3];
    cross_vec3(zAxis, xAxis, tmp_vector);
    normalize_vec3(tmp_vector, yAxis);

    dest[0]  = xAxis[0];  dest[1]  = xAxis[1];  dest[2]  = xAxis[2];  dest[3]  = 0;
    dest[4]  = yAxis[0];  dest[5]  = yAxis[1];  dest[6]  = yAxis[2];  dest[7]  = 0;
    dest[8]  = zAxis[0];  dest[9]  = zAxis[1];  dest[10] = zAxis[2];  dest[11] = 0;
    dest[12] = eye[0];    dest[13] = eye[1];    dest[14] = eye[2];    dest[15] = 1;

}

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

void inverse_4x4_mat(float * m, float * dest) {
    float m00 = m[0 * 4 + 0];
    float m01 = m[0 * 4 + 1];
    float m02 = m[0 * 4 + 2];
    float m03 = m[0 * 4 + 3];
    float m10 = m[1 * 4 + 0];
    float m11 = m[1 * 4 + 1];
    float m12 = m[1 * 4 + 2];
    float m13 = m[1 * 4 + 3];
    float m20 = m[2 * 4 + 0];
    float m21 = m[2 * 4 + 1];
    float m22 = m[2 * 4 + 2];
    float m23 = m[2 * 4 + 3];
    float m30 = m[3 * 4 + 0];
    float m31 = m[3 * 4 + 1];
    float m32 = m[3 * 4 + 2];
    float m33 = m[3 * 4 + 3];

    float tmp0 = m22 * m33;
    float tmp1 = m32 * m23;
    float tmp2 = m12 * m33;
    float tmp3 = m32 * m13;
    float tmp4 = m12 * m23;
    float tmp5 = m22 * m13;
    float tmp6 = m02 * m33;
    float tmp7 = m32 * m03;
    float tmp8 = m02 * m23;
    float tmp9 = m22 * m03;
    float tmp10 = m02 * m13;
    float tmp11 = m12 * m03;
    float tmp12 = m20 * m31;
    float tmp13 = m30 * m21;
    float tmp14 = m10 * m31;
    float tmp15 = m30 * m11;
    float tmp16 = m10 * m21;
    float tmp17 = m20 * m11;
    float tmp18 = m00 * m31;
    float tmp19 = m30 * m01;
    float tmp20 = m00 * m21;
    float tmp21 = m20 * m01;
    float tmp22 = m00 * m11;
    float tmp23 = m10 * m01;

    float t0 = (tmp0 * m11 + tmp3 * m21 + tmp4 * m31) -
               (tmp1 * m11 + tmp2 * m21 + tmp5 * m31);
    float t1 = (tmp1 * m01 + tmp6 * m21 + tmp9 * m31) -
               (tmp0 * m01 + tmp7 * m21 + tmp8 * m31);
    float t2 = (tmp2 * m01 + tmp7 * m11 + tmp10 * m31) -
               (tmp3 * m01 + tmp6 * m11 + tmp11 * m31);
    float t3 = (tmp5 * m01 + tmp8 * m11 + tmp11 * m21) -
               (tmp4 * m01 + tmp9 * m11 + tmp10 * m21);

    float d = 1 / (m00 * t0 + m10 * t1 + m20 * t2 + m30 * t3);

    dest[0] = d * t0;
    dest[1] = d * t1;
    dest[2] = d * t2;
    dest[3] = d * t3;

    dest[4] = d * ((tmp1 * m10 + tmp2 * m20 + tmp5 * m30) -
                  (tmp0 * m10 + tmp3 * m20 + tmp4 * m30));
    dest[5] = d * ((tmp0 * m00 + tmp7 * m20 + tmp8 * m30) -
                  (tmp1 * m00 + tmp6 * m20 + tmp9 * m30));
    dest[6] = d * ((tmp3 * m00 + tmp6 * m10 + tmp11 * m30) -
                  (tmp2 * m00 + tmp7 * m10 + tmp10 * m30));
    dest[7] = d * ((tmp4 * m00 + tmp9 * m10 + tmp10 * m20) -
                  (tmp5 * m00 + tmp8 * m10 + tmp11 * m20));

    dest[8] = d * ((tmp12 * m13 + tmp15 * m23 + tmp16 * m33) -
                  (tmp13 * m13 + tmp14 * m23 + tmp17 * m33));
    dest[9] = d * ((tmp13 * m03 + tmp18 * m23 + tmp21 * m33) -
                  (tmp12 * m03 + tmp19 * m23 + tmp20 * m33));
    dest[10] = d * ((tmp14 * m03 + tmp19 * m13 + tmp22 * m33) -
                   (tmp15 * m03 + tmp18 * m13 + tmp23 * m33));
    dest[11] = d * ((tmp17 * m03 + tmp20 * m13 + tmp23 * m23) -
                   (tmp16 * m03 + tmp21 * m13 + tmp22 * m23));

    dest[12] = d * ((tmp14 * m22 + tmp17 * m32 + tmp13 * m12) -
                   (tmp16 * m32 + tmp12 * m12 + tmp15 * m22));
    dest[13] = d * ((tmp20 * m32 + tmp12 * m02 + tmp19 * m22) -
                   (tmp18 * m22 + tmp21 * m32 + tmp13 * m02));
    dest[14] = d * ((tmp18 * m12 + tmp23 * m32 + tmp15 * m02) -
                   (tmp22 * m32 + tmp14 * m02 + tmp19 * m12));
    dest[15] = d * ((tmp22 * m22 + tmp16 * m02 + tmp21 * m12) -
                   (tmp20 * m12 + tmp23 * m22 + tmp17 * m02));
}

void perspective_4x4_mat( float fieldOfViewYInRadians, int aspect, float zNear, float zFar, float * dest) {
    float f = tan(M_PI * 0.5 - 0.5 * fieldOfViewYInRadians);
    float rangeInv = 1 / (zNear - zFar);

    dest[0] = f / aspect;
    dest[1] = 0.0;
    dest[2] = 0.0;
    dest[3] = 0.0;

    dest[4] = 0.0;
    dest[5] = f;
    dest[6] = 0.0;
    dest[7] = 0.0;

    dest[8] = 0.0;
    dest[9] = 0.0;
    dest[10] = zFar * rangeInv;
    dest[11] = -1.0;

    dest[12] = 0.0;
    dest[13] = 0.0;
    dest[14] = zNear * zFar * rangeInv;
    dest[15] = 0.0;

}

void ortho_4x4_mat(int left, int right, int bottom, int top, int near, int far, float * dest) {
    dest[0] = 2.0 / (right - left);
    dest[1] = 0.0;
    dest[2] = 0.0;
    dest[3] = 0.0;

    dest[4] = 0.0;
    dest[5] = 2.0 / (top - bottom);
    dest[6] = 0.0;
    dest[7] = 0.0;

    dest[8]  = 0.0;
    dest[9]  = 0.0;
    dest[10] = 1.0 / (near - far);
    dest[11] = 0.0;

    dest[12] = (float)(right + left) / (left - right);
    dest[13] = (float)(top + bottom) / (bottom - top);
    dest[14] = (float)near / (near - far);
    dest[15] = 1.0;
};

void projection_4x4_mat(int width, int height, int depth, float * dest) {
    // Note: This matrix flips the Y axis so that 0 is at the top.
    dest[0]  = 2.0 / width,  dest[1]  = 0.0,            dest[2]   = 0.0,          dest[3]  = 0.0,
    dest[4]  = 0.0,          dest[5]  = -2.0 / height,  dest[6]   = 0.0,          dest[7]  = 0.0,
    dest[8]  = 0.0,          dest[9]  = 0.0,            dest[10]  = 0.5 / depth,  dest[11] = 0,0,
    dest[12] = -1.0,         dest[13] = 1.0,            dest[14]  = 0.5,          dest[15] = 1.0;
    ortho_4x4_mat(0.0, width, height, 0, depth, -depth, dest);

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

void lookAt(float * eye, float * target, float * up, float * dest) {
    float tmp_matrix[16];
    cameraAim_vec3(eye, target, up, tmp_matrix);
    inverse_4x4_mat(tmp_matrix, dest);
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
      RESOURCE_DIR "shader.wgsl");
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
                  .cullMode = WGPUCullMode_Back
                  },
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
          .depthStencil = &(WGPUDepthStencilState){
            .nextInChain = NULL,
            .format = WGPUTextureFormat_Depth24Plus,
            .depthWriteEnabled = true,
            .depthCompare = WGPUCompareFunction_Less,
            .stencilFront = (WGPUStencilFaceState){
                .compare = WGPUCompareFunction_Never, // magick value needed 
                .failOp = WGPUStencilOperation_Keep,
                .depthFailOp = WGPUStencilOperation_Keep,
                .passOp = WGPUStencilOperation_Keep,
            },
            .stencilBack = (WGPUStencilFaceState){
                .compare = WGPUCompareFunction_Never, // magick value needed 
                .failOp = WGPUStencilOperation_Keep,
                .depthFailOp = WGPUStencilOperation_Keep,
                .passOp = WGPUStencilOperation_Keep,
            },
            .stencilReadMask = 0,
            .stencilWriteMask = 0,
            .depthBias = 0,
            .depthBiasSlopeScale = 0.0,
            .depthBiasClamp = 0.0
          },
      });

  
  #define numFs 5
  WGPUBuffer uniformBuffer[numFs];
  WGPUBindGroup bindGroup[numFs];
  Uniforms uniformValues = {};
  const uint64_t uniformBufferSize = (16) * 4;
  int i;
  for (i = 0; i < numFs; ++i) {
    // matrix
    uniformBuffer[i] = wgpuDeviceCreateBuffer(
        device, &(WGPUBufferDescriptor){.label = "uniforms",
                                        .size = uniformBufferSize,
                                        .usage = WGPUBufferUsage_Uniform |
                                                WGPUBufferUsage_CopyDst,
                                        .mappedAtCreation = false});

    bindGroup[i] = wgpuDeviceCreateBindGroup(
        device,
        &(WGPUBindGroupDescriptor){
            .label = "bind group for object",
            .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
            .entries = (WGPUBindGroupEntry[]){{.binding = 0,
                                                .buffer = uniformBuffer[i],
                                                .size = uniformBufferSize,
                                                .offset = 0},},
            .entryCount = 1});

  };


  // createFVertices()
  float positions[] = {
    // left column
     -50,  75,  15,
     -20,  75,  15,
     -50, -75,  15,
     -20, -75,  15,

    // top rung
     -20,  75,  15,
      50,  75,  15,
     -20,  45,  15,
      50,  45,  15,

    // middle rung
     -20,  15,  15,
      20,  15,  15,
     -20, -15,  15,
      20, -15,  15,

    // left column back
     -50,  75, -15,
     -20,  75, -15,
     -50, -75, -15,
     -20, -75, -15,

    // top rung back
     -20,  75, -15,
      50,  75, -15,
     -20,  45, -15,
      50,  45, -15,

    // middle rung back
     -20,  15, -15,
      20,  15, -15,
     -20, -15, -15,
      20, -15, -15,
  };

  u_int32_t indices[] = {
     0,  2,  1,    2,  3,  1,   // left column
     4,  6,  5,    6,  7,  5,   // top run
     8, 10,  9,   10, 11,  9,   // middle run

    12, 13, 14,   14, 13, 15,   // left column back
    16, 17, 18,   18, 17, 19,   // top run back
    20, 21, 22,   22, 21, 23,   // middle run back

     0,  5, 12,   12,  5, 17,   // top
     5,  7, 17,   17,  7, 19,   // top rung right
     6, 18,  7,   18, 19,  7,   // top rung bottom
     6,  8, 18,   18,  8, 20,   // between top and middle rung
     8,  9, 20,   20,  9, 21,   // middle rung top
     9, 11, 21,   21, 11, 23,   // middle rung right
    10, 22, 11,   22, 23, 11,   // middle rung bottom
    10,  3, 22,   22,  3, 15,   // stem right
     2, 14,  3,   14, 15,  3,   // bottom
     0, 12,  2,   12, 14,  2,   // left
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
  
  int radius = 200;

  int fieldOfView = 100;
  int cameraAngle = 0;
  
  WGPUTexture depthTexture = NULL;
  
  while (!glfwWindowShouldClose(window)) {
    if (cameraAngle < 360){
        cameraAngle += 1;
    } else {
        cameraAngle = -360;
    }
    printf("cameraAngle: %i\n", cameraAngle);


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

    // If we don't have a depth texture OR if its size is different
    // from the canvasTexture when make a new depth texture
    if (!depthTexture ||
        wgpuTextureGetWidth(depthTexture) != window_width ||
        wgpuTextureGetHeight(depthTexture) != window_height) {
      if (depthTexture) {
        wgpuTextureDestroy(depthTexture);
      }
      depthTexture = wgpuDeviceCreateTexture(device, &(WGPUTextureDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .usage = WGPUTextureUsage_RenderAttachment,
        .dimension = WGPUTextureDimension_2D,
        .size = (WGPUExtent3D){
            .width = window_width,
            .height = window_height,
            .depthOrArrayLayers = 1,
        },
        .format = WGPUTextureFormat_Depth24Plus,
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 0,
        .viewFormats = (WGPUTextureFormat[1]){WGPUTextureFormat_Undefined},
      });
    }
   
   WGPUTextureView depthStencilAttachment_view = wgpuTextureCreateView(depthTexture, NULL);

    WGPURenderPassDescriptor renderPassDescriptor = {
        .label = "our basic canvas renderPass",
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
        .depthStencilAttachment = &(WGPURenderPassDepthStencilAttachment){
            .view = depthStencilAttachment_view,
            .depthLoadOp = WGPULoadOp_Clear,
            .depthStoreOp = WGPUStoreOp_Store,
            .depthClearValue = 1.0,
            .depthReadOnly = false,
            .stencilLoadOp = WGPULoadOp_Clear, // magick value needed 
            .stencilStoreOp = WGPUStoreOp_Store, // magick value needed 
            .stencilClearValue = 0,
            .stencilReadOnly = false,
        },
    };

    // make a render pass encoder to encode render specific commands
    WGPURenderPassEncoder pass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescriptor);
    wgpuRenderPassEncoderSetPipeline(pass, pipeline);
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertexBuffer, 0,
                                         vertexDataSize);

    
    int aspect = window_width / window_height;
    float projection[16];
    perspective_4x4_mat(
        degToRad(fieldOfView), 
        aspect, 
        1,           // zNear
        2000,         // zFar
        projection
    );

    // Compute the position of the first F
    float fPosition[3] = {radius, 0, 0};

    // Use matrix math to compute a position on a circle where
    // the camera is
    float tmp_matrix[16], tmp_matrix_2[16];
    rotationY(degToRad(cameraAngle), tmp_matrix);
    translate_4x4_mat(tmp_matrix, 0, 0, radius * 1.5, tmp_matrix_2);

    // Get the camera's position from the matrix we computed
    float eye[3] = {tmp_matrix_2[12], tmp_matrix_2[13], tmp_matrix_2[14]};

    float up[3] = {0, 1, 0};

    // Make a view matrix from the camera matrix.
    float viewMatrix[16];
    lookAt(eye, fPosition, up, viewMatrix);

    // combine the view and projection matrixes
    float viewProjectionMatrix[16];
    multiply_4x4_mat(projection, viewMatrix, viewProjectionMatrix);
    for (i = 0; i < numFs; ++i) {
        int angle = (float)i / numFs * M_PI * 2;
        int x = cosf(angle) * radius;
        int z = sinf(angle) * radius;
        translate_4x4_mat(viewProjectionMatrix, x, 0, z, uniformValues.matrix);

        // upload the uniform values to the uniform buffer
        wgpuQueueWriteBuffer(queue, uniformBuffer[i], 0, &uniformValues,
                            sizeof(uniformValues));

        wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup[i], 0, NULL);
        wgpuRenderPassEncoderDraw(pass, numVertices, 1, 0, 0);
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