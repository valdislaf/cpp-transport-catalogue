#pragma once

#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>

#include "json.h"

namespace json {

    using namespace std::literals;

    class Builder {

    public:

        class DictItemContext;
        class KeyItemContext;
        class ArrayItemContext;
        class BaseContext;

        class BaseContext {
        public:
            BaseContext(Builder& build) :build(build) {
            }
            DictItemContext StartDict();
            BaseContext Value(Node node);
            ArrayItemContext StartArray();
            BaseContext EndArray();
            Node Build();
            BaseContext EndDict();
            KeyItemContext Key(std::string);

        protected:
            Builder& build;
        };

        class DictItemContext : public BaseContext {
        public:
            DictItemContext(BaseContext build) :BaseContext(build) {
            }
            BaseContext EndDict();
            KeyItemContext Key(std::string);
            DictItemContext StartDict() = delete;
            Node Build() = delete;
            BaseContext Value(Node node) = delete;
            ArrayItemContext StartArray() = delete;
            BaseContext EndArray() = delete;
        };

        class ArrayItemContext : public BaseContext {
        public:
            ArrayItemContext(BaseContext build) :BaseContext(build) {
            }
            ArrayItemContext Value(Node node);
            DictItemContext StartDict();
            ArrayItemContext StartArray();
            BaseContext EndArray();
            KeyItemContext Key(std::string) = delete;
            Node Build() = delete;
            BaseContext EndDict() = delete;
        };

        class KeyItemContext : public BaseContext {
        public:
            KeyItemContext(BaseContext build) :BaseContext(build) {
            }
            DictItemContext Value(Node node);
            DictItemContext StartDict();
            ArrayItemContext StartArray();
            KeyItemContext Key(std::string) = delete;
            Node Build() = delete;
            BaseContext EndArray() = delete;
            BaseContext EndDict() = delete;
        };

        DictItemContext StartDict();
        KeyItemContext Key(std::string);
        Node Build();
        BaseContext Value(Node node);
        ArrayItemContext StartArray();
        BaseContext EndArray();
        BaseContext EndDict();

    protected:
        Node node_{};
        std::vector <std::string>  key_{};
        std::vector<Node> nodes_stack_{};
    };

}

