#pragma once

#include "transport_catalogue.h"

using namespace transport::catalogue;
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
 // с другими подсистемами приложения.
 // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
 /*
 class RequestHandler {
 public:
     // MapRenderer понадобится в следующей части итогового проекта
     RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

     // Возвращает информацию о маршруте (запрос Bus)
     std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

     // Возвращает маршруты, проходящие через
     const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

     // Этот метод будет нужен в следующей части итогового проекта
     svg::Document RenderMap() const;

 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
     const TransportCatalogue& db_;
     const renderer::MapRenderer& renderer_;
 };
 */


class RequestHandler {

public:

    RequestHandler(TransportCatalogue& db);

    const transport::catalogue::RouteInfo GetRouteInfo(const std::string& str);

    const transport::catalogue::StopInfo GetListBuses(const std::string& str);

    const std::deque<const Bus*> GetBuses();

    const std::deque<const Stop*> GetStops();

    const std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, transport::catalogue::TransportCatalogue::Hasher> GetStopsDistance();

    const std::unordered_map<std::string, double> GetRoutingSettings();

    RenderSettingsStruct GetRenderSettings();

private:

    TransportCatalogue& db_;
};