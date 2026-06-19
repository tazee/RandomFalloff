//
// RandomFalloff - A plugin for selecting co-linearvertices in Modo
//

#include "tool.hpp"
#include "lxsdk/lxvalue.h"

/*
 * On create we add our one tool attribute. We also allocate a vector type
 * and select mode mask.
 */
CRandomFalloff::CRandomFalloff()
{
    CLxUser_PacketService sPkt;
    CLxUser_MeshService   sMesh;

    static const LXtTextValueHint random_sources[] = {
        { SOURCE_ELEMENT, "element" },
        { SOURCE_ISLAND, "island" },
        { SOURCE_PARTTAG, "partTag" },
        { 0, "=random_sources" }, 0
    };

    dyna_Add(ATTRs_SOURCE, LXsTYPE_INTEGER);
    dyna_SetHint(ATTRa_SOURCE, random_sources);
    dyna_Add(ATTRs_SEED, LXsTYPE_INTEGER);
    dyna_Add(ATTRs_BIPOLAR, LXsTYPE_BOOLEAN);

    tool_Reset();

    sPkt.NewVectorType(LXsCATEGORY_TOOL, v_type);
    sPkt.AddPacket(v_type, LXsP_TOOL_FALLOFF, LXfVT_SET);
    sPkt.AddPacket(v_type, LXsP_TOOL_VIEW_EVENT, LXfVT_GET);
    sPkt.AddPacket(v_type, LXsP_TOOL_SCREEN_EVENT, LXfVT_GET);
    sPkt.AddPacket(v_type, LXsP_TOOL_SUBJECT2, LXfVT_GET);
    sPkt.AddPacket(v_type, LXsP_TOOL_INPUT_EVENT, LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_EVENTTRANS,  LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_ACTCENTER,   LXfVT_GET);

    offset_view = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_VIEW_EVENT);
    offset_screen = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_SCREEN_EVENT);
    offset_falloff = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_FALLOFF);
    offset_subject = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_SUBJECT2);
    offset_input = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_INPUT_EVENT);
	offset_event  = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_EVENTTRANS);
	offset_center = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_ACTCENTER);
	offset_xfrm   = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_XFRM);
    mode_select = sMesh.SetMode("select");
}

/*
 * Reset sets the attributes back to defaults.
 */
void CRandomFalloff::tool_Reset()
{
    dyna_Value(ATTRa_SOURCE).SetInt(SOURCE_ISLAND);
    dyna_Value(ATTRa_SEED).SetInt(1074);
    dyna_Value(ATTRa_BIPOLAR).SetInt(0);
}

/*
 * Boilerplate methods that identify this as an action (state altering) tool.
 */
LXtObjectID CRandomFalloff::tool_VectorType()
{
    return v_type.m_loc;  // peek method; does not add-ref
}

const char* CRandomFalloff::tool_Order()
{
    return LXs_ORD_WGHT;
}

LXtID4 CRandomFalloff::tool_Task()
{
    return LXi_TASK_WGHT;
}

/*
 * We employ the simplest possible tool model -- default hauling. We indicate
 * that we want to haul one attribute, we name the attribute, and we implement
 * Initialize() which is what to do when the tool activates or re-activates.
 * In this case set the axis to the current value.
 */
unsigned CRandomFalloff::tmod_Flags()
{
    return LXfTMOD_I0_INPUT;
}

LxResult CRandomFalloff::tmod_Enable(ILxUnknownID obj)
{
    CLxUser_Message msg(obj);
    unsigned int primary_index = 0;

    if (TestVertex(primary_index) == false)
    {
        msg.SetCode(LXe_CMD_DISABLED);
        msg.SetMessage(SRVNAME_TOOL, "NoVertex", 0);
        return LXe_DISABLED;
    }
    return LXe_OK;
}

LxResult CRandomFalloff::tmod_Down(ILxUnknownID vts, ILxUnknownID adjust)
{
    return LXe_TRUE;
}

void CRandomFalloff::tmod_Move(ILxUnknownID vts, ILxUnknownID adjust)
{
}

void CRandomFalloff::tmod_Up(ILxUnknownID vts, ILxUnknownID adjust)
{
}

void CRandomFalloff::tmod_Initialize (ILxUnknownID vts, ILxUnknownID adjust, unsigned int flags)
{
}

LxResult CRandomFalloff::atrui_DisableMsg (unsigned int index, ILxUnknownID msg)
{
    return LXe_OK;
}

LxResult CRandomFalloff::atrui_UIHints(unsigned int index, ILxUnknownID hints)
{
	CLxLoc_UIHints		 uiHints(hints);

    if (index == ATTRa_SEED)
    {
        uiHints.MinInt(2);
    }
    return LXe_OK;
}

bool CRandomFalloff::TestVertex(unsigned int& primary_index)
{
    /*
     * Start the scan in read-only mode.
     */
    CLxUser_LayerScan scan;
    CLxUser_Mesh      mesh;
    unsigned          i, n, count;
    bool              ok = false;

    primary_index = 0;

    s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE | LXf_LAYERSCAN_MARKVERTS, scan);

    if (scan)
    {
        n = scan.NumLayers();
        for (i = 0; i < n; i++)
        {
            scan.BaseMeshByIndex(i, mesh);
            mesh.PointCount(&count);
            if (count > 0)
            {
                ok = true;
                primary_index = i;
                break;
            }
        }
        scan.Apply();
    }

    /*
     * Return false if there is no polygons in any active layers.
     */
    return ok;
}

/*
 * Tool evaluation uses layer scan interface to walk through all the active
 * meshes and visit all the selected polygons.
 */
void CRandomFalloff::tool_Evaluate(ILxUnknownID vts)
{
    static CLxSpawner<CFalloffPacket> spawer(SRVNAME_FALLOFF);

    CLxUser_VectorStack vec(vts);
    CLxUser_Subject2Packet subject;
    if (vec.ReadObject(offset_subject, subject) == false)
        return;

    CLxUser_VectorStack vecStack(vts);
    void* packet_obj;
    printf("tool_Evaluate\n");
    CFalloffPacket* packet = spawer.Alloc(&packet_obj);

    int source = CRandomMap::SOURCE_PARTTAG;
    int seed = 1074;
    int bipolar = 0;
    dyna_Value(ATTRa_SOURCE).GetInt(&source);
    dyna_Value(ATTRa_SEED).GetInt(&seed);
    dyna_Value(ATTRa_BIPOLAR).GetInt(&bipolar);

    CLxUser_LayerScan scan;
    CLxUser_Mesh      mesh;
    LXtID4 type = subject.Type();

    s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE | LXf_LAYERSCAN_MARKALL, scan);
    unsigned int count = scan.NumLayers();
    for (auto i = 0u; i < count; i++)
    {
        scan.BaseMeshByIndex(i, mesh);
        CRandomMap map(mesh, seed, source, static_cast<bool>(bipolar), type);
        packet->m_maps.push_back(map);
    }
    vecStack.SetPacket(offset_falloff, packet_obj);
}

/*
 * Export tool server.
 */
void initialize()
{
    CLxGenericPolymorph* srv;

    srv = new CLxPolymorph<CRandomFalloff>;
    srv->AddInterface(new CLxIfc_Tool<CRandomFalloff>);
    srv->AddInterface(new CLxIfc_ToolModel<CRandomFalloff>);
    srv->AddInterface(new CLxIfc_Attributes<CRandomFalloff>);
    srv->AddInterface(new CLxIfc_AttributesUI<CRandomFalloff>);
    srv->AddInterface(new CLxIfc_ChannelUI<CRandomFalloff>);
    lx::AddServer(SRVNAME_TOOL, srv);

    srv = new CLxPolymorph<CFalloffPacket>;
    srv->AddInterface(new CLxIfc_FalloffPacket<CFalloffPacket>);
    lx::AddSpawner(SRVNAME_FALLOFF, srv);
}
