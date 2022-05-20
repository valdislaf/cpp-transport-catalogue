#include "transport_router.h"

using Dist = const std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, transport::catalogue::TransportCatalogue::Hasher>;

std::unordered_map<std::string_view, size_t> FillingGraphStops(DirectedWeightedGraph<double>& graf_stops,
    double bus_wait_time, const std::deque<const Stop*>& stops) {
    std::unordered_map<std::string_view, size_t>stops_info;
    stops_info.reserve(stops.size() * 2);
    size_t j = 0;
    for (size_t i = 0; i < stops.size(); ++i) {
        stops_info[stops[i]->name] = j;
        graf_stops.AddEdge({ j ,j + 1 , bus_wait_time , stops[i]->name,0,true });
        j = j + 2;
    }
    return stops_info;
}

DirectedWeightedGraph<double> FillingGraphBuses(DirectedWeightedGraph<double>& graf_stops,
    std::unordered_map<std::string_view, size_t>& stops_info,
    std::deque<const Bus*>& buses, Dist& stops_distance, double bus_velocity) {
    for (const auto& name : buses) {
        for (size_t j = 1; j < name->stops.size(); ++j) {
            for (size_t i = 0; i < name->stops.size() - j; ++i) {
                double route_lenght = 0.0;
                size_t index = 0;
                size_t index_next = 0;
                index = stops_info.at(std::string_view(name->stops[i]->name));
                index_next = stops_info.at(std::string_view(name->stops[i + j]->name));
                size_t n = 0;
                while ((n < j)) {

                    if (stops_distance.count({ name->stops[i + n] ,name->stops[i + 1 + n] })) {
                        route_lenght += stops_distance.at({ name->stops[i + n] ,name->stops[i + 1 + n] });
                    }
                    else {
                        route_lenght += stops_distance.at({ name->stops[i + 1 + n] ,name->stops[i + n] });
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
    return graf_stops;
}

TransportRouter::TransportRouter(RequestHandler& handler, RouteSettings route_settings)
    :handler_(handler), route_settings_(route_settings) {

    const std::deque<const Stop*> stops_ = handler.GetStops();

    DirectedWeightedGraph<double> graf_stops(stops_.size() * 2);

    stops_info_ = FillingGraphStops(graf_stops, route_settings_.bus_wait_time, stops_);

    std::deque<const Bus*> buses = handler.GetBuses();

    const auto& stops_distance_ = handler.GetStopsDistance();

    graf_stops_ = std::move(FillingGraphBuses(graf_stops, stops_info_, buses, stops_distance_, route_settings_.bus_velocity));
}

DirectedWeightedGraph<double>& TransportRouter::GetGraf() {
    return graf_stops_;
}

std::unordered_map<std::string_view, size_t>& TransportRouter::GetStopInfo() {
    return stops_info_;
}