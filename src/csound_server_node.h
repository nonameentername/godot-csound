#ifndef CSOUND_SERVER_NODE_H
#define CSOUND_SERVER_NODE_H

#include <godot_cpp/classes/node.hpp>

namespace godot {

class CsoundServerNode : public Node {
    GDCLASS(CsoundServerNode, Node);

private:
protected:
public:
    CsoundServerNode();
    ~CsoundServerNode();

    void _process();

    static void _bind_methods();
};
} // namespace godot

#endif
