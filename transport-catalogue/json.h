#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace json {

    using namespace std::literals;

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Node_JSON = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

struct OstreamNodePrinter {
    std::ostream& out;

    void operator()(std::nullptr_t) const;

    void operator()(const Array& arr) const;

    void operator()(const Dict& dict) const;

    void operator()(bool b) const;

    void operator()(int i) const;

    void operator()(double d) const;

    void operator()(const std::string& str) const;

};

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    explicit Node() = default;

    template<typename T>
    Node(T value)
        :json_node_(std::move(value)) {
    }   
    
    explicit Node(Array&& value);
    explicit Node(Dict&& value);
    bool operator == (const Node& rhs) const;
    bool operator != (const Node& rhs) const;
    bool IsNull() const;
    bool IsInt() const;
    bool IsDouble() const;
    double AsDouble() const;
    bool IsPureDouble() const;
    bool  IsString() const;
    bool IsBool() const;
    bool AsBool() const;
    bool IsArray() const;
    bool IsMap() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    const std::string& AsString() const;
    const  Node_JSON& GetData() const;

protected:
    Node_JSON json_node_;   
};

class Document :public Node {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    const  Node_JSON& GetData() const;

    bool operator == (const Document& rhs) const;
    bool operator != (const Document& rhs) const;

private:
    Node root_;
};

Node LoadNumber(std::istream& input);

Document Load(std::istream& input);


void Print(const Document& doc, std::ostream& output);

}  // namespace json