CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib $(shell pkgconf --static --libs glfw3) -lvulkan

vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))

TARGET = VulkanTest
$(TARGET): $(vertObjFiles) $(fragObjFiles)
$(TARGET): *.cpp *.hpp
	g++ $(CFLAGS) -o $(TARGET) *.cpp $(LDFLAGS)

%.spv: %
	glslc $< -o $@

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest
	rm -f *.spv
