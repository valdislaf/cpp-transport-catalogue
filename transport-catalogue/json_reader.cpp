#include "json_builder.h"
#include "json_reader.h"
#include "request_handler.h"
#include "router.h"
#include "transport_router.h"
//#include "log_duration.h"

using namespace transport::catalogue;
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

json::Dict JsonBuses(const RouteInfo& response, json::Node& doc) {

    if (response.route_length == 0.0 || response.bus == nullptr) {
        return  json::Builder{}.StartDict()
            .Key("error_message"s).Value("not found"s)
            .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
            .EndDict().Build().AsDict();
    }
    else {
        return  json::Builder{}.StartDict()
            .Key("curvature"s).Value(response.curvature)
            .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
            .Key("route_length"s).Value(response.route_length)
            .Key("stop_count"s).Value(static_cast<int>(response.bus_stops_size))
            .Key("unique_stop_count"s).Value(static_cast<int>(response.unique_stops))
            .EndDict().Build().AsDict();
    }
    return json::Dict();
}

json::Dict JsonStops(const StopInfo& response, json::Node& doc) {

    if (response.found == false) {
        return  json::Builder{}.StartDict()
            .Key("error_message"s).Value("not found"s)
            .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
            .EndDict().Build().AsDict();
    }
    else {
        json::Array arr;
        for (auto buses : response.sw) {
            arr.push_back(static_cast<std::string>(buses.data()));
        }
        return  json::Builder{}.StartDict()
            .Key("buses"s).Value(arr)
            .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
            .EndDict().Build().AsDict();
    }

    return json::Dict();
}

json::Dict JsonMap(RequestHandler& handler, json::Node& doc) {
    json::Dict dict;
    std::vector<json::Node> data;
    RenderXml render_xml(handler);
    svg::Document&& doc_svg = std::move(render_xml.GetXml());
    std::stringstream  inputemp;
    doc_svg.Render(inputemp);
    return json::Builder{}.StartDict()
        .Key("map"s).Value(inputemp.str())
        .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
        .EndDict().Build().AsDict();
}

void AddStopJson(TransportCatalogue& TC, json::Node load_stop) {
    std::unordered_map<std::string, int> road_distances;
    json::Dict  rd = load_stop.AsDict().at("road_distances"s).AsDict();
    for (const auto& x : rd) {
        road_distances.insert({ x.first ,x.second.AsInt() });
    }


    TC.AddStop({
        load_stop.AsDict().at("name"s).AsString(),
        load_stop.AsDict().at("latitude"s).AsDouble(),
        load_stop.AsDict().at("longitude"s).AsDouble(),
        road_distances
        });
}

void AddStopDistance(TransportCatalogue& TC, json::Node load_stop) {
    std::string name_stop = load_stop.AsDict().at("name"s).AsString();
    const Stop* stop_from = TC.GetStop(name_stop);
    for (const auto& [name_to_stop, road_distances] : load_stop.AsDict().at("road_distances"s).AsDict()) {
        const Stop* stop_to = TC.GetStop(name_to_stop);
        TC.AddStopsDistance({ stop_from, stop_to }, road_distances.AsInt());
    }
}

void AddBusJson(TransportCatalogue& TC, json::Node load_bus) {
    std::string name;
    std::deque<const Stop*> stops;
    bool is_roundtrip;
    if (load_bus.AsDict().at("is_roundtrip"s).AsBool()) {            //kolzo
        is_roundtrip = true;
        name = load_bus.AsDict().at("name"s).AsString();
        for (const auto& s : load_bus.AsDict().at("stops"s).AsArray()) {
            stops.push_back(TC.GetStop(s.AsString()));
        }
    }
    else {
        is_roundtrip = false;
        name = load_bus.AsDict().at("name"s).AsString();
        for (const auto& s : load_bus.AsDict().at("stops"s).AsArray()) {
            stops.push_back(TC.GetStop(s.AsString()));
        }

        size_t sz = stops.size();
        for (size_t i = sz - 1; i > 0; i--) {
            stops.push_back(stops[i - 1]);
        }
    }

    TC.AddBus({ name, stops, is_roundtrip });
}

void FormatResponse(json::Document& load_input, RequestHandler& handler, std::ostream& out) {
    //выводим в файл
    std::ofstream out_("out.json"s);
    std::streambuf* coutbuf = out.rdbuf(); //save old buf 
    out.rdbuf(out_.rdbuf()); //redirect std::cout to out.t
  //  LOG_DURATION("test");
    json::Array stat_requests = load_input.GetRoot().AsDict().at("stat_requests"s).AsArray();
    json::Array final_array;

    using namespace graph;
    RouteSettings route_settings;
    std::unordered_map<std::string, double> rs = handler.GetRoutingSettings();
    if (rs.at("bus_velocity"s) == 0) { route_settings.bus_velocity = 0.0; }
    else {
        route_settings.bus_velocity = 0.06 / rs.at("bus_velocity"s);
    }
    route_settings.bus_wait_time = rs.at("bus_wait_time"s);

    TransportRouter TR(handler, route_settings);

    auto& graf_stops = TR.GetGraf();
    auto& stops_info = TR.GetStopInfo();
    graph::Router router_stops(graf_stops);
    const std::vector<graph::Edge<double>>& vec_info = graf_stops.GetEdges();

    for (auto& doc : stat_requests) {
        if (doc.AsDict().at("id") == 1151953891) {
            int stop = 0;
        }
        if (doc.AsDict().at("type"s).AsString() == "Bus"s) {
            const auto& json_buses = JsonBuses(handler.GetRouteInfo(doc.AsDict().at("name"s).AsString()), doc);
            final_array.push_back(json_buses);
        }

        if (doc.AsDict().at("type"s).AsString() == "Stop"s) {
            const auto& json_stops = JsonStops(handler.GetListBuses(doc.AsDict().at("name"s).AsString()), doc);
            final_array.push_back(json_stops);
        }

        if (doc.AsDict().at("type"s).AsString() == "Map"s) {
            const auto& json_map = JsonMap(handler, doc);
            final_array.push_back(json_map);
        }

        if (doc.AsDict().at("type"s).AsString() == "Route"s) {

            const auto& json_stop_from = doc.AsDict().at("from"s).AsString();//остановка от
            const auto& json_stop_to = doc.AsDict().at("to"s).AsString();//остановка до 

            json::Array Items;
            json::Dict dict;
            if (stops_info.count(json_stop_from) == 0 || stops_info.count(json_stop_to) == 0) {
                dict = json::Builder{}.StartDict()
                    .Key("error_message"s).Value("not found"s)
                    .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
                    .EndDict().Build().AsDict();
            }
            else {
                size_t index_from = stops_info.at(std::string_view(json_stop_from));
                size_t index_to = stops_info.at(std::string_view(json_stop_to));
                const std::optional<graph::Router <double>::RouteInfo>& min_route_g = router_stops.BuildRoute(index_from, index_to);

                if (min_route_g.has_value()) {
                    for (const auto& e : min_route_g.value().edges) {
                        const  auto& edge = vec_info.at(e);
                        if (edge.stop) {
                            json::Dict wait = json::Builder{}.StartDict()
                                .Key("type"s).Value("Wait"s)
                                .Key("stop_name"s).Value(std::string(edge.name))
                                .Key("time"s).Value(edge.weight).EndDict().Build().AsDict();
                            Items.push_back(wait);
                        }
                        else {
                            json::Dict bus = json::Builder{}.StartDict()
                                .Key("type"s).Value("Bus"s)
                                .Key("bus"s).Value(std::string(edge.name))
                                .Key("span_count"s).Value(static_cast<int>(edge.span_count))
                                .Key("time"s).Value(edge.weight)
                                .EndDict().Build().AsDict();
                            Items.push_back(bus);
                        }
                    }

                    dict = json::Builder{}.StartDict()
                        .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
                        .Key("total_time"s).Value(min_route_g.value().weight)
                        .Key("items"s).Value(Items)
                        .EndDict().Build().AsDict();
                }
                else {
                    dict = json::Builder{}.StartDict()
                        .Key("error_message"s).Value("not found"s)
                        .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
                        .EndDict().Build().AsDict();
                }
            }
            final_array.push_back(dict);
        }
    }

    json::Document doc(final_array);
    json::Print(doc, out);

}

void JsonReaderMakeBase(std::istream& in) {

    TransportCatalogue TC;
    RequestHandler handler(TC);
    json::Document load_input = json::Load(in);//std::cin  
    json::Array base_requests = load_input.GetRoot().AsDict().at("base_requests"s).AsArray();
    json::Array base_stops;
    json::Array base_buses;

    for (const auto& doc : base_requests) {

        if (doc.AsDict().at("type"s).AsString() == "Stop"s) {
            base_stops.push_back(doc.AsDict());
        }

        if (doc.AsDict().at("type"s).AsString() == "Bus"s) {
            base_buses.push_back(doc.AsDict());
        }
    }

    for (const auto& doc : base_stops) {
        AddStopJson(TC, doc);
    }

    for (const auto& doc : base_stops) {
        AddStopDistance(TC, doc);
    }

    for (const auto& doc : base_buses) {
        AddBusJson(TC, doc);
    }

    if (load_input.GetRoot().AsDict().count("routing_settings"s)) {
        json::Dict  routing_settings = load_input.GetRoot().AsDict().at("routing_settings"s).AsDict();
        std::unordered_map<std::string, double> routing_settings_;
        routing_settings_.insert({ "bus_velocity"s,routing_settings.at("bus_velocity"s).AsInt() });
        routing_settings_.insert({ "bus_wait_time"s,routing_settings.at("bus_wait_time"s).AsInt() });
        TC.AddRoutingSettings(routing_settings_);
    }

    json::Dict serialization_settings = load_input.GetRoot().AsDict().at("serialization_settings"s).AsDict();

    json::Dict render_settings = load_input.GetRoot().AsDict().at("render_settings"s).AsDict();

    RenderSettingsStruct map;
    for (const auto& v : render_settings) {

        if (v.first == "bus_label_font_size"s) {
            map.bus_label_font_size = v.second.AsInt();
        }

        if (v.first == "bus_label_offset"s) {
            std::vector<double>  offset;
            for (const auto& n : v.second.AsArray()) {
                offset.push_back(n.AsDouble());
            }          
            map.bus_label_offset = offset;
        }

        if (v.first == "color_palette"s) {
            std::vector<svg::Color>  colors;
            for (const auto& n : v.second.AsArray()) {
                colors.push_back(GetColorFromDict(n));
            }           
            map.color_palette = colors;
        }

        if (v.first == "height"s) {
            map.height = v.second.AsDouble();
        }

        if (v.first == "line_width"s) {
            map.line_width = v.second.AsDouble();
        }

        if (v.first == "padding"s) {
            map.padding = v.second.AsDouble();
        }

        if (v.first == "stop_label_font_size"s) {
            map.stop_label_font_size = v.second.AsInt();
        }

        if (v.first == "stop_label_offset"s) {
            std::vector<double>  offset;
            for (const auto& n : v.second.AsArray()) {
                offset.push_back(n.AsDouble());
            }
            map.stop_label_offset = offset;
        }

        if (v.first == "stop_radius"s) {
            map.stop_radius = v.second.AsDouble();
        }

        if (v.first == "underlayer_color"s) {    
            map.underlayer_color = GetColorFromDict(v.second);
        }

        if (v.first == "underlayer_width"s) {
            map.underlayer_width = v.second.AsDouble();
        }

        if (v.first == "width"s) {
            map.width = v.second.AsDouble();
        }
    }
    TC.AddRenderSettings(map);

    Serialize(serialization_settings.at("file").AsString(), handler);
}

void JsonReaderProcessRequests(std::istream& in, std::ostream& out) {
    TransportCatalogue TC;
    RequestHandler handler(TC);
    json::Document load_input = json::Load(in);//std::cin  
    json::Dict serialization_settings = load_input.GetRoot().AsDict().at("serialization_settings"s).AsDict();

    Deserialize(serialization_settings.at("file").AsString(), TC);

    FormatResponse(load_input, handler, out);
}