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

using namespace transport::catalogue;
using namespace std::literals;
using namespace svg;

void AddStopJson(TransportCatalogue& TC, json::Node LoadStop);

void AddBusJson(TransportCatalogue& TC, json::Node LoadBus);

void FormatResponse(const json::Document& LoadInput, TransportCatalogue& TC);

Color GetColorFromDict(json::Node color);

RenderSettings SetRenerSettings(json::Dict render_settings);



void JsonReader(std::istream& in);