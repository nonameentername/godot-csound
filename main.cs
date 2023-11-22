using Godot;
using System;

public partial class main : Node2D
{
    private Node csound_server;
    private Node csound;

    [Export(PropertyHint.ResourceType, "MidiFileReader")]
    private Resource midi_file;

    public override void _Ready()
    {
        csound_server = (Node)Engine.GetSingleton("CsoundServer");
        csound_server.Connect("csound_layout_changed", new Callable(this, nameof(_csound_layout_changed)));
    }

    public void _csound_layout_changed()
    {
        csound = (Node)csound_server.Call("get_csound", "Main");
        csound.Call("send_control_channel", "cutoff", 1);
        //csound.Call("play_midi");
    }

    public void _on_fluidsynth_ready()
    {
        GD.Print("csound _ready");
    }

    public override bool _Set(StringName property, Variant value)
    {
        GD.Print(property, value);
        return base._Set(property, value);
    }


    public void _on_CheckButton_toggled(bool button_pressed)
    {
        if (button_pressed)
        {
            csound.Call("note_on", 1, 60, 90);
        }
        else
        {
            csound.Call("note_off", 1, 60);
        }
    }

    public void _on_CheckButton2_toggled(bool button_pressed)
    {
        if (button_pressed)
        {
            csound.Call("note_on", 2, 64, 90);
        }
        else
        {
            csound.Call("note_off", 2, 64);
        }
    }

    public void _on_CheckButton3_toggled(bool button_pressed)
    {
        if (button_pressed)
        {
            csound.Call("note_on", 3, 67, 90);
        }
        else
        {
            csound.Call("note_off", 3, 67);
        }
    }

    public void _on_v_slider_value_changed(double value)
    {
        csound.Call("send_control_channel", "cutoff", value);
    }

    public void _on_button_pressed()
    {
        csound.Call("play_midi", midi_file);
    }

}
