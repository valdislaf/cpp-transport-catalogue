#pragma once

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
            auto AsTuple() const {
                return tie(name_, latitude_, longitude_, tostop_);
            }

        public:

            Stop(std::string_view name,
                double latitude,
                double longitude)
                :name_(name),
                latitude_(latitude),
                longitude_(longitude) {
            }

            Stop(std::string_view name,
                double latitude,
                double longitude, std::unordered_map<std::string, uint64_t>tostop)
                :name_(name),
                latitude_(latitude),
                longitude_(longitude),
                tostop_(tostop) {
            }

            size_t Hash() const {
                std::hash<double> d_hasher;
                auto h1 = d_hasher(latitude_);
                auto h2 = d_hasher(longitude_);
                return  static_cast<size_t>(h1 * 37 + h2);
            }

            std::string_view name() const {
                return name_;
            }

            double latitude() const {
                return latitude_;
            }

            double longitude() const {
                return longitude_;
            }

            std::unordered_map<std::string, uint64_t> tostop() const {
                return tostop_;
            }

            bool operator==(const Stop& other) const {
                return AsTuple() == other.AsTuple();
            }
        };

        struct Bus {
            std::string name;
            std::deque<const Stop*>stops;
        };

        class TransportCatalogue
        {

        public:

            const Bus* GetBus(std::string_view stop);

            const Stop* GetStop(std::string_view stop);

            void AddStop(Stop&& stop);

            void AddBus(Bus&& bus);

            size_t GetUniqs(const Bus* bus);

            double GetDistance(const Bus* bus);

            std::unordered_map<const Stop*, std::deque<const Bus*>> GetListBuses();

            uint64_t GetRouteLength(const Bus* bus);

            double GetCurvature(const Bus* bus, uint64_t routelength);

        private:

            std::deque<Bus> buses_;
            std::deque<Stop> stops_;
            std::unordered_map<const Stop*, std::deque<const Bus*>> stopbuses_;

        };
    }

    namespace detail {
        
        struct HasherBus {
            size_t operator()(const catalogue::Stop* stop) const {
                return static_cast<size_t>((*stop).Hash());
            }
        };

    }
}
