CLANG_FORMAT = clang-format

.PHONY: build
test:
	mkdir -p build
	cd build && cmake ..
	cd build && make -j12
	cd build && ./test

.PHONY: clean
clean:
	rm -rf build/

.PHONY: format
format:
	$(CLANG_FORMAT) -i include/* src/*
