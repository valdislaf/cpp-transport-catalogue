#pragma once

#include <deque>
#include <string>
#include <unordered_map>

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


class Stop {

private:

    std::string name_;
    double latitude_;
    double longitude_;
    std::unordered_map<std::string, int>tostop_;
    std::tuple<std::string,
        double,
        double,
        std::unordered_map<std::string, int>> AsTuple() const;

public:

    Stop(std::string_view name,
        double latitude,
        double longitude);

    Stop(std::string_view name,
        double latitude,
        double longitude,
        std::unordered_map<std::string,
        int>tostop);

    size_t Hash() const;

    std::string_view name() const;

    double latitude() const;

    double longitude() const;

    std::unordered_map<std::string, int> tostop() const;

    bool operator==(const Stop& other) const;
};

struct Bus {
    std::string name;
    std::deque<const Stop*>stops;
    bool is_roundtrip;
};