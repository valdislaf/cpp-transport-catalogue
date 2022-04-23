#include "json_builder.h"

namespace json {

    using namespace std::literals;

    DictItemContext Builder::StartDict()
    {
        DictItemContext Item(*this);
        key_open = "close";
        if (!(lock_builder_.back() == "builder" || lock_builder_.back() == "key" || lock_builder_.back() == "startarray")) {
            throw std::logic_error("logic_error"s);
        }
        node_ = dict_;
        nodes_stack_.push_back(node_);
        lock_builder_.push_back("startdict");
        ++dict_count;
        return  Item;
    }

    Node Builder::Build() {   

        if (key_open == "open") { throw std::logic_error("logic_error"s); }
        if (lock_builder_.back() == "builder") {
            throw std::logic_error("logic_error"s);
        }
             
        if (nodes_all_.empty()) {
            throw std::logic_error("logic_error"s);
        }
        if (nodes_stack_.size()!=0){
            throw std::logic_error("logic_error"s);
        }
        lock_builder_.push_back("build");
        return nodes_all_.back();       
    }

    BaseContext Builder::Value(Node node) {
        BaseContext Item(*this);
        key_open = "close";
       if (!(lock_builder_.back() == "builder" || lock_builder_.back() == "key" || lock_builder_.back() == "startarray" || array_count % 2 != 0 )) {
            throw std::logic_error("logic_error"s);
        }
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
        }else{ nodes_all_.push_back(node); }
        if (lock_builder_.back() == "startarray") { lock_builder_.push_back("startarray"); }
        else { lock_builder_.push_back("value"); }        
        return static_cast<BaseContext>(Item);
    }


    KeyItemContext Builder::Key(std::string key) {       
        KeyItemContext Item(*this);       
        if (key_open == "open") { throw std::logic_error("logic_error"s); }
        key_.push_back(key);
        lock_builder_.push_back("key");
        key_open = "open";
        return Item;
    }

    ArrayItemContext Builder::StartArray() {
        ArrayItemContext Item(*this);
        key_open = "close";
        if (!(lock_builder_.back() == "builder" || lock_builder_.back() == "key" || lock_builder_.back() == "startarray" )) {
            throw std::logic_error("logic_error"s);
        }
        node_ = arr_;
        nodes_stack_.push_back(node_);
        lock_builder_.push_back("startarray");
        ++array_count;
        return Item;
    }    

    Builder& Builder::EndArray() {
        if (nodes_stack_.size() == 0) { throw std::logic_error("logic_error"s); }
        if (!nodes_stack_.back().IsArray()) { throw std::logic_error("logic_error"s); }
        nodes_all_.push_back(nodes_stack_.back());
        nodes_stack_.pop_back();
        if (nodes_stack_.size() != 0) {
            if (nodes_stack_.back().IsArray()) { nodes_stack_.back().AsArray().push_back(nodes_all_.back()); }
            if (nodes_stack_.back().IsDict()) { nodes_stack_.back().AsDict()[key_.back()] = nodes_all_.back(); key_.pop_back(); }
        }
        lock_builder_[0] = "endarray";
        lock_builder_.pop_back();
        --array_count;
        return *this;
    }

    EndDictItemContext Builder::EndDict() { 
        EndDictItemContext Item(*this); 
        if (nodes_stack_.size() == 0) { throw std::logic_error("logic_error"s); }
        if(!nodes_stack_.back().IsDict()){ throw std::logic_error("logic_error"s); }
        nodes_all_.push_back(nodes_stack_.back());
        nodes_stack_.pop_back();
        if (nodes_stack_.size() != 0) {
            if (nodes_stack_.back().IsArray()) { nodes_stack_.back().AsArray().push_back(nodes_all_.back()); }
            if (nodes_stack_.back().IsDict()) { nodes_stack_.back().AsDict()[key_.back()] = nodes_all_.back(); key_.pop_back(); }
        }
        lock_builder_[0] = "enddict";
        lock_builder_.pop_back();
        --dict_count;
        return Item;
    }

    EndDictItemContext DictItemContext::EndDict() {
        return build.EndDict();
    }

    KeyItemContext DictItemContext::Key(std::string str) {
        return build.Key(str);
    }

    ValueKeyItemContext KeyItemContext::Value(Node node) {
        return static_cast<ValueKeyItemContext>(build.Value(node));
    }

    DictItemContext KeyItemContext::StartDict() {
        return build.StartDict();
    }

    ArrayItemContext KeyItemContext::StartArray() {
        return build.StartArray();
    }

    KeyItemContext EndDictItemContext::Key(std::string str) {
        return build.Key(str);
    }

    Node EndDictItemContext::Build()  {
        return build.Build();
    }

    DictItemContext EndDictItemContext::StartDict() {
        return build.StartDict();
    }

    ValueItemContext EndDictItemContext::Value(Node node) {
        return static_cast<ValueItemContext>(build.Value(node));
    }
 
    ValueItemContext ArrayItemContext::Value(Node node) {
        return static_cast<ValueItemContext>(build.Value(node));
    }

    DictItemContext ArrayItemContext::StartDict() {
        return build.StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return build.StartArray();
    }

    Builder& ArrayItemContext::EndArray() {
        return build.EndArray();
    } 

    ValueItemContext ValueItemContext::Value(Node node) {
        return static_cast<ValueItemContext>(build.Value(node));
    }

    DictItemContext ValueItemContext::StartDict() {
        return build.StartDict();
    }

    Builder& ValueItemContext::EndArray() {
        return build.EndArray();
    }

    ArrayItemContext ValueItemContext::StartArray() {
        return build.StartArray();
    }

    EndDictItemContext ValueKeyItemContext::EndDict() {
        return build.EndDict();
    }

    KeyItemContext ValueKeyItemContext::Key(std::string str) {
        return build.Key(str);
    }

    DictItemContext BaseContext::StartDict() {
        return build.StartDict();
    }

    KeyItemContext BaseContext::Key(std::string str) {
        return build.Key(str);
    }

    Node BaseContext::Build() {
        return build.Build();
    }

    BaseContext BaseContext::Value(Node node) {
        return build.Value(node);
    }

    ArrayItemContext BaseContext::StartArray() {
        return build.StartArray();
    }

    Builder& BaseContext::EndArray() {
        return build.EndArray();
    }

    EndDictItemContext BaseContext::EndDict() {
        return build.EndDict();
    }

}