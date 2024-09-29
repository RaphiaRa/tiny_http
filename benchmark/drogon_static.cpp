// g++ static.cpp -O3 -ldrogon -ljsoncpp -ltrantor -luuid -lz -o static

#include <drogon/drogon.h>

int main()
{
    drogon::app()
        .addListener("0.0.0.0", 8080)
        .setDocumentRoot("./testfiles")
        .run();
    drogon::app().enableGzip(false);
    return 0;
}
