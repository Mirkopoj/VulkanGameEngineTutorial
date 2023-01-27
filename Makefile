CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include 
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib $(shell pkgconf --static --libs glfw3) -lvulkan

vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))
SRCS = $(shell find -type f -name "*.cpp")
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

TARGET = VulkanTest
$(TARGET): $(vertObjFiles) $(fragObjFiles)
$(TARGET): $(OBJS)
	g++ $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
%.o: %.cpp
	g++ $(CFLAGS) -c $< -o $@ $(LDFLAGS)

%.spv: %
	glslc $< -o $@

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest
	rm -f *.spv
	rm -f *.o
