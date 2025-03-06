#include "requests.cpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cpr/cpr.h>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

class Thread {
    std::string From;
    std::string To;
    std::vector<std::string> Transport;
    std::string DepartureTime;
    std::string ArrivalTime;
public:
    explicit Thread(const std::string& From, const std::string& To, const std::vector<std::string> T, const std::string& DT, const std::string& AT) {
        this->From = From;
        this->To = To;
        this->Transport = T;
        this->DepartureTime = DT;
        this->ArrivalTime = AT;
    }

    void PrintThread() const {
        std::cout << From << " -> " << To << '\n';
        for (int i = 0; i < Transport.size(); i++) {
            std::cout << Transport[i] << '\t';
        }
        std::cout << '\n';

        std::cout << "departure time: " << DepartureTime << '\n';
        std::cout << "Arrival time:   " << ArrivalTime << '\n';
    }
};


template <typename T>
void appropriateAdding(const json& obj, const std::string& key, T& target) {
    if (obj.contains(key) && !obj[key].is_null()) {
        target = obj[key].get<T>();
    }
}

void makeJsonCache(const json& w) {
    using namespace std::chrono;
    const auto p1 = system_clock::now();
    json jsonForCache;
    jsonForCache["date"] = duration_cast<seconds>(p1.time_since_epoch()).count();
    jsonForCache["data"] = w;

    std::ofstream cache_file;
    cache_file.open("cache_file.json");
    cache_file << jsonForCache.dump(4);
    cache_file.close();
}


void schedule(const json& data) {

    std::cout << "\nНайдено ниток: " << data["pagination"]["total"] << "\n" << "\n"; // падаём прямо тут
    for (const auto& seg : data["segments"]) {
        std::string city1, city2;
        std::string dep_time, arr_time;
        std::vector<std::string> transports;


        appropriateAdding(seg.value("from", json{}), "title", city1);
        if (city1.size() == 0) {appropriateAdding(seg.value("departure_from", json{}), "title", city1);}
        appropriateAdding(seg.value("to", json{}), "title", city2);

        if (city2.size() == 0) appropriateAdding(seg.value("arrival_to", json{}), "title", city2);

        if ((seg.contains("details") && seg["details"].is_array() && !seg["details"].empty())){

            for (const auto& i : seg["details"]) {
                if (i.contains("thread")) {
                    transports.push_back(i["thread"]["transport_type"]);
                }
            }

        } else {
            transports.push_back(seg["from"]["transport_type"]);
        }

        appropriateAdding(seg, "departure", dep_time);
        appropriateAdding(seg, "arrival", arr_time);


        Thread current_thread = Thread(city1, city2, transports, dep_time, arr_time);

        current_thread.PrintThread();
        std::cout << "________________________________" << '\n';
    }
}

int main(int argc, char** argv) {
// argv[1] вида YYYY-MM-DD
    using namespace std::chrono;

    std::string date;
    if (argc < 2) {
        std::cout << "you didn't give me date, so it'll be 2025-02-27" << '\n';
        date = "2025-02-27";
    }else date = argv[1];

    std::pair<std::string, std::string> town_codes = townCodes();

    json w;
    if (std::filesystem::exists("./cache_file.json")) {
        std::ifstream ofs("./cache_file.json");
        json cached_json;
        ofs >> cached_json;
        //static_cast<int>(cached_json["date"]) - время из кэша

        int timeNow = duration_cast<seconds>(system_clock::now().time_since_epoch()).count(); // время прямо сейчас


        if ( ((timeNow - static_cast<int>(cached_json["date"])) < 3600) && (cached_json["data"]["search"]["date"] == date)) {
            w = cached_json["data"];
            std::cout << "Кэш актуален!" << '\n';
        } else {
            std::cout << "Кэш устарел или выбрана другая дата" << "\n";
            w = getWays(town_codes.first, town_codes.second, date);
            makeJsonCache(w);
        }

    } else {
        std::cout << "Кэша ещё нет, но он будет!" << "\n";
        w = getWays(town_codes.first, town_codes.second, date);
        makeJsonCache(w);
    }

    schedule(w);

    return 0;
}
