#include "transport_router.h"

TransportRouter::TransportRouter(RequestHandler& handler, double bus_velocity, double bus_wait_time) 
    :handler_(handler), bus_velocity_(bus_velocity), bus_wait_time_(bus_wait_time) {


    const std::deque<const Stop*>stops_ = handler.GetStops();

    DirectedWeightedGraph<double> graf_stops(stops_.size() * 2);
    stopsinfo_.reserve(stops_.size() * 2);
    size_t j = 0;
    for (size_t i = 0; i < stops_.size(); ++i) {
        stopsinfo_[stops_[i]->name] = j;
        graf_stops.AddEdge({ j ,j + 1 , bus_wait_time , stops_[i]->name,0,true });
        j = j + 2;
    }
    std::deque<const Bus*>buses = handler.GetBuses();

    const auto& stops_distance_ = handler.GetStopsDistance();

    {
        for (const auto& name : buses) {
            for (size_t j = 1; j < name->stops.size(); ++j) {
                for (size_t i = 0; i < name->stops.size() - j; ++i) {
                    double route_lenght = 0.0;
                    size_t index = 0;
                    size_t index_next = 0;
                    index = stopsinfo_.at(std::string_view(name->stops[i]->name));
                    index_next = stopsinfo_.at(std::string_view(name->stops[i + j]->name));
                    size_t n = 0;
                    while ((n < j)) {

                        if (stops_distance_.count({ name->stops[i + n] ,name->stops[i + 1 + n] })) {
                            route_lenght += stops_distance_.at({ name->stops[i + n] ,name->stops[i + 1 + n] });
                        }
                        else {
                            route_lenght += stops_distance_.at({ name->stops[i + 1 + n] ,name->stops[i + n] });
                        }
                        n++;
                    }
                    if (route_lenght > 0.0) {
                        double edge_weight = route_lenght * bus_velocity;
                        graf_stops.AddEdge({ index + 1 ,index_next, edge_weight ,name->name ,j ,false });
                    }
                }
            }
        }
    }
    graf_stops_ = std::move(graf_stops);
}

DirectedWeightedGraph<double>& TransportRouter::GetGraf() {
    return graf_stops_;
}

std::unordered_map<std::string_view, size_t>& TransportRouter::GetStopInfo() {
    return stopsinfo_;
}
