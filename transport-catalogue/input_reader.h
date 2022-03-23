#pragma once
 
#include "transport_catalogue.h"
 
namespace transport {

	using namespace catalogue;

	namespace input {

		namespace  stringprocessing {

			Stop SpliStop(std::string_view str);

			Bus SplitBus(TransportCatalogue& TC, std::string_view str);
		}

		void read(TransportCatalogue& TC);
	}
}