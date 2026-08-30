CLANG_FORMAT = clang-format
CLANG_TIDY = clang-tidy
SOURCES := $(wildcard include/iron_query/* src/*.cpp src/impl/* tests/*)
SANITIZE_FLAGS = -fsanitize=address,undefined

.PHONY: build
test: format-check
	mkdir -p build
	cd build && cmake ..
	cd build && make -j12
	cd build && ./iron-query-gtest

.PHONY: sanitize
sanitize:
	mkdir -p build-sanitize
	cd build-sanitize && cmake -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="$(SANITIZE_FLAGS) -fno-omit-frame-pointer" \
		-DCMAKE_EXE_LINKER_FLAGS="$(SANITIZE_FLAGS)" ..
	cd build-sanitize && make -j12
	cd build-sanitize && ./iron-query-gtest

.PHONY: docs
docs:
	doxygen Doxyfile

.PHONY: clean
clean:
	rm -rf build/ build-sanitize/

.PHONY: format
format:
	$(CLANG_FORMAT) -i $(SOURCES)

.PHONY: format-check
format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES)

.PHONY: tidy
tidy:
	mkdir -p build
	cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
	$(CLANG_TIDY) -p build $(filter %.cpp,$(SOURCES))
