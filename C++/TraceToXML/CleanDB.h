#pragma once

#include "../DBUtils/dsDatabase.h"
#include "../DBUtils/dsTable.h"
#include "../DBUtils/JsonUtils/jsonparser.h"
#include "VirusInfoLoaders.h"

#include <string>
#include "unordered_map"
#include "array"
#include "sstream"
#include <fstream>
#include <iostream>

namespace clean_DB_utils
{
	void UpdateSystemCallsCount(LPCTSTR sDBPath)
	{
		dsDatabase db;
		db.OpenDB(sDBPath);

		CSystemCalls systemCallsLoader(&db);
		CVirusFiles loaderFiles(&db);
		loaderFiles.MoveFirst();
		while (!loaderFiles.IsEOF()) {
			const int nID = loaderFiles.GetId();

			int nCount(0);
			if (systemCallsLoader.SeekByfkVirusFile(nID)) {
				while (!systemCallsLoader.IsEOF()) {
					if (nID == systemCallsLoader.GetfkVirusFile()){
						nCount++;
					}

					systemCallsLoader.MoveNext();
				}
			}

			std::wcout << nCount <<L"\n";

			loaderFiles.Edit();
			loaderFiles.SetSystemCallsCount(nCount);
			loaderFiles.Update();

			loaderFiles.MoveNext();
		}
	}

	void DeleteTMP(LPCTSTR sDBPath)
	{
		dsDatabase db;
		db.OpenDB(sDBPath);

		CSystemCalls systemCallsLoader(&db);
		CVirusFiles loaderFiles(&db);
		loaderFiles.MoveFirst();
		int nCount(0);
		while (!loaderFiles.IsEOF()) {
			const std::wstring sFileName = loaderFiles.GetName();
			const size_t nPos = sFileName.find(L".tmp.");

			if (std::wstring::npos != nPos) {
				const int nID = loaderFiles.GetId();
				nCount++;
				systemCallsLoader.Edit();
				systemCallsLoader.DeleteAllByfkVirusFile(nID);
				systemCallsLoader.Update();
			}

			loaderFiles.MoveNext();
		}

		std::wcout << nCount << L"\n";
	}

	void DeleteNotVirusShareFile(LPCTSTR sDBPath)
	{
		dsDatabase db;
		db.OpenDB(sDBPath);

		CSystemCalls systemCallsLoader(&db);
		CVirusFiles loaderFiles(&db);
		loaderFiles.MoveFirst();
		int nCount(0);
		while (!loaderFiles.IsEOF()) {
			const std::wstring sFileName = loaderFiles.GetName();
			const size_t nPos = sFileName.find(L"VirusShare");

			if (std::wstring::npos == nPos) {
				const int nID = loaderFiles.GetId();
				nCount++;
				systemCallsLoader.Edit();
				systemCallsLoader.DeleteAllByfkVirusFile(nID);
				systemCallsLoader.Update();
			}

			loaderFiles.MoveNext();
		}

		std::wcout << nCount << L"\n";
	}

	void DeleteLessThan1000(LPCTSTR sDBPath)
	{
		dsDatabase db;
		db.OpenDB(sDBPath);

		CSystemCalls systemCallsLoader(&db);
		CVirusFiles loaderFiles(&db);
		loaderFiles.MoveFirst();
		int nCount(0);
		while (!loaderFiles.IsEOF()) {
			const int nSCCount = loaderFiles.GetSystemCallsCount();

			if (1000 > nSCCount) {
				const int nID = loaderFiles.GetId();
				nCount++;
				systemCallsLoader.Edit();
				systemCallsLoader.DeleteAllByfkVirusFile(nID);
				systemCallsLoader.Update();
			}

			loaderFiles.MoveNext();
		}

		std::wcout << nCount << L"\n";
	}

	void GetKasperskyResult(LPCTSTR sDBPath)
	{
		dsDatabase db;
		db.OpenDB(sDBPath);

		CVirusInfo loaderVirus(&db);
		loaderVirus.MoveFirst();
		while (!loaderVirus.IsEOF()) {
			ds_json::object objScanRes;
			std::string sRes = loaderVirus.GetVirusScanResUTF8();
			ds_json::str2obj(sRes.c_str(), objScanRes);

			ds_json::object objKaspersky;
			objScanRes.GetJsonObject("Kaspersky", objKaspersky);

			std::wstring sDetected = objKaspersky.GetText("detected");
			if (sDetected == L"true") {
				std::wstring sResult = objKaspersky.GetText("result");
				loaderVirus.Edit();
				loaderVirus.SetKasperskyResult(sResult);
				loaderVirus.Update();

				std::wcout << sResult << L"\n";
			}

			
			loaderVirus.MoveNext();
		}
	}

	void GetSymantecResult(LPCTSTR sDBPath)
	{
		dsDatabase db;
		db.OpenDB(sDBPath);

		CVirusInfo loaderVirus(&db);
		loaderVirus.MoveFirst();
		while (!loaderVirus.IsEOF()) {
			ds_json::object objScanRes;
			std::string sRes = loaderVirus.GetVirusScanResUTF8();
			ds_json::str2obj(sRes.c_str(), objScanRes);

			ds_json::object objKaspersky;
			objScanRes.GetJsonObject("Symantec", objKaspersky);

			std::wstring sDetected = objKaspersky.GetText("detected");
			if (sDetected == L"true") {
				std::wstring sResult = objKaspersky.GetText("result");
				loaderVirus.Edit();
				int nFind = sResult.find(L".");
				sResult = sResult.substr(0, nFind);
				loaderVirus.SetSymantec(sResult);
				loaderVirus.Update();

				std::wcout << sResult << L"\n";
			}


			loaderVirus.MoveNext();
		}
	}

	void GetSystemCallsInfoFromDB(LPCTSTR sDBPath, std::unordered_map<std::wstring, int> &mapSystemCalls,
								  std::unordered_map<std::wstring, int> &mapScanResults,
							      std::vector<std::pair<std::wstring, std::vector<std::wstring>>> &mapSystemCallsBy,
								  LPCTSTR sResultField, int nSystemCallsCount, std::wstring sOverrideType, bool bEnable2InARow, int nMinMalwareCount)
	{
		dsDatabase db;
		db.OpenDB(sDBPath);
        std::wstring sType;

        std::vector<std::wstring> arrCalls;
		int nCount(0);
		std::wstring sMinus1;
		std::wstring sMinus2;
		std::wstring sMinus3;
        std::wstring sSystemCall;

		CVirusFiles loaderVirusFiles(&db);
		CSystemCalls loaderSystemCalls(&db);		

		CVirusInfo loaderVirus(&db);
		loaderVirus.MoveFirst();
        std::wstring sResult;
        int nVirusID;
        int nMax;
		int nVirusFileIdMax;
		while (!loaderVirus.IsEOF()) {
			sResult = loaderVirus.GetFieldString(sResultField);
			if (sResult.empty()) {
				loaderVirus.MoveNext();
				continue;
			}

			mapScanResults[sResult] = 0;
			nVirusID = loaderVirus.GetId();

			if (!loaderVirusFiles.SeekByfkVirus(nVirusID)) {
				loaderVirus.MoveNext();
				continue;
			}

			nMax = loaderVirusFiles.GetSystemCallsCount();
			nVirusFileIdMax = loaderVirusFiles.GetId();
			while (nVirusID == loaderVirusFiles.GetfkVirus() && !loaderVirusFiles.IsEOF()) {
				if (nMax < loaderVirusFiles.GetSystemCallsCount()) {
					nVirusFileIdMax = loaderVirusFiles.GetId();
					nMax = loaderVirusFiles.GetSystemCallsCount();
				}

				loaderVirusFiles.MoveNext();
			}

			if (!loaderSystemCalls.SeekByfkVirusFile(nVirusFileIdMax)) {
				loaderVirus.MoveNext();
				continue;
			}

            arrCalls.clear();
            nCount = 0;
			while (!loaderSystemCalls.IsEOF() && nVirusFileIdMax == loaderSystemCalls.GetfkVirusFile() && nCount < nSystemCallsCount) {
				sSystemCall = loaderSystemCalls.GetSystemCall();
                
                // To skip broken system calls
                size_t index = sSystemCall.find(L"Ã");
                if (index != std::wstring::npos) {
                    loaderSystemCalls.MoveNext();
				    continue;
                }
                index = sSystemCall.find(L"Å");
                if (index != std::wstring::npos) {
                    loaderSystemCalls.MoveNext();
				    continue;
                }
                if (bEnable2InARow) {
					sMinus3 = sMinus2;
					sMinus2 = sMinus1;
					sMinus1 = sSystemCall;
					if (sMinus3 == sMinus2 && sMinus2 == sMinus1) {
						loaderSystemCalls.MoveNext();
						continue;
					}
				}

				mapSystemCalls[sSystemCall] = 1;
				arrCalls.push_back(sSystemCall);
				loaderSystemCalls.MoveNext();
				nCount++;
			}

			if (nCount >= nSystemCallsCount) {
				sType = sOverrideType.empty() ? sResult : sOverrideType;
				mapSystemCallsBy.push_back(std::make_pair(sType, arrCalls));
			}/*
			else {
				ASSERT(FALSE);
			}*/

			loaderVirus.MoveNext();
		}
	}

	void ExportToARFF(LPCTSTR sDBPath, LPCTSTR sDBPathClean, int nSystemCallsCount, int nMinMalwareCount, LPCTSTR sResultField, LPCTSTR sSuffix)
	{
		std::wstring sDataFileName = L"c:\\DB\\data";
		sDataFileName += std::to_wstring(nSystemCallsCount);
		sDataFileName += sResultField;
		sDataFileName += sSuffix;
		sDataFileName += L".arff";
		std::wofstream jsonFile(sDataFileName.c_str());
		jsonFile << L"@RELATION SystemCalls\n";
		for (int i = 0; i < nSystemCallsCount; ++i) {
			jsonFile << L"@ATTRIBUTE " << L"SystemCall" << i << L"\tNUMERIC\n";
		}

		std::unordered_map<std::wstring, int> mapSystemCalls;
		std::unordered_map<std::wstring, int> mapScanResults;
		std::vector<std::pair<std::wstring, std::vector<std::wstring>>> mapSystemCallsBy;

		bool bEnable2InARow = false;

		GetSystemCallsInfoFromDB(sDBPath, mapSystemCalls, mapScanResults, mapSystemCallsBy, sResultField, nSystemCallsCount, L"Malware", bEnable2InARow, nMinMalwareCount);
		GetSystemCallsInfoFromDB(sDBPathClean, mapSystemCalls, mapScanResults, mapSystemCallsBy, sResultField, nSystemCallsCount, L"Clean", bEnable2InARow, nMinMalwareCount);

		int nCallNr(0);
		for (auto &it : mapSystemCalls) {
			it.second = nCallNr;
			nCallNr++;
		}

		for (auto &callsBy : mapSystemCallsBy) {
			mapScanResults[callsBy.first] += 1;
		}

		std::wstring sClass;
		for (auto &res : mapScanResults) {
			if (res.second < nMinMalwareCount) {
				continue;
			}

			sClass += res.first;
			sClass += L",";
		}

		sClass.pop_back();
		jsonFile << L"@ATTRIBUTE class {" << sClass << L"}\n";

		jsonFile << L"\n@DATA\n";

		int nCountDataRows(0);
		for (auto &it2 : mapSystemCallsBy) {
			auto find_res = mapScanResults.find(it2.first);
			if (find_res == mapScanResults.end()) {
				continue;
			}

			if (find_res->second < nMinMalwareCount) {
				continue;
			}

			for (auto &it3 : it2.second) {
				auto find = mapSystemCalls.find(it3);
				jsonFile << find->second << L",";
			}

			jsonFile << it2.first << L"\n";

			nCountDataRows++;

			/*if (nCountDataRows == 10) {
				break;
			}*/
		}

		jsonFile.close();
	}

    void SaveToFile(const std::unordered_map<std::wstring, int> &mapScanResultsFinal,
                    const std::vector<std::pair<std::wstring, std::vector<std::wstring>>> &mapSystemCallsBy,
                    const std::unordered_map<std::wstring, int> &mapSystemCalls,
                    int nSystemCallsCount, bool bExportText)
    {
        const int32_t nClassCount = mapScanResultsFinal.size();

		std::wstring sDataFileName = L"c:\\DB\\";
		sDataFileName += std::to_wstring(nSystemCallsCount);
		sDataFileName += L"_";
		sDataFileName += std::to_wstring(nClassCount);
		sDataFileName += L".csv";
		std::wofstream jsonFile(sDataFileName.c_str());

        int nCountDataRows(0);
		for (const auto &it2 : mapSystemCallsBy) {
			const auto find_res = mapScanResultsFinal.find(it2.first);
			if (find_res == mapScanResultsFinal.end()) {
				continue;
			}

			//Output system calls
            int nOutputedNumber = 0;
			for (auto &it3 : it2.second) {
				if (bExportText) {
                    jsonFile << it3 << L",";
                }
                else {
                    auto find = mapSystemCalls.find(it3);
                    jsonFile << find->second << L",";
                }
                nOutputedNumber++;
                if (nOutputedNumber == nSystemCallsCount) {
                    break;
                }
			}

			//Output class name
			jsonFile << find_res->first << L"\n";
			////Output class index
			//jsonFile << find_res->second << L"\n";

			nCountDataRows++;

			/*if (nCountDataRows == 10) {
				break;
			}*/
		}

        jsonFile.close();
    }

    void SaveMappingToFile(const std::unordered_map<std::wstring, int> &mapScanResultsFinal,
                           const std::unordered_map<std::wstring, int> &mapSystemCalls,
                           int nSystemCallsCount)
    {
        const int32_t nClassCount = mapScanResultsFinal.size();

		std::wstring sDataFileName = L"c:\\DB\\";
		sDataFileName += std::to_wstring(nSystemCallsCount);
		sDataFileName += L"_";
		sDataFileName += std::to_wstring(nClassCount);
        sDataFileName += L"_mapping";
		sDataFileName += L".csv";
		std::wofstream jsonFile(sDataFileName.c_str());

        for (auto &it : mapSystemCalls) {
			jsonFile << it.first << L"," << it.second << L"\n";
		}

        jsonFile.close();
    }

	void ExportToCSV(LPCTSTR sDBPath, LPCTSTR sDBPathClean, int nSystemCallsCount, int nMinMalwareCount,
					 LPCTSTR sResultField, bool bMax2, bool bOverrideMalware, bool bExportText)
	{
		std::unordered_map<std::wstring, int> mapSystemCalls;
		std::unordered_map<std::wstring, int> mapScanResults;
		std::vector<std::pair<std::wstring, std::vector<std::wstring>>> mapSystemCallsBy;

        std::wstring sOverrideMalware;
		if (bOverrideMalware) {
            sOverrideMalware = L"Malware";
        }
        
    	//MalwareDB
        GetSystemCallsInfoFromDB(sDBPath, mapSystemCalls, mapScanResults, mapSystemCallsBy,
							     sResultField, nSystemCallsCount, sOverrideMalware.c_str(), bMax2, nMinMalwareCount);

        //Clean DB
        if (0 != ::wcslen(sDBPathClean)) {
		    const std::wstring sOverrideClean   = L"Clean";
            GetSystemCallsInfoFromDB(sDBPathClean, mapSystemCalls, mapScanResults, mapSystemCallsBy,
			                         sResultField, nSystemCallsCount, sOverrideClean.c_str(), bMax2, nMinMalwareCount);
        }

		int nCallNr(0);
		for (auto &it : mapSystemCalls) {
			it.second = nCallNr;
			nCallNr++;
		}

		// Map string to int
        for (auto &callsBy : mapSystemCallsBy) {
			mapScanResults[callsBy.first] += 1;
		}

		int nIndex(0);
		std::unordered_map<std::wstring, int> mapScanResultsFinal;
		for (auto const & [sScanRes, nCount] : mapScanResults) {
			if (nCount < nMinMalwareCount) {
				continue;
			}

			mapScanResultsFinal[sScanRes] = nIndex;
			nIndex++;
		}

        SaveToFile(mapScanResultsFinal, mapSystemCallsBy, mapSystemCalls, nSystemCallsCount, bExportText);
        SaveMappingToFile(mapScanResultsFinal, mapSystemCalls, nSystemCallsCount);
	}

    
    void ExportToCSVGlobalImpl(int nSystemCallsCount, int nMinMalwareCount, bool bExportText,
                               const std::unordered_map<std::wstring, int> &mapSystemCalls,
		                       const std::unordered_map<std::wstring, int> &mapScanResults,
		                       const std::vector<std::pair<std::wstring, std::vector<std::wstring>>> &mapSystemCallsBy)
	{
		int nIndex(0);
		std::unordered_map<std::wstring, int> mapScanResultsFinal;
		for (auto const & [sScanRes, nCount] : mapScanResults) {
			if (nCount < nMinMalwareCount) {
				continue;
			}

			mapScanResultsFinal[sScanRes] = nIndex;
			nIndex++;
		}

        SaveToFile(mapScanResultsFinal, mapSystemCallsBy, mapSystemCalls, nSystemCallsCount, bExportText);
        SaveMappingToFile(mapScanResultsFinal, mapSystemCalls, nSystemCallsCount);
	}

    // Maps all possible system calls from 2000 system calls. Then filters fy required number of system calls
    void ExportToCSVGlobal(LPCTSTR sDBPath, LPCTSTR sDBPathClean, int nMinMalwareCount,
					       LPCTSTR sResultField, bool bMax2, bool bOverrideMalware, bool bExportText)
	{
		std::unordered_map<std::wstring, int> mapSystemCalls;
		std::unordered_map<std::wstring, int> mapScanResults;
		std::vector<std::pair<std::wstring, std::vector<std::wstring>>> mapSystemCallsBy;

        std::wstring sOverrideMalware;
		if (bOverrideMalware) {
            sOverrideMalware = L"Malware";
        }

        const int nGlobalSystemCountValue = 1000;
        
    	//MalwareDB
        GetSystemCallsInfoFromDB(sDBPath, mapSystemCalls, mapScanResults, mapSystemCallsBy,
							     sResultField, nGlobalSystemCountValue, sOverrideMalware.c_str(), bMax2, nMinMalwareCount);

        //Clean DB
        if (0 != ::wcslen(sDBPathClean)) {
		    const std::wstring sOverrideClean   = L"Clean";
            GetSystemCallsInfoFromDB(sDBPathClean, mapSystemCalls, mapScanResults, mapSystemCallsBy,
			                         sResultField, nGlobalSystemCountValue, sOverrideClean.c_str(), bMax2, nMinMalwareCount);
        }

		int nCallNr(0);
		for (auto &it : mapSystemCalls) {
			it.second = nCallNr;
			nCallNr++;
		}

		// Map string to int
        for (auto &callsBy : mapSystemCallsBy) {
			mapScanResults[callsBy.first] += 1;
		}

        ExportToCSVGlobalImpl(10, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(20, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(40, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(60, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(80, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(100, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(200, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(400, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(600, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(800, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
        ExportToCSVGlobalImpl(1000, nMinMalwareCount, bExportText, mapSystemCalls, mapScanResults, mapSystemCallsBy);
	}

	void CopySystemCalls(const wchar_t* sSrc, const wchar_t* sTrg)
	{
		dsDatabase dbSrc;
		dbSrc.OpenDB(sSrc);

		dsDatabase dbTrg;
		dbTrg.OpenDB(sTrg);

		CSystemCalls loaderSystemCallsTrg(&dbTrg);
		dbTrg.BeginTrans();
			loaderSystemCallsTrg.Flush();
		dbTrg.CommitTrans();

		CSystemCalls loaderSystemCallsSrc(&dbSrc);

		CVirusFiles loaderVirusFilesTrg(&dbTrg);
		loaderVirusFilesTrg.MoveFirst();
		dbTrg.BeginTrans();

		while (!loaderVirusFilesTrg.IsEOF()) {
			const int nVirusFileID = loaderVirusFilesTrg.GetId();
			if (loaderSystemCallsSrc.SeekByfkVirusFile(nVirusFileID)) {
				while (!loaderSystemCallsSrc.IsEOF()) {
					const int nVirusFileIDRef = loaderSystemCallsSrc.GetfkVirusFile();
					if (nVirusFileIDRef == nVirusFileID) {
						loaderSystemCallsTrg.AddNew();

						loaderSystemCallsTrg.SetId(loaderSystemCallsSrc.GetId());
						loaderSystemCallsTrg.SetfkVirusFile(loaderSystemCallsSrc.GetfkVirusFile());
						loaderSystemCallsTrg.SetCallNumber(loaderSystemCallsSrc.GetCallNumber());
						loaderSystemCallsTrg.SetSystemCall(loaderSystemCallsSrc.GetSystemCall());
						
						ds_json::object objArgs;
						loaderSystemCallsSrc.GetArguments(objArgs);
						loaderSystemCallsTrg.SetArguments(objArgs);

						ds_json::object objRetArgs;
						loaderSystemCallsSrc.GetRetArguments(objRetArgs);
						loaderSystemCallsTrg.SetRetArguments(objRetArgs);
						
						loaderSystemCallsTrg.SetReturn(loaderSystemCallsSrc.GetReturn());
						loaderSystemCallsTrg.SetSuccess(loaderSystemCallsSrc.GetSuccess());

						VERIFY(loaderSystemCallsTrg.Update());
					}

					loaderSystemCallsSrc.MoveNext();
				}
			}

			loaderVirusFilesTrg.MoveNext();
		}

		dbTrg.CommitTrans();
	}
}
