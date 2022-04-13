#include "domain.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

Stop::Stop(std::string_view name, double latitude, double longitude)
    :name_(name),
    latitude_(latitude),
    longitude_(longitude) {
}

Stop::Stop(std::string_view name, double latitude, double longitude, std::unordered_map<std::string, int> tostop)
    :name_(name),
    latitude_(latitude),
    longitude_(longitude),
    tostop_(tostop) {
}

size_t Stop::Hash() const {
    std::hash<double> d_hasher;
    auto h1 = d_hasher(latitude_);
    auto h2 = d_hasher(longitude_);
    return  static_cast<size_t>(h1 * 37 + h2);
}

std::string_view Stop::name() const {
    return name_;
}

double Stop::latitude() const {
    return latitude_;
}

double Stop::longitude() const {
    return longitude_;
}

std::unordered_map<std::string, int> Stop::tostop() const {
    return tostop_;
}

bool Stop::operator==(const Stop& other) const {
    return AsTuple() == other.AsTuple();
}

std::tuple<std::string, double, double, std::unordered_map<std::string, int>>  Stop::AsTuple() const {
    return tie(name_, latitude_, longitude_, tostop_);
}
