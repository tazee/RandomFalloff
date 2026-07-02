//
// RandomFalloff - Random falloff tool in Modo
//

#pragma once

#include <lxsdk/lx_wrap.hpp>
#include <lxsdk/lxw_tool.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lxw_seltypes.hpp>
#include <lxsdk/lx_value.hpp>

#include <vector>
#include <unordered_map>
#include <string>

#ifndef LXx_OVERRIDE
#define LXx_OVERRIDE override
#endif

struct CPointGroup
{
    std::vector<LXtPointID> m_points;
    double weight = 1.0;
    double zero_weight = false;
};

class CRandomMap
{
public:
    CRandomMap(CLxUser_Mesh& mesh, int seed, int source, bool bipolar, LXtID4 type);
    bool Evaluate(LXtPointID vertID, double& weight);
    bool Validate();

    std::vector<CPointGroup> m_groups;
    std::unordered_map<LXtPointID, unsigned int> m_map;
    CLxUser_Mesh m_mesh;
    CLxUser_Polygon m_poly;
    CLxUser_Point m_vert;
    LXtMarkMode m_mark_pick;
    LXtMarkMode m_mark_done;
    CLxUser_MeshService mesh_svc;
    unsigned int m_npart;

    int m_source, m_seed, m_bipolar;
    LXtID4 m_type;
    int m_nvert;
    bool m_validated;

    enum RandomSource {
        SOURCE_ELEMENT = 0,
        SOURCE_ISLAND = 1,
        SOURCE_PARTTAG = 2
    };
private:
    void FalloffElement(LXtID4 type);
    void FalloffIsland(LXtID4 type);
    void FalloffPartTag();
};

class CLxImpl_FalloffPacket
{
public:
    virtual ~CLxImpl_FalloffPacket() = default;

    virtual double fp_Evaluate([[maybe_unused]] LXtFVector pos, [[maybe_unused]] LXtPointID vrx, [[maybe_unused]] LXtPolygonID poly)
    {
        return 1.0;
    }

    virtual double fp_Screen([[maybe_unused]] LXtObjectID vts, [[maybe_unused]] int x, [[maybe_unused]] int y)
    {
        return 1.0;
    }
};

template <class T>
class CLxIfc_FalloffPacket : public CLxInterface
{
public:
    CLxIfc_FalloffPacket()
    {
        vt.Evaluate = Evaluate;
        vt.Screen   = Screen;
        vTable      = &vt.iunk;
        iid         = &lx::guid_FalloffPacket;
    }

    static auto Evaluate(LXtObjectID wcom, LXtFVector pos, LXtPointID vrt, LXtPolygonID poly) -> double
    {
        LXCWxINST(CLxImpl_FalloffPacket, loc);
        return loc->fp_Evaluate(pos, vrt, poly);
    }

    static auto Screen(LXtObjectID wcom, LXtObjectID vts, int x, int y) -> double
    {
        LXCWxINST(CLxImpl_FalloffPacket, loc);
        return loc->fp_Screen(vts, x, y);
    }

private:
    ILxFalloffPacket vt;
};

class CFalloffPacket : public CLxImpl_FalloffPacket
{
public:
    double fp_Evaluate(LXtFVector, LXtPointID, LXtPolygonID) LXx_OVERRIDE;

    std::vector<CRandomMap> m_maps;
};
