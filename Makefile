PROFILE := debug

c = \e[35;1m
r = \e[0m

ifeq ($(PROFILE),debug)
flags := $(debug_flags)
else
flags := $(release_flags)
endif

build: build-deps build-shaders cmake
	@echo -e "    $(c)Building$(r) raytrace"
	@make --dir=target/$(PROFILE)

run: build
	@echo -e "     $(c)Running$(r) raytrace"
	@target/$(PROFILE)/raytrace

cmake: target/$(PROFILE)/Makefile

target/debug/Makefile:
	@mkdir -p target/$(PROFILE)
	@cmake -Btarget/$(PROFILE) -H. -DCMAKE_DEBUG_BUILD=ON -DCMAKE_INSTALL_PREFIX=$(shell pwd)/target/$(PROFILE)

target/release/Makefile:
	@mkdir -p target/$(PROFILE)
	@cmake -Btarget/$(PROFILE) -H. -DCMAKE_DEBUG_BUILD=OFF -DCMAKE_INSTALL_PREFIX=$(shell pwd)/target/$(PROFILE)

build-shaders: shader/comp.spv

shader/comp.spv: shader/main.comp
	@glsllangValidator -V shader/main.comp -o shader/comp.spv

build-deps: target/$(PROFILE)/include/vulkan

target/$(PROFILE)/include/vulkan: target/deps/Vulkan-Hpp
	@mkdir -p target/$(PROFILE)/include
	@cp -r target/deps/Vulkan-Hpp/vulkan target/$(PROFILE)/include/vulkan

target/deps/Vulkan-Hpp:
	@echo -e " $(c)Downloading$(r) Vulkan-Hpp"
	@mkdir -p target/deps/Vulkan-Hpp
	@git clone https://github.com/KhronosGroup/Vulkan-Hpp target/deps/Vulkan-Hpp

.PHONY: clean
clean:
	@rm -rf target
