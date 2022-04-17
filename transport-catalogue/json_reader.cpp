#include "json_reader.h"
#include "request_handler.h"
using namespace transport::catalogue;
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

json::Dict JsonBuses(const RouteInfo& response, json::Node& doc) {

    json::Dict dict;
    if (response.bus == nullptr) {
        dict["error_message"s] = "not found"s;
        dict["request_id"s] = doc.AsMap().at("id"s).AsInt();
    }
    else {
        dict["curvature"s] = response.curvature;
        dict["request_id"s] = doc.AsMap().at("id"s).AsInt();
        dict["route_length"s] = response.routelength;
        dict["stop_count"s] = static_cast<int>(response.bus_stops_size);
        dict["unique_stop_count"s] = static_cast<int>(response.unique_stops);
    }

    return dict;
}

json::Dict JsonStops(const StopInfo& response, json::Node& doc) {

    json::Dict dict;
    json::Array arr;
    if (response.found == false) {
        dict["error_message"s] = "not found"s;
        dict["request_id"s] = doc.AsMap().at("id"s).AsInt();
    }
    else {
        for (auto buses : response.sw) {
            arr.push_back(static_cast<std::string>(buses.data()));
        }

        dict["buses"s] = arr;
        dict["request_id"s] = doc.AsMap().at("id").AsInt();
    }

    return dict;
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

json::Dict JsonMap(RequestHandler& handler, json::Node& doc, json::Document& load_input) {

    json::Dict dict;
    std::vector<json::Node> data;
    json::Dict render_settings = load_input.GetRoot().AsMap().at("render_settings"s).AsMap();
    RenderXml render_xml(handler, render_settings);
    svg::Document&& doc_svg = std::move(render_xml.GetXml());
    std::stringstream  inputemp;
    doc_svg.Render(inputemp);
    dict["map"s] = inputemp.str();
    dict["request_id"s] = doc.AsMap().at("id"s).AsInt();

    return dict;
}

void AddStopJson(RequestHandler& handler, json::Node load_stop) {

    std::string name_stop = load_stop.AsMap().at("name"s).AsString();
    std::unordered_map<std::string, int>tostop_;
    for (const auto& [name_to_stop, road_distances] : load_stop.AsMap().at("road_distances"s).AsMap()) {
        handler.AddStopsLength(name_stop + name_to_stop, road_distances.AsInt());
    }

    handler.AddStop({
        name_stop,
        load_stop.AsMap().at("latitude"s).AsDouble(),
        load_stop.AsMap().at("longitude"s).AsDouble()
        });
}

void AddBusJson(RequestHandler& handler, json::Node load_bus) {

    std::string name;
    std::deque<const Stop*> stops;
    bool is_roundtrip;
    if (load_bus.AsMap().at("is_roundtrip"s).AsBool()) {            //kolzo
        is_roundtrip = true;
        name = load_bus.AsMap().at("name"s).AsString();
        for (const auto& s : load_bus.AsMap().at("stops"s).AsArray()) {
            stops.push_back(handler.GetStop(s.AsString()));
        }
    }
    else {
        is_roundtrip = false;
        name = load_bus.AsMap().at("name"s).AsString();
        for (const auto& s : load_bus.AsMap().at("stops"s).AsArray()) {
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

    json::Array stat_requests = load_input.GetRoot().AsMap().at("stat_requests"s).AsArray();
    json::Array final_array;

    for (auto& doc : stat_requests) {
        if (doc.AsMap().at("type"s).AsString() == "Bus"s) {
            const auto& jsonbuses = JsonBuses(handler.GetRouteInfo(doc.AsMap().at("name"s).AsString()), doc);
            final_array.push_back(jsonbuses);
        }

        if (doc.AsMap().at("type"s).AsString() == "Stop"s) {
            const auto& jsonstops = JsonStops(handler.GetListBuses(doc.AsMap().at("name"s).AsString()), doc);
            final_array.push_back(jsonstops);
        }

        if (doc.AsMap().at("type"s).AsString() == "Map"s) {
            const auto& jsonmap = JsonMap(handler, doc, load_input);
            final_array.push_back(jsonmap);
        }
    }

    json::Document d(final_array);
    json::Print(d, out);

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
