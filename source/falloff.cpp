//
// RandomFalloff - Random falloff tool in Modo
//

#include "falloff.hpp"

#include <vector>
#include <random>
#include <string>
#include <unordered_map>
#include <numeric>


// Falloff Evaluator
double CFalloffPacket::fp_Evaluate(LXtFVector pos, LXtPointID vertID, LXtPolygonID polyID)
{
    for (auto& map : m_maps)
    {
        double weight;
        if (map.Evaluate(vertID, weight))
        {
            return weight;
        }
    } 
    return 1.0;
}

// Setup random groups per vertex element
class VertElementVisitor : public CLxImpl_AbstractVisitor
{
public:
    LxResult Evaluate()
    {
        struct CPointGroup group;

        LXtPointID vertID = m_vert.ID();

        auto part = m_context->m_groups.size();

        if (m_context->m_map.find(vertID) == m_context->m_map.end())
        {
            group.m_points.push_back(vertID);
            m_context->m_map[vertID] = part;
        }

        if (group.m_points.size() > 0)
            m_context->m_groups.push_back(group);

        return LXe_OK;
    }
    CLxUser_Mesh    m_mesh;
    CLxUser_Point   m_vert;
    LXtMarkMode     m_mark_pick;
    class CRandomMap* m_context;
};

// Setup random groups per edge element
class EdgeElementVisitor : public CLxImpl_AbstractVisitor
{
public:
    LxResult Evaluate()
    {
        struct CPointGroup group;

        std::vector<LXtPointID> points(2);
        m_edge.Endpoints(&points[0], &points[1]);

        auto part = m_context->m_groups.size();

        for (auto vertID : points)
        {
            if (m_context->m_map.find(vertID) == m_context->m_map.end())
            {
                group.m_points.push_back(vertID);
                m_context->m_map[vertID] = part;
            }
        }

        if (group.m_points.size() > 0)
            m_context->m_groups.push_back(group);

        return LXe_OK;
    }
    CLxUser_Mesh    m_mesh;
    CLxUser_Point   m_vert;
    CLxUser_Edge    m_edge;
    LXtMarkMode     m_mark_pick;
    class CRandomMap* m_context;
};

// Setup random groups per polygon element
class PolyElementVisitor : public CLxImpl_AbstractVisitor
{
public:
    LxResult Evaluate()
    {
        struct CPointGroup group;

        unsigned int nvert;
        m_poly.VertexCount(&nvert);

        auto part = m_context->m_groups.size();

        for (auto i = 0u; i < nvert; i++)
        {
            LXtPointID vertID;
            m_poly.VertexByIndex(i, &vertID);
            m_vert.Select(vertID);
            if (m_context->m_map.find(vertID) == m_context->m_map.end())
            {
                group.m_points.push_back(vertID);
                m_context->m_map[vertID] = part;
            }
        }

        if (group.m_points.size() > 0)
            m_context->m_groups.push_back(group);

        return LXe_OK;
    }
    CLxUser_Mesh    m_mesh;
    CLxUser_Point   m_vert;
    CLxUser_Polygon m_poly;
    LXtMarkMode     m_mark_pick;
    class CRandomMap* m_context;
};

// Setup random groups per mesh element
void CRandomMap::FalloffElement(LXtID4 type)
{
    m_groups.clear();
    m_map.clear();

    if (type == LXiSEL_VERTEX)
    {
        VertElementVisitor vis;
        vis.m_mesh = m_mesh;
        vis.m_vert.fromMesh(m_mesh);
        vis.m_mark_pick = m_mark_pick;
        vis.m_context = this;
        vis.m_vert.Enum(&vis, m_mark_pick);
    }
    else if (type == LXiSEL_EDGE)
    {
        EdgeElementVisitor vis;
        vis.m_mesh = m_mesh;
        vis.m_edge.fromMesh(m_mesh);
        vis.m_vert.fromMesh(m_mesh);
        vis.m_mark_pick = m_mark_pick;
        vis.m_context = this;
        vis.m_edge.Enum(&vis, m_mark_pick);
    }
    else if (type == LXiSEL_POLYGON)
    {
        PolyElementVisitor vis;
        vis.m_mesh = m_mesh;
        vis.m_poly.fromMesh(m_mesh);
        vis.m_vert.fromMesh(m_mesh);
        vis.m_mark_pick = m_mark_pick;
        vis.m_context = this;
        vis.m_poly.Enum(&vis, m_mark_pick);
    }
}

// Setup random groups per vertex island
class VertIslandVisitor : public CLxImpl_AbstractVisitor
{
public:
    LxResult Evaluate()
    {
        if (m_context == nullptr)
        {
            m_vert.SetMarks(m_mark_done);
            return LXe_OK;
        }
    
        if (m_vert.TestMarks(m_mark_done) == LXe_TRUE)
            return LXe_OK;

        CLxUser_Point vert, vert0, vert1;
        vert.fromMesh(m_mesh);
        vert0.fromMesh(m_mesh);
        vert1.fromMesh(m_mesh);

        std::vector<LXtPointID> stack;
        LXtPointID vertID = m_vert.ID();
        stack.push_back(vertID);
        m_vert.SetMarks(m_mark_done);

        unsigned int part = m_context->m_groups.size();
        struct CPointGroup group;

        while (!stack.empty())
        {
            vertID = stack.back();
            stack.pop_back();
            unsigned int index;
            vert.Select(vertID);
            vert.Index(&index);
            if (m_context->m_map.find(vertID) == m_context->m_map.end())
            {
                group.m_points.push_back(vertID);
                m_context->m_map[vertID] = part;
            }
            unsigned int npoint;
            vert.PointCount(&npoint);
            for (auto i = 0u; i < npoint; i++)
            {
                vert.PointByIndex(i, &vertID);
                vert0.Select(vertID);
                vert0.Index(&index);
                if (vert0.TestMarks(m_mark_done) == LXe_TRUE)
                    continue;
                if (vert0.TestMarks(m_mark_pick) == LXe_FALSE)
                    continue;
                stack.push_back(vertID);
                vert0.SetMarks(m_mark_done);
            }
        }
        m_context->m_groups.push_back(group);
        return LXe_OK;
    }
    CLxUser_Mesh    m_mesh;
    CLxUser_Edge    m_edge;
    CLxUser_Point   m_vert;
    LXtMarkMode     m_mark_pick;
    LXtMarkMode     m_mark_done;
    class CRandomMap* m_context;
};

// Setup random groups per edge island
class EdgeIslandVisitor : public CLxImpl_AbstractVisitor
{
public:
    LxResult Evaluate()
    {
        if (m_context == nullptr)
        {
            m_edge.SetMarks(m_mark_done);
            return LXe_OK;
        }
    
        if (m_edge.TestMarks(m_mark_done) == LXe_TRUE)
            return LXe_OK;

        CLxUser_Edge edge, edge1;
        edge.fromMesh(m_mesh);
        edge1.fromMesh(m_mesh);

        std::vector<LXtEdgeID> stack;
        LXtEdgeID              edgeID = m_edge.ID();
        stack.push_back(edgeID);
        m_edge.SetMarks(m_mark_done);

        unsigned int part = m_context->m_groups.size();
        struct CPointGroup group;

        while (!stack.empty())
        {
            edgeID = stack.back();
            stack.pop_back();
            unsigned int index;
            edge.Select(edgeID);
            edge.Index(&index);
            std::vector<LXtPointID> points(2);
            edge.Endpoints(&points[0], &points[1]);
            for (auto i = 0u; i < points.size(); i++)
            {
                LXtPointID vertID = points[i];
                m_vert.Select(vertID);
                if (m_context->m_map.find(vertID) == m_context->m_map.end())
                {
                    group.m_points.push_back(m_vert.ID());
                    m_context->m_map[m_vert.ID()] = part;
                }
                unsigned int nedge;
                m_vert.EdgeCount(&nedge);
                for (auto j = 0u; j < nedge; j++)
                {
                    m_vert.EdgeByIndex(j, &edgeID);
                    edge1.Select(edgeID);
                    if (edge1.TestMarks(m_mark_done) == LXe_TRUE)
                        continue;
                    if (edge1.TestMarks(m_mark_pick) == LXe_FALSE)
                        continue;
                    edge1.Index(&index);
                    stack.push_back(edgeID);
                    edge1.SetMarks(m_mark_done);
                }
            }
        }
        m_context->m_groups.push_back(group);
        return LXe_OK;
    }
    CLxUser_Mesh    m_mesh;
    CLxUser_Edge    m_edge;
    CLxUser_Point   m_vert;
    LXtMarkMode     m_mark_pick;
    LXtMarkMode     m_mark_done;
    class CRandomMap* m_context;
};

// Setup random groups per polygon island
class PolyIslandVisitor : public CLxImpl_AbstractVisitor
{
public:
    LxResult Evaluate()
    {
        if (m_context == nullptr)
        {
            m_poly.SetMarks(m_mark_done);
            return LXe_OK;
        }
    
        if (m_poly.TestMarks(m_mark_done) == LXe_TRUE)
            return LXe_OK;

        CLxUser_Polygon poly, poly1;
        poly.fromMesh(m_mesh);
        poly1.fromMesh(m_mesh);

        std::vector<LXtPolygonID> stack;
        LXtPolygonID              pol = m_poly.ID();
        stack.push_back(pol);
        m_poly.SetMarks(m_mark_done);

        unsigned int part = m_context->m_groups.size();
        struct CPointGroup group;

        while (!stack.empty())
        {
            pol = stack.back();
            stack.pop_back();
            int index;
            poly.Select(pol);
            poly.Index(&index);
            unsigned int nvert = 0u, npol = 0u;
            poly.VertexCount(&nvert);
            for (auto i = 0u; i < nvert; i++)
            {
                LXtPointID vertID;
                poly.VertexByIndex(i, &vertID);
                m_vert.Select(vertID);
                if (m_context->m_map.find(vertID) == m_context->m_map.end())
                {
                    group.m_points.push_back(m_vert.ID());
                    m_context->m_map[m_vert.ID()] = part;
                }
                m_vert.PolygonCount(&npol);
                for (auto j = 0u; j < npol; j++)
                {
                    m_vert.PolygonByIndex(j, &pol);
                    poly1.Select(pol);
                    if (poly1.TestMarks(m_mark_done) == LXe_TRUE)
                        continue;
                    if (poly1.TestMarks(m_mark_pick) == LXe_FALSE)
                        continue;
                    poly1.Index(&index);
                    stack.push_back(pol);
                    poly1.SetMarks(m_mark_done);
                }
            }
        }
        m_context->m_groups.push_back(group);
        return LXe_OK;
    }
    CLxUser_Mesh    m_mesh;
    CLxUser_Polygon m_poly;
    CLxUser_Point   m_vert;
    LXtMarkMode     m_mark_pick;
    LXtMarkMode     m_mark_done;
    class CRandomMap* m_context;
};

// Setup random groups per selection island
void CRandomMap::FalloffIsland(LXtID4 type)
{
    m_groups.clear();
    m_map.clear();

    unsigned int part, npart = 0, nselect = 0;
    unsigned int nvert = m_mesh.NPoints();
    for (auto i = 0u; i < nvert; i++)
    {
        m_vert.SelectByIndex(i);
        m_vert.Part(&part);
        if (part > npart)
            npart = part;
        if (m_vert.TestMarks(m_mark_pick) == LXe_TRUE)
            nselect += 1;
    }
    npart += 1;
    // Use element part index when nothing is selected.
    if ((nselect == nvert) && (type == LXiSEL_VERTEX))
    {
        m_groups.resize(npart);
        for (auto i = 0u; i < nvert; i++)
        {
            m_vert.SelectByIndex(i);
            m_vert.Part(&part);
            m_groups[part].m_points.push_back(m_vert.ID());
            m_map[m_vert.ID()] = part;
        }
    }
    else if (type == LXiSEL_VERTEX)
    {
        VertIslandVisitor vis;
        vis.m_mesh = m_mesh;
        vis.m_vert.fromMesh(m_mesh);
        vis.m_mark_done = mesh_svc.ClearMode(LXsMARK_USER_0);
        vis.m_context = nullptr;
        vis.m_vert.Enum(&vis, LXiMARK_ANY);
        vis.m_mark_done = mesh_svc.SetMode(LXsMARK_USER_0);
        vis.m_mark_pick = m_mark_pick;
        vis.m_context = this;
        vis.m_vert.Enum(&vis, m_mark_pick);
    }
    else if (type == LXiSEL_EDGE)
    {
        EdgeIslandVisitor vis;
        vis.m_mesh = m_mesh;
        vis.m_edge.fromMesh(m_mesh);
        vis.m_vert.fromMesh(m_mesh);
        vis.m_mark_done = mesh_svc.ClearMode(LXsMARK_USER_0);
        vis.m_context = nullptr;
        vis.m_edge.Enum(&vis, LXiMARK_ANY);
        vis.m_mark_done = mesh_svc.SetMode(LXsMARK_USER_0);
        vis.m_mark_pick = m_mark_pick;
        vis.m_context = this;
        vis.m_edge.Enum(&vis, m_mark_pick);
    }
    else if (type == LXiSEL_POLYGON)
    {
        PolyIslandVisitor vis;
        vis.m_mesh = m_mesh;
        vis.m_poly.fromMesh(m_mesh);
        vis.m_vert.fromMesh(m_mesh);
        vis.m_mark_done = mesh_svc.ClearMode(LXsMARK_USER_0);
        vis.m_context = nullptr;
        vis.m_poly.Enum(&vis, LXiMARK_ANY);
        vis.m_mark_done = mesh_svc.SetMode(LXsMARK_USER_0);
        vis.m_mark_pick = m_mark_pick;
        vis.m_context = this;
        vis.m_poly.Enum(&vis, m_mark_pick);
    }
}

// Setup random groups per part polygon tag
class PolyPartVisitor : public CLxImpl_AbstractVisitor
{
public:
    LxResult Evaluate()
    {

        m_ptag.set(m_poly);
	    if (!m_ptag.test())
            return LXe_OK;

        const char* partTag = "";
        if (m_ptag.Get(LXi_PTAG_PART, &partTag) == LXe_NOTFOUND)
        {
            printf("** No part tag found for polygon %p\n", m_poly.ID());
            return LXe_OK;
        }

        std::string strTag(partTag);
        unsigned int part;

        if (m_map.find(strTag) == m_map.end())
        {
            part = m_context->m_groups.size();
            m_context->m_groups.resize(part + 1);
            m_map[strTag] = part;
        }
        else
            part = m_map[strTag];

        struct CPointGroup& group = m_context->m_groups[part];

        unsigned int nvert;
        m_poly.VertexCount(&nvert);

        for (auto i = 0u; i < nvert; i++)
        {
            LXtPointID vertID;
            m_poly.VertexByIndex(i, &vertID);
            m_vert.Select(vertID);
            if (m_context->m_map.find(vertID) == m_context->m_map.end())
            {
                group.m_points.push_back(vertID);
                m_context->m_map[vertID] = part;
            }
        }

        return LXe_OK;
    }
    CLxUser_Mesh    m_mesh;
    CLxUser_Point   m_vert;
    CLxUser_Polygon m_poly;
	CLxUser_StringTag m_ptag;
    LXtMarkMode     m_mark_pick;
    std::unordered_map<std::string, unsigned int> m_map;
    class CRandomMap* m_context;
};

// Setup random groups per part polygon tag
void CRandomMap::FalloffPartTag()
{
    m_groups.clear();
    m_map.clear();
    PolyPartVisitor vis;
    vis.m_mesh = m_mesh;
    vis.m_poly.fromMesh(m_mesh);
    vis.m_vert.fromMesh(m_mesh);
    vis.m_mark_pick = m_mark_pick;
    vis.m_context = this;
    vis.m_poly.Enum(&vis, LXiMARK_ANY);
}

CRandomMap::CRandomMap(CLxUser_Mesh& mesh, int seed, int source, bool bipolar, LXtID4 type)
{
    m_mesh = mesh;
    m_poly.fromMesh(mesh);
    m_vert.fromMesh(mesh);
    m_mark_pick = mesh_svc.SetMode(LXsMARK_SELECT);

    if (source == SOURCE_ELEMENT)
        FalloffElement(type);
    else if (source == SOURCE_ISLAND)
        FalloffIsland(type);
    else
        FalloffPartTag();

    std::vector<int> data(m_groups.size());
    std::iota(data.begin(), data.end(), 0);
    
    std::vector<int> combined_seed;
    combined_seed.push_back(seed);
    combined_seed.insert(combined_seed.end(), data.begin(), data.end());
    std::seed_seq seq(combined_seed.begin(), combined_seed.end());
    std::mt19937 engine(seq);

    double weight_min = bipolar ? -1.0 : 0.0;
    double weight_max = 1.0;

    std::uniform_real_distribution<double> dist(weight_min, weight_max);

    for (auto i = 0u; i < m_groups.size(); i++)
    {
        m_groups[i].weight = dist(engine);
        //printf("[%u] weight = %f points (%zu)\n", i, m_groups[i].weight, m_groups[i].m_points.size());
    }
}

bool CRandomMap::Evaluate(LXtPointID vertID, double& weight)
{
    if (m_map.find(vertID) != m_map.end())
    {
        if (m_groups[m_map[vertID]].zero_weight)
            weight = 0.0;
        else
            weight =  m_groups[m_map[vertID]].weight;
        return true;
    }
    return false;
}
