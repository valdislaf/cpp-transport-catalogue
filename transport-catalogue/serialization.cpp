#include "serialization.h"

#include <map>
#include <string>
#include <unordered_map>

using namespace transport::catalogue;
using namespace std::literals;

void Serialize(const std::filesystem::path& path, RequestHandler& handler) {

    std::ofstream out_file(path, std::ios::binary);
    transport_catalogue_serialize::Stops stops;
    transport_catalogue_serialize::Buses buses;
    transport_catalogue_serialize::TransportCatalogue db;
    transport_catalogue_serialize::Routesettings rs;

    if (handler.GetRoutingSettings().size() != 0) {
        rs.set_bus_velocity(handler.GetRoutingSettings().at("bus_velocity"s));
        rs.set_bus_wait_time(handler.GetRoutingSettings().at("bus_wait_time"s));
        *db.mutable_rs() = rs;
    }

    transport_catalogue_serialize::Table tabl;
    uint32_t imya = 0;
    for (const auto& s : handler.GetStops()) {
        tabl.mutable_table_stops()->insert({ s->name, imya });       
        ++imya;
    }

    *db.mutable_table()=tabl;

    for (const auto& s : handler.GetStops()) {
        transport_catalogue_serialize::Stop stop;
        stop.set_name(tabl.table_stops().at(s->name));
        stop.set_latitude(s->coord.lat);
        stop.set_longitude(s->coord.lng);
        const auto& road_stops = s->road_distances;
        for (const auto& rs : road_stops) {
            transport_catalogue_serialize::Road road;
            road.set_to(tabl.table_stops().at(rs.first));
            road.set_distance(rs.second);
            *stop.add_road_distances() = road;
        }
        *stops.add_stops() = stop;
    }

    for (const auto& b : handler.GetBuses()) {
        transport_catalogue_serialize::Bus bus;
        bus.set_name(b->name);
        bus.set_is_roundtrip(b->is_roundtrip);
        for (const auto& s : b->stops) {
            transport_catalogue_serialize::Stop stop;
            stop.set_name(tabl.table_stops().at(s->name));
            *bus.add_stops() = stop;
        }   
        *buses.add_buses() = bus;
    }

    const auto& rends_tc = handler.GetRenderSettings();
    transport_catalogue_serialize::Rendersettings rendersettings;
    rendersettings.set_bus_label_font_size(std::get<int>(rends_tc.at("bus_label_font_size"s)));
    rendersettings.add_bus_label_offset(std::get<std::vector<double>>(rends_tc.at("bus_label_offset"s))[0]);
    rendersettings.add_bus_label_offset(std::get<std::vector<double>>(rends_tc.at("bus_label_offset"s))[1]);
    for (const auto& col : std::get<std::vector<svg::Color>>(rends_tc.at("color_palette"s))) {        
        if (std::holds_alternative<std::string>(col)) {
            rendersettings.add_color_palette()->mutable_collortxt()->set_color(std::get<std::string>(col));
        }
        else if (std::holds_alternative<svg::Rgb>(col)) {
            transport_catalogue_serialize::Rgb rgb;           
            rgb.set_red(std::get<svg::Rgb>(col).red);
            rgb.set_green(std::get<svg::Rgb>(col).green);
            rgb.set_blue(std::get<svg::Rgb>(col).blue);
            *rendersettings.add_color_palette()->mutable_rgb() = rgb;
        }
        else if (std::holds_alternative<svg::Rgba>(col)) {
            transport_catalogue_serialize::Rgba rgba;
            rgba.set_red(std::get<svg::Rgba>(col).red);
            rgba.set_green(std::get<svg::Rgba>(col).green);
            rgba.set_blue(std::get<svg::Rgba>(col).blue);
            rgba.set_opacity(std::get<svg::Rgba>(col).opacity);
            *rendersettings.add_color_palette()->mutable_rgba() = rgba;
        }
    }

    try {
        rendersettings.set_height(std::get<double>(rends_tc.at("height"s)));
    }

    catch (...) {
        rendersettings.set_height(std::get<int>(rends_tc.at("height"s)));
    }

    try {
        rendersettings.set_line_width(std::get<double>(rends_tc.at("line_width"s)));
    }

    catch (...) {
        rendersettings.set_line_width(std::get<int>(rends_tc.at("line_width"s)));
    }

    try {
        rendersettings.set_padding(std::get<double>(rends_tc.at("padding"s)));
    }

    catch (...) {
        rendersettings.set_padding(std::get<int>(rends_tc.at("padding"s)));
    }   
   
    rendersettings.set_stop_label_font_size(std::get<int>(rends_tc.at("stop_label_font_size"s)));
    rendersettings.add_stop_label_offset(std::get<std::vector<double>>(rends_tc.at("stop_label_offset"s))[0]);
    rendersettings.add_stop_label_offset(std::get<std::vector<double>>(rends_tc.at("stop_label_offset"s))[1]);

    try {
        rendersettings.set_stop_radius(std::get<double>(rends_tc.at("stop_radius"s)));
    }
    catch (...) {
        rendersettings.set_stop_radius(std::get<int>(rends_tc.at("stop_radius"s)));
    }   

    const auto& colr = std::get<svg::Color>(rends_tc.at("underlayer_color"s));
    if (std::holds_alternative<std::string>(colr)) {
        rendersettings.mutable_underlayer_color()->mutable_collortxt()->set_color(std::get<std::string>(colr));
    }

    else if (std::holds_alternative<svg::Rgb>(colr)) {
        transport_catalogue_serialize::Rgb rgb;
        rgb.set_red(std::get<svg::Rgb>(colr).red);
        rgb.set_green(std::get<svg::Rgb>(colr).green);
        rgb.set_blue(std::get<svg::Rgb>(colr).blue);
        *rendersettings.mutable_underlayer_color()->mutable_rgb() = rgb;
    }

    else if (std::holds_alternative<svg::Rgba>(colr)) {
        transport_catalogue_serialize::Rgba rgba;
        rgba.set_red(std::get<svg::Rgba>(colr).red);
        rgba.set_green(std::get<svg::Rgba>(colr).green);
        rgba.set_blue(std::get<svg::Rgba>(colr).blue);
        rgba.set_opacity(std::get<svg::Rgba>(colr).opacity);
        *rendersettings.mutable_underlayer_color()->mutable_rgba() = rgba;
    }

    try {
        rendersettings.set_underlayer_width(std::get<double>(rends_tc.at("underlayer_width"s)));
    }

    catch (...) {
        rendersettings.set_underlayer_width(std::get<int>(rends_tc.at("underlayer_width"s)));
    }    

    try {
        rendersettings.set_width(std::get<double>(rends_tc.at("width"s)));
    }

    catch (...) {
        rendersettings.set_width(std::get<int>(rends_tc.at("width"s)));
    }

    *db.mutable_rends() = rendersettings;
    *db.mutable_stops() = stops;
    *db.mutable_buses() = buses;
    db.SerializeToOstream(&out_file);
}

void Deserialize(const std::filesystem::path& path, TransportCatalogue& TC) {   

    std::ifstream in_file(path, std::ios::binary);  

    transport_catalogue_serialize::TransportCatalogue db;
    if (!db.ParseFromIstream(&in_file)) {
        return  throw;
    }

    transport_catalogue_serialize::Table tabl;
    transport_catalogue_serialize::Stops stops;
    transport_catalogue_serialize::Buses buses;
    transport_catalogue_serialize::Rendersettings rendersettings;
    tabl = db.table();
    stops = db.stops();
    buses = db.buses();
    rendersettings = db.rends();
    transport_catalogue_serialize::Tableinvert tabl_invert;
  
    for (const auto& s : tabl.table_stops()) {
        tabl_invert.mutable_table_invert_stops()->insert({ s.second, s.first });
    }

    size_t size_stops = stops.stops_size();
    for (int i = 0; i < size_stops; ++i) {
        transport_catalogue_serialize::Stop stop;
        stop = stops.stops(i);
        std::unordered_map<std::string, int> road_distances;
        size_t road_stops = stop.road_distances_size();
        for (int k = 0; k < road_stops; ++k) {
            std::string key = tabl_invert.table_invert_stops().at(stop.road_distances(k).to());
            int value = stop.road_distances(k).distance();
            road_distances.emplace(key, value);
        }
        TC.AddStop({ tabl_invert.table_invert_stops().at(stop.name()),stop.latitude(),stop.longitude(),road_distances });         
    }

    std::deque<const Stop*>tops_tc = TC.GetStops();
    size_t size_stops_tc= tops_tc.size();
    for (int i = 0; i < size_stops_tc; ++i) {
        std::string name_stop = tops_tc[i]->name;
        const Stop* stop_from = TC.GetStop(name_stop);

        for (const auto& [name_to_stop, distance] : tops_tc[i]->road_distances) {
            const Stop* stop_to = TC.GetStop(name_to_stop);
            TC.AddStopsDistance({ stop_from, stop_to }, distance);
        }
    }

 size_t size_buses = buses.buses_size();
    for (int i = 0; i < size_buses; ++i) {
        transport_catalogue_serialize::Bus bus;
        bus = buses.buses(i);
       
        std::deque<const Stop*>stops;

        size_t bus_stops = bus.stops_size();
        for (int k = 0; k < bus_stops; ++k) {
            stops.push_back(TC.GetStop(tabl_invert.table_invert_stops().at(bus.stops(k).name())));
        }

        TC.AddBus({ bus.name(),stops, bus.is_roundtrip() });
    }
   
    transport_catalogue_serialize::Routesettings rs;
    rs = db.rs();

    std::unordered_map<std::string, double> routing_settings_;
    routing_settings_.insert({ "bus_velocity"s,rs.bus_velocity() });
    routing_settings_.insert({ "bus_wait_time"s,rs.bus_wait_time()});
    TC.AddRoutingSettings(routing_settings_);  

    using value_4_map = std::variant<std::monostate, int, double, std::vector<double>, svg::Color, std::vector<svg::Color>>;
    std::map<std::string, value_4_map>map;
    map.insert({ "bus_label_font_size"s, (int)rendersettings.bus_label_font_size() });
    std::vector<double> bus_label_offset;
    bus_label_offset.push_back(rendersettings.bus_label_offset().at(0));
    bus_label_offset.push_back(rendersettings.bus_label_offset().at(1));
    map.insert({ "bus_label_offset"s, bus_label_offset });
    std::vector<svg::Color> palett;

    for (const auto& c : rendersettings.color_palette()) {
       if( c.has_collortxt()){
           palett.push_back(c.collortxt().color());
       }
       else if (c.has_rgb()) {
           palett.push_back(svg::Rgb(c.rgb().red(), c.rgb().green(), c.rgb().blue()));
       }
       else  if (c.has_rgba()) {
           palett.push_back(svg::Rgba(c.rgba().red(), c.rgba().green(), c.rgba().blue(), c.rgba().opacity()));
       }
    }
    map.insert({ "color_palette"s, palett });

    map.insert({ "height"s, rendersettings.height() });
    map.insert({ "line_width"s, rendersettings.line_width() });
    map.insert({ "padding"s, rendersettings.padding() });
    map.insert({ "stop_label_font_size"s,(int)rendersettings.stop_label_font_size() });

    std::vector<double> stop_label_offset;
    stop_label_offset.push_back(rendersettings.stop_label_offset().at(0));
    stop_label_offset.push_back(rendersettings.stop_label_offset().at(1));
    map.insert({ "stop_label_offset"s, stop_label_offset });

    map.insert({ "stop_radius"s, rendersettings.stop_radius() });
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

    map.insert({ "underlayer_color"s, ucolor });
    map.insert({ "underlayer_width"s, rendersettings.underlayer_width() });
    map.insert({ "width"s, rendersettings.width() });

    TC.AddRenderSettings(map);
}
