
CXX=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
LD=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
AAFLIB_ROOT=$(HOME)/swdevl/AAF-src-1.2.0-DR1
MACOSX_SDK=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk


all:
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
	-I$(AAFLIB_ROOT)/out/build/Utilities/aafembed/AAF-SDK.build/Debug/aafembed.build/DerivedSources/x86_64 \
	-I$(AAFLIB_ROOT)/out/build/Utilities/aafembed/AAF-SDK.build/Debug/aafembed.build/DerivedSources \
	-F$(AAFLIB_ROOT)/out/target/clang7-x86_64/Debug/Utilities/aafembed -D_DEBUG -MMD -MT dependencies \
	-MF $(AAFLIB_ROOT)/out/build/Utilities/aafembed/AAF-SDK.build/Debug/aafembed.build/Objects-normal/x86_64/aafembed.d \
	--serialize-diagnostics $(AAFLIB_ROOT)/out/build/Utilities/aafembed/AAF-SDK.build/Debug/aafembed.build/Objects-normal/x86_64/aafembed.dia \
	-c aafmodify.cpp \
	-o aafmodify.o
	$(LD) -arch x86_64 \
	-isysroot $(MACOSX_SDK) \
	-L./ \
	-F./ \
	-filelist $(AAFLIB_ROOT)/out/build/Utilities/aafembed/AAF-SDK.build/Debug/aafembed.build/Objects-normal/x86_64/aafembed.LinkFileList \
	-mmacosx-version-min=10.13 -Xlinker -object_path_lto \
	-Xlinker $(AAFLIB_ROOT)/out/build/Utilities/aafembed/AAF-SDK.build/Debug/aafembed.build/Objects-normal/x86_64/aafembed_lto.o \
	-Xlinker -no_deduplicate -framework CoreFoundation -framework CoreServices -Wl,-search_paths_first -Wl,-headerpad_max_install_names \
	$(AAFLIB_ROOT)/out/target/clang7-x86_64/Debug/RefImpl/libaafiid.a \
	$(AAFLIB_ROOT)/out/target/clang7-x86_64/Debug/RefImpl/libaaflib.a \
	-Xlinker -dependency_info \
	-Xlinker $(AAFLIB_ROOT)/out/build/Utilities/aafembed/AAF-SDK.build/Debug/aafembed.build/Objects-normal/x86_64/aafembed_dependency_info.dat \
	-o aafmodify

clean:
	rm -fv aafmodify.o aafmodify
