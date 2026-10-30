#include "csound_channel_buffer.h"

#include <algorithm>

using namespace godot;

CsoundChannelBuffer::CsoundChannelBuffer() {
    samples.resize(CHANNEL_BUFFER_SIZE);
    samples.fill(0);
}

void CsoundChannelBuffer::write(const MYFLT *p_buffer, int p_frames) {
    for (int frame = 0; frame < p_frames; frame++) {
        samples.write[(frames_written + frame) % CHANNEL_BUFFER_SIZE] = p_buffer[frame];
    }
    frames_written += p_frames;
}

void CsoundChannelBuffer::start_behind_writer(CsoundChannelReader &reader, int p_frames) {
    int64_t position = frames_written - p_frames - CHANNEL_BLOCK_SIZE * reader.blocks_behind;
    reader.started = position >= 0;
    reader.position = position;
}

void CsoundChannelBuffer::read_or_silence(CsoundChannelReader &reader, MYFLT *p_buffer, int p_frames) {
    int64_t frames_ready = frames_written - reader.position;
    bool ran_dry = reader.started && frames_ready < p_frames;
    bool overwritten = reader.started && frames_ready > CHANNEL_BUFFER_SIZE;

    if (ran_dry) {
        reader.blocks_behind = std::min(reader.blocks_behind * 2, MOST_BLOCKS_BEHIND);
    }
    if (!reader.started || ran_dry || overwritten) {
        start_behind_writer(reader, p_frames);
    }

    if (!reader.started) {
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame] = 0;
        }
        return;
    }

    for (int frame = 0; frame < p_frames; frame++) {
        p_buffer[frame] = samples[(reader.position + frame) % CHANNEL_BUFFER_SIZE];
    }
    reader.position += p_frames;
}
