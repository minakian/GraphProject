#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "route_model.h"
#include "navigation.h"

// Load osm file into mem as binary data
static std::vector<std::byte> readFile(const std::string &file_path){
  std::ifstream input_file_stream{file_path, std::ios::binary | std::ios::ate};
  auto file_size = input_file_stream.tellg();
  std::vector<std::byte> file_contents(file_size);

  // Move cursor to beginning of file
  input_file_stream.seekg(0);
  // read file into memory
  input_file_stream.read((char *)file_contents.data(), file_size);

  return std::move(file_contents);
}

int main(void){
  std::string osm_file = "../map.osm";
  std::vector<std::byte> osm_data = readFile(osm_file);

  // Build Model.
  RouteModel model{osm_data};

  // Get user input
  float start_x, start_y, end_x, end_y;
  std::cout << "Please enter a start point: \nLongitude between " << model.minLon << " and " << model.maxLon << ": ";
  std::cin >> start_x;
  start_x = (start_x - model.minLon)/(model.maxLon-model.minLon)*100;
  start_x = start_x < 0 ? 0 : start_x > 100 ? 100 : start_x;
  std::cout << "Latitude between " << model.minLat << " and " << model.maxLat << ": ";
  std::cin >> start_y;
  start_y = (start_y - model.minLat)/(model.maxLat-model.minLat)*100;
  start_y = start_y < 0 ? 0 : start_y > 100 ? 100 : start_y;

  std::cout << "Please enter an end point:\nLongitude between " << model.minLon << " and " << model.maxLon << ": ";
  std::cin >> end_x;
  end_x = (end_x - model.minLon)/(model.maxLon-model.minLon)*100;
  end_x = end_x < 0 ? 0 : end_x > 100 ? 100 : end_x;
  std::cout << "Latitude between " << model.minLat << " and " << model.maxLat << ": ";
  std::cin >> end_y;
  end_y = (end_y - model.minLat)/(model.maxLat-model.minLat)*100;
  end_y = end_y < 0 ? 0 : end_y > 100 ? 100 : end_y;
  std::cout << start_x << " " << start_y << " " << end_x << " " << end_y << std::endl;

  // Create Navigation object and perform A* search.
  Navigation navigate{model, start_x, start_y, end_x, end_y};
  navigate.aStarSearch();

  // Output
  std::cout << "Distance: " << navigate.getDistance() << " meters. \n";

  for(auto n:model.path){
      std::cout << n.lat << "," << n.lon << "/";
  }
  std::cout << " END" << std::endl;
}