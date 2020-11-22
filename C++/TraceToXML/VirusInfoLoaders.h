#pragma once

#include "../DBUtils/dsDatabase.h"
#include "../DBUtils/dsTable.h"
#include "../DBUtils/JsonUtils/jsonparser.h"

class CVirusInfo : public dsTable
{
	// Construction/Destruction
public:
	CVirusInfo(dsDatabase *pDatabase)
		: dsTable(pDatabase, L"VirusInfo") { }
	virtual ~CVirusInfo() { }

public:
	KEY_LONG(Id, "ID");
	KEY_TEXT(Name, "Name");
	FIELD_TEXT(VirusScanRes, "VirusScanRes");
	FIELD_LONG(VirusShare, "VirusShare");
	FIELD_LONG(TotalScans, "TotalScans");
	FIELD_LONG(Positives, "Positives");
	FIELD_TEXT(URL, "URL");
	FIELD_TEXT(Behaviour, "Behaviour");
	FIELD_TEXT(KasperskyResult, "KasperskyResult");
	FIELD_TEXT(Symantec, "Symantec");
};

class CVirusFiles : public dsTable
{
	// Construction/Destruction
public:
	CVirusFiles(dsDatabase *pDatabase)
		: dsTable(pDatabase, L"VirusFiles") { }
	virtual ~CVirusFiles() { }

public:
	KEY_LONG(Id, "ID");
	KEY_LONG(fkVirus, "fkVirus")
	KEY_TEXT(Name, "FileName");
	FIELD_LONG(SystemCallsCount, "SystemCallsCount")
};

class CSystemCalls : public dsTable
{
	// Construction/Destruction
public:
	CSystemCalls(dsDatabase *pDatabase)
		: dsTable(pDatabase, L"SystemCalls") { }
	virtual ~CSystemCalls() { }

public:
	FIELD_LONG(Id, "ID");
	KEY_LONG(fkVirusFile, "fkVirusFile");
	FIELD_LONG(CallNumber, "CallNumber");
	FIELD_TEXT(SystemCall, "SystemCall");
	FIELD_JSON(Arguments, "Arguments");
	FIELD_JSON(RetArguments, "RetArguments");
	FIELD_TEXT(Return, "Return");
	FIELD_LONG(Success, "Success");
};