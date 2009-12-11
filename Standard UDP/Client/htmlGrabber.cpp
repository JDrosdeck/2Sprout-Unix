#include <curl/curl.h>
#include <iostream>
#include <strings.h>
#include <stdlib.h>

using namespace std;


/*
Libcurl callback which will write the html recieved into memory instead of passing it to stdout
*/
static int writer(char *data, size_t size, size_t nmemb,
                  std::string *writerData)
{
  if (writerData == NULL)
    return 0;

  writerData->append(data, size*nmemb);
  return size * nmemb;
}




string getHtml(string url, string postFields)
{
	char errorBuffer[CURL_ERROR_SIZE];
	string buffer;
	
	CURL *curl;
	CURLcode res;

	buffer.clear();
	bzero(errorBuffer, sizeof(errorBuffer));
	
	curl = curl_easy_init();
	if (curl == NULL)
	{
		fprintf(stderr, "Failed to create CURL connection\n");
		exit(EXIT_FAILURE);
	}

	res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    if (res != CURLE_OK)
	{
		fprintf(stderr, "Failed to set ssl verify peer [%d]\n", res);
		return false;
	}

	res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    if(res != CURLE_OK)
	{
		fprintf(stderr, "Failed to set ssl verify host [%d]\n", res);
		return false;
	}

	res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "Failed to set error buffer [%d]\n", res);
		return false;
	}

	res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if (res != CURLE_OK)
	{
		fprintf(stderr, "Failed to set URL [%s]\n", errorBuffer);
		return false;
	}

	if (postFields != "")
	{
		printf("POSTING\n");
	  res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
	  if (res != CURLE_OK)
	  {
	    fprintf(stderr, "Failed to set postField [%s]\n", errorBuffer);
	    return false;
	  }

	}

	res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
		return false;
	}

	res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "Failed to set writer [%s]\n", errorBuffer);
		return false;
	}

	res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "Failed to set write data [%s]\n", errorBuffer);
		return false;
	}

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	return buffer;
}


