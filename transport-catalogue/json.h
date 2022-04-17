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
using NodeJson = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

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

struct OstreamNodePrinter2 {
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

class Node : public NodeJson {
    using variant::variant;
public:  
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
    const  NodeJson& GetData() const;
};

class Document :public Node {
public:

    explicit Document(Node root);

    const Node& GetRoot() const;
    const  NodeJson& GetData() const;

    bool operator == (const Document& rhs) const;
    bool operator != (const Document& rhs) const;

private:
    Node root_;
};

Node LoadNumber(std::istream& input);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json