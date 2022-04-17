#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

Color GetColorFromDict(json::Node color) {
    Color color_;
    if (color.IsArray()) {
        int rgba = 4;
        int rgb = 3;
        if (color.AsArray().size() == rgba) {
            color_ = Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble());
        }
        if (color.AsArray().size() == rgb) {
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
    for (const auto& color : render_settings.at("color_palette").AsArray()) {
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

int GetStopCount(const Stop* stop, const std::deque<const Bus*>& buses) {
    int chek_stop = 0;
    for (auto& bus : buses) {
        std::string name = stop->name;
        for (const auto& stop : bus->stops) {
            if (stop->name == name) {
                ++chek_stop;
            }
        }
    }
    return chek_stop;
}


std::deque<Coordinates> GetCoordStops(const std::deque<const Stop*>& stops, const std::deque<const Bus*>& buses) {
    std::deque<Coordinates>coord_stops;
    for (const auto& stop : stops) {

        if (GetStopCount(stop, buses) > 0) {
            // только непустые остановки!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            coord_stops.push_back({
                stop->coord.lat, stop->coord.lng
                });
        }
    }
    return coord_stops;
}

void AddBusesToXml(svg::Document& doc_svg,
    RenderSettings& rs,
    SphereProjector& scale,
    const std::deque<const Bus*>& buses) {

    std::vector<svg::Text> text_bus;
    int col_i = 0;
    for (const auto& bus : buses) {

        std::cerr << "stops count: " << bus->stops.size() << " bus_name: " << bus->name.data() << "\n";
        if (bus->stops.size() != 0) {
            svg::Polyline polyline;
            for (const auto& stop : bus->stops) {
                polyline.AddPoint(scale({
                    stop->coord.lat, stop->coord.lng
                    }));
            }

            doc_svg.Add(polyline.SetFillColor("none")
                .SetStrokeColor(rs.color_palette[col_i])
                .SetStrokeWidth(rs.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            );

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

            svg::Text text_roundtrip = text;
            text_roundtrip.SetPosition(scale({
                    bus->stops[0]->coord.lat,
                    bus->stops[0]->coord.lng
                }));
            svg::Text text_p_roundtrip = text_p;
            text_p_roundtrip.SetPosition(scale({
                    bus->stops[0]->coord.lat,
                    bus->stops[0]->coord.lng
                }));

            text_bus.push_back(text_roundtrip);
            text_bus.push_back(text_p_roundtrip);

            if (!bus->is_roundtrip) { // i pervaya i konez ne kolzo! i  pervaya != konez

                if (bus->stops[0] != bus->stops[(bus->stops.size() - 1) / 2]) {
                    svg::Text text_no_roundtrip = text;
                    text_no_roundtrip.SetPosition(scale({
                            bus->stops[(bus->stops.size() - 1) / 2]->coord.lat,
                             bus->stops[(bus->stops.size() - 1) / 2]->coord.lng
                        }));;

                    svg::Text text_p_no_roundtrip = text_p;
                    text_p_no_roundtrip.SetPosition(scale({
                             bus->stops[(bus->stops.size() - 1) / 2]->coord.lat,
                             bus->stops[(bus->stops.size() - 1) / 2]->coord.lng
                        }));
                    text_bus.push_back(text_no_roundtrip);
                    text_bus.push_back(text_p_no_roundtrip);
                }
            }

            if (bus->stops.size() != 0) {
                ++col_i;
            }

            if (rs.color_palette.size() == col_i) {
                col_i = 0;
            }
        }
    }

    for (const auto& txt : text_bus) {
        doc_svg.Add(txt);
    }
}

void AddStopsToXml(svg::Document& doc_svg,
    RenderSettings& rs,
    SphereProjector& scale,
    const std::deque<const Bus*>& buses,
    const std::deque<const Stop*>& stops) {

    std::vector<svg::Text> text;
    for (const auto& stop : stops) {

        int chek_stop = GetStopCount(stop, buses);

        if (chek_stop > 0) {
            std::cerr << "buses count: " << chek_stop << " stop_name: " << stop->name << "\n";
            svg::Circle circle;
            doc_svg.Add(circle.SetCenter(scale({
                stop->coord.lat, stop->coord.lng
                }))
                .SetRadius(rs.stop_radius)
                .SetFillColor("white")
            );

            svg::Text txt;
            text.push_back(txt.SetData(stop->name)
                .SetPosition(scale({
                   stop->coord.lat,
                    stop->coord.lng
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
            text.push_back(txtt.SetData(stop->name)
                .SetFillColor("black")
                .SetPosition(scale({
                   stop->coord.lat,
                    stop->coord.lng
                    }))
                .SetOffset(rs.stop_label_offset)
                .SetFontSize(rs.stop_label_font_size)
                .SetFontFamily("Verdana")
            );
        }
    }

    for (const auto& txt : text) {
        doc_svg.Add(txt);
    }
}

RenderXml::RenderXml(RequestHandler& handler, json::Dict render_settings) {

    RenderSettings rs = SetRenerSettings(render_settings);

    const auto& stops = handler.GetStops();
    const auto& buses = handler.GetBuses();

    std::deque<Coordinates>coord_stops = GetCoordStops(stops, buses);

    SphereProjector scale(coord_stops.begin(), coord_stops.end(), rs.width, rs.height, rs.padding);

    std::cerr << "----------buses--------------------------" << "\n";

    AddBusesToXml(doc_svg_, rs, scale, buses);

    std::cerr << "----------stops--------------------------" << "\n";

    AddStopsToXml(doc_svg_, rs, scale, buses, stops);

}
