#pragma once

#include "string"

#include "../curl/include/curl/curl.h"

static std::string sPubApiKey = "b83145c8d2d79cd029df9b977546275903324f60f470f9cb4dd334fccde2f41d";

namespace http_request_utils
{
	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	bool GetVirusInfo(CURL *curl, const char *sFileHash, ds_json::object &data_json, std::string &sRet)
	{
		std::string sRequest = "https://www.virustotal.com/vtapi/v2/file/report?apikey=";
		sRequest += sPubApiKey;
		sRequest += "&resource=";
		sRequest += sFileHash;
		sRequest += "&allinfo=true";

		curl_easy_setopt(curl, CURLOPT_URL, sRequest.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sRet);
		CURLcode res = curl_easy_perform(curl);

		if (CURLE_OK == res && !sRet.empty()) {
			ds_json::str2obj(sRet.c_str(), data_json);
			return true;
		}

		return false;
	}
}