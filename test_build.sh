#!/usr/bin/bash

# triangle from wgpu-native examples folder
timeout 5 build/triangle_/triangle

# basics
timeout 5 build/basics/01_fundamentals/webgpu-simple-compute 
timeout 5 build/basics/01_fundamentals/webgpu-simple-triangle 
timeout 5 build/basics/02_inter-stage-variables/webgpu-fragment-shader-builtin-position 
timeout 5 build/basics/02_inter-stage-variables/webgpu-inter-stage-variables-triangle 
timeout 5 build/basics/02_inter-stage-variables/webgpu-inter-stage-variables-triangle-by-fn-param 
timeout 5 build/basics/03_uniforms/webgpu-simple-triangle-uniforms 
timeout 5 build/basics/03_uniforms/webgpu-simple-triangle-uniforms-multiple 
timeout 5 build/basics/03_uniforms/webgpu-simple-triangle-uniforms-split 
timeout 5 build/basics/04_storage-buffers/webgpu-simple-triangle-storage-buffer-split 
timeout 5 build/basics/04_storage-buffers/webgpu-simple-triangle-storage-split-minimal-changes 
timeout 5 build/basics/04_storage-buffers/webgpu-storage-buffer-vertices 
timeout 5 build/basics/05_vertex-buffers/webgpu-vertex-buffers 
timeout 5 build/basics/05_vertex-buffers/webgpu-vertex-buffers-2-attributes 
timeout 5 build/basics/05_vertex-buffers/webgpu-vertex-buffers-2-attributes-8bit-colors 
timeout 5 build/basics/05_vertex-buffers/webgpu-vertex-buffers-2-buffers 
timeout 5 build/basics/05_vertex-buffers/webgpu-vertex-buffers-index-buffer 
timeout 5 build/basics/06_textures/webgpu-simple-textured-quad 
timeout 5 build/basics/06_textures/webgpu-simple-textured-quad-linear 
timeout 5 build/basics/06_textures/webgpu-simple-textured-quad-minfilter 
timeout 5 build/basics/06_textures/webgpu-simple-textured-quad-mipmap 
timeout 5 build/basics/06_textures/webgpu-simple-textured-quad-mipmapfilter 
timeout 5 build/basics/07_importing_images/webgpu-simple-textured-quad-import 
timeout 5 build/basics/07_importing_images/webgpu-simple-textured-quad-import-no-mips 
if test -f build/basics/07_importing_images/webgpu-simple-textured-quad-import-video; then
  timeout 5 build/basics/07_importing_images/webgpu-simple-textured-quad-import-video
fi

# 3d_math
timeout 5 build/3d_math/01_translation/webgpu-translation 
timeout 5 build/3d_math/01_translation/webgpu-translation-prep 
timeout 5 build/3d_math/02_rotation/webgpu-rotation 
timeout 5 build/3d_math/02_rotation/webgpu-rotation-via-unit-circle 
timeout 5 build/3d_math/03_scale/webgpu-scale 
timeout 5 build/3d_math/04_matrix_math/webgpu-matrix-math-transform-five-fs-3x3 
timeout 5 build/3d_math/04_matrix_math/webgpu-matrix-math-transform-just-matrix-3x3 
timeout 5 build/3d_math/04_matrix_math/webgpu-matrix-math-transform-move-origin-3x3 
timeout 5 build/3d_math/04_matrix_math/webgpu-matrix-math-transform-srt-3x3 
timeout 5 build/3d_math/04_matrix_math/webgpu-matrix-math-transform-trs 
timeout 5 build/3d_math/04_matrix_math/webgpu-matrix-math-transform-trs-3x3 
timeout 5 build/3d_math/05_orthographic_projection/webgpu-orthographic-projection-step-1-flat-f 
timeout 5 build/3d_math/05_orthographic_projection/webgpu-orthographic-projection-step-2-3d-f 
timeout 5 build/3d_math/05_orthographic_projection/webgpu-orthographic-projection-step-3-colored-3d-f 
timeout 5 build/3d_math/05_orthographic_projection/webgpu-orthographic-projection-step-4-cullmode-back 
timeout 5 build/3d_math/05_orthographic_projection/webgpu-orthographic-projection-step-5-order-fixed 
timeout 5 build/3d_math/05_orthographic_projection/webgpu-orthographic-projection-step-6-depth-texture 
timeout 5 build/3d_math/05_orthographic_projection/webgpu-orthographic-projection-step-7-ortho 
timeout 5 build/3d_math/06_perspective_projection/webgpu-perspective-projection-step-1-fudge-factor 
timeout 5 build/3d_math/06_perspective_projection/webgpu-perspective-projection-step-2-gpu-divide-by-w 
timeout 5 build/3d_math/06_perspective_projection/webgpu-perspective-projection-step-3-perspective-z-to-w 
timeout 5 build/3d_math/06_perspective_projection/webgpu-perspective-projection-step-4-perspective 
timeout 5 build/3d_math/07_cameras/webgpu-cameras-step-1-direct-math 
timeout 5 build/3d_math/07_cameras/webgpu-cameras-step-2-camera-aim 
timeout 5 build/3d_math/07_cameras/webgpu-cameras-step-3-look-at 
timeout 5 build/3d_math/07_cameras/webgpu-cameras-step-4-aim-Fs

# compute_shaders
timeout 5 build/compute_shaders/01_compute_shaders_basics/webgpu-compute-shaders-builtins