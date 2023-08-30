struct Uniforms {
  color: vec4<f32>,
  matrix: mat3x3<f32>,
};
 
struct Vertex {
  @location(0) position: vec2<f32>,
};
 
struct VSOutput {
  @builtin(position) position: vec4<f32>,
};
 
@group(0) @binding(0) var<uniform> uni: Uniforms;
 
@vertex fn vs(vert: Vertex) -> VSOutput {
  var vsOut: VSOutput;

  let clipSpace = (uni.matrix * vec3<f32>(vert.position, 1.0)).xy;
 
  vsOut.position = vec4<f32>(clipSpace, 0.0, 1.0);
  return vsOut;
}
 
@fragment fn fs(vsOut: VSOutput) -> @location(0) vec4<f32> {
  return uni.color;
}