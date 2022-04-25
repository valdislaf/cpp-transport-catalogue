#include "json_builder.h"

namespace json {

    using namespace std::literals;

    Builder::DictItemContext Builder::StartDict()
    {
        DictItemContext Item(*this);
        nodes_stack_.push_back(Dict());
        return  Item;
    }

    Node  Builder::Build() {

        if (node_ == nullptr) {
            throw std::logic_error("logic_error"s);
        }
        if (nodes_stack_.size() != 0) {
            throw std::logic_error("logic_error"s);
        }

        return node_;
    }

    Builder::BaseContext Builder::Value(Node node) {
        BaseContext Item(*this);

        if (nodes_stack_.size() != 0) {
            if (nodes_stack_.back().IsArray()) {
                nodes_stack_.back().AsArray().push_back(node);
            }
            else  if (nodes_stack_.back().IsDict()) {

                nodes_stack_.back().AsDict()[(key_.back())] = node;
                key_.pop_back();
            }
            else {
                nodes_stack_.push_back(node);
            }
        }
        else {
            node_ = node;
        }

        return static_cast<BaseContext>(Item);
    }

    Builder::KeyItemContext Builder::Key(std::string key) {
        KeyItemContext Item(*this);

        key_.push_back(key);

        return Item;
    }
    
    Builder::ArrayItemContext Builder::StartArray() {

        ArrayItemContext Item(*this);
        nodes_stack_.push_back(Array());

        return Item;
    }

    Builder::BaseContext Builder::EndArray() {
        BaseContext Item(*this);
        if (nodes_stack_.size() == 0) {
            throw std::logic_error("logic_error"s);
        }
        if (!nodes_stack_.back().IsArray()) {
            throw std::logic_error("logic_error"s);
        }

        node_ = nodes_stack_.back();
        nodes_stack_.pop_back();
        if (nodes_stack_.size() != 0) {
            if (nodes_stack_.back().IsArray()) {
                nodes_stack_.back().AsArray().push_back(node_);
            }
            if (nodes_stack_.back().IsDict()) {
                nodes_stack_.back().AsDict()[key_.back()] = node_; key_.pop_back();
            }
        }

        return Item;
    }

    Builder::BaseContext Builder::EndDict() {
        BaseContext Item(*this);
        if (nodes_stack_.size() == 0) {
            throw std::logic_error("logic_error"s);
        }
        if (!nodes_stack_.back().IsDict()) {
            throw std::logic_error("logic_error"s);
        }

        node_ = nodes_stack_.back();
        nodes_stack_.pop_back();
        if (nodes_stack_.size() != 0) {
            if (nodes_stack_.back().IsArray()) {
                nodes_stack_.back().AsArray().push_back(node_);
            }
            if (nodes_stack_.back().IsDict()) {
                nodes_stack_.back().AsDict()[key_.back()] = node_; key_.pop_back();
            }
        }

        return Item;
    }

    Builder::BaseContext Builder::DictItemContext::EndDict() {
        return build.EndDict();
    }

    Builder::KeyItemContext Builder::DictItemContext::Key(std::string str) {
        return build.Key(str);
    }

    Builder::DictItemContext Builder::KeyItemContext::Value(Node node) {
        return static_cast<DictItemContext>(build.Value(node));
    }

    Builder::DictItemContext Builder::KeyItemContext::StartDict() {
        return build.StartDict();
    }

    Builder::ArrayItemContext Builder::KeyItemContext::StartArray() {
        return build.StartArray();
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node node) {
        return static_cast<ArrayItemContext>(build.Value(node));
    }

    Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
        return build.StartDict();
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
        return build.StartArray();
    }

    Builder::BaseContext Builder::ArrayItemContext::EndArray() {
        return build.EndArray();
    }

    Builder::DictItemContext Builder::BaseContext::StartDict() {
        return build.StartDict();
    }

    Builder::KeyItemContext Builder::BaseContext::Key(std::string str) {
        return build.Key(str);
    }

    Node Builder::BaseContext::Build() {
        return build.Build();
    }

    Builder::BaseContext Builder::BaseContext::Value(Node node) {
        return build.Value(node);
    }

    Builder::ArrayItemContext Builder::BaseContext::StartArray() {
        return build.StartArray();
    }

    Builder::BaseContext Builder::BaseContext::EndArray() {
        return build.EndArray();
    }

    Builder::BaseContext Builder::BaseContext::EndDict() {
        return build.EndDict();
    }

}
