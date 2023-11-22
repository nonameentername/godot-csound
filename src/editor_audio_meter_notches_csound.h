#ifndef EDITORAUDIOMETERNOTCHESCSOUND_H
#define EDITORAUDIOMETERNOTCHESCSOUND_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/font.hpp>

namespace godot {

class EditorAudioMeterNotchesCsound : public Control {
    GDCLASS(EditorAudioMeterNotchesCsound, Control);

private:
    struct AudioNotch {
        float relative_position = 0;
        float db_value = 0;
        bool render_db_value = false;

        _FORCE_INLINE_ AudioNotch(float r_pos, float db_v, bool rndr_val) {
            relative_position = r_pos;
            db_value = db_v;
            render_db_value = rndr_val;
        }

        _FORCE_INLINE_ AudioNotch(const AudioNotch &n) {
            relative_position = n.relative_position;
            db_value = n.db_value;
            render_db_value = n.render_db_value;
        }

        _FORCE_INLINE_ void operator=(const EditorAudioMeterNotchesCsound::AudioNotch &n) {
            relative_position = n.relative_position;
            db_value = n.db_value;
            render_db_value = n.render_db_value;
        }

        _FORCE_INLINE_ AudioNotch() {
        }
    };

    float editor_scale = 1;
    List<AudioNotch> notches;

    struct ThemeCache {
        Color notch_color;

        Ref<Font> font;
        int font_size = 0;
    } theme_cache;

public:
    const float line_length = 5.0f;
    const float label_space = 2.0f;
    const float btm_padding = 9.0f;
    const float top_padding = 5.0f;

    void add_notch(float p_normalized_offset, float p_db_value, bool p_render_value = false);
    Vector2 _get_minimum_size() const override;

private:
    virtual void _update_theme_item_cache();

    static void _bind_methods();
    void _notification(int p_what);
    void _draw_audio_notches();
    float _scaled_db_to_normalized_volume(float db);
    void set_editor_scale(float p_editor_scale);
    float get_editor_scale();

public:
    EditorAudioMeterNotchesCsound();
};

} // namespace godot

#endif // EDITOR_AUDIO_BUSES_H
