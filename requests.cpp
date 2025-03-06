#include <iostream>
#include <cpr/cpr.h>
#include <string>
#include <nlohmann/json.hpp>

#pragma once

using json = nlohmann::json;


std::pair<std::string, std::string> townCodes() {
    std::string key = std::getenv("YNDX_API");

    std::string request = "http://api.rasp.yandex.net/v3.0/nearest_settlement/?";

    std::string lat = "59.9343";
    std::string lng = "30.3351";
    std::string distance = "40";

    request += ("apikey=" + key + "&lat=" + lat + "&lng=" + lng + "&distance=" + distance);

    cpr::Response resp = cpr::Get(cpr::Url(request));
	if(resp.status_code == 0) std::cerr << resp.error.message << std::endl;

    json piter = json::parse(resp.text);

    request = "http://api.rasp.yandex.net/v3.0/nearest_settlement/?";

    lat = "57.8136";
    lng = "28.3496";
    distance = "50";

    request += ("apikey=" + key + "&lat=" + lat + "&lng=" + lng + "&distance=" + distance);

    resp = cpr::Get(cpr::Url(request));
    if(resp.status_code == 0) std::cerr << resp.error.message << std::endl;

    json pskov = json::parse(resp.text);

    return {std::string(piter["code"]), std::string(pskov["code"])};
}

json getWays(std::string piter, std::string pskov, std::string date) {
	std::string key = std::getenv("YNDX_API");
    std::string request = "http://api.rasp.yandex.net/v3.0/search/?";

    request +=("apikey=" + key);
    request += ("&from=" + piter);
    request += ("&to=" + pskov);
    request += ("&lang=ru_RU&page=1");
    request += ("&date=" + date);
    request += "&transfers=true";
    // std::cout << request << std::endl;

    cpr::Response resp = cpr::Get(cpr::Url(request));

    if(resp.status_code == 0) std::cerr << resp.error.message << std::endl;

    json ways = json::parse(resp.text);

    return ways;
}
