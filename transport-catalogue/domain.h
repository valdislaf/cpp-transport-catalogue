#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include "geo.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

struct Stop {
    std::string name;
    transport::geo_coordinates::Coordinates coord;
    std::unordered_map<std::string, int> road_distances;
};

struct Bus {
    std::string name;
    std::deque<const Stop*>stops;
    bool is_roundtrip;
};