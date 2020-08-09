// source credit Udacity.com
#include "model.h"
#include "pugixml.hpp"
#include <iostream>


static Model::Road::Type String2RoadType(std::string_view type)
{
    if( type == "motorway" )        return Model::Road::Motorway;
    if( type == "trunk" )           return Model::Road::Trunk;
    if( type == "primary" )         return Model::Road::Primary;
    if( type == "secondary" )       return Model::Road::Secondary;    
    if( type == "tertiary" )        return Model::Road::Tertiary;
    if( type == "residential" )     return Model::Road::Residential;
    if( type == "living_street" )   return Model::Road::Residential;    
    if( type == "service" )         return Model::Road::Service;
    if( type == "unclassified" )    return Model::Road::Unclassified;
    if( type == "footway" )         return Model::Road::Footway;
    if( type == "bridleway" )       return Model::Road::Footway;
    if( type == "steps" )           return Model::Road::Footway;
    if( type == "path" )            return Model::Road::Footway;
    if( type == "pedestrian" )      return Model::Road::Footway;    
    return Model::Road::Invalid;    
}

static Model::Landuse::Type String2LanduseType(std::string_view type)
{
    if( type == "commercial" )      return Model::Landuse::Commercial;
    if( type == "construction" )    return Model::Landuse::Construction;
    if( type == "grass" )           return Model::Landuse::Grass;
    if( type == "forest" )          return Model::Landuse::Forest;
    if( type == "industrial" )      return Model::Landuse::Industrial;
    if( type == "railway" )         return Model::Landuse::Railway;
    if( type == "residential" )     return Model::Landuse::Residential;    
    return Model::Landuse::Invalid;
}

Model::Model( const std::vector<std::byte> &xml )
{
    LoadData(xml);

    AdjustCoordinates();

    std::sort(roads.begin(), roads.end(), [](const auto &_1st, const auto &_2nd){
        return (int)_1st.type < (int)_2nd.type; 
    });
}

void Model::LoadData(const std::vector<std::byte> &xml)
{
    using namespace pugi;
    
    xml_document doc;
    if( !doc.load_buffer(xml.data(), xml.size()) )
        throw std::logic_error("failed to parse the xml file");
    
    if( auto bounds = doc.select_nodes("/osm/bounds"); !bounds.empty() ) {
        auto node = bounds.first().node();
        minLat = atof(node.attribute("minlat").as_string());
        maxLat = atof(node.attribute("maxlat").as_string());
        minLon = atof(node.attribute("minlon").as_string());
        maxLon = atof(node.attribute("maxlon").as_string());
    }
    else 
        throw std::logic_error("map's bounds are not defined");

    std::unordered_map<std::string, int> node_id_to_num;
    for( const auto &node: doc.select_nodes("/osm/node") ) {
        node_id_to_num[node.node().attribute("id").as_string()] = (int)nodes.size();
        nodes.emplace_back();        
        nodes.back().y = atof(node.node().attribute("lat").as_string());
        nodes.back().x = atof(node.node().attribute("lon").as_string());
    }

    std::unordered_map<std::string, int> way_id_to_num;    
    for( const auto &way: doc.select_nodes("/osm/way") ) {
        auto node = way.node();
        
        const auto way_num = (int)ways.size();
        way_id_to_num[node.attribute("id").as_string()] = way_num;
        ways.emplace_back();
        auto &new_way = ways.back();
        
        for( auto child: node.children() ) {
            auto name = std::string_view{child.name()}; 
            if( name == "nd" ) {
                auto ref = child.attribute("ref").as_string();
                if( auto it = node_id_to_num.find(ref); it != end(node_id_to_num) )
                    new_way.nodes.emplace_back(it->second);
            }
            else if( name == "tag" ) {
                auto category = std::string_view{child.attribute("k").as_string()};
                auto type = std::string_view{child.attribute("v").as_string()};
                if( category == "highway" ) {
                    if( auto road_type = String2RoadType(type); road_type != Road::Invalid ) {
                        roads.emplace_back();
                        roads.back().way = way_num;
                        roads.back().type = road_type;
                    }
                }
            }
        }
    }
}

void Model::AdjustCoordinates()
{    
    const auto pi = 3.14159265358979323846264338327950288;
    const auto deg_to_rad = 2. * pi / 360.;
    const auto earth_radius = 6378137.;
    const auto lat2ym = [&](double lat) { return log(tan(lat * deg_to_rad / 2 +  pi/4)) / 2 * earth_radius; };
    const auto lon2xm = [&](double lon) { return lon * deg_to_rad / 2 * earth_radius; };     
    const auto dx = lon2xm(maxLon) - lon2xm(minLon);
    const auto dy = lat2ym(maxLat) - lat2ym(minLat);
    const auto min_y = lat2ym(minLat);
    const auto min_x = lon2xm(minLon);
    metricScale = std::min(dx, dy);

    for( auto &node: nodes ) {
        node.lon = node.x;
        node.lat = node.y;
        node.x = (lon2xm(node.x) - min_x) / metricScale;
        node.y = (lat2ym(node.y) - min_y) / metricScale;        
    }
}

static bool TrackRec(const std::vector<int> &open_ways,
                     const Model::Way *ways,
                     std::vector<bool> &used,
                     std::vector<int> &nodes) 
{
    if( nodes.empty() ) {
        for( int i = 0; i < open_ways.size(); ++i )
            if( !used[i] ) {
                used[i] = true;
                const auto &way_nodes = ways[open_ways[i]].nodes;
                nodes = way_nodes;
                if( TrackRec(open_ways, ways, used, nodes) )
                    return true;
                nodes.clear();
                used[i] = false;
            }
        return false;
    }
    else {
        const auto head = nodes.front();
        const auto tail = nodes.back();
        if( head == tail && nodes.size() > 1 )
            return true;
        for( int i = 0; i < open_ways.size(); ++i )
            if( !used[i] ) {
                const auto &way_nodes = ways[open_ways[i]].nodes;
                const auto way_head = way_nodes.front();
                const auto way_tail = way_nodes.back();
                if( way_head == tail || way_tail == tail ) {
                    used[i] = true;
                    const auto len = nodes.size();
                    if( way_head == tail ) 
                        nodes.insert(nodes.end(), way_nodes.begin(), way_nodes.end());
                    else
                        nodes.insert(nodes.end(), way_nodes.rbegin(), way_nodes.rend());
                    if( TrackRec(open_ways, ways, used, nodes) )
                        return true;
                    nodes.resize(len);                    
                    used[i] = false;
                }
            }
        return false;
    }
}

static std::vector<int> Track(std::vector<int> &open_ways, const Model::Way *ways)
{
    assert( !open_ways.empty() );
    std::vector<bool> used(open_ways.size(), false);
    std::vector<int> nodes;    
    if( TrackRec(open_ways, ways, used, nodes) )
        for( int i = 0; i < open_ways.size(); ++i )
            if( used[i] )
                open_ways[i] = -1;
    return nodes;
}
