#include "csound_server_node.h"
#include "csound_server.h"

using namespace godot;

CsoundServerNode::CsoundServerNode() {
}

CsoundServerNode::~CsoundServerNode() {
}

void CsoundServerNode::_process() {
    CsoundServer::get_singleton()->process();
}

void CsoundServerNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("process"), &CsoundServerNode::_process);
}
