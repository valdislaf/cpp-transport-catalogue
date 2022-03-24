#include "stat_reader.h"

using namespace std;

namespace transport {

    using namespace catalogue;

    namespace output {

        namespace  requestseparator {

            std::ostream& operator<<(std::ostream& out, const  RouteInfo& routeinfo) {

                if (routeinfo.bus == nullptr) {
                    out << "Bus " << routeinfo.bus_name
                        << ": not found"
                        << std::endl;
                }
                else {
                    out << "Bus " << routeinfo.bus_name
                        << ": "
                        << routeinfo.bus_stops_size
                        << " stops on route, "
                        << routeinfo.unique_stops
                        << " unique stops, "
                        << routeinfo.routelength
                        << " route length, "
                        << std::setprecision(6)
                        << routeinfo.curvature
                        << " curvature"
                        << std::endl;
                }
                return out;
            }

            std::ostream& operator<<(std::ostream& out, const StopInfo& request)
            {
                out << "Stop ";
                out << request.name_bus << ": ";
                std::string buses;
                buses = "buses";
                for (auto s : request.sw) {
                    buses += " ";
                    buses += s;
                }
                if (!request.found) { buses = "not found"; }
                if (request.found && request.size == 0) { buses = "no buses"; }
                out << buses << std::endl;
                return out;
            }

            void RequestHandler::handle(TransportCatalogue& TC, std::string_view str)
            {
                if (str.substr(0, 3) == "Bus") {                                      
                    cout << TC.GetRouteInfo(str.substr(4));
                }

                if (str.substr(0, 4) == "Stop") {
                    cout << TC.GetListBuses(str.substr(5));
                }
            }
        }

        void out(TransportCatalogue& TC, std::istream& input)
        {
            requestseparator::RequestHandler T;
            string n;
            std::getline(input, n);
            std::string str;
            vector<string>query;
            for (int i = 0; i < stoi(n); ++i) {
                std::getline(input, str);
                query.push_back(str);
            }
            for (auto& s : query) {               
                T.handle(TC, s);           
            }
        }
    }
}