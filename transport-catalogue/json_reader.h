#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <variant>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "serialization.h"

using namespace std::literals;
using namespace svg;
using namespace transport::catalogue;

struct RouteSettings
{
    double bus_velocity = 0.0;
    double bus_wait_time = 0.0;
};

json::Dict JsonBuses(const RouteInfo& response, json::Node& doc);

json::Dict JsonStops(const StopInfo& response, json::Node& doc);

void FormatResponse(json::Document& load_input, RequestHandler& handler, std::ostream& out);

json::Dict JsonMap(RequestHandler& handler, json::Node& doc);

void AddStopJson(TransportCatalogue& TC, json::Node load_stop);

void AddBusJson(TransportCatalogue& TC, json::Node load_bus);

void AddStopDistance(TransportCatalogue& TC, json::Node load_stop);


void JsonReaderMakeBase(std::istream& in);

void JsonReaderProcessRequests(std::istream& in, std::ostream& out);