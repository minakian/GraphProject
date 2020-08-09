#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>

class Model
{
public:
    struct Node {
        double x = 0.f;
        double y = 0.f;
        double lat = 0.f;
        double lon = 0.f;
    };
    
    struct Way {
        std::vector<int> nodes;
    };
    
    struct Road {
        enum Type { Invalid, Unclassified, Service, Residential,
            Tertiary, Secondary, Primary, Trunk, Motorway, Footway };
        int way;
        Type type;
    };

    struct Railway {
        int way;
    };    
    
    struct Multipolygon {
        std::vector<int> outer;
        std::vector<int> inner;
    };
    
    struct Building : Multipolygon {};
    
    struct Leisure : Multipolygon {};
    
    struct Water : Multipolygon {};
    
    struct Landuse : Multipolygon {
        enum Type { Invalid, Commercial, Construction, Grass, Forest, Industrial, Railway, Residential };
        Type type;
    };
    
    double minLat = 0.;
    double maxLat = 0.;
    double minLon = 0.;
    double maxLon = 0.;
    
    Model( const std::vector<std::byte> &xml );
    
    auto MetricScale() const noexcept { return metricScale; }    
    
    auto &Nodes() const noexcept { return nodes; }
    auto &Ways() const noexcept { return ways; }
    auto &Roads() const noexcept { return roads; }
    auto &Buildings() const noexcept { return m_Buildings; }
    auto &Leisures() const noexcept { return m_Leisures; }
    auto &Waters() const noexcept { return m_Waters; }
    auto &Landuses() const noexcept { return m_Landuses; }
    auto &Railways() const noexcept { return m_Railways; }
    void reverseCoordinates(double x, double y);
    
private:
    void AdjustCoordinates();
    
    void LoadData(const std::vector<std::byte> &xml);
    
    std::vector<Node> nodes;
    std::vector<Way> ways;
    std::vector<Road> roads;
    std::vector<Node> m_Nodes;
    std::vector<Way> m_Ways;
    std::vector<Road> m_Roads;
    std::vector<Railway> m_Railways;
    std::vector<Building> m_Buildings;
    std::vector<Leisure> m_Leisures;
    std::vector<Water> m_Waters;
    std::vector<Landuse> m_Landuses;
    
    double metricScale = 1.f;
};