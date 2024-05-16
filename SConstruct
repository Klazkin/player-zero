#!/usr/bin/env python
import os
import sys
import scons_compiledb

env = SConscript("godot-cpp/SConstruct")
scons_compiledb.enable(env)
# scons_compiledb.enable_with_cmdline(env)
extension_path = 'gaf6/bin/'

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(
    CPPPATH=[
        "src/",
        'thirdparty/onnxruntime-win-x64-gpu-1.17.1/include/'
        # 'thirdparty/onnxruntime-win-x64-1.16.3/include/'
        # 'thirdparty/onnxruntime-directml/build/native/include/'
    ],
    LIBPATH=[
        # 'thirdparty/onnxruntime-directml/runtimes/win-x64/native/'
        'thirdparty/onnxruntime-win-x64-gpu-1.17.1/lib/'
        # 'thirdparty/onnxruntime-win-x64-1.16.3/lib/'
    ],
    LIBS=[
        'onnxruntime',
        'onnxruntime_providers_cuda',
        'onnxruntime_providers_shared',
        'onnxruntime_providers_tensorrt'
    ]
)

sources = Glob("src/*.cpp") + Glob("src/*/*.cpp")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        extension_path + "libgdexample.{}.{}.framework/libgdexample.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        extension_path + "libgdexample{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

executable = env.Program(target=extension_path + 'standalone_runnable', source=sources)

Default(library)
Default(executable)
env.CompileDb()