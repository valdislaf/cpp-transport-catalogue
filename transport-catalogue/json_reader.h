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

#include "json.h"
#include "svg.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
using namespace transport::catalogue;
using namespace std::literals;
using namespace svg;

std::vector<json::Node> JsonBuses(const RouteInfo& response, json::Node& doc);

std::vector<json::Node> JsonStops(const StopInfo& response, json::Node& doc);

void FormatResponse(const json::Document& load_input, RequestHandler& handler, std::ostream& out);

std::string PrintSVG(std::istream& input);

std::vector<json::Node> JsonMap(RequestHandler& handler, json::Node& doc, json::Document& load_input);

void AddStopJson(RequestHandler& handler, json::Node load_stop);

void AddBusJson(RequestHandler& handler, json::Node load_bus);

void JsonReader(std::istream& in, std::ostream& out);