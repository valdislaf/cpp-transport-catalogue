#include "transport_catalogue.h"

using namespace std;

namespace transport {

    namespace catalogue {

        const Bus* TransportCatalogue::GetBus(string_view bus) {
            if (buses_.count(bus) != 0) {
                return buses_.at(bus);
            }
            return nullptr;
        }

        const std::deque<const Bus*> TransportCatalogue::GetBuses()
        {
            
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

        double TransportCatalogue::GetRouteLength(const Bus* bus)
        {
            double len = 0.0;

            for (int i = 0; i < bus->stops.size() - 1; ++i) {
                string_view name = (*bus).stops[i + 1]->name();
                if (bus->stops[i]->tostop().count(name.data())) {
                    len += bus->stops[i]->tostop().at(name.data());
                }
                else {
                    len += GetStop(name.data())->tostop().at(bus->stops[i]->name().data());
                }
            }

            return len;
        }

        double TransportCatalogue::GetCurvature(const Bus* bus, int routelength)
        {
            return static_cast<double>(routelength) / static_cast<double>(GetDistance(bus));
        }

        const RouteInfo TransportCatalogue::GetRouteInfo(string_view str)
        {
            const Bus* bus = GetBus(str);
            if (bus == nullptr) { return { bus,0,str ,0,0,0 }; }
            double routelength = GetRouteLength(bus);
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

    }
}

size_t transport::detail::HasherBus::operator()(const Stop* stop) const {
    return static_cast<size_t>((*stop).Hash());
}
