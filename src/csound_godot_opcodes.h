#ifndef CSOUND_GODOT_OPCODES_H
#define CSOUND_GODOT_OPCODES_H

#include <plugin.h>

namespace csnd {

struct DictionarySetValue : csnd::Plugin<0, 2> {
    int kperf();
    static void load_plugin(Csound *csound);
};

/*
struct DictionaryGetValue : csnd::Plugin<1, 1> {
    int kperf();
    static void load_plugin(Csound *csound);
};
*/

struct StrToFile : csnd::Plugin<1, 3> {
    int kperf();
    static void load_plugin(Csound *csound);
};

} // namespace csnd

#endif
