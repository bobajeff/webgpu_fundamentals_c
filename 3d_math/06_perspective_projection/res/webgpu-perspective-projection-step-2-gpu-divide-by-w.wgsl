struct Uniforms {
  matrix: mat4x4<f32>,
  fudgeFactor: f32,
};
 
struct Vertex {
  @location(0) position: vec4<f32>,
  @location(1) color: vec4<f32>,
};
 
struct VSOutput {
  @builtin(position) position: vec4<f32>,
  @location(0) color: vec4<f32>,
};
 
@group(0) @binding(0) var<uniform> uni: Uniforms;
 
@vertex fn vs(vert: Vertex) -> VSOutput {
  var vsOut: VSOutput;

  let position = uni.matrix * vert.position;

  let zToDivideBy = 1.0 + position.z * uni.fudgeFactor;

  vsOut.position = vec4f(position.xyz, zToDivideBy);

  vsOut.color = vert.color;
  return vsOut;
}
 
@fragment fn fs(vsOut: VSOutput) -> @location(0) vec4<f32> {
  return vsOut.color;
}