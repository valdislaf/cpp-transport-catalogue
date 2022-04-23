#include "json_builder.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace transport::catalogue;
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

json::Dict JsonBuses(const RouteInfo& response, json::Node& doc) {

    if (response.bus == nullptr) {
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
    //std::ofstream out_("out.txt"s);
    //std::streambuf* coutbuf = out.rdbuf(); //save old buf 
    //out.rdbuf(out_.rdbuf()); //redirect std::cout to out.t

    json::Array stat_requests = load_input.GetRoot().AsDict().at("stat_requests"s).AsArray();
    json::Array final_array;

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

    for (const auto& doc : base_buses) {
        AddBusJson(handler, doc);
    }

    FormatResponse(load_input, handler, out);
}
