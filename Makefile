PROFILE := debug

c++ := g++
includes := -Isrc -Itarget/$(PROFILE)/deps/Vulkan-Hpp
common_flags := -std=c++17 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers $(includes) -lvulkan -lglfw
debug_flags := $(common_flags) -g -DDEBUG -D_DEBUG
release_flags := $(common_flags) -O3 -ffast-math -march=native -DNDEBUG

c = \e[35;1m
r = \e[0m

ifeq ($(PROFILE),debug)
flags := $(debug_flags)
else
flags := $(release_flags)
endif

build: download-deps
	@echo -e "    $(c)Building$(r) raytrace"
	@mkdir -p target/$(PROFILE)
	@$(c++) $(flags) -o target/$(PROFILE)/raytrace 	\
		src/app/app.cpp								\
		src/app/error.cpp							\
		src/app/instance.cpp						\
		src/app/window.cpp   						\
		src/main.cpp								\

run: build
	@echo -e "     $(c)Running$(r) raytrace"
	@target/$(PROFILE)/raytrace

download-deps: target/$(PROFILE)/deps/Vulkan-Hpp

target/$(PROFILE)/deps/Vulkan-Hpp:
	@echo -e " $(c)Downloading$(r) Vulkan-Hpp"
	@mkdir -p target/$(PROFILE)/deps
	@cd target/$(PROFILE)/deps && git clone https://github.com/KhronosGroup/Vulkan-Hpp

.PHONY: clean
clean:
	@rm -rf target
