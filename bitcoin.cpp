#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <array>
#include <cstdio>

#include <curl/curl.h>
#include "json.hpp"

typedef std::unique_ptr<CURL, std::function<void(CURL*)>> CURL_ptr;

class CurlHandle {
    private:
        CURL_ptr curlptr;
        constexpr static auto deleter = [](CURL* c) {
            curl_easy_cleanup(c);
            curl_global_cleanup();
        };
};


int main() {

    std::cout << "Hello, world!" << std::endl;

}