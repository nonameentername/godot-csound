#ifndef CSOUND_CHANNEL_BUFFER_H
#define CSOUND_CHANNEL_BUFFER_H

#include <csound.hpp>
#include <cstdint>

#include <godot_cpp/templates/vector.hpp>

namespace godot {

static const int CHANNEL_BLOCK_SIZE = 512;
static const int FIRST_BLOCKS_BEHIND = 4;
static const int MOST_BLOCKS_BEHIND = 16;
static const int CHANNEL_BUFFER_SIZE = CHANNEL_BLOCK_SIZE * (MOST_BLOCKS_BEHIND + 2);

struct CsoundChannelReader {
    bool started = false;
    int64_t position = 0;
    int blocks_behind = FIRST_BLOCKS_BEHIND;
};

class CsoundChannelBuffer {

private:
    Vector<MYFLT> samples;
    int64_t frames_written = 0;

    void start_behind_writer(CsoundChannelReader &reader, int p_frames);

public:
    CsoundChannelBuffer();

    void write(const MYFLT *p_buffer, int p_frames);
    void read_or_silence(CsoundChannelReader &reader, MYFLT *p_buffer, int p_frames);
};

} // namespace godot

#endif
