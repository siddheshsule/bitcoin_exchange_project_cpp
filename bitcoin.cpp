#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <array>
#include <string>
#include <cstdio>

#include <curl/curl.h>
#include "json.hpp"

// Define a unique pointer type for CURL with a custom deleter function
typedef std::unique_ptr<CURL, std::function<void(CURL*)>> CURL_ptr;

// External C function used as a callback to handle data received by CURL
extern "C" std::size_t dataHandler(const char* buffer, std::size_t size,
    std::size_t nmemb, std::string* userData) {
        if (userData == nullptr) {
            return 0;
        }
        // Append the received data to the user-defined string
        userData->append(buffer, (size * nmemb));
        return size * nmemb;
    }

// Class representing a wrapper for a CURL handle
class CurlHandle {
    private:
        // Unique pointer for managing the CURL handle with a custom deleter
        CURL_ptr curlptr;
        // Custom deleter function for the CURL handle
        constexpr static auto deleter = [](CURL* c) {
            curl_easy_cleanup(c);
            curl_global_cleanup();
        };
        // String to store the fetched data
        std::string data;
    public:
        // Constructor initializes the CURL handle and sets callback function and data pointer
        CurlHandle() : curlptr(curl_easy_init(), deleter) {
            curl_global_init(CURL_GLOBAL_ALL);
            curl_easy_setopt(curlptr.get(), CURLOPT_WRITEFUNCTION, dataHandler);
            curl_easy_setopt(curlptr.get(), CURLOPT_WRITEDATA, &data);
        }

        // Set the URL for the CURL handle
        void setUrl(std::string url) {
            curl_easy_setopt(curlptr.get(), CURLOPT_URL, url.c_str());
        }

        // Perform the CURL request and return the result code
        CURLcode fetch() {
            return curl_easy_perform(curlptr.get());
        }

        // Get the fetched data as a string
        std::string getFetchedData() {
            return data;
        }
};

// Class representing Bitcoin data retrieval using the Blockchain.info API
class Bitcoin {
    using json = nlohmann::json;

    private:
        // Instance of CurlHandle for handling CURL requests
        CurlHandle curlHandle;
        // URL for the Blockchain.info API
        static constexpr const char* API_URL = "https://blockchain.info/ticker";

    public:
        // Constructor initializes the CurlHandle instance and sets the API URL
        Bitcoin() : curlHandle({}) {
            curlHandle.setUrl(API_URL);
        }

        // Fetch Bitcoin data from the API and return it as JSON
        json fetchBitcoinData() {
            curlHandle.fetch();
            return json::parse(curlHandle.getFetchedData());
        }
};

// Main function
int main() {
    using namespace std;
    using json = nlohmann::json;

    try {
        // Create an instance of the Bitcoin class
        Bitcoin bitcoin;
        // Fetch Bitcoin data from the API
        json bitcoinData = bitcoin.fetchBitcoinData();

        // Display Bitcoin exchange rates
        cout << "1 BTC = \n";
        for (auto it = bitcoinData.begin(); it != bitcoinData.end(); ++it) {
            printf("\t(%3s)%10d %s\n", it.key().c_str(), it.value()["last"].get<int>(), it.value()["symbol"].get<string>().c_str());
        }
    }
    catch(...) {
        // Handle exceptions
        cerr << "Failed to fetch Bitcoin exchange rates!";
    }
}
