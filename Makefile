CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include 
ifeq ($(DEBUG),1)
	CFLAGS := -g3 $(CFLAGS)
endif
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib $(shell pkgconf --static --libs glfw3) -lvulkan -limgui

vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))
SRCS = $(shell find -type f -name "*.cpp")
OBJS = $(patsubst ./%.cpp, obj/%.o, $(SRCS))

TARGET = VulkanTest
$(TARGET): $(vertObjFiles) $(fragObjFiles)
$(TARGET): $(OBJS)
	g++ $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
obj/%.o: %.cpp
	@mkdir -p $(@D)
	g++ $(CFLAGS) -c $< -o $@ $(LDFLAGS)

%.spv: %
	glslc $< -o $@

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest
	rm -f shaders/*.spv
	rm -rf obj/
