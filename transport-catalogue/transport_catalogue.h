#pragma once

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport {

    namespace catalogue {

        class Stop {

        private:

            std::string name_;
            double latitude_;
            double longitude_;
            std::unordered_map<std::string, uint64_t>tostop_;
            std::tuple<std::string, 
                double,
                double, 
                std::unordered_map<std::string, uint64_t>> AsTuple() const; 
          
        public:

            Stop(std::string_view name, 
                double latitude, 
                double longitude);

            Stop(std::string_view name, 
                double latitude, 
                double longitude, 
                std::unordered_map<std::string, 
                uint64_t>tostop);
               
            size_t Hash() const; 

            std::string_view name() const;

            double latitude() const; 

            double longitude() const; 

            std::unordered_map<std::string, uint64_t> tostop() const; 

            bool operator==(const Stop& other) const;
        };

        struct Bus {
            std::string name;
            std::deque<const Stop*>stops;
        };

        struct RouteInfo {
            const Bus* bus;
            uint64_t routelength;
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

            const Stop* GetStop(std::string_view stop);

            void AddStop(Stop&& stop);

            void AddBus(Bus&& bus);

            size_t GetUnique(const Bus* bus);

            double GetDistance(const Bus* bus);

            const StopInfo GetListBuses(std::string_view str);

            uint64_t GetRouteLength(const Bus* bus);

            double GetCurvature(const Bus* bus, uint64_t routelength);

            const RouteInfo GetRouteInfo(std::string_view str);

        private:

            std::deque<Bus> dequebuses_;
            std::unordered_map<std::string_view, const Bus*> buses_;
            std::deque<Stop> dequestops_;
            std::unordered_map<std::string_view, const Stop*> stops_;
            std::unordered_map<const Stop*, std::deque<const Bus*>> stopbuses_;

        };
    }

    namespace detail {        
        struct HasherBus {
            size_t operator()(const catalogue::Stop* stop) const;
        };

    }
}
