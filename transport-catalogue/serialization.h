#pragma once

#include <filesystem>
#include <fstream>
#include <transport_catalogue.pb.h>
#include <google/protobuf/message.h>

#include "json_builder.h"
#include "request_handler.h"
#include "transport_catalogue.h"

using namespace transport::catalogue;

void Serialize(const std::filesystem::path& path, RequestHandler& handler);

void Deserialize(const std::filesystem::path& path, TransportCatalogue& TC);