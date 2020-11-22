#pragma once

#include "unordered_map"

class CTraceItem
{
public:
	void Clear() {
		m_sSystemCallName.clear();
		m_mapArguments.clear();
		m_bSucceeded = false;
		m_sRetValue.clear();
	}

public:
	std::wstring m_sSystemCallName;
	std::unordered_map<std::wstring, std::pair<std::wstring, std::wstring>> m_mapArguments;
	bool m_bSucceeded;
	std::wstring m_sRetValue;
};
