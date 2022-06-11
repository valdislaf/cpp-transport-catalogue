#include <fstream>
#include <iostream>
#include <cassert>
#include <string_view>

#include "json_reader.h"

using namespace std::literals;


void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        //std::string full_path = "s14_2_opentest_3_make_base.json"s;
        // std::ifstream is;
        // is.open(full_path.c_str(), std::ios::in | std::ios::binary);//ios::binary - бинарное открытие,ios::in - операции ввода
        // std::string line;
        // if (is.is_open()) {
        //     JsonReaderMakeBase(is);
        // }
        // is.close();     // закрываем файл
        JsonReaderMakeBase(std::cin);

    }
    else if (mode == "process_requests"sv) {

        //std::string full_path = "s14_2_opentest_3_process_requests.json"s;
        // std::ifstream is;
        // is.open(full_path.c_str(), std::ios::in | std::ios::binary);//ios::binary - бинарное открытие,ios::in - операции ввода
        // std::string line;
        // if (is.is_open()) {
        //     JsonReaderProcessRequests(is, std::cout);             
        // }
        // is.close();     // закрываем файл
        JsonReaderProcessRequests(std::cin, std::cout);

         //std::ifstream f1("s14_1_opentest_3_answer.json"s, std::ios::binary);
         //std::ifstream f2("out.json"s, std::ios::binary);
         //if (f1.fail() || f2.fail()) {
         //    return false; //file problem
         //}
         //if (f1.tellg() != f2.tellg()) {
         //    return false; //size mismatch
         //}
         ////seek back to beginning and use std::equal to compare contents
         //f1.seekg(0, std::ifstream::beg);
         //f2.seekg(0, std::ifstream::beg);
         //assert(std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
         //    std::istreambuf_iterator<char>(),
         //    std::istreambuf_iterator<char>(f2.rdbuf())));

    }
    else {
        PrintUsage();
        return 1;
    }
}
