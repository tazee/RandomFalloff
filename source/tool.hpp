//
// RandomFalloff - Random falloff tool in Modo
//

#pragma once

#include <lxsdk/lxu_attributes.hpp>
#include <lxsdk/lxu_select.hpp>
#include <lxsdk/lxu_attributes.hpp>
#include <lxsdk/lxu_vector.hpp>
#include <lxsdk/lxu_math.hpp>

#include <lxsdk/lx_layer.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_log.hpp>
#include <lxsdk/lx_plugin.hpp>
#include <lxsdk/lx_seltypes.hpp>
#include <lxsdk/lx_tool.hpp>
#include <lxsdk/lx_toolui.hpp>
#include <lxsdk/lx_layer.hpp>
#include <lxsdk/lx_vector.hpp>
#include <lxsdk/lx_pmodel.hpp>
#include <lxsdk/lx_vmodel.hpp>
#include <lxsdk/lx_channelui.hpp>
#include <lxsdk/lx_draw.hpp>
#include <lxsdk/lx_handles.hpp>
#include <lxsdk/lx_vp.hpp>

#include <lxsdk/lx_value.hpp>
#include <lxsdk/lx_select.hpp>
#include <lxsdk/lx_seltypes.hpp>

#include <iostream>

#include "falloff.hpp"

using namespace lx_err;

const char* SRVNAME_TOOL = "tool.randomFalloff";
const char* SRVNAME_FALLOFF = "falloff.random";

#define ATTRs_SOURCE    "source"
#define ATTRs_SEED      "seed"
#define ATTRs_BIPOLAR   "bipolar"

#define ATTRa_SOURCE    0
#define ATTRa_SEED      1
#define ATTRa_BIPOLAR   2

#ifndef LXx_OVERRIDE
#define LXx_OVERRIDE override
#endif

/*
 * Basic tool and tool model methods are defined here. The
 * attributes interface is inherited from the utility class.
 */

class CRandomFalloff : public CLxImpl_Tool, public CLxImpl_ToolModel, public CLxDynamicAttributes, public CLxImpl_ChannelUI
{
public:
    CRandomFalloff();

    void        tool_Reset() LXx_OVERRIDE;
    LXtObjectID tool_VectorType() LXx_OVERRIDE;
    const char* tool_Order() LXx_OVERRIDE;
    LXtID4      tool_Task() LXx_OVERRIDE;
	void		tool_Evaluate   (ILxUnknownID vts) LXx_OVERRIDE;

    unsigned    tmod_Flags() LXx_OVERRIDE;
	void        tmod_Initialize (ILxUnknownID vts, ILxUnknownID adjust, unsigned int flags) LXx_OVERRIDE;
    LxResult    tmod_Enable(ILxUnknownID obj) LXx_OVERRIDE;
    LxResult    tmod_Down(ILxUnknownID vts, ILxUnknownID adjust) LXx_OVERRIDE;
    void        tmod_Move(ILxUnknownID vts, ILxUnknownID adjust) LXx_OVERRIDE;
    void        tmod_Up(ILxUnknownID vts, ILxUnknownID adjust) LXx_OVERRIDE;

    LxResult	atrui_DisableMsg (unsigned int index, ILxUnknownID msg) LXx_OVERRIDE;
	LxResult	atrui_UIHints   (unsigned int index, ILxUnknownID hints) LXx_OVERRIDE;

    bool TestVertex(unsigned int& primary_index);
    bool Validate(CLxUser_Subject2Packet& subject, int source, int seed, int bipolar);
    bool ValidateSelectPackets(LXtID4 type);

    void* m_packet_obj;
    CFalloffPacket* m_packet;
    int m_source, m_seed, m_bipolar;
    LXtID4 m_type;
    std::vector<void*> m_select_packets;
    bool m_validated;

    CLxUser_LogService   s_log;
    CLxUser_LayerService s_layer;
    CLxUser_VectorType   v_type;
    CLxUser_SelectionService s_sel;
    CLxUser_MeshService mesh_svc;

    unsigned offset_view;
    unsigned offset_screen;
    unsigned offset_falloff;
    unsigned offset_subject;
    unsigned offset_input;
    unsigned offset_event;
	unsigned offset_center;
	unsigned offset_xfrm;
    unsigned mode_select;

    enum RandomSources {
        SOURCE_ELEMENT = 0,
        SOURCE_ISLAND = 1,
        SOURCE_PARTTAG = 2
    };
};

