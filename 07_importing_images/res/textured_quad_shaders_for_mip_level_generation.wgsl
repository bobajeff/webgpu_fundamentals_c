struct VSOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) texcoord: vec2<f32>,
};

@vertex fn vs(
    @builtin(vertex_index) vertexIndex : u32
) -> VSOutput {
    var pos = array<vec2<f32>, 6>(

        vec2<f32>( 0.0,  0.0),  // center
        vec2<f32>( 1.0,  0.0),  // right, center
        vec2<f32>( 0.0,  1.0),  // center, top

        // 2st triangle
        vec2<f32>( 0.0,  1.0),  // center, top
        vec2<f32>( 1.0,  0.0),  // right, center
        vec2<f32>( 1.0,  1.0),  // right, top
    );

    var vsOutput: VSOutput;
    let xy = pos[vertexIndex];
    vsOutput.position = vec4<f32>(xy * 2.0 - 1.0, 0.0, 1.0);
    vsOutput.texcoord = vec2<f32>(xy.x, 1.0 - xy.y);
    return vsOutput;
}

@group(0) @binding(0) var ourSampler: sampler;
@group(0) @binding(1) var ourTexture: texture_2d<f32>;

@fragment fn fs(fsInput: VSOutput) -> @location(0) vec4<f32> {
    return textureSample(ourTexture, ourSampler, fsInput.texcoord);
}