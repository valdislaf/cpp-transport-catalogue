#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {

        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }


    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }


    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "  <polyline points=\""sv;
        bool fl = true;
        for (Point p : points_) {
            if (fl) {
                out << p.x << "," << p.y;
                fl = false;
            }
            else {
                out << " " << p.x << "," << p.y;
            }
        }

        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;

    }


    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;

    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;

        out << "  <text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv;
        out << pos_.x << "\""sv;
        out << " y=\""sv;
        out << pos_.y << "\""sv;
        out << " dx=\""sv;
        out << offset_.x << "\""sv;
        out << " dy=\""sv;
        out << offset_.y << "\""sv;
        out << " font-size=\""sv;
        out << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv;
            out << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv;
            out << font_weight_ << "\""sv;
        }
        out << ">"sv;
        out << data_;
        out << "</text>"sv;

    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap)
        {
        case StrokeLineCap::BUTT: return out << "butt"s;
        case StrokeLineCap::ROUND: return out << "round"s;
        case StrokeLineCap::SQUARE: return out << "square"s;
        default: return out << "butt"s;
        }
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join)
        {
        case StrokeLineJoin::ARCS: return out << "arcs"s;
        case StrokeLineJoin::BEVEL: return out << "bevel"s;
        case StrokeLineJoin::MITER: return out << "miter"s;
        case StrokeLineJoin::MITER_CLIP: return out << "miter-clip"s;
        case StrokeLineJoin::ROUND: return out << "round"s;
        default: return out << "miter"s;
        }
    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ptx(out, 0, 0);
        for (auto& doc : objects_) {
            doc.get()->Render(ptx);
        }
        out << "</svg>"sv;
    }
    void Document::RenderSVG(std::ostream& out) const
    {
        /*out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;*/
        RenderContext ptx(out, 0, 0);
        for (auto& doc : objects_) {
            doc.get()->Render(ptx);
        }
        /*out << "</svg>"sv;*/
    }
    Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue) :red(red), green(green), blue(blue) {
    }

    Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) :red(red), green(green), blue(blue), opacity(opacity) {
    }

    void OstreamColorPrinter::operator()(std::monostate) const {
        out << "none"sv;
    }
    void OstreamColorPrinter::operator()(std::string str) const {
        out << str;
    }

    void OstreamColorPrinter::operator()(Rgb rgb) const {
        out << "rgb("s << std::to_string(rgb.red) << ","s << std::to_string(rgb.green) << ","s << std::to_string(rgb.blue) << ")"s;
    }

    void OstreamColorPrinter::operator()(Rgba rgba) const {
        out << "rgba("s << std::to_string(rgba.red) << ","s << std::to_string(rgba.green) << ","s << std::to_string(rgba.blue) << ","s << rgba.opacity << ")"s;
    }

}  // namespace svg