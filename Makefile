.PHONY: build
build:
	mkdir -p build
	cd build && cmake ..
	cd build && make -j12
	cd build && ./test
