#include "transport_catalogue.h"
#include <cassert>

using namespace std;

namespace transport {

    namespace catalogue {

        const Bus* TransportCatalogue::GetBus(string_view bus) {
            if (buses_.count(bus) != 0) {
                return buses_.at(bus);
            }
            return nullptr;
        }

        const std::deque<const Bus*> TransportCatalogue::GetBuses() {

            std::deque<const Bus*>buses;
            std::set<std::string_view>buses_names;

            for (auto& st : buses_) {
                buses_names.insert(st.first);
            }
            for (auto& st : buses_names) {
                buses.push_back(buses_.at(st));
            }
            return buses;
        }

        const Stop* TransportCatalogue::GetStop(string_view stop) {
            if (stops_.count(stop) != 0) {
                return stops_.at(stop);
            }
            return nullptr;
        }

        const std::deque<const Stop*> TransportCatalogue::GetStops() {

            std::deque<const Stop*>stops;
            std::set<std::string_view>stops_names;

            for (auto& st : stops_) {
                stops_names.insert(st.first);
            }
            for (auto& st : stops_names) {
                stops.push_back(stops_.at(st));
            }

            return stops;
        }

        void TransportCatalogue::AddStop(Stop&& stop) {
            deque_stops_.push_back(move(stop));
            stop_buses_[&deque_stops_.back()];
            stops_[deque_stops_.back().name] = &deque_stops_.back();

        }

        void TransportCatalogue::AddStopsLength(std::string stops, int Length) {
         
            stop_to_stop_[stops] = Length;
            
        }

        void TransportCatalogue::AddStopsDistance(std::pair<const Stop*, const Stop*> stop, size_t Length) {
            stops_distance_[stop] = Length;
        }

        

        void TransportCatalogue::AddBus(Bus&& bus) {
            deque_buses_.push_back(move(bus));
            for (auto s : deque_buses_.back().stops) {
                stop_buses_[s].push_back(&deque_buses_.back());
            }
            buses_[deque_buses_.back().name] = &deque_buses_.back();

        }

        const StopInfo TransportCatalogue::GetListBuses(string_view str) {
            bool found = false;
            std::set<string_view> sw{};
            for (auto& s : stop_buses_) {
                if (s.first->name == str) {
                    found = true;
                    for (auto a : s.second) {
                        sw.insert(a->name);
                    }
                }
            }

            return { str, found , sw, sw.size() };
        }

        double TransportCatalogue::GetRouteLength(const Bus* bus) {         
            double route_lenght = 0.0;
            for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
           
            const  string& name_stop = (*bus).stops[i]->name;
            const  string& name_next = (*bus).stops[i + 1]->name;


                if (stop_to_stop_.count(name_stop + name_next)) {
                    route_lenght += stop_to_stop_.at(name_stop + name_next);
                }
                else {
                    route_lenght += stop_to_stop_.at(name_next + name_stop);
                }

            }
        
            return route_lenght;
        }

        double TransportCatalogue::GetCurvature(const Bus* bus, int routelength) {
            return static_cast<double>(routelength) / static_cast<double>(GetDistance(bus));
        }

        const RouteInfo TransportCatalogue::GetRouteInfo(string_view str) {
            const Bus* bus = GetBus(str);
            if (bus == nullptr) { return { bus,0,str ,0,0,0 }; }
            double routelength = GetRouteLength(bus);
            return { bus, routelength, str, bus->stops.size(), GetUnique(bus), GetCurvature(bus, routelength) };
        }

        const std::unordered_map<std::string, int> TransportCatalogue::GetStopsLengths()
        {
            return stop_to_stop_;
        }

        const std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, transport::catalogue::TransportCatalogue::Hasher> TransportCatalogue::GetStopsDistance()
        {
            return stops_distance_;
        }

        size_t TransportCatalogue::GetUnique(const Bus* bus) {
            std::unordered_set<const Stop*> stops_uniq_;
            stops_uniq_.insert(bus->stops.begin(), bus->stops.end());

            return stops_uniq_.size();
        }

        double TransportCatalogue::GetDistance(const Bus* bus) {
            return  transform_reduce(
                next(bus->stops.begin()), bus->stops.end(), bus->stops.begin(),  // входной диапазон
                0.0,  // начальное значение
                plus<>{},  // reduce-операция (группирующая функция)
                [](const Stop* lhs, const Stop* rhs) {
                    return geo_coordinates::ComputeDistance(
                        { (*lhs).coord.lat,(*lhs).coord.lng },
                        { (*rhs).coord.lat,(*rhs).coord.lng }
                    );
                }  // map-операция
            );
        }

    }
}
