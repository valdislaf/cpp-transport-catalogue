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
            double routelength;
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

        class TransportCatalogue
        {

        public:

            const Bus* GetBus(std::string_view stop);
            const std::deque<const Bus*> GetBuses();
            const Stop* GetStop(std::string_view stop);
            const std::deque<const Stop*> GetStops();

            void AddStop(Stop&& stop);

            void AddStopsLength(std::string stops, int Length);

            void AddBus(Bus&& bus);

            size_t GetUnique(const Bus* bus);

            double GetDistance(const Bus* bus);

            const StopInfo GetListBuses(std::string_view str);

            double GetRouteLength(const Bus* bus);

            double GetCurvature(const Bus* bus, int routelength);

            const RouteInfo GetRouteInfo(std::string_view str);

        private:

            std::deque<Bus> deque_buses_;
            std::unordered_map<std::string_view, const Bus*> buses_;
            std::deque<Stop> deque_stops_;
            std::unordered_map<std::string_view, const Stop*> stops_;
            std::unordered_map<const Stop*, std::deque<const Bus*>> stop_buses_;
            std::unordered_map<std::string, int>stop_to_stop_;
        };
    }

}
