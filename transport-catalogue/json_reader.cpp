#include "json_builder.h"
#include "json_reader.h"
#include "request_handler.h"
#include "router.h"
#include "log_duration.h"

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

json::Dict JsonMap(RequestHandler& handler, json::Node& doc, json::Document& load_input) {
    json::Dict dict;
    std::vector<json::Node> data;
    json::Dict render_settings = load_input.GetRoot().AsDict().at("render_settings"s).AsDict();
    RenderXml render_xml(handler, render_settings);
    svg::Document&& doc_svg = std::move(render_xml.GetXml());
    std::stringstream  inputemp;
    doc_svg.Render(inputemp);
    return json::Builder{}.StartDict()
        .Key("map"s).Value(inputemp.str())
        .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
        .EndDict().Build().AsDict();
}

void AddStopJson(RequestHandler& handler, json::Node load_stop) {
    std::string name_stop = load_stop.AsDict().at("name"s).AsString();
    std::unordered_map<std::string, int>tostop_;
    for (const auto& [name_to_stop, road_distances] : load_stop.AsDict().at("road_distances"s).AsDict()) {
        handler.AddStopsLength(name_stop + name_to_stop, road_distances.AsInt());
    }
    handler.AddStop({
        name_stop,
        load_stop.AsDict().at("latitude"s).AsDouble(),
        load_stop.AsDict().at("longitude"s).AsDouble()
        });
}

void AddStopDistance(RequestHandler& handler, json::Node load_stop) {
    std::string name_stop = load_stop.AsDict().at("name"s).AsString();
    const Stop* stop1 = handler.GetStop(name_stop);
    std::unordered_map<std::string, int>tostop_;
    for (const auto& [name_to_stop, road_distances] : load_stop.AsDict().at("road_distances"s).AsDict()) {
        const Stop* stop2 = handler.GetStop(name_to_stop);
        handler.AddStopsDistance({ stop1, stop2 }, road_distances.AsInt());
    }
}

void AddBusJson(RequestHandler& handler, json::Node load_bus) {
    std::string name;
    std::deque<const Stop*> stops;
    bool is_roundtrip;
    if (load_bus.AsDict().at("is_roundtrip"s).AsBool()) {            //kolzo
        is_roundtrip = true;
        name = load_bus.AsDict().at("name"s).AsString();
        for (const auto& s : load_bus.AsDict().at("stops"s).AsArray()) {
            stops.push_back(handler.GetStop(s.AsString()));
        }
    }
    else {
        is_roundtrip = false;
        name = load_bus.AsDict().at("name"s).AsString();
        for (const auto& s : load_bus.AsDict().at("stops"s).AsArray()) {
            stops.push_back(handler.GetStop(s.AsString()));
        }

        size_t sz = stops.size();
        for (size_t i = sz - 1; i > 0; i--) {
            stops.push_back(stops[i - 1]);
        }
    }

    handler.AddBus({ name, stops, is_roundtrip });
}

void FormatResponse(json::Document& load_input, RequestHandler& handler, std::ostream& out) {
    //выводим в файл
    std::ofstream out_("out.txt"s);
    std::streambuf* coutbuf = out.rdbuf(); //save old buf 
    out.rdbuf(out_.rdbuf()); //redirect std::cout to out.t

    json::Array stat_requests = load_input.GetRoot().AsDict().at("stat_requests"s).AsArray();
    json::Array final_array;

    using namespace graph;
    const auto& rs = load_input.GetRoot().AsDict().at("routing_settings"s);
    double bus_velocity = 0.06 / rs.AsDict().at("bus_velocity"s).AsDouble();
    double bus_wait_time = rs.AsDict().at("bus_wait_time"s).AsDouble();

    const std::deque<const Stop*>stops_ = handler.GetStops();

    std::unordered_map<std::string_view, size_t> stopsinfo;
    DirectedWeightedGraph<double> graf_stops(stops_.size() * 2);
    stopsinfo.reserve(stops_.size() * 2);
    size_t j = 0;
    for (size_t i = 0; i < stops_.size(); ++i) {
        stopsinfo[stops_[i]->name] = j;
        graf_stops.AddEdge({ j ,j + 1 , bus_wait_time , stops_[i]->name,0,true });
        j = j + 2;
    }
    std::deque<const Bus*>buses = handler.GetBuses();

    const auto& stops_distance_ = handler.GetStopsDistance();

    const auto& stop_to_stop_ = handler.GetStopsLengths();
    {
        LOG_DURATION("test");

        for (const auto& name : buses) {
            for (size_t j = 1; j < name->stops.size(); ++j) {
                for (size_t i = 0; i < name->stops.size() - j; ++i) {
                    double route_lenght = 0.0;
                    size_t index = 0;
                    size_t index_next = 0;
                    index = stopsinfo.at(std::string_view(name->stops[i]->name));
                    index_next = stopsinfo.at(std::string_view(name->stops[i + j]->name));
                    size_t n = 0;
                    while ((n < j)) {

                        if (stops_distance_.count({ name->stops[i + n] ,name->stops[i + 1 + n] })) {
                            route_lenght += stops_distance_.at({ name->stops[i + n] ,name->stops[i + 1 + n] });
                        }
                        else {
                            route_lenght += stops_distance_.at({ name->stops[i + 1 + n] ,name->stops[i + n] });
                        }
                        n++;
                    }
                    if (route_lenght > 0.0) {
                        double edge_weight = route_lenght * bus_velocity;
                        graf_stops.AddEdge({ index + 1 ,index_next, edge_weight ,name->name ,j ,false });
                    }
                }
            }
        }
    }

    graph::Router router_stops(graf_stops);
    const std::vector<graph::Edge<double>>& vec_info = graf_stops.GetEdges();

    for (auto& doc : stat_requests) {
        if (doc.AsDict().at("type"s).AsString() == "Bus"s) {
            const auto& json_buses = JsonBuses(handler.GetRouteInfo(doc.AsDict().at("name"s).AsString()), doc);
            final_array.push_back(json_buses);
        }

        if (doc.AsDict().at("type"s).AsString() == "Stop"s) {
            const auto& json_stops = JsonStops(handler.GetListBuses(doc.AsDict().at("name"s).AsString()), doc);
            final_array.push_back(json_stops);
        }

        if (doc.AsDict().at("type"s).AsString() == "Map"s) {
            const auto& json_map = JsonMap(handler, doc, load_input);
            final_array.push_back(json_map);
        }

        if (doc.AsDict().at("type"s).AsString() == "Route"s) {

            const auto& json_stop_from = doc.AsDict().at("from"s).AsString();//остановка от
            const auto& json_stop_to = doc.AsDict().at("to"s).AsString();//остановка до 

            json::Array Items;
            json::Dict dict;
            if (stopsinfo.count(json_stop_from) == 0 || stopsinfo.count(json_stop_to) == 0) {
                dict = json::Builder{}.StartDict()
                    .Key("error_message"s).Value("not found"s)
                    .Key("request_id"s).Value(doc.AsDict().at("id"s).AsInt())
                    .EndDict().Build().AsDict();
            }
            else {
                size_t index_from = stopsinfo.at(std::string_view(json_stop_from));
                size_t index_to = stopsinfo.at(std::string_view(json_stop_to));
                const   std::optional<graph::Router <double>::RouteInfo>& min_route_g = router_stops.BuildRoute(index_from, index_to);

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

void JsonReader(std::istream& in, std::ostream& out) {

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
        AddStopJson(handler, doc);
    }

    for (const auto& doc : base_stops) {
        AddStopDistance(handler, doc);
    }

    for (const auto& doc : base_buses) {
        AddBusJson(handler, doc);
    }

    FormatResponse(load_input, handler, out);
}
