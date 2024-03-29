#include "stdafx.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "TraceItem.h"
#include "VirusInfoLoaders.h"
#include "GetInfoFromDatabase.h"
#include "HTTPRequest.h"
#include "CleanDB.h"

#include "sstream"
#include "iomanip"
#include "limits"
#include "algorithm"
#include "functional"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <filesystem>

void WriteJson(std::wofstream &jsonFile, int nVirussShare, LPCTSTR sVirusName, const std::unordered_map<std::wstring, std::vector<CTraceItem>> &items)
{
	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF16<> > writer(strbuf);
    writer.StartObject();
    writer.String(L"VirusName");
    writer.String(sVirusName);
    writer.String(L"VirusShare");
    writer.Int(nVirussShare);

    writer.String(L"VirusFiles");
    writer.StartArray();
    for (auto fileNode : items) {
        writer.StartObject();
        writer.String(L"FileName");
        writer.String(fileNode.first.c_str());
        
        writer.String(L"SystemCalls");
        writer.StartArray();
            for (auto callNode : fileNode.second) {
                writer.StartObject();
                writer.String(L"SystemCall");
                writer.String(callNode.m_sSystemCallName.c_str());
    
                writer.String(L"Arguments");
                writer.StartArray();
                for (auto it : callNode.m_mapArguments) {
                    writer.String(it.second.first.c_str());
                }
                writer.EndArray();

                writer.String(L"RetArguments");
                writer.StartArray();
                for (auto it : callNode.m_mapArguments) {
                    writer.String(it.second.second.c_str());
                }
                writer.EndArray();

                writer.String(L"Success");
                writer.Int(callNode.m_bSucceeded ? 1 : 0);

                writer.String(L"Return");
                writer.String(callNode.m_sRetValue.c_str());

                writer.EndObject();
            }
        writer.EndArray();
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    jsonFile << strbuf.GetString();
}

void ExportToJson(LPCTSTR sPathToLogs, LPCTSTR sPathToJSON)
{
    int nFileNumber(1);
    for(auto& p: std::experimental::filesystem::directory_iterator(sPathToLogs)) {
        std::unordered_map<std::wstring, std::vector<CTraceItem>> items;
        std::experimental::filesystem::path p_json = p.path();
        //p_json.(L".json");
        std::wstring sStem = p_json.stem();
        std::wstring sFileNameJson = std::wstring(sPathToJSON)+sStem+L".json";
        std::wofstream jsonFile(sFileNameJson.c_str());
        //std::wofstream jsonFile(sFileNameJson);
        std::experimental::filesystem::path pDir = p.path();
        std::wstring sRoot = pDir.stem();

        int nPos = sRoot.find(L"VirusShare_");
        bool bVirusShare(false);
        if (-1 != nPos)
        {
            bVirusShare = true;
            sRoot.erase(0, 11);
            nPos = sRoot.find(L".");
            if (-1 != nPos) {
                int nlen = sRoot.size() - nPos;
                sRoot.erase(nPos, nlen);
            }
        }

        for(auto& p1: std::experimental::filesystem::directory_iterator(p)) {
            if (p1.path().extension() == L".log") {
                std::experimental::filesystem::path pLogFile = p1.path();

                std::wstring sFile = pLogFile;
                std::wstring sFile_ = pLogFile.stem();
                std::wstring sFileNameFullLog = p1.path();
                std::wifstream infile(sFileNameFullLog.c_str());

                std::vector<CTraceItem> items_array;

                std::wstring sNumber = std::to_wstring(nFileNumber);
                std::wcout << L"File number: " << sNumber << L"\n";
                nFileNumber++;

                bool bArg(false);
                bool bResult(false);
                bool bRetVal(false);
                bool bNewItem(true);

                bool bFirst(true);
                CTraceItem item;
                std::wstring line;
                int nCallNumber(1);
                while (std::getline(infile, line)) {
                    if(line.empty()) {
                        continue;
                    }

                    bArg = false;
                    bNewItem = false;

                    if (line[0] == '\t') {
                        if (line[1] == 'a') {
                            bArg = true;
                        }
                        else {//'result'
                            bRetVal = true;
                        }
                    }
                    else if (line[0] == ' ') {
                        bResult = true;
                        item.m_bSucceeded = line.find(L"succeeded");
                    }
                    else {
                        bNewItem = true;
                    }

                    if (bNewItem){
                        if (!bFirst) {
                            items_array.push_back(item);
                            nCallNumber++;
                            if (nCallNumber > 10000) {//only first 10000 traces are recorded
                                break;
                            }
                            item.Clear();
                            bArg = false;
                            bResult = false;
                            bRetVal = false;
                        }
                        bFirst = false;
                        item.m_sSystemCallName = line;
                    }

                    if (bArg && !bResult){
                        std::size_t pos = line.find(L":");
                        std::wstring sArgNr = line.substr(1, pos-1);
                        pos += 2;
                        std::wstring str = line.substr(pos);
                        //item.m_args.push_back(str);
                        item.m_mapArguments[sArgNr] = std::make_pair(str, L"");
                    }

                    if (bArg && bResult){
                        std::size_t pos = line.find(L":");
                        std::wstring sArgNr = line.substr(1, pos-1);
                        pos += 2;
                        std::wstring str = line.substr(pos);
                        //item.m_argsRet.push_back(str);
                        auto find = item.m_mapArguments.find(sArgNr);
                        if (find != item.m_mapArguments.end()) {
                            find->second.second = str;
                        }
                    }

                    if (!bArg && bRetVal){
                        std::size_t pos = line.find(L":");
                        pos += 2;
                        std::wstring str = line.substr(pos);
                        item.m_sRetValue = str;
                    }   
                }

                items[sFile_] = items_array;
            }
        }

        WriteJson(jsonFile, bVirusShare ? 1 : 0, sRoot.c_str(), items);
    }
}

void WriteJson(int nVirusFileID, CSystemCalls &loaderCalls, const CTraceItem &item, int nCallNumber)
{
    loaderCalls.AddNew();

    loaderCalls.SetfkVirusFile(nVirusFileID);
    loaderCalls.SetSystemCall(item.m_sSystemCallName.c_str());
    loaderCalls.SetCallNumber(nCallNumber);

    ds_json::array args;
    for (auto it : item.m_mapArguments) {
        args.AddString(it.second.first.c_str());
    }
    loaderCalls.SetArguments(args);

    ds_json::array argsRet;
    for (auto it : item.m_mapArguments) {
        argsRet.AddString(it.second.second.c_str());
    }
    loaderCalls.SetRetArguments(argsRet);


    loaderCalls.SetSuccess(item.m_bSucceeded ? 1 : 0);
    loaderCalls.SetReturn(item.m_sRetValue.c_str());

    loaderCalls.Update();
}

void ConvertToDB(int nVirusFileID, CSystemCalls &loaderCalls, LPCTSTR sLogFile)
{
    std::wifstream infile(sLogFile);

    bool bArg(false);
    bool bResult(false);
    bool bRetVal(false);
    bool bNewItem(true);

    bool bFirst(true);
    CTraceItem item;
    std::wstring line;
    int nCallNumber(1);
    while (std::getline(infile, line)) {
        if(line.empty()) {
            continue;
        }

        bArg = false;
        bNewItem = false;

        if (line[0] == '\t') {
            if (line[1] == 'a') {
                bArg = true;
            }
            else {//'result'
                bRetVal = true;
            }
        }
        else if (line[0] == ' ') {
            bResult = true;
            item.m_bSucceeded = line.find(L"succeeded");
        }
        else {
            bNewItem = true;
        }

        if (bNewItem){
            if (!bFirst) {
                WriteJson(nVirusFileID, loaderCalls, item, nCallNumber);
                nCallNumber++;
                if (nCallNumber > 10000) {//only first 10000 traces are recorded
                    break;
                }
                item.Clear();
                bArg = false;
                bResult = false;
                bRetVal = false;
            }
            bFirst = false;
            item.m_sSystemCallName = line;
        }

        if (bArg && !bResult){
            std::size_t pos = line.find(L":");
            std::wstring sArgNr = line.substr(1, pos-1);
            pos += 2;
            std::wstring str = line.substr(pos);
            //item.m_args.push_back(str);
            item.m_mapArguments[sArgNr] = std::make_pair(str, L"");
        }

        if (bArg && bResult){
            std::size_t pos = line.find(L":");
            std::wstring sArgNr = line.substr(1, pos-1);
            pos += 2;
            std::wstring str = line.substr(pos);
            //item.m_argsRet.push_back(str);
            auto find = item.m_mapArguments.find(sArgNr);
            if (find != item.m_mapArguments.end()) {
                find->second.second = str;
            }
        }

        if (!bArg && bRetVal){
            std::size_t pos = line.find(L":");
            pos += 2;
            std::wstring str = line.substr(pos);
            item.m_sRetValue = str;
        }   
    }
}

void ExportToSQLITE(LPCTSTR sDBPath, LPCTSTR sLogPath)
{
    dsDatabase db;
    db.OpenDB(sDBPath);

    CSystemCalls loaderCalls(&db);
    loaderCalls.Flush();

    CVirusFiles filesLoader(&db);
    filesLoader.Flush();

    CVirusInfo infoLoader(&db);
    infoLoader.Flush();

    int nFileNumber(1);

    for(auto& p: std::experimental::filesystem::directory_iterator(sLogPath)) {
        if (std::experimental::filesystem::is_directory(p)) {
            std::experimental::filesystem::path pDir = p.path();
            std::wstring sRoot = pDir.stem();

            int nPos = sRoot.find(L"VirusShare_");
            bool bVirusShare(false);
            if (-1 != nPos)
            {
                bVirusShare = true;
                sRoot.erase(0, 11);
                nPos = sRoot.find(L".");
                if (-1 != nPos) {
                    int nlen = sRoot.size() - nPos;
                    sRoot.erase(nPos, nlen);
                }
            }

            //if(virusInfoLoaderSrc.SeekByName(sRoot)) {
            //    if(virusInfoLoaderSrc.GetPositives() < 15) {
            //        continue;
            //    }
            //}

            db.BeginTrans();

            infoLoader.AddNew();
            infoLoader.SetName(sRoot);
            infoLoader.SetVirusScanRes(L"");
            infoLoader.SetVirusShare(0);
            infoLoader.SetTotalScans(0);
            infoLoader.SetPositives(0);
            infoLoader.SetURL(L"");
            infoLoader.SetBehaviour(L"");
            infoLoader.Update();

            const int nVirusID = infoLoader.GetId();

            for(auto& p1: std::experimental::filesystem::directory_iterator(p)) {
                if (!std::experimental::filesystem::is_directory(p1)) {
                    std::experimental::filesystem::path pLogFile = p1.path();
                    std::wstring sExt = pLogFile.extension();
                    if (sExt == L".json") {
                        continue;
                    }

                    std::wstring sFile = pLogFile;
                    std::wstring sFile_ = pLogFile.stem();

                    filesLoader.AddNew();
                    filesLoader.SetName(sFile_);
                    filesLoader.SetfkVirus(nVirusID);
                    filesLoader.Update();

                    std::wstring sNumber = std::to_wstring(nFileNumber);
                    std::wcout << L"File number: " << sNumber << L"\n";
                    nFileNumber++;

                    int nFileID = filesLoader.GetId();
                    ConvertToDB(nFileID, loaderCalls, sFile.c_str());
                }
            }

            db.CommitTrans();
        }
    }
}

std::wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

#include <chrono>
#include <thread>

namespace fs = std::experimental::filesystem;

void getTime(std::string &sTime)
{
	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %I:%M:%S", &timeinfo);
	sTime = buffer;
}

void GetZeroDetectionFiles(const wchar_t *sSrc, const wchar_t *sTrg)
{
	int nCount(0);

	CURL *curl = curl_easy_init();
	if (!curl) {
		return;
	}

	for (const auto& p : fs::directory_iterator(sSrc)) {
		fs::path pDir = p.path();
		std::wstring sFile = pDir.stem();

		std::string sTime;
		getTime(sTime);

		int nPos = sFile.find(L"VirusShare_");
		if (-1 != nPos) {
			sFile.erase(0, 11);
		}		

		ds_json::object data_json;
		std::string sRet;
		const bool bRet = http_request_utils::GetVirusInfo(curl, ws2s(sFile.c_str()).c_str(), data_json, sRet);
		if (!bRet) {
			std::wcout << s2ws(sTime) << L" : CURL failed. STOP!\n\r";
			std::cout << sRet;
			break;
		}

		const int nResponse = data_json.GetInt32("response_code");

		if (nResponse != 1) {
			std::wcout << s2ws(sTime) << L" : Response is " << nResponse << L".\n\r";
			std::cout << sRet;
			continue;
		}

		const bool bNullTotal = data_json.IsNull("total");
		const bool bNullPositives = data_json.IsNull("positives");

		if (bNullTotal || bNullPositives) {
			std::wcout << s2ws(sTime) << L" : NULL values. STOP!\n\r";
			return;
		}

		const int nTotal = data_json.GetInt32("total");
		const int nPositives = data_json.GetInt32("positives");

		if (0 != nTotal && 0 == nPositives) {
			nCount++;
			std::wstring sNewCleanPath = sTrg;
			sNewCleanPath += L"\\";
			sNewCleanPath += pDir.stem();
			if (!fs::exists(sNewCleanPath.c_str())) {
				fs::copy(pDir.c_str(), sNewCleanPath.c_str());
			}
		}

		fs::remove(pDir);

		std::wcout << s2ws(sTime) << L" " << sFile << L" : \t Clean count: " << nCount << L"\n\r";

		//std::wcout << "Sleep for 1 seconds.\r\n";
		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	curl_easy_cleanup(curl);
}

bool DoesFileIsExecutable(const wchar_t *sFileName)
{
	HANDLE hFile = CreateFile(sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//std::cout << "\n CreateFile failed \n";
		return true;
	}

	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

	if (hFileMapping == 0)
	{
		//std::cout << "\n CreateFileMapping failed \n";
		CloseHandle(hFile);
		return true;
	}

	LPVOID lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	if (lpFileBase == 0)
	{
		//std::cout << "\n MapViewOfFile failed \n";
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return true;
	}

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpFileBase;

	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE && pDosHeader->e_magic != IMAGE_OS2_SIGNATURE && pDosHeader->e_magic != IMAGE_OS2_SIGNATURE_LE
		&& pDosHeader->e_magic != IMAGE_VXD_SIGNATURE && pDosHeader->e_magic != IMAGE_NT_SIGNATURE)
	{
		UnmapViewOfFile(lpFileBase);
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return false;
	}

	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + (DWORD)pDosHeader->e_lfanew);
	if (0 != IsBadReadPtr(pNTHeader, 0)) {
		UnmapViewOfFile(lpFileBase);
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return false;
	}

	bool bRet = false;
	try
	{
		//if ((pNTHeader->FileHeader.Characteristics & IMAGE_FILE_DLL))
		//	return false;
		if ((pNTHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
			bRet = true;
		}
	}
	catch (...)
	{
		bRet = false;
	}

	UnmapViewOfFile(lpFileBase);
	CloseHandle(hFileMapping);
	CloseHandle(hFile);

	return bRet;
}

void MoveExecutables(const wchar_t *sSrc, const wchar_t *sTrg)
{
	int nCount(0);
	for (const auto& p : fs::recursive_directory_iterator(sSrc)) {
		if (fs::is_directory(p)) {
			continue;
		}
		fs::path pDir = p.path();
		std::wstring sPath = pDir;
		if (DoesFileIsExecutable(sPath.c_str())) {
			nCount++;
			const std::wstring sFile = pDir.stem();

			std::wcout << pDir << L" | " << L"Count: " << nCount <<L"\n\r";
			const std::wstring sFullPath = pDir.c_str();
			const std::wstring sNewPath = sTrg + sFile;
			if (!fs::exists(sNewPath)) {
				fs::copy(sFullPath.c_str(), sNewPath.c_str());
			}
		}		
	}
}

void DeleteNonExecutables(const wchar_t *sSrc)
{
	int nCount(0);
	for (const auto& p : fs::recursive_directory_iterator(sSrc)) {
		if (fs::is_directory(p)) {
			continue;
		}
		const fs::path pDir = p.path();
		const std::wstring sPath = pDir;
		const std::wstring sFile = pDir.stem();

		int nPos = sFile.find(L"VirusShare_");
		if (-1 == nPos) {
			std::wcout << L"OK !VirusShare | " << sFile << L" | " << L" | Deleted count: " << nCount << L"\n\r";
			continue;
		}

		if (!DoesFileIsExecutable(sPath.c_str())) {
			nCount++;
			std::wcout << sFile << L" | " << L" | Deleted count: " << nCount << L"\n\r";
			fs::remove(pDir);
		}
		else {
			std::wcout << L"OK | " << sFile << L" | " << L" | Deleted count: " << nCount << L"\n\r";
		}
	}
}

void GetVirusInfo()
{
    dsDatabase db;
    db.OpenDB(L"d:\\Doktor\\SystemCalls.db");

    CVirusInfo infoLoader(&db);

    int nFileNumber(1);
    for(auto& p: std::experimental::filesystem::directory_iterator("y:\\LOG_VGTU")) {
        if (nFileNumber <= 8000) {
            std::wcout << L"Less than 8000. SKIPPING.\n";
            nFileNumber++;
            continue;
        }

        
        std::unordered_map<std::wstring, std::vector<CTraceItem>> items;
        std::experimental::filesystem::path p_json = p.path();
        std::wstring sStem = p_json.stem();
        std::wstring sFileNameJson = std::wstring(L"y:\\JSON\\")+sStem+L".json";
        std::wofstream jsonFile(sFileNameJson.c_str());

        std::experimental::filesystem::path pDir = p.path();
        std::wstring sRoot = pDir.stem();

        int nPos = sRoot.find(L"VirusShare_");
        bool bVirusShare(false);
        if (-1 != nPos)
        {
            bVirusShare = true;
            sRoot.erase(0, 11);
            nPos = sRoot.find(L".");
            if (-1 != nPos) {
                int nlen = sRoot.size() - nPos;
                sRoot.erase(nPos, nlen);
                ds_json::object data_json;
				std::string sRet;
//                HTTPReq(ws2s(sRoot.c_str()).c_str(), data_json, sRet);
      
                ds_json::object scans;
                data_json.GetJsonObject("scans", scans);
                std::string sScans;
                ds_json::obj2str(scans, sScans);

                const int nTotal = data_json.GetInt32("total");
                const int nPositives = data_json.GetInt32("positives");
                const std::wstring sURL = data_json.GetText("permalink");

                ds_json::object addInfo;
                data_json.GetJsonObject("additional_info", addInfo);

                ds_json::object behave;
                addInfo.GetJsonObject("behaviour-v1", behave);
                std::string sBehaviour;
                ds_json::obj2str(behave, sBehaviour);

                if (!sScans.empty()) {
                    if (infoLoader.SeekByName(sRoot.c_str())) {
                        db.BeginTrans();
                        infoLoader.Edit();
                        infoLoader.SetVirusScanResUTF8(sScans.c_str());
                        infoLoader.SetTotalScans(nTotal);
                        infoLoader.SetPositives(nPositives);
                        infoLoader.SetURL(sURL.c_str());
                        infoLoader.SetBehaviourUTF8(sBehaviour.c_str());
                        infoLoader.Update();
                        db.CommitTrans();
                    }
                    
                }
                else {
                    std::wcout << "************************No info or limit reached.\r\n";
                    return;
                }

                std::wcout << "Sleep for 4 seconds.\r\n";
                std::this_thread::sleep_for(std::chrono::seconds(4));
            }
        }

        std::wstring sNumber = std::to_wstring(nFileNumber);
        std::wcout << L"File number: " << sNumber << L"\n";

        /*if (8000 == nFileNumber) {
            std::wcout << L"8000 limit reached. STOP.\n";
            return;
        }*/
        nFileNumber++;
    }
}

void UpdateVirusInfo()
{
    dsDatabase db;
    db.OpenDB(L"c:\\DB\\SystemCallsClean.db");

    CVirusInfo infoLoader(&db);
    if (!infoLoader.MoveFirst()) {
        return;
    }

	CURL *curl = curl_easy_init();
	if (!curl) {
		return;
	}

	const std::string sExe = ".exe";
    while (!infoLoader.IsEOF()) {
        std::string sVirusHash = infoLoader.GetNameUTF8();
		std::string::size_type i = sVirusHash.find(sExe);

		if (i != std::string::npos)
			sVirusHash.erase(i, sExe.length());

		std::wcout << "Working on: " << sVirusHash.c_str() << ".\r\n";

        if (infoLoader.IsNullURL()) {
            //std::wcout << "Sleep for 1 seconds.\r\n";
            //std::this_thread::sleep_for(std::chrono::seconds(1));

            ds_json::object data_json;
			std::string sRet;
			http_request_utils::GetVirusInfo(curl, sVirusHash.c_str(), data_json, sRet);

            ds_json::object scans;
            data_json.GetJsonObject("scans", scans);
            std::string sScans;
            ds_json::obj2str(scans, sScans);

            const int nTotal = data_json.GetInt32("total");
            const int nPositives = data_json.GetInt32("positives");
            const std::wstring sURL = data_json.GetText("permalink");

            ds_json::object addInfo;
            data_json.GetJsonObject("additional_info", addInfo);

            ds_json::object behave;
            addInfo.GetJsonObject("behaviour-v1", behave);
            std::string sBehaviour;
            ds_json::obj2str(behave, sBehaviour);

            if (!sScans.empty()) {
                db.BeginTrans();
                infoLoader.Edit();
                infoLoader.SetVirusScanResUTF8(sScans.c_str());
                infoLoader.SetTotalScans(nTotal);
                infoLoader.SetPositives(nPositives);
                infoLoader.SetURL(sURL.c_str());
                infoLoader.SetBehaviourUTF8(sBehaviour.c_str());
                infoLoader.Update();
                db.CommitTrans();                    
            }
            else {
                std::wcout << "************************No info or limit reached.\r\n";
            }
        }

        infoLoader.MoveNext();
    }

	curl_easy_cleanup(curl);
}

void TransferData()
{
    dsDatabase dbSrc;
    dbSrc.OpenDB(L"d:\\Doktor\\SystemCalls.db");

    CSystemCalls callsLoaderSrc(&dbSrc);
    CVirusFiles filesLoaderSrc(&dbSrc);
    CVirusInfo virusInfoLoaderSrc(&dbSrc);

    dsDatabase dbTrg;
    dbTrg.OpenDB(L"d:\\Doktor\\SystemCalls2.db");

    CSystemCalls callsLoaderTrg(&dbTrg);
    CVirusFiles filesLoaderTrg(&dbTrg);
    CVirusInfo virusInfoLoaderTrg(&dbTrg);
    dbTrg.BeginTrans();

    int nCallNumber(0);
    virusInfoLoaderSrc.MoveFirst();
    while(!virusInfoLoaderSrc.IsEOF()) {
        nCallNumber++;
        if (virusInfoLoaderSrc.GetPositives() < 15) {
            virusInfoLoaderSrc.MoveNext();
            continue;
        }

        std::wstring sCallNumber = std::to_wstring(nCallNumber);
        std::wcout << L"Firus Number: " << sCallNumber << L"\n";

        virusInfoLoaderTrg.AddNew();
        int nIDTrg = virusInfoLoaderTrg.GetId();
        virusInfoLoaderTrg.SetName(virusInfoLoaderSrc.GetName());
        virusInfoLoaderTrg.SetVirusScanRes(virusInfoLoaderSrc.GetVirusScanRes());
        virusInfoLoaderTrg.SetVirusShare(virusInfoLoaderSrc.GetVirusShare());
        virusInfoLoaderTrg.SetTotalScans(virusInfoLoaderSrc.GetTotalScans());
        virusInfoLoaderTrg.SetPositives(virusInfoLoaderSrc.GetPositives());
        virusInfoLoaderTrg.SetURL(virusInfoLoaderSrc.GetURL());
        virusInfoLoaderTrg.SetBehaviour(virusInfoLoaderSrc.GetBehaviour());
        virusInfoLoaderTrg.Update();

        const int nIDSrc = virusInfoLoaderSrc.GetId();
        if (filesLoaderSrc.SeekByfkVirus(nIDSrc)) {
            while (filesLoaderSrc.GetfkVirus() == nIDSrc) {
                filesLoaderTrg.AddNew();
                int nFileTrgID = filesLoaderTrg.GetId();
                filesLoaderTrg.SetfkVirus(nIDTrg);
	            filesLoaderTrg.SetName(filesLoaderSrc.GetName());
                filesLoaderTrg.Update();

                const int nIDFileSrc = filesLoaderSrc.GetId();
                if (callsLoaderSrc.SeekByfkVirusFile(nIDFileSrc)) {
                    while (callsLoaderSrc.GetfkVirusFile() == nIDFileSrc) {
                        callsLoaderTrg.AddNew();

                  	    callsLoaderTrg.SetfkVirusFile(nFileTrgID);
                        callsLoaderTrg.SetCallNumber(callsLoaderSrc.GetCallNumber());
	                    callsLoaderTrg.SetSystemCall(callsLoaderSrc.GetSystemCall());
                        ds_json::object args;
                        callsLoaderSrc.GetArguments(args);
                        callsLoaderTrg.SetArguments(args);
                        ds_json::object retargs;
                        callsLoaderSrc.GetRetArguments(retargs);
                        callsLoaderTrg.SetRetArguments(retargs);
                        callsLoaderTrg.SetReturn(callsLoaderSrc.GetReturn());
                        callsLoaderTrg.SetSuccess(callsLoaderSrc.GetSuccess());
                        callsLoaderTrg.Update();

                        callsLoaderSrc.MoveNext();
                    }
                }

                filesLoaderSrc.MoveNext();
            }
        }

        virusInfoLoaderSrc.MoveNext();
    }

    dbTrg.CommitTrans();
}

int main()
{
    const std::wstring sMalwareDB = L"C:\\DB\\SystemCalls1000AndMore.db";
    std::wstring sCleanDB = L"";
    const std::wstring sEngine = L"KasperskyResult";
	//clean_DB_utils::CopySystemCalls(L"C:\\DB\\SystemCalls.db", L"C:\\DB\\SystemCalls1000AndMore.db");
	//clean_DB_utils::UpdateSystemCallsCount(L"c:\\DB\\SystemCallsClean.db");
	//ExportToSQLITE(L"c:\\DB\\SystemCallsClean.db", L"c:\\DB\\log");
	//const int nSystemCallsCount = 100;
	
	const int nMinMalwareCount = 100;
	const bool bMax2 = true;
    const bool bOverrideMalware = true;
    const bool bUseClean = true;
    const bool bExportText = false;

    if (bUseClean) {
        sCleanDB = L"C:\\DB\\SystemCallsClean.db";
    }

    clean_DB_utils::ExportToCSVGlobal(sMalwareDB.c_str(), sCleanDB.c_str(), nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);

	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 10, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 20, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 40, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 60, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 80, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 100, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 200, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 400, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 600, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 800, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1000, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware, bExportText);
	
	
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1100, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1200, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1300, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1400, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1500, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1600, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1700, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1800, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
 //   clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 1900, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2000, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2100, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2200, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2300, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2400, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2500, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2600, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2700, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2800, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 2900, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);
	//clean_DB_utils::ExportToCSV(sMalwareDB.c_str(), sCleanDB.c_str(), 3000, nMinMalwareCount, sEngine.c_str(), bMax2, bOverrideMalware);

	//clean_DB_utils::GetSymantecResult(L"C:\\DB\\SystemCalls1000.db");
	//clean_DB_utils::GetKasperskyResult(L"C:\\DB\\SystemCalls1000.db");
	//MoveExecutables(L"Z:\\down\\VirusShare_00284", L"W:\\EXE\\");
	//GetZeroDetectionFiles(L"W:\\EXE\\", L"W:\\!VIRUS_NEW\\EXE_CLEAN");
    ////TransferData();
    //std::vector<int> arrArgAloved;
    //arrArgAloved.push_back(0);
    //arrArgAloved.push_back(2);
    //arrArgAloved.push_back(3);
    //arrArgAloved.push_back(5);
    //arrArgAloved.push_back(6);
    //arrArgAloved.push_back(7);
    //arrArgAloved.push_back(8);

    //GetParamsInfo(L"NtCreateFile", arrArgAloved);
    //GetArgsInfo();
    //GetCallsInfo();
    //GetSuccessInfo();
    //UpdateVirusInfo();
    //GetVirusInfo();
    //ExportToSQLITE();
    //ExportToJson(L"c:\\DB\\log", L"c:\\DB\\JSON");

    return 0;
}

