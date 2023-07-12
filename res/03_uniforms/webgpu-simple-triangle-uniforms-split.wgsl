struct OurStruct {
    color: vec4<f32>,
    offset: vec2<f32>
};

struct OtherStruct {
    scale: vec2<f32>,
};

@group(0) @binding(0) var<uniform> ourStruct: OurStruct;
@group(0) @binding(1) var<uniform> otherStruct: OtherStruct;


@vertex fn vs(
@builtin(vertex_index) vertexIndex : u32
) -> @builtin(position) vec4<f32> {
    var pos = array<vec2<f32>, 3>(
        vec2<f32>( 0.0,  0.5),  // top center
        vec2<f32>(-0.5, -0.5),  // bottom left
        vec2<f32>( 0.5, -0.5)   // bottom right
    );

    return vec4<f32>(pos[vertexIndex] * otherStruct.scale + ourStruct.offset, 0.0, 1.0);
}

@fragment fn fs() -> @location(0) vec4<f32> {
    return ourStruct.color;
}