# webgpu fundamentals c
> **Note:** I'm freezing the wgpu version at `v0.17.0.2` because the [webgpu native API](https://github.com/webgpu-native/webgpu-headers) is still very much in flux and it's alot of work to update all of the examples for breaking changes.

## following along
The examples in `basics` and `3d_math` folder are following the guide found here: https://webgpufundamentals.org/

Since things change (and likely changed while working on this) [the guide from this revision](https://github.com/gfxfundamentals/webgpufundamentals/tree/d0a945b86f5541ae98368b2d904ba0d10dc77fd2) should be the similar to what I was following.

Unlike the guide I did not use any GUI library (too complicated). Instead I built demos that, to my understanding, demonstrated what the GUI was showing.

## correctness
Also, at this time I have not verified if my results match the guides since none of my browsers support webgpu on my hardware yet. I would not be surprised if at least a few of them are off somehow.

## works on my machine...
The code has been tested on Kubuntu 23.04, Linux Mint 21.1 and Linux Mint 21.3. I hope it works on other machines though 😁.

## building
### get the source
```
git clone git@github.com:bobajeff/webgpu_fundamentals_c.git
```
### get the dependecies
* wgpu-native v0.17.0.2 (required) -
    * Download the [wgpu-native v0.17.0.2](https://github.com/gfx-rs/wgpu-native/releases/tag/v0.17.0.2) build
    * extract in source directory 
    * rename folder to `wgpu`
* ffmpeg (optional - only one example uses it)
    * get the ffmpeg libraries for your system. For ubuntu based systems the command is: `sudo apt-get install pkg-config libavdevice-dev libavfilter-dev libavformat-dev`


### build the project
In source directory run:
```
cmake -B build
cmake --build build
```

### running examples
Run the executables found in their respective build subdirectories or run the `test_build.sh` script

# commentary
I had a lot of joy playing with this. It seems that porting from javascript to c is not such a bad idea. I had a much better time than doing something [very similar](https://github.com/bobajeff/learn_webgpu_c_1) in porting some C++ to C. I mostly just wanted to learn the API but It was good to get my hands wet with GPU GL programming and familiarize myself with 3d graphics concepts again.

Thanks to webgpufundamentals.org for providing a easy to follow guide and helping to explain how these things work.
