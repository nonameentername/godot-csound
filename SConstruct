#!/usr/bin/env python

libname = "libgdmidiplayer"

env = SConscript("godot-cpp/SConstruct")

csound_library = "csound64"
env.Append(LIBS=[csound_library])
env.Append(LIBPATH=['bin/csound/lib'])
env.Append(CPPPATH=["bin/csound/include/csound"])
env.Append(RPATH=['bin/csound/lib', '.'])

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

if env["platform"] == "macos":
    platlibname = "{}.{}.{}".format(libname, env["platform"], env["target"])
    library = env.SharedLibrary(
        "bin/{}.framework/{}".format(platlibname, platlibname),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "bin/{}{}{}".format(libname, env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
