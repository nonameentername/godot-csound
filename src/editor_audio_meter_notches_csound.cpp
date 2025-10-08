#include "editor_audio_meter_notches_csound.h"
#include <godot_cpp/classes/editor_plugin.hpp>

using namespace godot;

EditorAudioMeterNotchesCsound::EditorAudioMeterNotchesCsound() {
    for (float db = 6.0f; db >= -80.0f; db -= 6.0f) {
        bool renderNotch = (db >= -6.0f || db == -24.0f || db == -72.0f);
        add_notch(_scaled_db_to_normalized_volume(db), db, renderNotch);
    }
    set_mouse_filter(MOUSE_FILTER_PASS);
}

float EditorAudioMeterNotchesCsound::_scaled_db_to_normalized_volume(float db) {
    /* Inversion of equations found in _normalized_volume_to_scaled_db.
     * IMPORTANT: If one function changes, the other much change to reflect it. */
    if (db > -2.88) {
        return (db + 16.2f) / 22.22f;
    } else if (db < -38.602f) {
        return (db + 80.00f) / 830.72f;
    } else {
        if (db < 0.0) {
            /* To accommodate for NaN on negative numbers for root, we will mirror the
             * results of the positive db range in order to get the desired numerical
             * value on the negative side. */
            float positive_x = Math::pow(Math::abs(db) / 45.0f, 1.0f / 3.0f) + 1.0f;
            Vector2 translation = Vector2(1.0f, 0.0f) - Vector2(positive_x, Math::abs(db));
            Vector2 reflected_position = Vector2(1.0, 0.0f) + translation;
            return reflected_position.x;
        } else {
            return Math::pow(db / 45.0f, 1.0f / 3.0f) + 1.0f;
        }
    }
}

void EditorAudioMeterNotchesCsound::add_notch(float p_normalized_offset, float p_db_value, bool p_render_value) {
    notches.push_back(AudioNotch(p_normalized_offset, p_db_value, p_render_value));
}

Vector2 EditorAudioMeterNotchesCsound::_get_minimum_size() const {
    Ref<Font> font = get_theme_font("font", "Label");
    int font_size = get_theme_font_size("font_size", "Label");
    float font_height = font->get_height(font_size);

    float width = 0;
    float height = top_padding + btm_padding;

    for (const EditorAudioMeterNotchesCsound::AudioNotch &notch : notches) { 
        if (notch.render_db_value) {
            width = MAX(width, font->get_string_size(String::num(Math::abs(notch.db_value)) + "dB", HORIZONTAL_ALIGNMENT_LEFT, -1, font_size).x);
            height += font_height;
        }
    }
    width += line_length + label_space;

    return Size2(width, height);
}

void EditorAudioMeterNotchesCsound::_update_theme_item_cache() {
    theme_cache.notch_color = get_theme_color("font_color", "Editor");

    theme_cache.font = get_theme_font("font", "Label");
    theme_cache.font_size = get_theme_font_size("font_size", "Label");
}

void EditorAudioMeterNotchesCsound::set_editor_scale(float p_editor_scale) {
    editor_scale = p_editor_scale;
}

float EditorAudioMeterNotchesCsound::get_editor_scale() {
    return editor_scale;
}

void EditorAudioMeterNotchesCsound::_notification(int p_what) {
    switch (p_what) {
    case NOTIFICATION_DRAW: {
        _draw_audio_notches();
    } break;
    case NOTIFICATION_THEME_CHANGED: {
        _update_theme_item_cache();
        break;
    }
    }
}

void EditorAudioMeterNotchesCsound::_draw_audio_notches() {
    float font_height = theme_cache.font->get_height(theme_cache.font_size);

    for (const EditorAudioMeterNotchesCsound::AudioNotch &n : notches) { 
        draw_line(Vector2(0, (1.0f - n.relative_position) * (get_size().y - btm_padding - top_padding) + top_padding),
                  Vector2(line_length * editor_scale,
                          (1.0f - n.relative_position) * (get_size().y - btm_padding - top_padding) + top_padding),
                  theme_cache.notch_color, Math::round(editor_scale));

        if (n.render_db_value) {
            draw_string(theme_cache.font,
                        Vector2((line_length + label_space) * editor_scale,
                                (1.0f - n.relative_position) * (get_size().y - btm_padding - top_padding) +
                                    (font_height / 4) + top_padding),
                        String::num(Math::abs(n.db_value)) + "dB", HORIZONTAL_ALIGNMENT_LEFT, -1, theme_cache.font_size,
                        theme_cache.notch_color);
        }
    }
}

void EditorAudioMeterNotchesCsound::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_notch"), &EditorAudioMeterNotchesCsound::add_notch);
    ClassDB::bind_method(D_METHOD("_draw_audio_notches"), &EditorAudioMeterNotchesCsound::_draw_audio_notches);

    ClassDB::bind_method(D_METHOD("get_editor_scale"), &EditorAudioMeterNotchesCsound::get_editor_scale);
    ClassDB::bind_method(D_METHOD("set_editor_scale", "editor_scale"),
                         &EditorAudioMeterNotchesCsound::set_editor_scale);

    ClassDB::add_property("EditorAudioMeterNotchesCsound", PropertyInfo(Variant::FLOAT, "editor_scale"),
                          "set_editor_scale", "get_editor_scale");
}
