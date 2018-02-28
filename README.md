## Clip

Example OpenGL project that clips a cube using a plane.

To compile this, you will need a `deps` folder. I'm doing something a little unusual here, so that I can quickly create prototype projects. My deps folder has the subdirectories `chipmunk`, `glad`, `portaudio`, `sdl2` and `stb`. Under `stb` I dumped the whole [stb repo](https://github.com/nothings/stb). Under `glad` I dumped the `c` branch of the [glad repo](https://github.com/Dav1dde/glad/tree/c). The `chipmunk`, `sdl2` and `portaudio` projects are structured with `include` and `lib` folders. Under the `lib` folders are subdirectories `macos`, `linux` and `win32` which contain the static libs to link for their respective platforms.

I know that's a pain, but it ended up being better for me than trying to manage git sub-repos.

The project uses cmake, so all you have to do once you set up the `deps` folder is:

```
mkdir build
cd build
cmake ..
make
```

