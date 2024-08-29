#!/usr/bin/env python
import os
import sys


def normalize_path(val, env):
    return val if os.path.isabs(val) else os.path.join(env.Dir("#").abspath, val)


def validate_parent_dir(key, val, env):
    if not os.path.isdir(normalize_path(os.path.dirname(val), env)):
        raise UserError("'%s' is not a directory: %s" % (key, os.path.dirname(val)))


libname = "libcsoundgodot"
projectdir = "."

if sys.platform == "windows":
    localEnv = Environment(tools=["mingw"], PLATFORM="")
else:
    localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Add(
    BoolVariable(
        key="compiledb",
        help="Generate compilation DB (`compile_commands.json`) for external tools",
        default=localEnv.get("compiledb", False),
    )
)
opts.Add(
    PathVariable(
        key="compiledb_file",
        help="Path to a custom `compile_commands.json` file",
        default=localEnv.get("compiledb_file", "compile_commands.json"),
        validator=validate_parent_dir,
    )
)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()
env["compiledb"] = False

env.Tool("compilation_db")
compilation_db = env.CompilationDatabase(
    normalize_path(localEnv["compiledb_file"], localEnv)
)
env.Alias("compiledb", compilation_db)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

libraries = [
    "sndfile",
    "vorbis",
    "FLAC++",
    "FLAC",
    "mp3lame",
    "mpg123",
    "ogg",
    "opus",
    "samplerate",
    "vorbisenc",
    "vorbisfile"
]

if env["platform"] == "web":
    csound_library = "csound"
    env.Append(LIBS=[csound_library] + libraries)
elif env["platform"] == "macos":
    csound_library = "CsoundLib64"
elif env["platform"] == "ios":
    csound_library = "CsoundLib"
    env.Append(CPPFLAGS=["-stdlib=libc++"])
    env.Append(LINKFLAGS=["-stdlib=libc++"])
elif env["platform"] == "android":
    csound_library = "csound"
    env.Append(LIBS=[csound_library])
elif env["platform"] == "windows":
    csound_library = "csound64"
    env.Append(LIBS=[csound_library] + libraries + ["shlwapi", "ssp"])
else:
    csound_library = "csound64"
    env.Append(LIBS=[csound_library])

if env["platform"] == "windows":
    if env["dev_build"]:
        env.Append(LIBPATH=["addons/csound/bin/windows/debug/lib", "modules/csound/build/mingw/debug/vcpkg_installed/x64-mingw-static/lib"])
        env.Append(CPPPATH=["addons/csound/bin/windows/debug/include/csound", "modules/csound/build/mingw/debug/vcpkg_installed/x64-mingw-static/include"])
    else:
        env.Append(LIBPATH=["addons/csound/bin/windows/release/lib", "modules/csound/build/mingw/release/vcpkg_installed/x64-mingw-static/lib"])
        env.Append(CPPPATH=["addons/csound/bin/windows/release/include/csound", "modules/csound/build/mingw/release/vcpkg_installed/x64-mingw-static/include"])
    env.Append(CPPFLAGS=["-DMINGW"])
elif env["platform"] == "web":
    if env["dev_build"]:
        env.Append(LIBPATH=["addons/csound/bin/web/debug/lib", "modules/csound/build/web/debug/vcpkg_installed/wasm32-emscripten/lib"])
        env.Append(CPPPATH=["addons/csound/bin/web/debug/include/csound", "modules/csound/build/web/debug/vcpkg_installed/wasm32-emscripten/include"])
        
        env.Append(CPPFLAGS=["-g"])
        env.Append(LINKFLAGS=["-g", "-s", "ERROR_ON_UNDEFINED_SYMBOLS=1"])
    else:
        env.Append(LIBPATH=["addons/csound/bin/web/release/lib", "modules/csound/build/web/release/vcpkg_installed/wasm32-emscripten/lib"])
        env.Append(CPPPATH=["addons/csound/bin/web/release/include/csound", "modules/csound/build/web/release/vcpkg_installed/wasm32-emscripten/include"])
elif env["platform"] == "linux":
    if env["dev_build"]:
        env.Append(LIBPATH=["addons/csound/bin/linux/debug/lib"])
        env.Append(RPATH=["addons/csound/bin/linux/debug/lib", "."])
        env.Append(CPPPATH=["addons/csound/bin/linux/debug/include/csound"])
    else:
        env.Append(LIBPATH=["addons/csound/bin/linux/release/lib"])
        env.Append(RPATH=["addons/csound/bin/linux/release/lib", "."])
        env.Append(CPPPATH=["addons/csound/bin/linux/release/include/csound"])
elif env["platform"] == "macos":
    if env["dev_build"]:
        env.Append(LIBPATH=["modules/csound/build/osxcross/debug/vcpkg_installed/univeral-osxcross/lib"])
        env.Append(CPPPATH=["modules/csound/build/osxcross/debug/vcpkg_installed/univeral-osxcross/include"])

        env.Append(LINKFLAGS=["-framework", csound_library])
        env.Append(LINKFLAGS=["-F", "addons/csound/bin/macos/debug/Library/Frameworks"])
        env.Append(LINKFLAGS=["-rpath", "@loader_path/../debug/Library/Frameworks", "-rpath", "@executable_path/../Frameworks"])
        env.Append(CPPPATH=["addons/csound/bin/macos/debug/Library/Frameworks/CsoundLib64.framework/Headers"])
    else:
        env.Append(LIBPATH=["modules/csound/build/osxcross/release/vcpkg_installed/univeral-osxcross/lib"])
        env.Append(CPPPATH=["modules/csound/build/osxcross/release/vcpkg_installed/univeral-osxcross/include"])

        env.Append(LINKFLAGS=["-framework", csound_library])
        env.Append(LINKFLAGS=["-F", "addons/csound/bin/macos/release/Library/Frameworks"])
        env.Append(LINKFLAGS=["-rpath", "@loader_path/../release/Library/Frameworks", "-rpath", "@executable_path/../Frameworks"])
        env.Append(CPPPATH=["addons/csound/bin/macos/release/Library/Frameworks/CsoundLib64.framework/Headers"])
elif env["platform"] == "ios":
    app_name = "csoundgodot.app"
    prefix = "sim_" if env["ios_simulator"] else ""
    if env["dev_build"]:
        env.Append(LIBPATH=["modules/csound/build/ioscross-universal/debug/vcpkg_installed/univeral-ioscross/lib"])
        env.Append(CPPPATH=["modules/csound/build/ioscross-universal/debug/vcpkg_installed/univeral-ioscross/include"])

        env.Append(LINKFLAGS=["-framework", csound_library])
        env.Append(LINKFLAGS=["-F", f"addons/csound/bin/ios/{prefix}debug/Library/Frameworks"])
        env.Append(LINKFLAGS=["-rpath", f"@loader_path/../{prefix}debug/Library/Frameworks", "-rpath", f"@executable_path/{app_name}/Frameworks"])
        env.Append(CPPPATH=[f"addons/csound/bin/ios/{prefix}debug/Library/Frameworks/CsoundLib.framework/Headers"])
    else:
        env.Append(LIBPATH=["modules/csound/build/ioscross-universal/release/vcpkg_installed/univeral-ioscross/lib"])
        env.Append(CPPPATH=["modules/csound/build/ioscross-universal/release/vcpkg_installed/univeral-ioscross/include"])

        env.Append(LINKFLAGS=["-framework", csound_library])
        env.Append(LINKFLAGS=["-F", f"addons/csound/bin/ios/{prefix}release/Library/Frameworks"])
        env.Append(LINKFLAGS=["-rpath", f"@loader_path/../{prefix}release/Library/Frameworks", "-rpath", f"@executable_path/{app_name}/Frameworks"])
        env.Append(CPPPATH=[f"addons/csound/bin/ios/{prefix}release/Library/Frameworks/CsoundLib.framework/Headers"])
elif env["platform"] == "android":
    arch_map = { "arm32": "arm", "arm64": "arm64", "x86_32": "x86", "x86_64": "x64" }
    arch = arch_map[env["arch"]]
    if env["dev_build"]:
        env.Append(LIBPATH=[f"addons/csound/bin/android-{arch}/debug/lib", f"modules/csound/build/android-{arch}/debug/vcpkg_installed/{arch}-android/lib"])
        env.Append(CPPPATH=[f"addons/csound/bin/android-{arch}/debug/include/csound", f"modules/csound/build/android-{arch}/debug/vcpkg_installed/{arch}-android/include"])
    else:
        env.Append(LIBPATH=[f"addons/csound/bin/android-{arch}/release/lib", f"modules/csound/build/android-{arch}/release/vcpkg_installed/{arch}-android/lib"])
        env.Append(CPPPATH=[f"addons/csound/bin/android-{arch}/release/include/csound", f"modules/csound/build/android-{arch}/release/vcpkg_installed/{arch}-android/include"])

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

file = "{}{}{}".format(libname, env["suffix"], env["SHLIBSUFFIX"])

if env["platform"] == "macos":
    platlibname = "{}.{}.{}".format(libname, env["platform"], env["target"])
    file = "{}.framework/{}".format(env["platform"], platlibname, platlibname)

libraryfile = "addons/csound/bin/{}/{}".format(env["platform"], file)
library = env.SharedLibrary(
    libraryfile,
    source=sources,
)

#copy = env.InstallAs("{}/bin/{}/lib{}".format(projectdir, env["platform"], file), library)

default_args = [library]
if localEnv.get("compiledb", False):
    default_args += [compilation_db]
Default(*default_args)
