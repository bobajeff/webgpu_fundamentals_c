struct OurVertexShaderOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>
};

@vertex fn vs(
@builtin(vertex_index) vertexIndex : u32
) -> OurVertexShaderOutput {
var pos = array<vec2<f32>, 3>(
    vec2<f32>( 0.0,  0.5),  // top center
    vec2<f32>(-0.5, -0.5),  // bottom left
    vec2<f32>( 0.5, -0.5)   // bottom right
);
var color = array<vec4<f32>, 3>(
    vec4<f32>(1.0, 0.0, 0.0, 1.0), // red
    vec4<f32>(0.0, 1.0, 0.0, 1.0), // green
    vec4<f32>(0.0, 0.0, 1.0, 1.0) // blue
);

var vsOutput: OurVertexShaderOutput;
vsOutput.position = vec4<f32>(pos[vertexIndex], 0.0, 1.0);
vsOutput.color = color[vertexIndex];
return vsOutput;
}

@fragment fn fs(@location(0) color: vec4<f32>) -> @location(0) vec4<f32> {
    return color;
}
