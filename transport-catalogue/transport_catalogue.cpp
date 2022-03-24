#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string_view>

using namespace std;

namespace transport {

    namespace catalogue {

        const Bus* TransportCatalogue::GetBus(string_view bus) {
            if (buses_.count(bus) != 0) {
                return buses_.at(bus);
            }
            return nullptr;
        }

        const Stop* TransportCatalogue::GetStop(string_view stop) {
            if (stops_.count(stop) != 0) {
                return stops_.at(stop);
            }
            return nullptr;
        }

        void TransportCatalogue::AddStop(Stop&& stop) {
            dequestops_.push_back(move(stop));
            stopbuses_[&dequestops_.back()];
            stops_[dequestops_.back().name()] = &dequestops_.back();
        }

        void TransportCatalogue::AddBus(Bus&& bus) {
            dequebuses_.push_back(move(bus));
            for (auto s : dequebuses_.back().stops) {
                stopbuses_[s].push_back(&dequebuses_.back());
            }
            buses_[dequebuses_.back().name] = &dequebuses_.back();

        }

        const StopInfo TransportCatalogue::GetListBuses(string_view str)
        {
            bool found = false;
            std::set<string_view> sw{};
            for (auto& s : stopbuses_) {
                if (s.first->name() == str) {
                    found = true;
                    for (auto a : s.second) {
                        sw.insert(a->name);
                    }
                }
            }

            return { str, found , sw, sw.size() };
        }

        uint64_t TransportCatalogue::GetRouteLength(const Bus* bus)
        {
            uint64_t len = 0;

            for (int i = 0; i < bus->stops.size() - 1; ++i) {
                string_view name = (*bus).stops[i + 1]->name();
                auto it = bus->stops[i]->tostop().count(name.data());
                if (it != 0) {
                    len += bus->stops[i]->tostop().at(name.data());
                }
                else {
                    len += GetStop(name.data())->tostop().at(bus->stops[i]->name().data());
                }
            }

            return len;
        }

        double TransportCatalogue::GetCurvature(const Bus* bus, uint64_t routelength)
        {
            return static_cast<double>(routelength) / static_cast<double>(GetDistance(bus));
        }

        const RouteInfo TransportCatalogue::GetRouteInfo(string_view str)
        {
            const Bus* bus = GetBus(str);
            if (bus == nullptr) { return { bus,0,str ,0,0,0}; }
            uint64_t routelength = GetRouteLength(bus);
            return { bus, routelength, str, bus->stops.size(), GetUnique(bus), GetCurvature(bus, routelength) };
        }

        size_t TransportCatalogue::GetUnique(const Bus* bus)
        {
            std::unordered_set<const Stop*, transport::detail::HasherBus> stops_uniq_;
            stops_uniq_.insert(bus->stops.begin(), bus->stops.end());

            return stops_uniq_.size();
        }

        double TransportCatalogue::GetDistance(const Bus* bus)
        {
            return  transform_reduce(
                next(bus->stops.begin()), bus->stops.end(), bus->stops.begin(),  // входной диапазон
                0.0,  // начальное значение
                plus<>{},  // reduce-операция (группирующая функция)
                [](const Stop* lhs, const Stop* rhs) {
                    return geocoordinates::ComputeDistance(
                        { (*lhs).latitude(),(*lhs).longitude() },
                        { (*rhs).latitude(),(*rhs).longitude() }
                    ); }  // map-операция
            );
        }

        std::tuple<std::string, double, double, std::unordered_map<std::string, uint64_t>>  Stop::AsTuple() const {
            return tie(name_, latitude_, longitude_, tostop_);
        }

        Stop::Stop(std::string_view name, double latitude, double longitude)
            :name_(name),
            latitude_(latitude),
            longitude_(longitude) {
        }

        Stop::Stop(std::string_view name, double latitude, double longitude, std::unordered_map<std::string, uint64_t> tostop)
            :name_(name),
            latitude_(latitude),
            longitude_(longitude),
            tostop_(tostop) {
        }

        size_t Stop::Hash() const {
            std::hash<double> d_hasher;
            auto h1 = d_hasher(latitude_);
            auto h2 = d_hasher(longitude_);
            return  static_cast<size_t>(h1 * 37 + h2);
        }

        std::string_view Stop::name() const {
            return name_;
        }

        double Stop::latitude() const {
            return latitude_;
        }

        double Stop::longitude() const {
            return longitude_;
        }

        std::unordered_map<std::string, uint64_t> Stop::tostop() const {
            return tostop_;
        }

        bool Stop::operator==(const Stop& other) const {
            return AsTuple() == other.AsTuple();
        }
    }
}

size_t transport::detail::HasherBus::operator()(const catalogue::Stop* stop) const {
    return static_cast<size_t>((*stop).Hash());
}
