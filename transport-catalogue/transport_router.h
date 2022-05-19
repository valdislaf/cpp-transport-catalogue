#pragma once

#include "router.h"
#include "request_handler.h"

#include <optional>

using namespace transport::catalogue;
using namespace graph;

class  TransportRouter {

public:
    TransportRouter(RequestHandler& handler, double bus_velocity, double bus_wait_time);		

    DirectedWeightedGraph<double>& GetGraf();

    std::unordered_map<std::string_view, size_t>& GetStopInfo();

private:
    RequestHandler handler_;
	double bus_velocity_;
	double bus_wait_time_;
	DirectedWeightedGraph<double> graf_stops_;
    std::unordered_map<std::string_view, size_t> stopsinfo_;
};