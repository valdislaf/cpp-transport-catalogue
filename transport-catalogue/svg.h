#pragma once

#include <cmath> // для round
#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>


namespace svg {
    using namespace std::literals;

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue);
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity);
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ "none"s };


    struct OstreamColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const;

        void operator()(std::string str) const;

        void operator()(Rgb rgb) const;

        void operator()(Rgba rgba) const;
    };


    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);


    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj) {
            objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }

        virtual  void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    protected:
        std::deque<std::unique_ptr<Object>>objects_;
        ~ObjectContainer() = default;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };


    template <typename Owner>
    class PathProps {

    public:

        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;



        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;


            if (fill_color_) {
                std::ostringstream strm;
                visit(OstreamColorPrinter{ strm }, *fill_color_);
                out << " fill=\""sv << strm.str() << "\""sv;
            }

            if (stroke_color_) {
                std::ostringstream strm;
                visit(OstreamColorPrinter{ strm }, *stroke_color_);
                out << " stroke=\""sv << strm.str() << "\""sv;
            }


            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv; out << *stroke_line_cap_; out << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv; out << *stroke_line_join_; out << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
    <svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg">
      <circle cx="50" cy="50" r="50"/>
    </svg>

     *
     *
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     *
    <svg viewBox="0 0 200 100" xmlns="http://www.w3.org/2000/svg">
      <!-- Example of a polyline with the default fill -->
      <polyline points="0,100 50,25 50,75 100,0" />
     *
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_{};

    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */

    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>
        // 
    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_{ 0.0, 0.0 };
        Point offset_{ 0.0, 0.0 };
        uint32_t size_ = 1;

        std::string  font_family_{};
        std::string font_weight_{};
        std::string data_{};
    };

    class Document :public ObjectContainer {
    public:

        void AddPtr(std::unique_ptr<Object>&& obj) {
            objects_.emplace_back(std::move(obj));
        }

        void Render(std::ostream& out) const;
        void RenderSVG(std::ostream& out) const;       

    };



}  // namespace svg