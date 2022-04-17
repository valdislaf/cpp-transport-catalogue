#include "json.h"

using namespace std;

namespace json {

    namespace  node {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            if (input.peek() == -1) {
                throw ParsingError("Failed to convert JSON"s);
            }

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }

                result.push_back(LoadNode(input));
            }
            return Node(move(result));
        }

        Node LoadString(istream& input) {
            string line;
            char ch = ' ';
            while (input) {

                ch = input.get();

                char ch_next = input.peek();
                if (ch == '\n' || ch == '\r' || ch == '\t' || ch == '\b' || ch == '\f') {
                    throw ParsingError("Failed to convert JSON"s);
                }
                else if (ch == -1) {
                    break;
                }

                else if ((ch == '\\') &&
                    (
                        ch_next == '\\'
                        || ch_next == 'n'
                        || ch_next == 'r'
                        || ch_next == 't'
                        || ch_next == 'f'
                        || ch_next == 'b'
                        || ch_next == '"'
                        )) {

                    line += ch_next; input.get();
                }

                else if (ch == '\"') {
                    break;
                }

                else {
                    line += ch;
                }

            }
            //if(ch!= '\"') { throw ParsingError("Failed to convert JSON"s); }
            return Node(move(line));
        }

        Node LoadDict(istream& input) {
            if (input.peek() == -1) {
                throw ParsingError("Failed to convert JSON"s);
            }

            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });

            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {

            char c;

            if ((input >> c).eof()) {
                throw ParsingError("Failed to convert JSON"s);
            }
            if (c == '\n' || c == '\r' || c == '\t' || c == '\b' || c == '\f') {
                throw ParsingError("Failed to convert JSON"s);
            }
            if (c == '[') {

                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (std::isalpha(c)) {
                std::string str_isalpha;

                while (std::isalpha(c)) {
                    str_isalpha += c;
                    c = input.get();
                }
                input.unget();
                if (str_isalpha == "null") {
                    return Node(nullptr);
                }
                else  if (str_isalpha == "false") {
                    return Node(false);
                }
                else  if (str_isalpha == "true") {
                    return Node(true);
                }
                else {
                    throw ParsingError("Failed to convert JSON"s);
                }
            }
            else {
                if (c < 0) {
                    return Node(nullptr);
                }
                input.putback(c);
                return LoadNumber(input);
            }
            throw ParsingError("Failed to convert JSON"s);
        }

    }  // namespace

    bool Node::operator==(const Node& rhs) const {
        return *this == rhs;
    }

    bool Node::operator!=(const Node& rhs) const {
        return !(*this == rhs);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<int>(*this) || std::holds_alternative<double>(*this);
    }

    double Node::AsDouble() const {
        if (std::holds_alternative<double>(*this)) { return static_cast<double>(std::get<double>(*this)); }
        else if (std::holds_alternative<int>(*this)) { return static_cast<double>(std::get<int>(*this)); }
        else { throw logic_error("Failed to convert JSON"s); }
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::AsBool() const {
        if (std::holds_alternative<bool>(*this)) { return std::get<bool>(*this); }
        else { throw logic_error("Failed to convert JSON"s); }
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    const Array& Node::AsArray() const {
        if (std::holds_alternative<Array>(*this)) { return std::get<Array>(*this); }
        else { throw logic_error("Failed to convert JSON"s); }
    }

    const Dict& Node::AsMap() const {
        if (std::holds_alternative<Dict>(*this)) { return std::get<Dict>(*this); }
        else { throw logic_error("Failed to convert JSON"s); }

    }

    int Node::AsInt() const {
        if (std::holds_alternative<int>(*this)) { return std::get<int>(*this); }
        else { throw logic_error("Failed to convert JSON"s); }

    }

    const string& Node::AsString() const {
        if (std::holds_alternative<std::string>(*this)) { return std::get<std::string>(*this); }
        else { throw logic_error("Failed to convert JSON"s); }
    }

    const NodeJson& Node::GetData() const {
        return *this;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    const NodeJson& Document::GetData() const {
        return this;
    }

    bool Document::operator==(const Document& rhs) const {
        return root_ == rhs.root_;
    }

    bool Document::operator!=(const Document& rhs) const {
        return !(root_ == rhs.root_);
    }

    Node LoadNumber(std::istream& input) {
        using namespace std::literals;

        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());
            if (!input) {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek())) {
                read_char();
            }
        };

        if (input.peek() == '-') {
            read_char();
        }
        // Парсим целую часть числа
        if (input.peek() == '0') {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        }
        else {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                // Сначала пробуем преобразовать строку в int
                try {
                    return Node(std::stoi(parsed_num));
                }
                catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return Node(std::stod(parsed_num));
        }
        catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    Document Load(istream& input) {
        return  Document{
            json::node::LoadNode(input)
        };
    }

    void Print(const Document& doc, std::ostream& output) {

        auto a = doc.GetRoot().GetData();

        std::ostringstream strm;
        std::visit(OstreamNodePrinter2{ strm }, doc.GetRoot().GetData());
        output << strm.str();
        // output << endl;

    }

    void OstreamNodePrinter::operator()(std::nullptr_t) const
    {
        out << "null";
    }

    void OstreamNodePrinter::operator()(const Array& arr) const
    {
        out << "[\n"s;
        bool fl = false;
        for (const auto& el : arr) {
            if (fl) {
                out << ",\n"s;
            }
            std::ostringstream strm;
            std::visit(OstreamNodePrinter{ strm }, el.GetData());
            // out << "           "s;
            out << strm.str();
            fl = true;
        }
        out << "\n]"s;
    }

    void OstreamNodePrinter::operator()(const Dict& dict) const
    {
        out << "    {\n"s;
        bool fl = false;
        for (const auto& el : dict) {
            if (fl) {
                out << ",\n"s;
            }
            out << "        "s;
            out << '\"';
            out << el.first;
            out << '\"';
            out << ": "s;


            std::ostringstream strm;
            std::visit(OstreamNodePrinter{
                strm
                }, el.second.GetData());
            out << strm.str();
            fl = true;
        }
        out << "\n    }"s;
    }

    void OstreamNodePrinter::operator()(bool b) const {
        out << boolalpha << b;
    }

    void OstreamNodePrinter::operator()(int i) const {
        out << i;
    }

    void OstreamNodePrinter::operator()(double d) const {
        out << d;
    }

    void OstreamNodePrinter::operator()(const std::string& str) const {
        string str_out;
        str_out += "\""s;
        for (const char& c : str) {
            if (c == '\n') {
                str_out += '\\';  str_out += 'n';
            }
            else  if (c == '\r') {
                str_out += '\\';  str_out += 'r';
            }
            else  if (c == '\"') {
                str_out += '\\'; str_out += '"';
            }
            else  if (c == '\\') {
                str_out += '\\'; str_out += '\\';
            }
            else {
                str_out += c;
            }
        }
        str_out += '\"';

        out << str_out;
    }


    void OstreamNodePrinter2::operator()(std::nullptr_t) const
    {
        out << "null";
    }

    void OstreamNodePrinter2::operator()(const Array& arr) const
    {
        out << "[\n"s;
        bool fl = false;
        for (const auto& el : arr) {
            if (fl) {
                out << ",\n"s;
            }

            std::ostringstream strm;
            std::visit(OstreamNodePrinter2{ strm }, el.GetData());
            // out << "           "s;
            out << strm.str();
            fl = true;
        }
        auto maa = arr[arr.size() - 1];
        if (maa.IsMap()) {
            out << "\n]"s;
        }
        else {
            out << "\n        ]"s;
        }
    }

    void OstreamNodePrinter2::operator()(const Dict& dict) const
    {
        out << "    {\n"s;
        bool fl = false;
        for (const auto& el : dict) {
            if (fl) {
                out << ",\n"s;
            }
            out << "        "s;
            out << '\"';
            out << el.first;
            out << '\"';
            out << ": "s;

            if (el.first == "buses") {
                std::ostringstream strm;
                std::visit(OstreamNodePrinter2{
                    strm
                    }, el.second.GetData());
                out << strm.str();
            }
            else {
                std::ostringstream strm;
                std::visit(OstreamNodePrinter{
                    strm
                    }, el.second.GetData());
                out << strm.str();
            }
            fl = true;
        }
        out << "\n    }"s;
    }

    void OstreamNodePrinter2::operator()(bool b) const {
        out << boolalpha << b;
    }

    void OstreamNodePrinter2::operator()(int i) const {
        out << i;
    }

    void OstreamNodePrinter2::operator()(double d) const {
        out << d;
    }

    void OstreamNodePrinter2::operator()(const std::string& str) const {
        string str_out;
        str_out += "            \""s;
        for (const char& c : str) {
            if (c == '\n') {
                str_out += '\\';  str_out += 'n';
            }
            else  if (c == '\r') {
                str_out += '\\';  str_out += 'r';
            }
            else  if (c == '\"') {
                str_out += '\\'; str_out += '"';
            }
            else  if (c == '\\') {
                str_out += '\\'; str_out += '\\';
            }
            else {
                str_out += c;
            }
        }
        str_out += '\"';

        out << str_out;
    }

}  // namespace json