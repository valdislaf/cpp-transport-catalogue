#pragma once
 
#include "transport_catalogue.h"
 
namespace transport {

	using namespace catalogue;

	namespace output {

		namespace  requestseparator {

			void Query(TransportCatalogue& TC, std::string_view str);
		}

		void out(TransportCatalogue& TC);

	}
}