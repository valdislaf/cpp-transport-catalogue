#pragma once

#include "router.h"
#include "request_handler.h"

#include <optional>
#include "json_reader.h"

using namespace transport::catalogue;
using namespace graph;

class  TransportRouter {

public:
    TransportRouter(RequestHandler& handler, RouteSettings route_settings);

    DirectedWeightedGraph<double>& GetGraf();

    std::unordered_map<std::string_view, size_t>& GetStopInfo();

private:
    RequestHandler handler_;
    RouteSettings route_settings_;
	DirectedWeightedGraph<double> graf_stops_;
    std::unordered_map<std::string_view, size_t> stops_info_;
};