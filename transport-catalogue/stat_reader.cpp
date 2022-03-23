#include "stat_reader.h"

using namespace std;

namespace transport {

    using namespace catalogue;

    namespace output {

        namespace  requestseparator {

            void Query(TransportCatalogue& TC, string_view str) {

                if (str.substr(0, 3) == "Bus") {

                    str = str.substr(4);
                    const Bus* bus = TC.GetBus(str);

                    if (bus == nullptr) {
                        cout << "Bus " << str
                            << ": not found"
                            << endl;
                    }
                    else {
                        auto routelength = TC.GetRouteLength(bus);
                        cout << "Bus " << bus->name
                            << ": "
                            << bus->stops.size()
                            << " stops on route, "
                            << TC.GetUniqs(bus)
                            << " unique stops, "
                            << routelength
                            << " route length, "
                            << setprecision(6)
                            << TC.GetCurvature(bus, routelength)
                            << " curvature"
                            << endl;
                    }
                }

                if (str.substr(0, 4) == "Stop") {
                    auto request = TC.GetListBuses();
                    cout << str << ": ";
                    bool found = false;
                    std::string buses;
                    buses = "buses";
                    std::set<string_view> sw;
                    for (auto& s : request) {
                        if (s.first->name() == str.substr(5)) {
                            found = true;
                            for (auto a : s.second) {
                                sw.insert(a->name);
                            }
                        }
                    }

                    for (auto s : sw) {
                        buses += " ";
                        buses += s;
                    }

                    if (!found) { buses = "not found"; }
                    if (found && buses == "buses") { buses = "no buses"; }
                    cout << buses << endl;
                }
            }
        }

        void out(TransportCatalogue& TC) {
            string n;
            std::getline(std::cin, n);
            std::string str;
            vector<string>query;
            for (int i = 0; i < stoi(n); ++i) {
                std::getline(std::cin, str);
                query.push_back(str);
            }
            for (auto& s : query) {
                requestseparator::Query(TC, s);
            }
        }
    }
}