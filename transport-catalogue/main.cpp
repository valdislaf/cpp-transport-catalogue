#include <fstream>
#include <iostream>

#include "json_reader.h"


int main() {
   
      /*
       * Примерная структура программы:
       * std::istringstream str (R"()"s);
       * Считать JSON из stdin
       * Построить на его основе JSON базу данных транспортного справочника
       * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
       * с ответами.
       * Вывести в stdout ответы в виде JSON
       */
    
    std::string full_path = "s10_final_opentest_3.json";   
    std::ifstream is;
    is.open(full_path.c_str(), std::ios::in | std::ios::binary);//ios::binary - бинарное открытие,ios::in - операции ввода
    std::string line;
    if (is.is_open()) {
       JsonReader(is,std::cout);
    }
    is.close();     // закрываем файл

   // JsonReader(std::cin,std::cout);
   



   return 0;

}


