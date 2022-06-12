#include "serialization.h"

#include <map>
#include <string>
#include <unordered_map>

using namespace transport::catalogue;
using namespace std::literals;

void AddRoutingSettings(RequestHandler& handler, transport_catalogue_serialize::TransportCatalogue& db) {
    transport_catalogue_serialize::Routesettings rs;
    if (handler.GetRoutingSettings().size() != 0) {
        rs.set_bus_velocity(handler.GetRoutingSettings().at("bus_velocity"s));
        rs.set_bus_wait_time(handler.GetRoutingSettings().at("bus_wait_time"s));
        *db.mutable_rs() = rs;
    }
}

void AddIdxNames(RequestHandler& handler, transport_catalogue_serialize::Table& tabl) {
    uint32_t idx_name = 0;
    for (const auto& stop_ : handler.GetStops()) {
        transport_catalogue_serialize::Names name;
        name.set_name(stop_->name);
        name.set_id(idx_name);
        *tabl.add_table_names() = name;
        ++idx_name;
    }   
}

void AddStops(RequestHandler& handler, transport_catalogue_serialize::Table& tabl, transport_catalogue_serialize::Stops& stops) {
    for (const auto& stop_ : handler.GetStops()) {
        transport_catalogue_serialize::Stop stop;
        auto& id = *std::find_if(tabl.table_names().begin(), tabl.table_names().end(), [&stop_](const auto& lhs) {return stop_->name == lhs.name(); });
        stop.set_name(id.id());
        stop.set_latitude(stop_->coord.lat);
        stop.set_longitude(stop_->coord.lng);
        const auto& road_stops = stop_->road_distances;
        for (const auto& rs : road_stops) {
            transport_catalogue_serialize::Road road;
            auto& id = *std::find_if(tabl.table_names().begin(), tabl.table_names().end(), [&rs](const auto& lhs) {return rs.first == lhs.name(); });
            road.set_to(id.id());
            road.set_distance(rs.second);
            *stop.add_road_distances() = road;
        }
        *stops.add_stops() = stop;
    }
}

void AddBuses(RequestHandler& handler, transport_catalogue_serialize::Table& tabl, transport_catalogue_serialize::Buses& buses) {
    for (const auto& bus_ : handler.GetBuses()) {
        transport_catalogue_serialize::Bus bus;
        bus.set_name(bus_->name);
        bus.set_is_roundtrip(bus_->is_roundtrip);
        for (const auto& stop_ : bus_->stops) {
            transport_catalogue_serialize::Stop stop;

            auto& id = *std::find_if(tabl.table_names().begin(), tabl.table_names().end(), [&stop_](const auto& lhs) {return stop_->name == lhs.name(); });
            stop.set_name(id.id());
            *bus.add_stops() = stop;
        }
        *buses.add_buses() = bus;
    }
}

void AddColor(transport_catalogue_serialize::Color* rendsett, const svg::Color& col) {
    if (std::holds_alternative<std::string>(col)) {
        rendsett->mutable_collortxt()->set_color(std::get<std::string>(col));
    }
    else if (std::holds_alternative<svg::Rgb>(col)) {
        transport_catalogue_serialize::Rgb rgb;
        rgb.set_red(std::get<svg::Rgb>(col).red);
        rgb.set_green(std::get<svg::Rgb>(col).green);
        rgb.set_blue(std::get<svg::Rgb>(col).blue);
        *rendsett->mutable_rgb() = rgb;
    }
    else if (std::holds_alternative<svg::Rgba>(col)) {
        transport_catalogue_serialize::Rgba rgba;
        rgba.set_red(std::get<svg::Rgba>(col).red);
        rgba.set_green(std::get<svg::Rgba>(col).green);
        rgba.set_blue(std::get<svg::Rgba>(col).blue);
        rgba.set_opacity(std::get<svg::Rgba>(col).opacity);
        *rendsett->mutable_rgba() = rgba;
    }
}

void AddRenderSettings(RequestHandler& handler, transport_catalogue_serialize::TransportCatalogue& db) {
    const auto& rends_tc = handler.GetRenderSettings();
    transport_catalogue_serialize::Rendersettings rendersettings;
    rendersettings.set_bus_label_font_size(rends_tc.bus_label_font_size);
    const auto& bus_offset = rends_tc.bus_label_offset;
    rendersettings.add_bus_label_offset(bus_offset.at(0));
    rendersettings.add_bus_label_offset(bus_offset.at(1));
    for (const auto& col : rends_tc.color_palette) {
        AddColor(rendersettings.add_color_palette(), col);
    }
    rendersettings.set_height(rends_tc.height);
    rendersettings.set_line_width(rends_tc.line_width);
    rendersettings.set_padding(rends_tc.padding);
    rendersettings.set_stop_label_font_size(rends_tc.stop_label_font_size);
    const auto& stop_offset = rends_tc.stop_label_offset;
    rendersettings.add_stop_label_offset(stop_offset.at(0));
    rendersettings.add_stop_label_offset(stop_offset.at(1));
    rendersettings.set_stop_radius(rends_tc.stop_radius);
    const auto& colr = rends_tc.underlayer_color;
    AddColor(rendersettings.mutable_underlayer_color(), colr);
    rendersettings.set_underlayer_width(rends_tc.underlayer_width);
    rendersettings.set_width(rends_tc.width);
    *db.mutable_rends() = rendersettings;
}

void Serialize(const std::filesystem::path& path, RequestHandler& handler) {
    std::ofstream out_file(path, std::ios::binary);
    transport_catalogue_serialize::Stops stops;
    transport_catalogue_serialize::Buses buses;
    transport_catalogue_serialize::Objectbase base;
    transport_catalogue_serialize::TransportCatalogue db;
    transport_catalogue_serialize::Table tabl;
    AddRoutingSettings(handler, db);
    AddIdxNames(handler, tabl);    
    AddStops(handler, tabl, stops);
    AddBuses(handler, tabl, buses);
    AddRenderSettings(handler, db);
    *base.mutable_table() = tabl;
    *base.mutable_stops() = stops;
    *base.mutable_buses() = buses;
    *db.mutable_base() = base;
    db.SerializeToOstream(&out_file);
}

void AddStopsToTC(TransportCatalogue& TC, transport_catalogue_serialize::Stops& stops, transport_catalogue_serialize::Table& tabl ) {
    size_t size_stops = stops.stops_size();
    for (int i = 0; i < size_stops; ++i) {
        transport_catalogue_serialize::Stop stop;
        stop = stops.stops(i);
        std::unordered_map<std::string, int> road_distances;
        size_t road_stops = stop.road_distances_size();
        for (int k = 0; k < road_stops; ++k) {
            std::string key = tabl.table_names().at(stop.road_distances(k).to()).name();
            int value = stop.road_distances(k).distance();
            road_distances.emplace(key, value);
        }
        TC.AddStop({ tabl.table_names().at(stop.name()).name(),stop.latitude(),stop.longitude(),road_distances });
    }
}

void AddStopsDistanceToTC(TransportCatalogue& TC ) {
    std::deque<const Stop*>tops_tc = TC.GetStops();
    size_t size_stops_tc = tops_tc.size();
    for (int i = 0; i < size_stops_tc; ++i) {
        std::string name_stop = tops_tc[i]->name;
        const Stop* stop_from = TC.GetStop(name_stop);

        for (const auto& [name_to_stop, distance] : tops_tc[i]->road_distances) {
            const Stop* stop_to = TC.GetStop(name_to_stop);
            TC.AddStopsDistance({ stop_from, stop_to }, distance);
        }
    }
}

void AddBusesToTC(TransportCatalogue& TC, transport_catalogue_serialize::Buses& buses, transport_catalogue_serialize::Table& tabl) {
    size_t size_buses = buses.buses_size();
    for (int i = 0; i < size_buses; ++i) {
        transport_catalogue_serialize::Bus bus;
        bus = buses.buses(i);

        std::deque<const Stop*>stops;

        size_t bus_stops = bus.stops_size();
        for (int k = 0; k < bus_stops; ++k) {
            stops.push_back(TC.GetStop(tabl.table_names().at(bus.stops(k).name()).name()));
        }

        TC.AddBus({ bus.name(),stops, bus.is_roundtrip() });
    }
}

void AddRoutingSettingsToTC(TransportCatalogue& TC, transport_catalogue_serialize::TransportCatalogue& db) {
    transport_catalogue_serialize::Routesettings rs;
    rs = db.rs();

    std::unordered_map<std::string, double> routing_settings_;
    routing_settings_.insert({ "bus_velocity"s,rs.bus_velocity() });
    routing_settings_.insert({ "bus_wait_time"s,rs.bus_wait_time() });
    TC.AddRoutingSettings(routing_settings_);
}

void AddRenderSettingsToTC(TransportCatalogue& TC, transport_catalogue_serialize::TransportCatalogue& db) {
    transport_catalogue_serialize::Rendersettings rendersettings;
    rendersettings = db.rends();
    RenderSettingsStruct map;
    map.bus_label_font_size = rendersettings.bus_label_font_size();
    std::vector<double> bus_label_offset;
    bus_label_offset.push_back(rendersettings.bus_label_offset().at(0));
    bus_label_offset.push_back(rendersettings.bus_label_offset().at(1));
    map.bus_label_offset = bus_label_offset;
    std::vector<svg::Color> palett;

    for (const auto& c : rendersettings.color_palette()) {
        if (c.has_collortxt()) {
            palett.push_back(c.collortxt().color());
        }
        else if (c.has_rgb()) {
            palett.push_back(svg::Rgb(c.rgb().red(), c.rgb().green(), c.rgb().blue()));
        }
        else  if (c.has_rgba()) {
            palett.push_back(svg::Rgba(c.rgba().red(), c.rgba().green(), c.rgba().blue(), c.rgba().opacity()));
        }
    }
    map.color_palette = palett;

    map.height = rendersettings.height();
    map.line_width = rendersettings.line_width();
    map.padding = rendersettings.padding();
    map.stop_label_font_size = rendersettings.stop_label_font_size();

    std::vector<double> stop_label_offset;
    stop_label_offset.push_back(rendersettings.stop_label_offset().at(0));
    stop_label_offset.push_back(rendersettings.stop_label_offset().at(1));
    map.stop_label_offset = stop_label_offset;

    map.stop_radius = rendersettings.stop_radius();
    svg::Color ucolor;
    const auto& c = rendersettings.underlayer_color();

    if (c.has_collortxt()) {
        ucolor = c.collortxt().color();
    }
    else if (c.has_rgb()) {
        ucolor = svg::Rgb(c.rgb().red(), c.rgb().green(), c.rgb().blue());
    }
    else  if (c.has_rgba()) {
        ucolor = svg::Rgba(c.rgba().red(), c.rgba().green(), c.rgba().blue(), c.rgba().opacity());
    }

    map.underlayer_color = ucolor;
    map.underlayer_width = rendersettings.underlayer_width();
    map.width = rendersettings.width();

    TC.AddRenderSettings(map);
}

void Deserialize(const std::filesystem::path& path, TransportCatalogue& TC) {

    std::ifstream in_file(path, std::ios::binary);
    transport_catalogue_serialize::TransportCatalogue db;
    if (!db.ParseFromIstream(&in_file)) {
        return  throw;
    }
    transport_catalogue_serialize::Objectbase base;
    transport_catalogue_serialize::Table tabl;
    transport_catalogue_serialize::Stops stops;
    transport_catalogue_serialize::Buses buses;  
    base = db.base();
    tabl = base.table();
    stops = base.stops();
    buses = base.buses();
    AddStopsToTC(TC, stops, tabl);
    AddStopsDistanceToTC(TC);
    AddBusesToTC(TC, buses, tabl);
    AddRoutingSettingsToTC(TC, db);
    AddRenderSettingsToTC(TC, db);

}
