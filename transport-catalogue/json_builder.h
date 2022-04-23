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

    class Builder; 
    class DictItemContext;
    class KeyItemContext;
    class EndDictItemContext;
    class ArrayItemContext;
    class BaseContext;  
    class ValueItemContext;
    class ValueKeyItemContext; 
   
    class Builder  {
    public:
       
        DictItemContext StartDict();
        KeyItemContext Key(std::string);
        Node Build();
        BaseContext Value(Node node) ;
        ArrayItemContext StartArray();     
        Builder& EndArray();
        EndDictItemContext EndDict();

    protected:     
        Node node_{};       
        std::vector <std::string>  key_{};
        Array arr_{};
        Dict dict_{};
        std::vector<Node> nodes_stack_;      
        std::vector<Node> nodes_all_;
        std::vector<std::string> lock_builder_{ "builder" };
        std::string dict_arr = "out_dict";
        int array_count = 0;
        int dict_count = 0;
        std::string key_open = "close";        
    };

    class BaseContext {
    public:
        BaseContext(Builder& build) :build(build) {
        }      
        DictItemContext StartDict();  
        BaseContext Value(Node node);
        ArrayItemContext StartArray();
        Builder& EndArray();
        Node Build();
        EndDictItemContext EndDict();
        KeyItemContext Key(std::string);
       
    protected:
            Builder& build;          
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext build) :BaseContext(build) {
        }
        EndDictItemContext EndDict();
        KeyItemContext Key(std::string);
        DictItemContext StartDict() = delete;
        Node Build() = delete;
        BaseContext Value(Node node) = delete;
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
    };   

    class EndDictItemContext : public BaseContext {
    public:
        EndDictItemContext(BaseContext build) :BaseContext(build) {
        }
        KeyItemContext Key(std::string);       
        Node Build();
        DictItemContext StartDict();
        ValueItemContext Value(Node node);
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
        EndDictItemContext EndDict() = delete;
    };  

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext build) :BaseContext(build) {
        }
      
        ValueItemContext Value(Node node);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        KeyItemContext Key(std::string) = delete;
        Node Build() = delete;
        EndDictItemContext EndDict() = delete;
    };

    class KeyItemContext : public BaseContext {
    public:
        KeyItemContext(BaseContext build) :BaseContext(build) {
        }
        ValueKeyItemContext Value(Node node);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        KeyItemContext Key(std::string) = delete;
        Node Build() = delete;
        Builder& EndArray() = delete;
        EndDictItemContext EndDict() = delete;
    };

    class ValueItemContext : public BaseContext {
    public:
        ValueItemContext(BaseContext build) :BaseContext(build) {
        }       
        ValueItemContext Value(Node node);
        DictItemContext StartDict();
        Builder& EndArray();
        ArrayItemContext StartArray();
        Node Build() = delete;
        KeyItemContext Key(std::string) = delete;
        EndDictItemContext EndDict() = delete;
    };

    class ValueKeyItemContext : public BaseContext {
    public:
        ValueKeyItemContext(BaseContext build) :BaseContext(build) {
        }
        EndDictItemContext EndDict();
        KeyItemContext Key(std::string);
        DictItemContext StartDict() = delete;
        Node Build() = delete;
        BaseContext Value(Node node) = delete;
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
    };

}

