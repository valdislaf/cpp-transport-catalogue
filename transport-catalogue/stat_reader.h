#pragma once
 
#include  <string>
#include  <iostream>
#include  <ostream>
#include <sstream>

#include "transport_catalogue.h"

namespace transport {
	
	using namespace catalogue;

	namespace output {

		namespace  requestseparator {

			std::ostream& operator<<(std::ostream& out, const  RouteInfo& routeinfo);

			std::ostream& operator<<(std::ostream& out, const  StopInfo& request);			

			class RequestHandler {
			public:
			void handle(TransportCatalogue& TC, std::string_view str);
			};
		}

		void out(TransportCatalogue& TC, std::istream& input);

	}
}