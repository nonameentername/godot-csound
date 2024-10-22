#include "csound_godot.h"
#include <csound_godot_opcodes.h>
#include <fstream>
#include <modload.h>
#include <plugin.h>
#include <string>

namespace godot {
class CsoundGodot;
}

namespace csnd {

int DictionarySetValue::kperf() {
    if (in_count() < 2) {
        csound->message("You did not pass have enough arguments to DictionarySetValue\n");
        return NOTOK;
    }

    MYFLT value = inargs[-1];
    MYFLT key = inargs[0];

    godot::CsoundGodot *csound_godot = (godot::CsoundGodot *)csound->host_data();
    csound_godot->set_value(key, value);

    return OK;
}

void DictionarySetValue::load_plugin(Csound *csound) {
    csnd::plugin<DictionarySetValue>(csound, "DictionarySetValue.ii", "", "ii", csnd::thread::i);
    csnd::plugin<DictionarySetValue>(csound, "DictionarySetValue.kk", "", "kk", csnd::thread::k);
}

int StrToFile::kperf() {

    int mode = 0;
    if (in_count() < 2) {
        csound->message("You did not pass have enough arguments to strtofile\n");
        return NOTOK;
    }

    char *inString = inargs.str_data(0).data;
    char *fileName = inargs.str_data(1).data;

    if (in_count() > 2)
        mode = inargs[2];

    std::ofstream fileStream;

    if (mode == 1) {
        fileStream.open(fileName, std::ios::in | std::ios::app);
    } else
        fileStream.open(fileName, std::ios::in | std::ios::trunc);

    fileStream << inString;

    if (!fileStream.is_open()) {
        csound->message("*** strToFile could not open file for writing ***");
        outargs[0] = 0;
    }

    fileStream.close();
    outargs[0] = 1;
    return OK;
}

void StrToFile::load_plugin(Csound *csound) {
    csnd::plugin<StrToFile>(csound, "strToFile.SSO", "k", "SSO", csnd::thread::k);
}

} // namespace csnd

inline void csnd::on_load(Csound *csound) {
    StrToFile::load_plugin(csound);
}
