#include <curl/curl.h>
#include <string.h>

using namespace std;

string getHtml(string url);
static int writer(char *data, size_t size, size_t nmemb,std::string *writerData);
