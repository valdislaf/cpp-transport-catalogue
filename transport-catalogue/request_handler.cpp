#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(TransportCatalogue& db) :db_(db) {
}

const transport::catalogue::RouteInfo RequestHandler::GetRouteInfo(const std::string& str) {
    return  db_.GetRouteInfo(str);
}

const transport::catalogue::StopInfo RequestHandler::GetListBuses(const std::string& str) {
    return  db_.GetListBuses(str);
}

const std::deque<const Stop*> RequestHandler::GetStops() {
    return db_.GetStops();
}

const std::deque<const Bus*> RequestHandler::GetBuses() {
    return db_.GetBuses();
}

void RequestHandler::AddStop(Stop&& stop){
    db_.AddStop(std::move(stop));
}

void RequestHandler::AddStopsLength(std::string stops, int Length){
    db_.AddStopsLength(stops, Length);
}

void RequestHandler::AddBus(Bus&& bus){
    db_.AddBus(std::move(bus));
}

const Stop* RequestHandler::GetStop(const std::string& stop){
    return db_.GetStop(stop);
}
