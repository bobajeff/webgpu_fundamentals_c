struct OurVertexShaderOutput {
    @builtin(position) position: vec4<f32>,
};

@vertex fn vs(
@builtin(vertex_index) vertexIndex : u32
) -> OurVertexShaderOutput {
var pos = array<vec2<f32>, 3>(
    vec2<f32>( 0.0,  0.5),  // top center
    vec2<f32>(-0.5, -0.5),  // bottom left
    vec2<f32>( 0.5, -0.5)   // bottom right
);

var vsOutput: OurVertexShaderOutput;
vsOutput.position = vec4<f32>(pos[vertexIndex], 0.0, 1.0);
return vsOutput;
}

@fragment fn fs(fsInput: OurVertexShaderOutput) -> @location(0) vec4<f32> {
    let red = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    let cyan = vec4<f32>(0.0, 1.0, 1.0, 1.0);

    let grid = vec2<u32>(fsInput.position.xy) / u32(8);
    let checker = (grid.x + grid.y) % u32(2) == u32(1);

    return select(red, cyan, checker);
}
