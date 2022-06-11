#pragma once

#include <algorithm>
#include <deque>

#include "geo.h"
#include "json.h"
#include "json_reader.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "request_handler.h"

using namespace transport::catalogue;
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

using namespace svg;
using namespace transport::geo_coordinates;

struct RenderSettings{
    double width;
    double height;
    double padding;
    double stop_radius;
    double line_width;
    uint32_t stop_label_font_size;
    Point stop_label_offset;
    Color underlayer_color;
    double underlayer_width;
    std::deque<Color> color_palette;
    uint32_t bus_label_font_size;
    Point  bus_label_offset;
};

 const double EPSILON = 1e-6;
 bool IsZero(double value);

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
        double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lng < rhs.lng;
                });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lat < rhs.lat;
                });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(Coordinates coords) const {
        return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

Color GetColorFromDict(json::Node color);

RenderSettings SetRenerSettings(RequestHandler& handler);

std::deque<Coordinates> GetCoordStops(const std::deque<const Stop*>& stops, const std::deque<const Bus*>& buses);

void AddBusesToXml(svg::Document& doc_svg, 
    RenderSettings& rs, 
    SphereProjector& scale, 
    const std::deque<const Bus*>& buses
);

void AddStopsToXml(svg::Document& doc_svg,
    RenderSettings& rs,
    SphereProjector& scale,
    const std::deque<const Bus*>& buses,
    const std::deque<const Stop*>& stops
);

class RenderXml {
public:
    RenderXml(RequestHandler& handler);

    svg::Document&& GetXml() {
       return std::move(doc_svg_);
    }

private:
    svg::Document doc_svg_;
};