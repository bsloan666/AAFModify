
PACKAGE_NAME=AAF-src-1.2.0-DR1
AAFLIB_ROOT=./$(PACKAGE_NAME)

# These may require some customization...
CXX=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
LD=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
MACOSX_SDK=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk


all: aafmodify

aafmodify: aafmodify.cpp
	$(CXX) -x c++ -arch x86_64 -fmessage-length=259 -fdiagnostics-show-note-include-stack -fmacro-backtrace-limit=0 \
	-fcolor-diagnostics -Wno-trigraphs -fpascal-strings -O0 -Wno-missing-field-initializers -Wno-missing-prototypes \
	-Wno-return-type -Wno-non-virtual-dtor -Wno-overloaded-virtual -Wno-exit-time-destructors -Wno-missing-braces \
	-Wparentheses -Wswitch -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-variable -Wunused-value \
	-Wno-empty-body -Wno-uninitialized -Wno-unknown-pragmas -Wno-shadow -Wno-four-char-constants -Wno-conversion \
	-Wno-constant-conversion -Wno-int-conversion -Wno-bool-conversion -Wno-enum-conversion -Wno-float-conversion \
	-Wno-non-literal-null-conversion -Wno-objc-literal-conversion -Wno-shorten-64-to-32 -Wno-newline-eof \
	-Wno-c++11-extensions \
	-isysroot $(MACOSX_SDK) \
	-fasm-blocks -fstrict-aliasing -Wdeprecated-declarations -Winvalid-offsetof -mmacosx-version-min=10.13 -g \
	-Wno-sign-conversion -Wno-infinite-recursion -Wno-move -Wno-comma -Wno-block-capture-autoreleasing -Wno-strict-prototypes \
	-Wno-range-loop-analysis -Wno-semicolon-before-method-body \
	-I$(AAFLIB_ROOT)/out/target/clang7-x86_64/Debug/Utilities/aafembed/include \
	-I$(AAFLIB_ROOT)/Utilities/aafembed -I$(AAFLIB_ROOT)/ref-impl/include \
	-I$(AAFLIB_ROOT)/ref-impl/include/ref-api \
	-c aafmodify.cpp \
	-o aafmodify.o
	$(LD) -arch x86_64 \
	-isysroot $(MACOSX_SDK) \
	-L./ \
	-F./ \
	-Xlinker -no_deduplicate -framework CoreFoundation -framework CoreServices -Wl,-search_paths_first -Wl,-headerpad_max_install_names \
	$(AAFLIB_ROOT)/out/target/clang7-x86_64/Debug/RefImpl/libaafiid.a \
	$(AAFLIB_ROOT)/out/target/clang7-x86_64/Debug/RefImpl/libaaflib.a \
	aafmodify.o \
	-o aafmodify

clean:
	rm -fv aafmodify.o aafmodify 
	rm -fv AAF-src-1.2.0-DR1.zip

clean_deps: clean
	rm -rfv AAF-src-1.2.0-DR1

deps:
	wget https://sourceforge.net/projects/aaf/files/AAF-src/1.2.0-DR1/$(PACKAGE_NAME).zip -O $(PACKAGE_NAME).zip
	unzip $(PACKAGE_NAME).zip
	cd $(PACKAGE_NAME)/ && \
	mkdir -p out/build && \
	cd out/build && \
    cmake -G "Xcode" -DPLATFORM=clang7 -DARCH=x86_64 ../.. && \
    cmake --build .

