struct OurStruct {
    color: vec4<f32>,
    offset: vec2<f32>
};

struct OtherStruct {
    scale: vec2<f32>,
};

struct Vertex {
    @location(0) position: vec2<f32>,
    @location(1) color: vec3<f32>,
};

struct VSOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
};

@group(0) @binding(0) var<storage, read> ourStructs: array<OurStruct>;
@group(0) @binding(1) var<storage, read> otherStructs: array<OtherStruct>;

@vertex fn vs(
vert: Vertex,
@builtin(instance_index) instanceIndex : u32
) -> VSOutput {

    let otherStruct = otherStructs[instanceIndex];
    let ourStruct = ourStructs[instanceIndex];

    var vsOut: VSOutput;
    vsOut.position = vec4<f32>(vert.position * otherStruct.scale + ourStruct.offset, 0.0, 1.0);
    vsOut.color = ourStruct.color * vec4<f32>(vert.color, 1.0);
    return vsOut;
}

@fragment fn fs(vsOut: VSOutput) -> @location(0) vec4<f32> {
    return vsOut.color;
}