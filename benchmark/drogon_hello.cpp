// g++ static.cpp -O3 -ldrogon -ljsoncpp -ltrantor -luuid -lz -o static

#include <drogon/drogon.h>

int main()
{
    drogon::app().setThreadNum(1).registerHandler(
        "/",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setBody("Hello World!");
            callback(resp);
        },
        {drogon::Get});
    drogon::app().addListener("0.0.0.0", 8080);
    drogon::app().enableGzip(false);
    drogon::app().run();
    return 0;
}
