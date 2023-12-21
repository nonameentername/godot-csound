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

if sys.platform == 'windows':
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

csound_library = "csound64"

env.Append(LIBS=[csound_library])

if env["platform"] == "windows":
    env.Append(LIBPATH=['bin/csound/lib64'])
    env.Append(CPPPATH=["bin/csound/include/csound"])
    #env.Append(RPATH=['bin/csound/bin', '.'])
elif env["platform"] == "web":
    if env['dev_build']:
        env.Append(LIBPATH=['bin/web/debug/lib'])
        env.Append(CPPPATH=["bin/web/debug/include/csound"])
    else:
        env.Append(LIBPATH=['bin/web/release/lib'])
        env.Append(CPPPATH=["bin/web/release/include/csound"])
elif env["platform"] == "linux":
    if env['dev_build']:
        env.Append(LIBPATH=['bin/linux/debug/lib'])
        env.Append(RPATH=['bin/linux/debug/lib', '.'])
        env.Append(CPPPATH=["bin/linux/debug/include/csound"])
    else:
        env.Append(LIBPATH=['bin/linux/release/lib'])
        env.Append(RPATH=['bin/linux/release/lib', '.'])
        env.Append(CPPPATH=["bin/linux/release/include/csound"])

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

file = "{}{}{}".format(libname, env["suffix"], env["SHLIBSUFFIX"])

if env["platform"] == "macos":
    platlibname = "{}.{}.{}".format(libname, env["platform"], env["target"])
    file = "{}.framework/{}".format(env["platform"], platlibname, platlibname)

libraryfile = "bin/{}/{}".format(env["platform"], file)
library = env.SharedLibrary(
    libraryfile,
    source=sources,
)

#copy = env.InstallAs("{}/bin/{}/lib{}".format(projectdir, env["platform"], file), library)

default_args = [library]
if localEnv.get("compiledb", False):
    default_args += [compilation_db]
Default(*default_args)
