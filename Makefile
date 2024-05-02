CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include 
ifeq ($(DEBUG),1)
	CFLAGS := -g3 $(CFLAGS)
endif
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib $(shell pkgconf --static --libs glfw3) -lvulkan -limgui

vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))
compSources = $(shell find ./shaders -type f -name "*.comp")
compObjFiles = $(patsubst %.comp, %.comp.spv, $(compSources))
SRCS = $(shell find -type f -name "*.cpp" -not -path "*/mains/*")
OBJS = $(patsubst ./%.cpp, obj/%.o, $(SRCS))
MAINOUTS = FirstApp SecondApp

$(MAINOUTS): $(OBJS) $(vertObjFiles) $(fragObjFiles) $(compObjFiles)
	@mkdir -p bin
	@mkdir -p obj/mains
	g++ $(CFLAGS) -c mains/$@.cpp -o obj/mains/$@.o
	g++ $(CFLAGS) -o bin/$@ obj/mains/$@.o $(OBJS) $(LDFLAGS)
obj/%.o: %.cpp
	@mkdir -p $(@D)
	g++ $(CFLAGS) -c $< -o $@ 

%.spv: %
	glslc $< -o $@

.PHONY: test clean

test1: FirstApp
	bin/FirstApp

test2: SecondApp
	bin/SecondApp

clean:
	rm -rf bin/
	rm -f shaders/*.spv
	rm -rf obj/
