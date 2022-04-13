#include "json_reader.h"
   
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

void PrintBuses(TransportCatalogue& TC, json::Node& doc) {
    transport::catalogue::RouteInfo response = TC.GetRouteInfo(doc.AsMap().at("name"s).AsString());
    if (response.bus == nullptr) {
        std::cout << "\n    {\n        \"error_message\": \"not found\","s;
        std::cout << "\n        \"request_id\": "s + std::to_string(doc.AsMap().at("id"s).AsInt());
        std::cout << "\n    }";
    }
    else {
        std::cout << "\n    {\n        \"curvature\": "s;        
        std::cout << response.curvature;
        std::cout << ","s;
        std::cout << "\n        \"request_id\": "s;
        std::cout << doc.AsMap().at("id"s).AsInt();
        std::cout << ","s;
        std::cout << "\n        \"route_length\": "s;
        std::cout << response.routelength;        
        std::cout << ","s;
        std::cout << "\n        \"stop_count\": "s;
        std::cout << response.bus_stops_size;
        std::cout << ","s;
        std::cout << "\n        \"unique_stop_count\": "s;
        std::cout << response.unique_stops;
        std::cout << "\n    }"s;       
    }
}

void PrintStops(TransportCatalogue& TC, json::Node& doc) {
    transport::catalogue::StopInfo response = TC.GetListBuses(doc.AsMap().at("name"s).AsString());

    json::Array stops;

    if (response.found == false) {
        std::cout << "\n    {\n        \"error_message\": \"not found\","s;
        std::cout << "\n        \"request_id\": "s + std::to_string(doc.AsMap().at("id"s).AsInt());
        std::cout << "\n    }";
    }

    else {

        std::cout << "\n    {\n        \"buses\": [\n"s;
        bool first_f = true;
        for (auto buses : response.sw) {
            if (first_f) {
                std::cout << "            \""s;
                std::cout << buses.data();

            }
            else {
                std::cout << ",\n            \""s;
                std::cout << buses.data();

            }
            std::cout << "\""s;
            first_f = false;
        }

        std::cout << "\n        ],\n        \"request_id\": "s + std::to_string(doc.AsMap().at("id").AsInt()) + "\n    }"s;

    }
}


void PrintSVG(std::istream& input) {
    while (input) {
        char c = input.get();
        if (c == -1)
        {
            break;
        }
        if (c == '\n') { std::cout << '\\';  std::cout << 'n'; }
        else  if (c == '\r') { std::cout << '\\';  std::cout << 'r'; }
        else  if (c == '\"') { std::cout << '\\'; std::cout << '"'; }
        else  if (c == '\\') { std::cout << '\\';  std::cout << '\\'; }
        else { std::cout << c; }
    }
}

void PrintMap(TransportCatalogue& TC, json::Node& doc, json::Dict render_settings) {

    RenderSettings rs = SetRenerSettings(render_settings);

    auto& stops = TC.GetStops();
    auto& buses = TC.GetBuses();

    std::deque<Coordinates>coord_stops;  

    for (auto& stop : stops) {
       int chek_stop = 0;
        for (auto& bus : buses)
        {
            std::string name = stop->name().data();
            for (auto& stop : bus->stops) {
                if (stop->name().data() == name) { ++chek_stop; break; }
            }

        }

        if (chek_stop > 0) {
            // только непустые остановки!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            coord_stops.push_back({ stop->latitude(), stop->longitude() });
        }
    }

    SphereProjector SP(coord_stops.begin(), coord_stops.end(), rs.width, rs.height, rs.padding);

    svg::Document docSVG;
    int col_i = 0;

    std::cout << "\n    {\n"s;
    std::cout << "        \"map\": "s;
    {
        std::cout << '\"';
     
        std::stringstream  input;
            input << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
            input << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        PrintSVG(input);
        
        std::cerr <<"----------buses--------------------------" << "\n";
    std::vector<svg::Text> text_bus;
    for (auto& bus : buses) {

        std::cerr << "stops count: " << bus->stops.size() << " bus_name: " << bus->name.data() << "\n";
        if (bus->stops.size() != 0) {
            svg::Polyline polyline;
            for (auto& stop : bus->stops) {
                polyline.AddPoint(SP({ stop->latitude(), stop->longitude() }));
            }
            svg::Document docSVG;
            docSVG.Add(polyline.SetFillColor("none")
                .SetStrokeColor(rs.color_palette[col_i])
                .SetStrokeWidth(rs.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            );
            {
              //  std::cout << "-----beg---------" << "\n";
                std::stringstream  input;
                docSVG.RenderSVG(input);
                PrintSVG(input);
               // std::cout << "---------end-------" << "\n";
            }

            svg::Text text;
              text.SetData(bus->name.data())                   
                    .SetOffset(rs.bus_label_offset)
                    .SetFontSize(rs.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetFillColor(rs.underlayer_color)
                    .SetStrokeColor(rs.underlayer_color)
                    .SetStrokeWidth(rs.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

           
              svg::Text text_p;
              text_p.SetData(bus->name.data())
                    .SetFillColor(rs.color_palette[col_i])
                    .SetOffset(rs.bus_label_offset)
                    .SetFontSize(rs.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold");

              svg::Text text1 = text;
              text1.SetPosition(SP({
                      bus->stops[0]->latitude(),
                      bus->stops[0]->longitude()
                  }));
              svg::Text text_p1= text_p;
              text_p1.SetPosition(SP({
                      bus->stops[0]->latitude(),
                      bus->stops[0]->longitude()
                  }));

              text_bus.push_back(text1);
              text_bus.push_back(text_p1);

            if (!bus->is_roundtrip) { // i pervaya i konez ne kolzo! i  pervaya != konez
               
                if (bus->stops[0] != bus->stops[(bus->stops.size() - 1) / 2]) {
                    svg::Text text2 = text;
                    text2.SetPosition(SP({
                            bus->stops[(bus->stops.size() - 1) / 2]->latitude(),
                             bus->stops[(bus->stops.size() - 1) / 2]->longitude()
                        }));;

                    svg::Text text_p2 = text_p;
                    text_p2.SetPosition(SP({
                             bus->stops[(bus->stops.size() - 1) / 2]->latitude(),
                             bus->stops[(bus->stops.size() - 1) / 2]->longitude()
                        }));
                    text_bus.push_back(text2);
                    text_bus.push_back(text_p2);
                } 
            }

            if (bus->stops.size() != 0) { ++col_i; }
            if (rs.color_palette.size() == col_i) { col_i = 0; }
        }
    }

    for (auto& txt : text_bus) {
        svg::Document docSVG;
        docSVG.Add(txt);
        {
            //  std::cout << "-----beg---------" << "\n";
            std::stringstream  input;
            docSVG.RenderSVG(input);
            PrintSVG(input);
            // std::cout << "---------end-------" << "\n";
        }
    }

    col_i = 0;
    std::cerr << "----------stops--------------------------" << "\n";
    std::vector<svg::Text> text;
    for (auto& stop : stops) {
        int chek_stop = 0;
        for (auto& bus : buses)
        {
            std::string name = stop->name().data();
            for (auto& stop : bus->stops) {
                if (stop->name().data() == name) { ++chek_stop;  }
            }

        }
        if (chek_stop > 0) {
            
            std::cerr << "buses count: " << chek_stop << " stop_name: " << stop->name().data() << "\n";
            svg::Circle circle;
            svg::Document docSVG;
            docSVG.Add(circle.SetCenter(SP({ stop->latitude(),stop->longitude() }))
                .SetRadius(rs.stop_radius)
                .SetFillColor("white")
            );
            {
                //  std::cout << "-----beg---------" << "\n";
                std::stringstream  input;
                docSVG.RenderSVG(input);
                PrintSVG(input);
                // std::cout << "---------end-------" << "\n";
            }
            svg::Text txt;
            text.push_back(txt.SetData(stop->name().data())
                .SetPosition(SP({
                   stop->latitude(),
                    stop->longitude()
                    }))
                .SetOffset(rs.stop_label_offset)
                .SetFontSize(rs.stop_label_font_size)
                .SetFontFamily("Verdana")

                .SetFillColor(rs.underlayer_color)
                .SetStrokeColor(rs.underlayer_color)
                .SetStrokeWidth(rs.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)

            );
            svg::Text txtt;
            text.push_back(txtt.SetData(stop->name().data())
                .SetFillColor("black")
                .SetPosition(SP({
                   stop->latitude(),
                    stop->longitude()
                    }))
                .SetOffset(rs.stop_label_offset)
                .SetFontSize(rs.stop_label_font_size)
                .SetFontFamily("Verdana")
            );
        }
    }
    for (auto& txt : text) {
        svg::Document docSVG;
        docSVG.Add(txt);
        {
            //  std::cout << "-----beg---------" << "\n";
            std::stringstream  input;
            docSVG.RenderSVG(input);
            PrintSVG(input);
            // std::cout << "---------end-------" << "\n";
        }
    }

    std::stringstream  input_end;
    input_end << "</svg>"sv;
    PrintSVG(input_end);
 
        std::cout << '\"';
    }
    std::cout << ",";
    std::cout << "\n        \"request_id\": "s + std::to_string(doc.AsMap().at("id"s).AsInt()) + "\n    }"s;

}

void AddStopJson(TransportCatalogue& TC, json::Node LoadStop) {
    // json::Document LoadStop = json::Load(inputStop);//std::cin
    std::unordered_map<std::string, int>tostop_;
    for (auto&& s : LoadStop.AsMap().at("road_distances").AsMap()) {
        tostop_[s.first] = s.second.AsInt();
    }

    TC.AddStop({
        LoadStop.AsMap().at("name").AsString(),
        LoadStop.AsMap().at("latitude").AsDouble(),
        LoadStop.AsMap().at("longitude").AsDouble()
       ,tostop_
        });
}

void AddBusJson(TransportCatalogue& TC, json::Node LoadBus) {

    std::string name;
    std::deque<const Stop*>stops;
    bool is_roundtrip;   

    if (LoadBus.AsMap().at("is_roundtrip").AsBool()) {            //kolzo
        is_roundtrip = true;
        name = LoadBus.AsMap().at("name").AsString();
        for (auto& s : LoadBus.AsMap().at("stops").AsArray()) {
            stops.push_back(TC.GetStop(s.AsString()));
        }
    }
    else {
        is_roundtrip = false;
        name = LoadBus.AsMap().at("name").AsString();
        for (auto& s : LoadBus.AsMap().at("stops").AsArray()) {
            stops.push_back(TC.GetStop(s.AsString()));
        }
        size_t sz = stops.size();
        for (size_t i = sz - 1; i > 0; i--) {
            stops.push_back(stops[i - 1]);
        }

    }

    TC.AddBus({ name, stops, is_roundtrip });
}

void FormatResponse(json::Document& LoadInput, TransportCatalogue& TC) {
    //выводим в файл
   // std::ofstream out("out.txt");
  //  std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
  //  std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

    json::Array stat_requests = LoadInput.GetRoot().AsMap().at("stat_requests"s).AsArray();

    bool is_first = true;
    std::cout << "[";
    for (auto& doc : stat_requests) {
        if (!is_first) { std::cout << ","; }


        if (doc.AsMap().at("type"s).AsString() == "Bus"s) {
            PrintBuses(TC, doc);
        }

        if (doc.AsMap().at("type"s).AsString() == "Stop"s) {

            PrintStops(TC, doc);
        }

        if (doc.AsMap().at("type"s).AsString() == "Map"s) {

            json::Dict render_settings = LoadInput.GetRoot().AsMap().at("render_settings"s).AsMap();
            PrintMap(TC, doc, render_settings);
        }

        is_first = false;
    }
    std::cout << "\n]";

}

Color GetColorFromDict(json::Node color) {
    Color color_;
    if (color.IsArray()) {

        if (color.AsArray().size() == 4) {
            color_ = Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble());
        }
        if (color.AsArray().size() == 3) {
            color_ = Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt());
        }
    }

    if (color.IsString()) {
        color_ = color.AsString();
    }

    return  color_;
}

RenderSettings SetRenerSettings(json::Dict render_settings) {
    RenderSettings RS;
    RS.bus_label_font_size = static_cast<uint64_t>(render_settings.at("bus_label_font_size").AsInt());
    RS.bus_label_offset.x = render_settings.at("bus_label_offset").AsArray()[0].AsDouble();
    RS.bus_label_offset.y = render_settings.at("bus_label_offset").AsArray()[1].AsDouble();
    for (auto& color : render_settings.at("color_palette").AsArray()) {
        RS.color_palette.push_back(GetColorFromDict(color));
    }
    RS.height = render_settings.at("height").AsDouble();
    RS.line_width = render_settings.at("line_width").AsDouble();
    RS.padding = render_settings.at("padding").AsDouble();
    RS.stop_label_font_size = static_cast<uint64_t>(render_settings.at("stop_label_font_size").AsInt());
    RS.stop_label_offset.x = render_settings.at("stop_label_offset").AsArray()[0].AsDouble();
    RS.stop_label_offset.y = render_settings.at("stop_label_offset").AsArray()[1].AsDouble();
    RS.stop_radius = render_settings.at("stop_radius").AsDouble();
    RS.underlayer_color = GetColorFromDict(render_settings.at("underlayer_color"));
    RS.underlayer_width = render_settings.at("underlayer_width").AsDouble();
    RS.width = render_settings.at("width").AsDouble();

    return RS;
}


void JsonReader(std::istream& in) {

    TransportCatalogue TC;

    json::Document LoadInput = json::Load(in);//std::cin  
    json::Array base_requests = LoadInput.GetRoot().AsMap().at("base_requests"s).AsArray();

    json::Array base_Stops;
    json::Array base_Buses;

    for (auto& doc : base_requests) {
        if (doc.AsMap().at("type"s).AsString() == "Stop"s) { base_Stops.push_back(doc.AsMap()); }
        if (doc.AsMap().at("type"s).AsString() == "Bus"s) { base_Buses.push_back(doc.AsMap()); }
    }


    for (auto& doc : base_Stops) {
        AddStopJson(TC, doc);
    }

    for (auto& doc : base_Buses) {
        AddBusJson(TC, doc);
    }

    FormatResponse(LoadInput, TC);  
}
