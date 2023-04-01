// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <string>
#include <fstream>
#include <libxml++/libxml++.h>
#include <sstream>
#include <curl/curl.h>

void print_links(const xmlpp::Node* node)
{
    if(auto nodeElement = dynamic_cast<const xmlpp::Element*>(node))
    {
        if (nodeElement->get_name() == "a")
        {
            auto link = nodeElement->get_attribute("href")->get_value();
            if (link.find("http") == 0)
            std::cout << "Found a link: " << nodeElement->get_attribute("href")->get_value() << std::endl;
        }
    }
    auto nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
    if (!nodeContent)
    {
        for(const auto& child : node->get_children())
        {
            print_links(child);
        }
    }
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::string data(reinterpret_cast<const char*>(ptr), static_cast<size_t>(size * nmemb));
    reinterpret_cast<std::string*>(stream)->append(data);
    return size * nmemb;
}

int main(int argc, char* argv[]) {
    CURL *curl;
    CURLcode res;
    std::string str;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://libxmlplusplus.github.io/libxmlplusplus/");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    xmlpp::DomParser parser;
    parser.parse_memory(str);
    xmlpp::Document* doc = parser.get_document();

    print_links(doc->get_root_node());

    return 0;
}
