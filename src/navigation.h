#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <iostream>
#include <vector>
#include <string>
#include "route_model.h"


class Navigation {
  public:
    Navigation(RouteModel &model, float start_x, float start_y, float end_x, float end_y);
    float getDistance() const {return distance;}
    void aStarSearch();

    void AddNeighbors(RouteModel::Node *current_node);
    float CalculateHValue(RouteModel::Node *node);
    std::vector<RouteModel::Node> ConstructFinalPath(RouteModel::Node *);
    RouteModel::Node *NextNode();

  private:
    std::vector<RouteModel::Node*> open_list;
    RouteModel::Node *start_node;
    RouteModel::Node *end_node;

    float distance = 0.0f;
    RouteModel &m_Model;
};

#endif