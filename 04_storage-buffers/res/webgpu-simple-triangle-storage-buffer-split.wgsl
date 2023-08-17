struct OurStruct {
    color: vec4<f32>,
    offset: vec2<f32>
};

struct OtherStruct {
    scale: vec2<f32>,
};

struct VSOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
}

@group(0) @binding(0) var<storage, read> ourStructs: array<OurStruct>;
@group(0) @binding(1) var<storage, read> otherStructs: array<OtherStruct>;

@vertex fn vs(
@builtin(vertex_index) vertexIndex : u32,
@builtin(instance_index) instanceIndex : u32
) -> VSOutput {
    var pos = array<vec2<f32>, 3>(
        vec2<f32>( 0.0,  0.5),  // top center
        vec2<f32>(-0.5, -0.5),  // bottom left
        vec2<f32>( 0.5, -0.5)   // bottom right
    );
    let otherStruct = otherStructs[instanceIndex];
    let ourStruct = ourStructs[instanceIndex];

    var vsOut: VSOutput;
    vsOut.position = vec4<f32>(pos[vertexIndex] * otherStruct.scale + ourStruct.offset, 0.0, 1.0);
    vsOut.color = ourStruct.color;
    return vsOut;
}

@fragment fn fs(vsOut: VSOutput) -> @location(0) vec4<f32> {
    return vsOut.color;
}