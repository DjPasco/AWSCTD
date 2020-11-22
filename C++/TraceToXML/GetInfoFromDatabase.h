#pragma once

#include "unordered_map"
#include "array"
#include "sstream"
#include <fstream>
#include <iostream>

namespace get_info_from_database
{
	void GetParamsInfo(LPCTSTR sCallName, std::vector<int> &arrArgAloved)
	{
		dsDatabase db;
		db.OpenDB(L"d:\\Doktor\\SystemCalls.db");

		CSystemCalls callsLoader(&db);
		if (!callsLoader.MoveFirst()) {
			return;
		}

		CVirusFiles filesLoader(&db);
		CVirusInfo virusInfoLoader(&db);

		std::unordered_map<int, bool> okFiles;

		std::unordered_map<int, bool>::const_iterator it_find_file;

		const int nArgsCountConst = 11;

		std::array<std::unordered_map<std::wstring, int>, nArgsCountConst> arrArgs;
		std::unordered_map<std::wstring, int>::iterator argsFind_it;
		int nCallNumber(0);

		clock_t startTime = clock();
		std::wstring sCallNumber;
		std::wstring sCall;
		int nFileID;
		std::wstring sArg;
		size_t nPosString;
		std::wstring sConst;
		size_t nFindSpace;
		//    size_t nFind1;
		size_t nFind2;
		std::string sScans;
		ds_json::object kaspersky;
		ds_json::object scans;
		ds_json::array args;
		int nSize;
		while (!callsLoader.IsEOF()) {
			nCallNumber++;
			sCall = callsLoader.GetSystemCall();
			if (sCall != sCallName) {
				callsLoader.MoveNext();
				continue;
			}

			nFileID = callsLoader.GetfkVirusFile();
			it_find_file = okFiles.find(nFileID);
			if (okFiles.end() == it_find_file) {
				if (!filesLoader.SeekById(nFileID)) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				int nVirusID = filesLoader.GetfkVirus();
				if (!virusInfoLoader.SeekById(nVirusID)) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				sScans = virusInfoLoader.GetVirusScanResUTF8();
				ds_json::str2obj(sScans.c_str(), scans);

				scans.GetJsonObject("Kaspersky", kaspersky);

				if (kaspersky.IsNull("result")) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}
				else {
					okFiles[nFileID] = true;
					sCallNumber = std::to_wstring(nCallNumber);
					std::wcout << L"Call Number: " << sCallNumber << L"\n";
				}
			}
			else {
				if (it_find_file->second == false) {
					callsLoader.MoveNext();
					continue;
				}
			}

			ds_json::array args;
			callsLoader.GetArguments(args);

			nSize = args.GetSize();
			for (int i = 0; i < nSize; ++i) {

				auto it_find_aloved = std::find(arrArgAloved.begin(), arrArgAloved.end(), i);

				if (arrArgAloved.end() == it_find_aloved) {
					continue;
				}
				sArg = args.GetString(i);
				nPosString = sArg.find(L"\"");
				if (std::wstring::npos == nPosString) {
					nFindSpace = sArg.find(L" ");
					sConst = sArg.substr(0, nFindSpace);
				}
				else {
					nFind2 = sArg.rfind(L"\"");

					if (std::wstring::npos == nFind2) {
						continue;
					}

					sConst = sArg.substr(nPosString + 1, nFind2 - nPosString - 1);
				}
				argsFind_it = arrArgs[i].find(sConst);

				if (argsFind_it == arrArgs[i].end()) {
					arrArgs[i].insert(std::make_pair(sConst, 1));
				}
				else {
					argsFind_it->second += 1;
				}
			}

#ifdef _DEBUG
			if (nCallNumber == 200) {
				//break;
			}
#endif

			callsLoader.MoveNext();
		}

		double dTime = double(clock() - startTime) / (double)CLOCKS_PER_SEC;
		std::wstring sTime = std::to_wstring(dTime);

		std::wcout << sTime.c_str() << L" seconds.\n";

		std::wofstream jsonFile(std::wstring(sCallName) + L".txt");

		for (size_t k = 0; k < arrArgs.size(); ++k) {
			jsonFile << std::to_wstring(k) << L"\n";
			for (auto item : arrArgs[k]) {
				std::wstring sNumber = std::to_wstring(item.second);
				jsonFile << item.first.c_str() << L"|" << sNumber << L"\n";
			}
		}
	}

	void GetCallsInfo()
	{
		dsDatabase db;
		db.OpenDB(L"d:\\Doktor\\SystemCalls.db");

		CSystemCalls callsLoader(&db);
		if (!callsLoader.MoveFirst()) {
			return;
		}

		CVirusFiles filesLoader(&db);
		CVirusInfo virusInfoLoader(&db);

		std::unordered_map<int, bool> okFiles;

		static int narraysize = 3;
		std::unordered_map<std::wstring, std::array<int, (size_t)3> > mapCalls;
		int nCallNumber(0);

		clock_t startTime = clock();
		std::wstring sCallNumber;
		std::wstring sCall;
		while (!callsLoader.IsEOF()) {
			nCallNumber++;
			int nFileID = callsLoader.GetfkVirusFile();
			auto it_find_file = okFiles.find(nFileID);

			if (okFiles.end() == it_find_file) {
				if (!filesLoader.SeekById(nFileID)) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				int nVirusID = filesLoader.GetfkVirus();
				if (!virusInfoLoader.SeekById(nVirusID)) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				int nPositives = virusInfoLoader.GetPositives();
				if (nPositives < 15) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				ds_json::object scans;
				std::string sScans = virusInfoLoader.GetVirusScanResUTF8();
				ds_json::str2obj(sScans.c_str(), scans);

				ds_json::object kaspersky;
				scans.GetJsonObject("Kaspersky", kaspersky);

				if (kaspersky.IsNull("result")) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}
				else {
					okFiles[nFileID] = true;
					sCallNumber = std::to_wstring(nCallNumber);
					std::wcout << L"Call Number: " << sCallNumber << L"\n";
				}
			}
			else {
				if (it_find_file->second == false) {
					callsLoader.MoveNext();
					continue;
				}
			}

			sCall = callsLoader.GetSystemCall();
			auto find = mapCalls.find(sCall);

			int nSuccess = callsLoader.GetSuccess();

			if (find == mapCalls.end()) {
				std::array<int, 3> arrDef = { 1, 0, 0 };
				arrDef[0] = 1;
				arrDef[1] = nSuccess == 1 ? 1 : 0;
				arrDef[2] = nSuccess == 1 ? 0 : 1;
				mapCalls.insert(std::make_pair(sCall, arrDef));
			}
			else {
				find->second[0] += 1;
				find->second[1] += nSuccess == 1 ? 1 : 0;
				find->second[2] += nSuccess == 1 ? 0 : 1;
			}

#ifdef _DEBUG
			if (nCallNumber == 200) {
				//break;
			}
#endif

			callsLoader.MoveNext();
		}

		double dTime = double(clock() - startTime) / (double)CLOCKS_PER_SEC;
		std::wstring sTime = std::to_wstring(dTime);

		std::wcout << sTime.c_str() << L" seconds.\n";

		std::wofstream jsonFile(L"calls.txt");

		for (auto call : mapCalls) {
			std::wstring sNumber = std::to_wstring(call.second[0]);
			std::wstring sTrue = std::to_wstring(call.second[1]);
			std::wstring sFalse = std::to_wstring(call.second[2]);
			jsonFile << call.first.c_str() << L"|" << sNumber << L"|" << sTrue << L"|" << sFalse << L"\n";
		}
	}

	void GetArgsInfo()
	{
		dsDatabase db;
		db.OpenDB(L"d:\\Doktor\\SystemCalls.db");

		CSystemCalls callsLoader(&db);
		if (!callsLoader.MoveFirst()) {
			return;
		}

		CVirusFiles filesLoader(&db);
		CVirusInfo virusInfoLoader(&db);

		std::unordered_map<int, bool> okFiles;

		static int narraysize = 3;
		std::unordered_map<std::wstring, int> mapArgs;
		int nCallNumber(0);

		clock_t startTime = clock();
		std::wstring sCallNumber;
		while (!callsLoader.IsEOF()) {
			nCallNumber++;
			int nFileID = callsLoader.GetfkVirusFile();
			auto it_find_file = okFiles.find(nFileID);

			if (okFiles.end() == it_find_file) {
				if (!filesLoader.SeekById(nFileID)) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				int nVirusID = filesLoader.GetfkVirus();
				if (!virusInfoLoader.SeekById(nVirusID)) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				int nPositives = virusInfoLoader.GetPositives();
				if (nPositives < 15) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				ds_json::object scans;
				std::string sScans = virusInfoLoader.GetVirusScanResUTF8();
				ds_json::str2obj(sScans.c_str(), scans);

				ds_json::object kaspersky;
				scans.GetJsonObject("Kaspersky", kaspersky);

				if (kaspersky.IsNull("result")) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}
				else {
					okFiles[nFileID] = true;
					sCallNumber = std::to_wstring(nCallNumber);
					//std::wcout << L"Call Number: " << sCallNumber << L"\n";
				}
			}
			else {
				if (it_find_file->second == false) {
					callsLoader.MoveNext();
					continue;
				}
			}

			ds_json::array args;
			callsLoader.GetArguments(args);

			int nSize = args.GetSize();
			for (int i = 0; i < nSize; ++i) {
				std::wstring sArg = args.GetString(i);
				size_t nFind = sArg.find(L"type=named constant");
				if (nFind != std::wstring::npos) {
					size_t nFindSpace = sArg.find(L" ");
					const std::wstring sConst = sArg.substr(0, nFindSpace);
					auto find = mapArgs.find(sConst);

					if (find == mapArgs.end()) {
						mapArgs.insert(std::make_pair(sConst, 1));
					}
					else {
						find->second += 1;
					}
				}
			}

			callsLoader.MoveNext();
		}

		double dTime = double(clock() - startTime) / (double)CLOCKS_PER_SEC;
		std::wstring sTime = std::to_wstring(dTime);

		std::wcout << sTime.c_str() << L" seconds.\n";

		std::wofstream jsonFile(L"args.txt");

		for (auto call : mapArgs) {
			std::wstring sNumber = std::to_wstring(call.second);
			jsonFile << call.first.c_str() << L";" << sNumber << L"\n";
		}

	}

	void GetSuccessInfo()
	{
		dsDatabase db;
		db.OpenDB(L"d:\\Doktor\\SystemCalls.db");

		CSystemCalls callsLoader(&db);
		if (!callsLoader.MoveFirst()) {
			return;
		}

		CVirusFiles filesLoader(&db);
		CVirusInfo virusInfoLoader(&db);

		std::unordered_map<int, bool> okFiles;

		long nTrue(0);
		long nFalse(0);

		int nCallNumber(1);

		clock_t startTime = clock();

		while (!callsLoader.IsEOF()) {
			std::wstring sCallNumber = std::to_wstring(nCallNumber);
			std::wcout << L"Call Number: " << sCallNumber << L"\n";
			nCallNumber++;
			int nFileID = callsLoader.GetfkVirusFile();
			auto it_find_file = okFiles.find(nFileID);

			if (okFiles.end() == it_find_file) {
				if (!filesLoader.SeekById(nFileID)) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				int nVirusID = filesLoader.GetfkVirus();
				if (!virusInfoLoader.SeekById(nVirusID)) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				int nPositives = virusInfoLoader.GetPositives();
				if (nPositives < 15) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}

				ds_json::object scans;
				std::string sScans = virusInfoLoader.GetVirusScanResUTF8();
				ds_json::str2obj(sScans.c_str(), scans);

				ds_json::object kaspersky;
				scans.GetJsonObject("Kaspersky", kaspersky);

				if (kaspersky.IsNull("result")) {
					callsLoader.MoveNext();
					okFiles[nFileID] = false;
					continue;
				}
				else {
					okFiles[nFileID] = true;
				}
			}
			else {
				if (it_find_file->second == false) {
					callsLoader.MoveNext();
					continue;
				}
			}

			int nSuccess = callsLoader.GetSuccess();
			if (1 == nSuccess) {
				nTrue += 1;
			}
			else {
				nFalse += 1;
			}

			//std::wcout << std::to_wstring(nTrue) << L" true.\n";
			//std::wcout << std::to_wstring(nFalse) << L" false.\n";

			callsLoader.MoveNext();
		}

		double dTime = double(clock() - startTime) / (double)CLOCKS_PER_SEC;
		std::wstring sTime = std::to_wstring(dTime);

		std::wcout << sTime.c_str() << L" seconds.\n";


		std::wcout << std::to_wstring(nTrue) << L" true.\n";
		std::wcout << std::to_wstring(nFalse) << L" false.\n";
	}
}