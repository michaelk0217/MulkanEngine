# MulkanEngine (Work In Progress)

## 🚀 Overview

### What is it?
- MulkanEngine is a fundamental 3D real-time rendering engine built from scratch using the VulkanSDK. Currently it establishes a robust Vulkan rendering pipeline, manages essential resources, and provides interactive camera control to navigate a basic scene. The next major mileston einvolves implementing comprehensive scene management and loading more complex 3D models.

## Current Focus:
- The immediate focus is on creating a solid foundation for real-time graphics, emphasizing correct Vulkan usage, robust resource management, and interactive camera movement.

## Key Achieved Features:
- **Core Vulkan Instance & Device Setup**: Initialization of Vulkan API, physical/logical device selection, queue family management.
- **Swapchain Management**: Dynamic swapchain creation, recreation on window resize.
- **Render Pass & Framebuffer Setup**: Basic rendering pipeline with clear attachments.
- **Graphics Pipeline Creation**: Vertex and fragment shader compilation, input assembly, rasterization, depth testing.
- **Resource Management**: Vertex/index buffers, uniform buffers.
- **Descriptor Set Management**: Basic descriptor set layouts and updates for camera uniforms.
- **Interactive Camera System**: Free-look camera (first-person/fly-through) with keyboard movement and mouse look, supporting perspective projection.
- **Basic Primitive rendering**: Rendering Mesh + Texture objects.
- **Validation Layer Integration**: Robust error checking during development.
- **Rendering multiple objects using dynamic uniform buffers**: enables efficient rendering of multiple objects without duplications.


## Showcase / Visuals

## Technologies Used

- **Graphics API**: VulkanSDK (version)
- **Language**: C++17, GLSL
- **Windowing/Input**: GLFW
- **Math Library**: GLM
- **Debugging/Validation**: Vulkan Validation Layers
- **Build System**:
- **Texture Loading**: `stb_image`
- **Model Loading**: `tinyobjloader`

## 📦 Getting Started

## 🏗️ Architecture (Early Insights)

> **High-level Overview**:
>> The `VulkanEngine` class (currently in `main.cpp`) is the highest level component that wraps all the core features together.

>> Under `VulkanEngine` there is the `VulkanRenderer` class and `Camera` class, which as the name describes, the `VulkanRenderer` uses Vulkan Wrapper Classes/Helpers to build the rendering system, and the `Camera` takes in input from `GLFW` windows.

> **Vulkan Wrapper Classes/Helpers**:
>> Helps manage lifetimes and simplify Vulkan API calls

> **Input Handling**:
>> The user mouse and keyboard input is processed through the `GLFW` window under the `Window` class, where it takes user polls and passes it into the `Camera` object, passing the changes of the movement into the buffers.

> **Planned Scene Integration**:


## 🛣️ Roadmap

### Immediate Next Steps:
1. [x] Core Vulkan SDK Integration
2. [x] Model/Texture Loading
3. [x] Camera Movement
3. [x] Basic Scene Management (Multiple Objects)
4. [ ] Basic Lighting


### Future Features:
- Physically Based Rendering (PBR)
- Shadow Mapping (Cascaded Shadow Maps)
- Post-Processing Effects (Bloom, Tone Mapping, etc)
- Custom Material System
- Compute Shaders for simulations (e.g., particle systems)
- Debugging Tools & UI - `Dear ImGui` integration
- Performance Profiling & Optimization
- Ray Tracing

## 📜 License

## 🧑‍💻 Author
- [Michael Kim](https://www.linkedin.com/in/michaeltk217/)


