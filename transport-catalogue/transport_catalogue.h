#pragma once

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <string_view>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"
#include "geo.h"

namespace transport {

    namespace catalogue {

        struct RouteInfo {
            const Bus* bus;
            double route_length;
            std::string_view bus_name;
            size_t bus_stops_size;
            size_t unique_stops;
            double curvature;
        };

        struct StopInfo {
            std::string_view name_bus;
            bool found;
            std::set<std::string_view> sw;
            size_t size;
        };

        struct StopToStop {
            std::string_view stop_from;
            std::string_view stop_to;
            bool direct;           
        };

        class TransportCatalogue
        {

        public:

            const Bus* GetBus(std::string_view stop);
            const std::deque<const Bus*> GetBuses();
            const Stop* GetStop(std::string_view stop);
            const std::deque<const Stop*> GetStops();

            void AddStop(Stop&& stop);

            void AddStopsLength(std::string stops, int Length);

            void AddStopsDistance(std::pair<const Stop*, const Stop*> stop, size_t Length);

            void AddBus(Bus&& bus);

            size_t GetUnique(const Bus* bus);

            double GetDistance(const Bus* bus);

            const StopInfo GetListBuses(std::string_view str);

            double GetRouteLength(const Bus* bus);

            double GetCurvature(const Bus* bus, int routelength);

            const RouteInfo GetRouteInfo(std::string_view str);

            const std::unordered_map<std::string, int> GetStopsLengths();

            struct Hasher {
                size_t operator()(std::pair<const Stop*, const Stop*> stop) const {
                    std::hash<const void*> ptr_hasher;
                    return 37 * ptr_hasher(stop.first) + ptr_hasher(stop.second);
                }
            };

            const  std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, Hasher> GetStopsDistance();

           
        private:

            std::deque<Bus> deque_buses_;
            std::unordered_map<std::string_view, const Bus*> buses_;
            std::deque<Stop> deque_stops_;
            std::unordered_map<std::string_view, const Stop*> stops_;
            std::unordered_map<const Stop*, std::deque<const Bus*>> stop_buses_;
            std::unordered_map<std::string, int>stop_to_stop_;
            std::unordered_map<std::pair<const Stop*, const Stop*>,size_t, Hasher>stops_distance_;

        };
    }

}
