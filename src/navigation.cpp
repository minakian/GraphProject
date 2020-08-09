#include "navigation.h"
#include <algorithm>

Navigation::Navigation(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    /**** Determine closest node to start,end ****/
    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y);
}


/**** CalculateHValue ****/

float Navigation::CalculateHValue(RouteModel::Node *node) {
    return node->distance(*end_node);
}


/**** AddNeighbors ****/
void Navigation::AddNeighbors(RouteModel::Node *current_node) {
    current_node->FindNeighbors();
    for(auto n:current_node->neighbors){
        n->parent = current_node;
        n->h_value = CalculateHValue(n);
        n->g_value = current_node->g_value + current_node->distance(*n);

        open_list.emplace_back(n);
        n->visited = true;
    }
}


/**** NextNode ****/
bool Compare(const RouteModel::Node* a, const RouteModel::Node* b){
    return a->h_value + a->g_value < b->h_value + b->g_value;
}

RouteModel::Node *Navigation::NextNode() {
    std::sort(open_list.begin(), open_list.end(), Compare);
    RouteModel::Node *lowest = open_list.front();
    open_list.erase(open_list.begin());
    return lowest;
}


/**** ConstructFinalPath ****/
std::vector<RouteModel::Node> Navigation::ConstructFinalPath(RouteModel::Node *current_node) {
    // Create path_found vector
    distance = 0.0f;
    std::vector<RouteModel::Node> path_found;

    RouteModel::Node parent_node;
    // TODO: Implement your solution here.
    while(current_node->parent != nullptr){
        path_found.push_back (*current_node);
        parent_node = *(current_node->parent);
        distance+= current_node->distance(parent_node);
        current_node = current_node->parent;
    }
    path_found.push_back(*(current_node));

    distance *= m_Model.MetricScale(); // Multiply the distance by the scale of the map to get meters.
    std::reverse(path_found.begin(), path_found.end());
    return path_found;

}


/**** A* Search ****/

void Navigation::aStarSearch() {
    RouteModel::Node *current_node = nullptr;
    start_node->visited = true;
    open_list.push_back(start_node);

    while(open_list.size() > 0){
        current_node = NextNode();

        if(current_node->distance(*end_node) == 0){
            m_Model.path = ConstructFinalPath(current_node);
            return;
        }

        AddNeighbors(current_node);
    }
}