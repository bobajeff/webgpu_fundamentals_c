struct Uniforms {
  color: vec4<f32>,
  resolution: vec2<f32>,
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

  // Multiply by a matrix
  let position = (uni.matrix * vec3<f32>(vert.position, 1.0)).xy;
 
  // convert the position from pixels to a 0.0 to 1.0 value
  let zeroToOne = position / uni.resolution;
 
  // convert from 0 <-> 1 to 0 <-> 2
  let zeroToTwo = zeroToOne * 2.0;
 
  // covert from 0 <-> 2 to -1 <-> +1 (clip space)
  let flippedClipSpace = zeroToTwo - 1.0;
 
  // flip Y
  let clipSpace = flippedClipSpace * vec2<f32>(1.0, -1.0);
 
  vsOut.position = vec4<f32>(clipSpace, 0.0, 1.0);
  return vsOut;
}
 
@fragment fn fs(vsOut: VSOutput) -> @location(0) vec4<f32> {
  return uni.color;
}