#include "serialization.h"
#include <unordered_map>
#include <map>
#include <string>

struct StopIn {
    std::string name;
    double latitude;
    double longitude;
};



using namespace transport::catalogue;
using namespace std::literals;

void Serialize(const std::filesystem::path& path, json::Array base_stops, json::Array base_buses, const json::Dict& rs_tc) {

    transport_catalogue_serialize::Stops stops;
    transport_catalogue_serialize::Buses buses;
    transport_catalogue_serialize::TransportCatalogue db;

    transport_catalogue_serialize::route_settings rs;
    if (rs_tc.size() != 0) {
        rs.set_bus_velocity(rs_tc.at("bus_velocity"s).AsDouble());
        rs.set_bus_wait_time(rs_tc.at("bus_wait_time"s).AsDouble());
        *db.mutable_rs() = rs;
    }
    std::map<std::string, const transport_catalogue_serialize::Stopclear*>Stopclear_ptr_idx;
    transport_catalogue_serialize::VectorStopclear Stops_in;
    // std::map <const std::string& name, const transport_catalogue_serialize::Stopclear*>Stopclear_ptr_idx;

    for (const auto& s : base_stops) {
        transport_catalogue_serialize::Stopclear stop;
        stop.set_name(s.AsDict().at("name"s).AsString());
        stop.set_latitude(s.AsDict().at("latitude"s).AsDouble());
        stop.set_longitude(s.AsDict().at("longitude"s).AsDouble());
        *Stops_in.add_stop() = stop;
        size_t size = Stops_in.stop_size();
        auto stoptt = &Stops_in.stop(size - 1);
        Stopclear_ptr_idx.emplace(s.AsDict().at("name"s).AsString(), stoptt);
    }


    for (const auto& s : base_stops) {
        transport_catalogue_serialize::Stop stop;
        *stop.mutable_stop() = *Stopclear_ptr_idx.at(s.AsDict().at("name"s).AsString());
        const auto& road_stops = s.AsDict().at("road_distances"s).AsDict();
        for (const auto& rs : road_stops) {
            transport_catalogue_serialize::Road road;
            *road.mutable_to() = *Stopclear_ptr_idx.at(rs.first);
            road.set_distance(rs.second.AsInt());
            *stop.add_road_distances() = road;
        }
        *stops.add_stops() = stop;
    }

    for (const auto& b : base_buses) {
        transport_catalogue_serialize::Bus bus;
        bus.set_name(b.AsDict().at("name"s).AsString());
        bus.set_is_roundtrip(b.AsDict().at("is_roundtrip"s).AsBool());
        for (const auto& s : b.AsDict().at("stops"s).AsArray()) {
            transport_catalogue_serialize::Stop stop;
            *stop.mutable_stop() = *Stopclear_ptr_idx.at(s.AsString());
            *bus.add_stops() = stop;
        }
        *buses.add_buses() = bus;
    }
    *db.mutable_stops() = stops;
    *db.mutable_buses() = buses;

    std::ofstream out_file(path, std::ios::binary);
    db.SerializeToOstream(&out_file);
}

json::Array Deserialize(const std::filesystem::path& path) {
    json::Array base_stops;
    json::Array base_buses;
    std::ifstream in_file(path, std::ios::binary);
    transport_catalogue_serialize::TransportCatalogue db;
    if (!db.ParseFromIstream(&in_file)) {
        return  json::Array();
    }
    transport_catalogue_serialize::Stops stops;
    transport_catalogue_serialize::Buses buses;
    stops = db.stops();
    buses = db.buses();

    size_t size_stops = stops.stops_size();
    for (size_t i = 0; i < size_stops; ++i) {
        transport_catalogue_serialize::Stop stop;
        stop = stops.stops(i);
        json::Dict dict_road_stops;
        size_t road_stops = stop.road_distances_size();
        for (size_t k = 0; k < road_stops; ++k) {
            std::string key = stop.road_distances(k).to().name();
            int value = stop.road_distances(k).distance();
            dict_road_stops.emplace(key, value);
        }

        json::Dict dict = json::Builder{}.StartDict()
            .Key("name"s).Value(stop.stop().name())
            .Key("latitude"s).Value(stop.stop().latitude())
            .Key("longitude"s).Value(stop.stop().longitude())
            .Key("road_distances"s).Value(dict_road_stops).EndDict().Build().AsDict();
        base_stops.push_back(dict);

    }

    size_t size_buses = buses.buses_size();
    for (size_t i = 0; i < size_buses; ++i) {
        transport_catalogue_serialize::Bus bus;
        bus = buses.buses(i);
        json::Array array_bus_stops;
        size_t bus_stops = bus.stops_size();
        for (size_t k = 0; k < bus_stops; ++k) {
            array_bus_stops.push_back(bus.stops(k).stop().name());
        }
        json::Dict dict = json::Builder{}.StartDict()
            .Key("is_roundtrip"s).Value(bus.is_roundtrip())
            .Key("name"s).Value(bus.name())
            .Key("stops"s).Value(array_bus_stops).EndDict().Build().AsDict();
        base_buses.push_back(dict);
    }

    json::Array result;
    result.push_back(base_stops);
    result.push_back(base_buses);



    json::Dict route_settings;
    transport_catalogue_serialize::route_settings rs;
    rs = db.rs();
    json::Dict dict = json::Builder{}.StartDict()
        .Key("bus_velocity"s).Value(rs.bus_velocity())
        .Key("bus_wait_time"s).Value(rs.bus_wait_time()).EndDict().Build().AsDict();
    result.push_back(dict);
    //// тут нужен move, поскольку возвращается другой тип
    return { std::move(result) };
}
