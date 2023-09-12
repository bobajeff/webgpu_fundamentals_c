# webgpu fundamentals c

## following along
The examples in `basics` and `3d_math` folder are following the guide found here: https://webgpufundamentals.org/

Since things change (and likely changed while working on this) [the guide from this revision](https://github.com/gfxfundamentals/webgpufundamentals/tree/d0a945b86f5541ae98368b2d904ba0d10dc77fd2) should be the similar to what I was following.

Unlike the guide I did not use any GUI library (too complicated). Instead I built demos that, to my understanding, demonstrated what the GUI was showing.

## correctness
Also, at this time I have not verified if my results match the guides since none of my browsers support webgpu on my hardware yet. I would not be surprised if at least a few of them are off somehow.

## works on my machine...
The code has been tested on Linux Mint 21.1 with this binary release of [wgpu](https://github.com/gfx-rs/wgpu-native/releases/tag/v0.17.0.2) with this revision of [glfw](https://github.com/glfw/glfw/tree/3eaf1255b29fdf5c2895856c7be7d7185ef2b241) with this revision of [cglm](https://github.com/recp/cglm/tree/509078817c1917867fde87ab9c3ade6ae12a4f48) and this build for [ffmpeg](https://github.com/BtbN/FFmpeg-Builds/releases/tag/autobuild-2023-07-31-12-50) (only one example uses it if present). I hope it works on other machines though 😁.

## building
### get the source
```
git clone git@github.com:bobajeff/webgpu_fundamentals_c.git
```
### get the dependecies
* wgpu-native (required)
    * Download [wgpu-native build](https://github.com/gfx-rs/wgpu-native/releases)
    * extract in source directory 
    * rename folder to `wgpu`
* ffmpeg (optional)
    * Download [ffmpeg build](https://github.com/BtbN/FFmpeg-Builds/releases) - **IMPORTANT: make sure to get one that has 'shared' in the name.** (eg. ffmpeg-N-111661-g567e78b283-linux64-lgpl-shared.tar.xz)
    * extract in source directory
    * rename folder to `ffmpeg`

### build the project
In source directory run:
```
cmake -B build
cmake --build build
```

### running examples
Run the executables found in their respective build subdirectories

# commentary
I had a lot of joy playing with this. It seems that porting from javascript to c is not such a bad idea. I had a much better time than doing something [very similar](https://github.com/bobajeff/learn_webgpu_c_1) in porting some C++ to C. I mostly just wanted to learn the API but It was good to get my hands wet with GPU GL programming and familiarize myself with 3d graphics concepts again.

Thanks to webgpufundamentals.org for providing a easy to follow guide and helping to explain how these things work.
