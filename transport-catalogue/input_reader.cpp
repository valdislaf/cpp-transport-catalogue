#include "input_reader.h"

using namespace std;

namespace transport {

    using namespace catalogue;

    namespace input {

        namespace  stringprocessing {

            //In case of extra spaces
            std::string TrimSpaces(string& str) {
                std::string temp = "";
                auto it1 = str.find_last_not_of(' ');
                str = str.substr(0, it1 + 1);

                while (it1 != string_view::npos) {
                    it1 = str.find_first_not_of(' ');
                    for (char& ch : str) {
                        ++it1;
                        if (ch == ' ') { break; }
                    }

                    if (temp.size() > 3) {
                        if ((str[0] == '>' || str[0] == '-') && (str[1] != ' ')) {
                            str.insert(str.begin() + 1, ' ');
                        }

                        if ((temp[temp.size() - 2] == '>' || temp[temp.size() - 2] == '-') && (temp[temp.size() - 3] != ' ')) {
                            temp.insert(temp.begin() + temp.size() - 2, ' ');
                        }
                    }

                    if (str[0] == ':' || str[0] == ',') {
                        temp = temp.substr(0, temp.size() - 1);
                    }
                    temp += str.substr(0, it1);
                    str = str.substr(it1);
                    if (str == "") { break; }
                    it1 = str.find_first_not_of(' ');
                    str = str.substr(it1);
                }

                return temp;
            }

            Stop SpliStop(string_view str) {
                // str = TrimSpaces(str);  //In case of extra spaces
                auto it_colon = str.find(':');
                auto it_comma = str.find(',');
                string_view name = str.substr(5, it_colon - 5);
                double latitude = stod(str.substr(it_colon + 2, it_comma - it_colon - 2).data());
                double longitude = stod(str.substr(it_comma + 2).data());

                size_t it_m = str.find('m');

                if (it_m != string_view::npos) {
                    std::unordered_map<std::string, uint64_t>tostop;
                    str = str.substr(it_comma + 2);
                    auto it_ = str.find(' ');
                    str = str.substr(it_ + 1);

                    while (it_comma != string_view::npos) {
                        it_m = str.find('m');
                        uint64_t len = stoi((str.substr(0, it_m)).data());
                        str = str.substr(it_m + 5);
                        it_comma = str.find(',');
                        string_view namestop = str.substr(0, it_comma);
                        tostop.insert({ {namestop.begin(),namestop.end() }, len
                            });
                        it_comma = str.find(',');
                        str = str.substr(it_comma + 2);
                    }

                    return { name, latitude, longitude, tostop };
                }

                return { name, latitude, longitude };
            }

            Bus SplitBus(TransportCatalogue& TC, string_view str) {
                Bus bus;
                // str = TrimSpaces(str);  //In case of extra spaces
                auto it_colon = str.find(':');
                bus.name = str.substr(4, it_colon - 4);
                str = str.substr(it_colon + 2);
                auto it_more = str.find('>');

                if (it_more != string_view::npos) {
                    while (it_more != string_view::npos) {
                        bus.stops.push_back(TC.GetStop(str.substr(0, it_more - 1)));
                        str = str.substr(it_more + 2);
                        it_more = str.find('>');
                    }
                    bus.stops.push_back(TC.GetStop(str.substr(0, it_more - 1)));
                }
                else {
                    auto  it_dash = str.find('-');

                    while (it_dash != string_view::npos) {
                        bus.stops.push_back(TC.GetStop(str.substr(0, it_dash - 1)));
                        str = str.substr(it_dash + 2);
                        it_dash = str.find('-');
                    }

                    bus.stops.push_back(TC.GetStop(str.substr(0, it_dash - 1)));
                    size_t sz = bus.stops.size();
                    for (size_t i = sz - 1; i > 0; i--) {
                        bus.stops.push_back(bus.stops[i - 1]);
                    }
                }

                return bus;
            }
        }

        void read(TransportCatalogue& TC) {
            string n;
            std::getline(std::cin, n);

            if (n != "") {
                std::string str;
                vector<string>buses;

                for (int i = 0; i < stoi(n); ++i) {
                    std::getline(std::cin, str);
                    if (str != "") {
                        auto it1 = str.find_first_not_of(' ');
                        str = str.substr(it1);

                        if (str.substr(0, 3) == "Bus") {
                            buses.push_back(str);
                        }
                        else {
                            TC.AddStop(stringprocessing::SpliStop(str));
                        }
                    }
                }

                for (auto& s : buses) {
                    TC.AddBus(stringprocessing::SplitBus(TC, s));
                }
            }
        }
    }
}