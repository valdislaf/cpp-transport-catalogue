#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string_view>

using namespace std;

namespace transport {

    namespace catalogue {

        const Bus* TransportCatalogue::GetBus(string_view bus) {
            const Bus* bu{};

            for (const auto& s : buses_) {
                if (s.name == bus) {
                    bu = &s; break;
                }
            }

            return bu;
        }

        const Stop* TransportCatalogue::GetStop(string_view stop) {
            const Stop* st{};

            for (const auto& s : stops_) {
                if (s.name() == stop) {
                    st = &s; break;
                }
            }

            return st;
        }

        void TransportCatalogue::AddStop(Stop&& stop) {
            stops_.push_back(move(stop));
            stopbuses_[&stops_.back()];
        }

        void TransportCatalogue::AddBus(Bus&& bus) {
            buses_.push_back(move(bus));
            for (auto s : buses_.back().stops) {
                stopbuses_[s].push_back(&buses_.back());
            }
        }

        std::unordered_map<const Stop*, std::deque<const Bus*>> TransportCatalogue::GetListBuses()
        {
            return stopbuses_;
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

        size_t TransportCatalogue::GetUniqs(const Bus* bus)
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
    }
}


