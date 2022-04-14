#include "json_reader.h"
#include "request_handler.h"
using namespace transport::catalogue;
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

std::vector<json::Node> JsonBuses(const RouteInfo& response, json::Node& doc) {
    std::vector<json::Node> data;

    if (response.bus == nullptr) {
        data.push_back("\n    {\n        \"error_message\": \"not found\","s);
        data.push_back("\n        \"request_id\": "s);
        data.push_back(doc.AsMap().at("id"s).AsInt());
        data.push_back("\n    }"s);
    }
    else {
        data.push_back("\n    {\n        \"curvature\": "s);
        data.push_back(response.curvature);
        data.push_back(","s);
        data.push_back("\n        \"request_id\": "s);
        data.push_back(doc.AsMap().at("id"s).AsInt());
        data.push_back(","s);
        data.push_back("\n        \"route_length\": "s);
        data.push_back(response.routelength);
        data.push_back(","s);
        data.push_back("\n        \"stop_count\": "s);
        data.push_back(static_cast<int>(response.bus_stops_size));
        data.push_back(","s);
        data.push_back("\n        \"unique_stop_count\": "s);
        data.push_back(static_cast<int>(response.unique_stops));
        data.push_back("\n    }"s);
    }
    return data;
}

std::vector<json::Node> JsonStops(const StopInfo& response, json::Node& doc) {

    std::vector<json::Node> data;

    if (response.found == false) {
        data.push_back("\n    {\n        \"error_message\": \"not found\","s);
        data.push_back("\n        \"request_id\": "s);
        data.push_back(doc.AsMap().at("id"s).AsInt());
        data.push_back("\n    }"s);
    }

    else {
        data.push_back("\n    {\n        \"buses\": [\n"s);
        bool first_f = true;
        for (auto buses : response.sw) {
            if (first_f) {
                data.push_back("            \""s);
                data.push_back(static_cast<std::string>(buses.data()));
            }
            else {
                data.push_back(",\n            \""s);
                data.push_back(static_cast<std::string>(buses.data()));
            }
            data.push_back("\""s);
            first_f = false;
        }
        data.push_back("\n        ],\n        \"request_id\": "s);
        data.push_back(doc.AsMap().at("id").AsInt());
        data.push_back("\n    }"s);
    }

    return data;
}

std::string PrintSVG(std::istream& input) {
    std::string str;
    while (input) {
        char c = input.get();
        if (c == -1) {
            break;
        }
        if (c == '\n') {
            str += '\\';  str += 'n';
        }
        else  if (c == '\r') {
            str += '\\';  str += 'r';
        }
        else  if (c == '\"') {
            str += '\\'; str += '"';
        }
        else  if (c == '\\') {
            str += '\\'; str += '\\';
        }
        else {
            str += c;
        }
    }

    return str;
}

std::vector<json::Node>  JsonMap(RequestHandler& handler, json::Node& doc, json::Document& load_input) {

    std::vector<json::Node> data;
    json::Dict render_settings = load_input.GetRoot().AsMap().at("render_settings"s).AsMap();
    RenderXml render_xml(handler, render_settings);
    svg::Document&& doc_svg = std::move(render_xml.GetXml());
    std::stringstream  inputemp;
    doc_svg.Render(inputemp);

    data.push_back("\n    {\n"s);
    data.push_back("        \"map\": "s);
    data.push_back("\""s);
    data.push_back(PrintSVG(inputemp));
    data.push_back("\""s);
    data.push_back(","s);
    data.push_back("\n        \"request_id\": "s);
    data.push_back(doc.AsMap().at("id"s).AsInt());
    data.push_back("\n    }"s);
    return data;
}

void AddStopJson(RequestHandler& handler, json::Node load_stop) {

    std::unordered_map<std::string, int>tostop_;
    for (const auto& [_, road_distances] : load_stop.AsMap().at("road_distances").AsMap()) {
        tostop_[_] = road_distances.AsInt();
    }

    handler.AddStop({
        load_stop.AsMap().at("name").AsString(),
        load_stop.AsMap().at("latitude").AsDouble(),
        load_stop.AsMap().at("longitude").AsDouble()
       ,tostop_
        });
}

void AddBusJson(RequestHandler& handler, json::Node load_bus) {

    std::string name;
    std::deque<const Stop*> stops;
    bool is_roundtrip;

    if (load_bus.AsMap().at("is_roundtrip").AsBool()) {            //kolzo
        is_roundtrip = true;
        name = load_bus.AsMap().at("name").AsString();
        for (const auto& s : load_bus.AsMap().at("stops").AsArray()) {
            stops.push_back(handler.GetStop(s.AsString()));
        }
    }
    else {
        is_roundtrip = false;
        name = load_bus.AsMap().at("name").AsString();
        for (const auto& s : load_bus.AsMap().at("stops").AsArray()) {
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
    std::ofstream out_("out.txt");
    std::streambuf* coutbuf = out.rdbuf(); //save old buf 
    out.rdbuf(out_.rdbuf()); //redirect std::cout to out.t

    json::Array stat_requests = load_input.GetRoot().AsMap().at("stat_requests"s).AsArray();

    bool is_first = true;
    out << "[";
    for (auto& doc : stat_requests) {
        if (!is_first) {
            out << ",";
        }

        if (doc.AsMap().at("type"s).AsString() == "Bus"s) {
            const auto& jsonbuses = JsonBuses(handler.GetRouteInfo(doc.AsMap().at("name"s).AsString()), doc);
            for (const auto& bus : jsonbuses) {
                if (bus.IsString()) { 
                    out << bus.AsString(); 
                }
                else  if (bus.IsInt()) {
                    out << bus.AsInt();
                }
                else  if (bus.IsDouble()) {
                    out << bus.AsDouble();
                }
            }
        }

        if (doc.AsMap().at("type"s).AsString() == "Stop"s) {

            const auto& jsonstops = JsonStops(handler.GetListBuses(doc.AsMap().at("name"s).AsString()), doc);
            for (const auto& stop : jsonstops) {
                if (stop.IsString()) { 
                    out << stop.AsString(); 
                }
                else  if (stop.IsInt()) {
                    out << stop.AsInt();
                }
                else  if (stop.IsDouble()) { 
                    out << stop.AsDouble();
                }
            }
        }

        if (doc.AsMap().at("type"s).AsString() == "Map"s) {

            const auto& jsonmap = JsonMap(handler, doc, load_input);
            for (const auto& map : jsonmap) {
                if (map.IsString()) {
                    out << map.AsString();
                }
                else  if (map.IsInt()) {
                    out << map.AsInt(); 
                }
                else  if (map.IsDouble()) {
                    out << map.AsDouble();
                }
            }
        }

        is_first = false;
    }
    out << "\n]";
}

void JsonReader(std::istream& in, std::ostream& out) {

    TransportCatalogue TC;
    RequestHandler handler(TC);

    json::Document load_input = json::Load(in);//std::cin  
    json::Array base_requests = load_input.GetRoot().AsMap().at("base_requests"s).AsArray();

    json::Array base_stops;
    json::Array base_buses;

    for (const auto& doc : base_requests) {

        if (doc.AsMap().at("type"s).AsString() == "Stop"s) {
            base_stops.push_back(doc.AsMap());
        }

        if (doc.AsMap().at("type"s).AsString() == "Bus"s) {
            base_buses.push_back(doc.AsMap());
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
