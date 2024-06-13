CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include \
			-Inativefiledialog-extended/src/include \
			$(shell pkg-config --cflags gtkmm-3.0)
ifeq ($(DEBUG),1)
	CFLAGS := -g3 $(CFLAGS)
endif
ifdef NFD
	CFLAGS := -DNFD
endif
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib -Lnativefiledialog-extended/build/src \
			 $(shell pkgconf --static --libs glfw3) \
			 $(shell pkg-config --libs gtkmm-3.0) \
			 -lvulkan -limgui -lnfd

vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))
compSources = $(shell find ./shaders -type f -name "*.comp")
compObjFiles = $(patsubst %.comp, %.comp.spv, $(compSources))
SRCS = $(shell find -type f -name "*.cpp" -not -path "*/mains/*" -not -path "*/nativefiledialog-extended/*")
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
	bin/FirstApp $(ARGS)

test2: SecondApp
	bin/SecondApp $(ARGS)

clean:
	rm -rf bin/
	rm -f shaders/*.spv
	rm -rf obj/
