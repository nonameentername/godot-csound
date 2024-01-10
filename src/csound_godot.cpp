#include "csound_godot.h"
#include "csound_server.h"
#include "godot_cpp/classes/audio_server.hpp"
#include "godot_cpp/classes/time.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

CsoundGodot::CsoundGodot() {
    csound = NULL;
    initialized = false;
    active = false;
}

CsoundGodot::~CsoundGodot() {
    if (csound != NULL) {
        delete csound;
        csound = NULL;
    }
}

void CsoundGodot::_ready() {
    finished = false;
    csound = new Csound();

    csound->CreateMessageBuffer(0);
    csound->SetDebug(true);
    csound->SetHostImplementedAudioIO(1, 0);

    // csound->SetHostImplementedMIDIIO(true);
    // csound->SetExternalMidiWriteCallback(write_midi_data);
    // csound->SetExternalMidiReadCallback(read_midi_data);

    set_process_internal(true);

    start();
}

void CsoundGodot::start() {
    // csound->Compile("test.csd");

    const char *csd_text = R"CSD(
<CsoundSynthesizer>
<CsOptions>
-+rtmidi=NULL -M0 --midi-key-cps=4 --midi-velocity-amp=5 -n
</CsOptions>
<CsInstruments>

sr = 44100
ksmps = 32
nchnls = 2
0dbfs = 1

chnset 1, "cutoff"

;load soundfonts
;isf	sfload	"assets/FluidR3_GM.sf2"
	;sfplist isf
	;sfplist ir
;	sfpassign	0, isf	

instr 1	; play guitar from score and midi keyboard - preset index = 0
    prints "hello world\n"

	mididefault	60, p3
	midinoteonkey	p4, p5
inum	init	p4
ivel	init	p5
ivel	init	ivel/127					;make velocity dependent
kamp	linsegr	1, 1, 1, .1, 0
kamp	= kamp/3000						;scale amplitude
kfreq	init	1						;do not change freq from sf
kCutoff chnget "cutoff"
;a1,a2	sfplay3	ivel, inum, kamp*ivel*kCutoff, kfreq, 0			;preset index = 0
a1 noise 1, 0
a2 noise 1, 0
;a2 = oscils kamp*ivel*kCutoff, 440, 0
    prints "instr1 inum = %f ivel = %f\n", inum, ivel
    chnset a1, "instr_1_left"
    chnset a2, "instr_1_right"
	outs	a1, a2
	
endin
	
instr 2	; play harpsichord from score and midi keyboard - preset index = 1

	mididefault	60, p3
	midinoteonkey	p4, p5
inum	init	p4
ivel	init	p5
ivel	init	ivel/127					;make velocity dependent
kamp	linsegr	1, 1, 1, .1, 0
kamp	= kamp/2000						;scale amplitude
kfreq	init	1						;do not change freq from sf
kCutoff chnget "cutoff"
;a1,a2	sfplay3	ivel, inum, kamp*ivel*kCutoff, kfreq, 3			;preset index = 1
a1 noise 1, 0
a2 noise 1, 0
kSig1 downsamp a1
kSig2 downsamp a2
    prints "instr2 inum = %f ivel = %f\n", inum, ivel
    printks "instr2 kamp = %f kCutoff = %f\n", 1, kamp, kCutoff
    printks "instr2 a1 = %f a2 = %f\n", 1, kSig1, kSig2
    chnset a1, "instr_2_left"
    chnset a2, "instr_2_right"
	outs	a1, a2
	
endin

instr 3
	mididefault	60, p3
	midinoteonkey	p4, p5
inum	init	p4
ivel	init	p5
ivel	init	ivel/127
kamp	linsegr	1, 1, 1, .1, 0
kCutoff chnget "cutoff"
al  lfo kamp, p4, 0
a1  chnget "instr_3_input_left"
a2  chnget "instr_3_input_right"

kSig1 downsamp a1
kSig2 downsamp a2

a1  = a1 * ivel * al * kCutoff
a2  = a2 * ivel * al * kCutoff
    prints "instr3 inum = %f ivel = %f\n", inum, ivel
    printks "instr3 kamp = %f kCutoff = %f\n", 1, kamp, kCutoff
    printks "instr3 a1 = %f a2 = %f\n", 1, kSig1, kSig2
    chnset a1, "instr_3_left"
    chnset a2, "instr_3_right"
	outs	a1, a2
endin


instr 4
    a1  chnget "instr_4_input_left"
    a2  chnget "instr_4_input_right"
    chnset a1, "instr_4_left"
    chnset a2, "instr_4_right"
endin


instr 5
    a1, a2 inch 1, 2
    outs	a1, a2
endin


</CsInstruments>
<CsScore>
f0 3600

;i1 0 1 60 100
;i1 + 1 62 <
;i1 + 1 65 <
;i1 + 1 69 10

;i2 5 1 60 100
;i2 + 1 62 <
;i2 7 1 65 <
;i2 7 1 69 10

;i3 0 3600

i4 0 3600
i5 0 3600


</CsScore>
</CsoundSynthesizer>
)CSD";

    csound->CompileCsdText(csd_text);
    csound->Start();

    if (output_channels.size() != csound->GetNchnls()) {
        input_channels.resize(csound->GetNchnls());
        output_channels.resize(csound->GetNchnls());

        int p_frames = 512;
        for (int j = 0; j < csound->GetNchnls(); j++) {
            input_channels.write[j].resize(p_frames);
            output_channels.write[j].buffer.resize(p_frames);

            for (int frame = 0; frame < p_frames; frame++) {
                input_channels.write[j].write[frame] = 0;
                output_channels.write[j].buffer.write[frame] = 0;
            }
        }
    }

    initialized = true;
}

void CsoundGodot::stop() {
    initialized = false;

    if (csound != NULL) {
        csound->Stop();
    }
}

void CsoundGodot::reset() {
    if (csound != NULL) {
        csound->Reset();
    }
    input_channels.clear();
    output_channels.clear();
    input_named_channels_buffer.clear();
    output_named_channels.clear();
    named_channels.clear();
}

void CsoundGodot::set_soundfont(Ref<SoundFontFileReader> p_soundfont) {
    soundfont = p_soundfont;

    /*
    if (ResourceLoader::get_singleton()->exists(node_path)) {
        Variant resource = ResourceLoader::get_singleton()->load(node_path);
        Ref<SoundFontFileReader> soundfont = resource;
        if (soundfont != NULL) {
        }
    }
    */
}

Ref<SoundFontFileReader> CsoundGodot::get_soundfont() {
    return soundfont;
}

void CsoundGodot::set_midi_file(Ref<MidiFileReader> p_midi_file) {
    midi_file = p_midi_file;
}

Ref<MidiFileReader> CsoundGodot::get_midi_file() {
    return midi_file;
}

void CsoundGodot::initialize_channels(int p_frames) {
    if (!initialized) {
        return;
    }

    controlChannelInfo_t *tmp;
    int num_channels = csound->ListChannels(tmp);

    for (int channel = 0; channel < num_channels; channel++) {
        char *name = tmp[channel].name;
        int type = tmp[channel].type;
        if ((type & CSOUND_CHANNEL_TYPE_MASK) == CSOUND_AUDIO_CHANNEL &&
            (type & (CSOUND_INPUT_CHANNEL | CSOUND_OUTPUT_CHANNEL))) {
            if (!input_named_channels_buffer.has(name)) {
                Vector<MYFLT> channel_buffer;
                channel_buffer.resize(p_frames);
                input_named_channels_buffer.insert(name, channel_buffer);
            }
            if (!named_channels.has(name)) {
                int index = output_named_channels.size();
                output_named_channels.resize(index + 1);
                output_named_channels.write[index].buffer.resize(p_frames);

                for (int frame = 0; frame < p_frames; frame++) {
                    output_named_channels.write[index].buffer.write[frame] = 0;
                }

                output_named_channels.write[index].name = name;
                named_channels.insert(name, index);
            }
        }
    }
}

int CsoundGodot::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    if (!initialized) {
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].left = 0;
            p_buffer[frame].right = 0;
        }
        return p_frames;
    }

    active = true;

    if (Time::get_singleton()) {
        last_mix_time = Time::get_singleton()->get_ticks_usec();
    }

    last_mix_frames = p_frames;

    int buffer_index = 0;
    int to_fill = p_frames;

    MYFLT *spin = csound->GetSpin();
    MYFLT *spout = csound->GetSpout();
    MYFLT scale = csound->Get0dBFS();

    initialize_channels(p_frames);
    if (temp_buffer.size() != csound->GetKsmps()) {
        temp_buffer.resize(csound->GetKsmps());
    }

    int ksmps_buffer_index = 0;

    float volume = godot::UtilityFunctions::db_to_linear(volume_db);

    if (CsoundServer::get_singleton()->get_solo_mode()) {
        if (!solo) {
            volume = 0.0;
        }
    } else {
        if (mute) {
            volume = 0.0;
        }
    }

    Vector<float> channel_peak;
    channel_peak.resize(output_channels.size());
    for (int i = 0; i < output_channels.size(); i++) {
        channel_peak.write[i] = 0;
    }
    Vector<float> named_channel_peak;
    named_channel_peak.resize(output_named_channels.size());
    for (int i = 0; i < output_named_channels.size(); i++) {
        named_channel_peak.write[i] = 0;
    }

    while (to_fill > 0) {
        for (int frame = 0; frame < csound->GetKsmps(); frame++) {
            for (int channel = 0; channel < csound->GetNchnls(); channel++) {
                spin[frame * csound->GetNchnls() + channel] =
                    input_channels[channel][ksmps_buffer_index + frame] * scale;
                input_channels.write[channel].write[ksmps_buffer_index + frame] = 0;
            }
        }

        for (KeyValue<String, Vector<MYFLT>> &E : input_named_channels_buffer) {
            for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                temp_buffer.write[frame] = input_named_channels_buffer.get(E.key)[ksmps_buffer_index + frame] * scale;
                input_named_channels_buffer.get(E.key).write[ksmps_buffer_index + frame] = 0;
            }
            csound->SetAudioChannel(E.key.utf8().get_data(), temp_buffer.ptrw());
        }

        int result = csound->PerformKsmps();
        if (result == 1) {
            finished = true;
        }

        for (int i = 0; i < csound->GetKsmps() * csound->GetNchnls(); i = i + csound->GetNchnls()) {
            if (bypass) {
                p_buffer[buffer_index].left = spin[i] * scale;
                p_buffer[buffer_index].right = spin[i + 1] * scale;
            } else {
                p_buffer[buffer_index].left = spout[i] / scale * volume;
                p_buffer[buffer_index].right = spout[i + 1] / scale * volume;
            }

            buffer_index = buffer_index + 1;
            to_fill = to_fill - 1;
        }

        if (bypass) {
            for (int channel = 0; channel < csound->GetNchnls(); channel++) {
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    output_channels.write[channel].buffer.write[ksmps_buffer_index + frame] =
                        input_channels[channel][ksmps_buffer_index + frame];
                }
            }
        } else {
            for (int channel = 0; channel < csound->GetNchnls(); channel++) {
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    float value = csound->GetSpoutSample(frame, channel) / scale * volume;
                    float p = ABS(value);
                    if (p > channel_peak[channel]) {
                        channel_peak.write[channel] = p;
                    }
                    output_channels.write[channel].buffer.write[ksmps_buffer_index + frame] = value;
                }
            }
        }

        if (bypass) {
            for (int channel = 0; channel < output_named_channels.size(); channel++) {
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    output_named_channels.write[channel].buffer.write[ksmps_buffer_index + frame] = 0;
                }
            }
        } else {
            for (int channel = 0; channel < output_named_channels.size(); channel++) {
                csound->GetAudioChannel(output_named_channels[channel].name.utf8().get_data(), temp_buffer.ptrw());
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    float value = temp_buffer[frame] / scale * volume;
                    float p = ABS(value);
                    if (p > named_channel_peak[channel]) {
                        named_channel_peak.write[channel] = p;
                    }
                    output_named_channels.write[channel].buffer.write[ksmps_buffer_index + frame] = value;
                }
            }
        }

        ksmps_buffer_index += csound->GetKsmps();
    }

    for (int i = 0; i < output_channels.size(); i++) {
        output_channels.write[i].peak_volume =
            godot::UtilityFunctions::linear_to_db(channel_peak[i] + AUDIO_PEAK_OFFSET);

        if (channel_peak[i] > 0) {
            output_channels.write[i].active = true;
        } else {
            output_channels.write[i].active = false;
        }
    }

    for (int i = 0; i < output_named_channels.size(); i++) {
        output_named_channels.write[i].peak_volume =
            godot::UtilityFunctions::linear_to_db(named_channel_peak[i] + AUDIO_PEAK_OFFSET);

        if (named_channel_peak[i] > 0) {
            output_named_channels.write[i].active = true;
        } else {
            output_named_channels.write[i].active = false;
        }
    }

    return p_frames;
}

void CsoundGodot::set_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right) {
    bool has_left_channel = left >= 0 && left < input_channels.size();
    bool has_right_channel = right >= 0 && right < input_channels.size();

    if (!has_left_channel && !has_right_channel) {
        return;
    }

    for (int frame = 0; frame < p_frames; frame += 1) {
        if (has_left_channel) {
            input_channels.write[left].write[frame] = p_buffer[frame].left;
        }
        if (has_right_channel) {
            input_channels.write[right].write[frame] = p_buffer[frame].right;
        }
    }
}

int CsoundGodot::get_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right) {
    bool has_left_channel =
        left >= 0 && left < output_channels.size() && p_frames <= output_channels[left].buffer.size();
    bool has_right_channel =
        right >= 0 && right < output_channels.size() && p_frames <= output_channels[right].buffer.size();

    for (int frame = 0; frame < p_frames; frame += 1) {
        if (has_left_channel && active) {
            p_buffer[frame].left = output_channels[left].buffer[frame];
        } else {
            p_buffer[frame].left = 0;
        }
        if (has_right_channel && active) {
            p_buffer[frame].right = output_channels[right].buffer[frame];
        } else {
            p_buffer[frame].right = 0;
        }
    }

    return p_frames;
}

void CsoundGodot::set_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left,
                                           String right) {
    bool has_left_channel = input_named_channels_buffer.has(left);
    bool has_right_channel = input_named_channels_buffer.has(right);

    if (!has_left_channel && !has_right_channel) {
        return;
    }

    for (int frame = 0; frame < p_frames; frame += 1) {
        if (has_left_channel) {
            input_named_channels_buffer.getptr(left)->write[frame] = p_buffer[frame].left;
        }
        if (has_right_channel) {
            input_named_channels_buffer.getptr(right)->write[frame] = p_buffer[frame].right;
        }
    }
}

int CsoundGodot::get_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left, String right) {
    bool has_left_channel = named_channels.has(left);
    bool has_right_channel = named_channels.has(right);

    for (int frame = 0; frame < p_frames; frame += 1) {
        if (has_left_channel && active) {
            p_buffer[frame].left = output_named_channels[named_channels[left]].buffer[frame];
        } else {
            p_buffer[frame].left = 0;
        }
        if (has_right_channel && active) {
            p_buffer[frame].right = output_named_channels[named_channels[right]].buffer[frame];
        } else {
            p_buffer[frame].right = 0;
        }
    }

    return p_frames;
}

void CsoundGodot::program_select(int chan, int bank_num, int preset_num) {
}

void CsoundGodot::note_on(int chan, int key, int vel) {
    if (!initialized) {
        return;
    }

    float instrnum = chan + chan / 100.0 + key / 100000.0;
    String note_on = vformat("i%f 0 -1 %d %d", instrnum, key, vel);
    csound->InputMessage(note_on.ascii().get_data());
}

void CsoundGodot::note_off(int chan, int key) {
    if (!initialized) {
        return;
    }

    float instrnum = chan + chan / 100.0 + key / 100000.0;
    String note_off = vformat("i-%f 0 0 %d", instrnum, key);
    csound->InputMessage(note_off.ascii().get_data());
}

void CsoundGodot::send_control_channel(String channel, float value) {
    if (!initialized) {
        return;
    }

    csound->SetControlChannel(channel.ascii().get_data(), value);
}

void CsoundGodot::pitch_bend(int chan, int val) {
}

void CsoundGodot::play_midi(Ref<MidiFileReader> p_midi_file) {
    Ref<MidiFileReader> local_midi_file;

    if (p_midi_file != NULL) {
        if (ResourceLoader::get_singleton()->exists(p_midi_file->get_path())) {
            Variant resource = ResourceLoader::get_singleton()->load(p_midi_file->get_path());
            local_midi_file = resource;
        }
    } else {
        local_midi_file = midi_file;
    }

    if (local_midi_file != NULL) {
        PackedByteArray byte_array = local_midi_file->get_data();
        if (byte_array.size() > 0) {
        }
    }
}

void CsoundGodot::_notification(int p_what) {
    switch (p_what) {
    case NOTIFICATION_INTERNAL_PROCESS: {
        process(get_process_delta_time());
    }
    }
}

void CsoundGodot::process(double delta) {
    if (!initialized) {
        return;
    }

    if (active) {
        double total = get_time_since_last_mix();
        double mix_buffer = 2 * last_mix_frames / AudioServer::get_singleton()->get_mix_rate();
        double time_to_future_mix = mix_buffer - total;

        if (time_to_future_mix < 0) {
            active = false;

            for (int channel = 0; channel < input_channels.size(); channel++) {
                for (int frame = 0; frame < last_mix_frames; frame++) {
                    input_channels.write[channel].write[frame] = 0;
                }
            }

            for (int channel = 0; channel < output_channels.size(); channel++) {
                output_channels.write[channel].active = false;
                output_channels.write[channel].peak_volume = AUDIO_MIN_PEAK_DB;

                for (int frame = 0; frame < last_mix_frames; frame++) {
                    output_channels.write[channel].buffer.write[frame] = 0;
                }
            }

            for (KeyValue<String, Vector<MYFLT>> &E : input_named_channels_buffer) {
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    input_named_channels_buffer.get(E.key).write[frame] = 0;
                }
            }

            for (int channel = 0; channel < output_named_channels.size(); channel++) {
                output_named_channels.write[channel].active = false;
                output_named_channels.write[channel].peak_volume = AUDIO_MIN_PEAK_DB;

                for (int frame = 0; frame < last_mix_frames; frame++) {
                    output_named_channels.write[channel].buffer.write[frame] = 0;
                }
            }
        }
    }

    for (int i = 0; i < csound->GetMessageCnt(); i++) {
        godot::UtilityFunctions::printraw(csound->GetFirstMessage());
        csound->PopFirstMessage();
    }
}

int CsoundGodot::write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes) {
    return 0;
}

int CsoundGodot::read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes) {
    return 0;
}

void CsoundGodot::set_csound_name(const String &name) {
    csound_name = name;
}

const String &CsoundGodot::get_csound_name() {
    return csound_name;
}

int CsoundGodot::get_channel_count() {
    if (csound != NULL) {
        return csound->GetNchnls();
    } else {
        return 2;
    }
}

int CsoundGodot::get_named_channel_count() {
    if (csound != NULL) {
        return output_named_channels.size();
    } else {
        return 0;
    }
}

String CsoundGodot::get_named_channel_name(int p_channel) {
    ERR_FAIL_INDEX_V(p_channel, output_named_channels.size(), "");

    return output_named_channels[p_channel].name;
}

double CsoundGodot::get_time_since_last_mix() {
    return (Time::get_singleton()->get_ticks_usec() - last_mix_time) / 1000000.0;
}

double CsoundGodot::get_time_to_next_mix() {
    double total = get_time_since_last_mix();
    double mix_buffer = last_mix_frames / AudioServer::get_singleton()->get_mix_rate();
    return mix_buffer - total;
}

void CsoundGodot::_bind_methods() {
    ClassDB::bind_method(D_METHOD("process", "delta"), &CsoundGodot::process);
    ClassDB::bind_method(D_METHOD("program_select"), &CsoundGodot::program_select);
    ClassDB::bind_method(D_METHOD("note_on"), &CsoundGodot::note_on);
    ClassDB::bind_method(D_METHOD("note_off"), &CsoundGodot::note_off);
    ClassDB::bind_method(D_METHOD("send_control_channel"), &CsoundGodot::send_control_channel);
    ClassDB::bind_method(D_METHOD("pitch_bend"), &CsoundGodot::pitch_bend);
    ClassDB::bind_method(D_METHOD("play_midi"), &CsoundGodot::play_midi);
    ClassDB::bind_method(D_METHOD("set_soundfont", "soundfont"), &CsoundGodot::set_soundfont);
    ClassDB::bind_method(D_METHOD("get_soundfont"), &CsoundGodot::get_soundfont);
    ClassDB::add_property(
        "CsoundGodot", PropertyInfo(Variant::OBJECT, "soundfont", PROPERTY_HINT_RESOURCE_TYPE, "SoundFontFileReader"),
        "set_soundfont", "get_soundfont");
    ClassDB::bind_method(D_METHOD("set_midi_file", "midi_file"), &CsoundGodot::set_midi_file);
    ClassDB::bind_method(D_METHOD("get_midi_file"), &CsoundGodot::get_midi_file);
    ClassDB::add_property("CsoundGodot",
                          PropertyInfo(Variant::OBJECT, "midi_file", PROPERTY_HINT_RESOURCE_TYPE, "MidiFileReader"),
                          "set_midi_file", "get_midi_file");
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &CsoundGodot::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &CsoundGodot::get_csound_name);
    ClassDB::add_property("CsoundGodot", PropertyInfo(Variant::STRING, "csound_name"), "set_csound_name",
                          "get_csound_name");
}
