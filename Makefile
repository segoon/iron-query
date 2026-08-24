CLANG_FORMAT = clang-format

.PHONY: build
test:
	mkdir -p build
	cd build && cmake ..
	cd build && make -j12
	cd build && ./test

.PHONY: sanitize
sanitize:
	mkdir -p build-sanitize
	cd build-sanitize && cmake -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
		-DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" ..
	cd build-sanitize && make -j12
	cd build-sanitize && ./test

.PHONY: clean
clean:
	rm -rf build/ build-sanitize/

.PHONY: format
format:
	$(CLANG_FORMAT) -i include/* src/*
