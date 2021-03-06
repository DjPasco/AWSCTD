#include "stdafx.h"
#include "dsTable.h"

#include "AbsDatabase.h"
#include "AbsRecordset.h"
#include "dsStrConv.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

dsTable::dsTable(dsDatabase *pDatabase, const wchar_t *sTableName)
:   m_pDatabase(pDatabase),
	m_sTableName(sTableName)
{
	ASSERT(m_pDatabase);
    ASSERT(m_pDatabase->m_pDatabase);
	m_pSet = m_pDatabase->m_pDatabase->CreateRecordset();
}

dsTable::dsTable(dsDatabase *pDatabase, const char *sTableNameUTF8)
: m_pDatabase(pDatabase)
{
    m_sTableName = ds_str_conv::ConvertFromUTF8(sTableNameUTF8);
    ASSERT(m_pDatabase);
    ASSERT(m_pDatabase->m_pDatabase);
	m_pSet = m_pDatabase->m_pDatabase->CreateRecordset();
}

dsTable::~dsTable()
{
	delete m_pSet;
}

bool dsTable::Open() noexcept
{
	return m_pSet->Open(m_sTableName.c_str());
}

void dsTable::OpenSQL(const wchar_t *sSQL) noexcept
{
	m_pSet->OpenSQL(sSQL);
}

bool dsTable::OpenView(const wchar_t *sViewName) noexcept
{
    m_sTableName = sViewName; // view and table name indicated as same
                              // related with the ::Open realization
                              // DAO provides different objects for the View and Table.
	return m_pSet->OpenView(sViewName);
}

void dsTable::Flush() noexcept
{
    if ( !Open() ) {
		return;
	}

    m_pSet->Flush();
}

void dsTable::SetFieldBinary(const char *sFieldName, unsigned char *pData, size_t nSize) noexcept
{
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
    m_pSet->SetFieldBinary(sFieldNameUTF16.c_str(), pData, nSize);
}

void dsTable::SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, size_t nSize) noexcept
{
	m_pSet->SetFieldBinary(sFieldName, pData, nSize);
}

void dsTable::GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, size_t &nSize) const noexcept
{
	m_pSet->GetFieldBinary(sFieldName, pData, nSize);
}

void dsTable::GetFieldBinary(const char *sFieldName, unsigned char **pData, size_t &nSize) const noexcept
{
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
    m_pSet->GetFieldBinary(sFieldNameUTF16.c_str(), pData, nSize);
}

void dsTable::FreeBinary(unsigned char *pData) noexcept
{
	m_pSet->FreeBinary(pData);
}

bool dsTable::IsEOF() const	noexcept 
{ 
	return m_pSet->IsEOF();
}

void dsTable::MoveNext() noexcept  
{ 
	m_pSet->MoveNext();                        
}

bool dsTable::MoveFirst() noexcept 
{ 
	VERIFY(Open());
	return m_pSet->MoveFirst(); 
}

COLORREF dsTable::GetFieldRGB(const char *sFieldName) const noexcept
{
    const std::string sRGB = GetFieldStringUTF8(sFieldName);
	COLORREF color = ::atol(sRGB.c_str());
	return color;
}

COLORREF dsTable::GetFieldRGB(const wchar_t *sFieldName) const noexcept
{
	const std::wstring sRGB = GetFieldString(sFieldName);
	COLORREF color = ::_wtol(sRGB.c_str());
	return color;
}

void dsTable::SetFieldRGB(const char *sFieldName, COLORREF color) noexcept
{
    const std::string sRGB = std::to_string(color);
	m_pSet->SetFieldStringUTF8(sFieldName, sRGB.c_str());
}

void dsTable::SetFieldRGB(const wchar_t *sFieldName, COLORREF color) noexcept
{
	const std::wstring sRGB = std::to_wstring(color);
	m_pSet->SetFieldString(sFieldName, sRGB.c_str());
}

bool dsTable::SeekIndex(const char *sIndex, const char *sValue) noexcept
{
    VERIFY(Open());
	return m_pSet->SeekByString(sIndex, sValue);
}

bool dsTable::SeekIndex(const wchar_t *sIndex, const wchar_t *sValue) noexcept
{
	VERIFY(Open());
	return m_pSet->SeekByString(sIndex, sValue);
}

bool dsTable::SeekIndex(const wchar_t *sIndex, int32_t nValue) noexcept
{
	VERIFY(Open());
	return m_pSet->SeekByLong(sIndex, nValue);
}

bool dsTable::SeekIndex(const char *sIndex, int32_t nValue) noexcept
{
    VERIFY(Open());
    return m_pSet->SeekByLong(sIndex, nValue);
}

bool dsTable::IsFieldValueNull(const char *sFieldName) const noexcept
{
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
    return m_pSet->IsFieldValueNull(sFieldNameUTF16.c_str());
}

bool dsTable::IsFieldValueNull(const wchar_t *sFieldName) const noexcept
{
	return m_pSet->IsFieldValueNull(sFieldName);
}

std::wstring dsTable::GetFieldString(const wchar_t *sFieldName) const noexcept
{
	return m_pSet->GetFieldString(sFieldName);
}

void dsTable::SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue) noexcept 
{
	m_pSet->SetFieldString(sFieldName, sValue);
}

int32_t dsTable::GetFieldLong(const char *sFieldName) const noexcept
{
    return m_pSet->GetFieldInt32(sFieldName);
}

int32_t dsTable::GetFieldLong(const wchar_t *sFieldName) const noexcept
{
	return m_pSet->GetFieldInt32(sFieldName);
}

void dsTable::SetFieldLong(const wchar_t *sFieldName, int32_t nValue) noexcept
{
	m_pSet->SetFieldInt32(sFieldName, nValue);
}

void dsTable::SetFieldLong(const char *sFieldName, int32_t nValue) noexcept
{
    m_pSet->SetFieldInt32(sFieldName, nValue);
}

double dsTable::GetFieldDouble(const char *sFieldName) const noexcept
{
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
    return m_pSet->GetFieldDouble(sFieldNameUTF16.c_str());
}

double dsTable::GetFieldDouble(const wchar_t *sFieldName) const noexcept
{
	return m_pSet->GetFieldDouble(sFieldName);
}

void dsTable::SetFieldDouble(const char *sFieldName, double dValue) noexcept
{
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
    m_pSet->SetFieldDouble(sFieldNameUTF16.c_str(), dValue);
}

void dsTable::SetFieldDouble(const wchar_t *sFieldName, double dValue) noexcept
{
	m_pSet->SetFieldDouble(sFieldName, dValue);
}

bool dsTable::GetFieldBool(const char *sFieldName) const noexcept
{
    return m_pSet->GetFieldInt32(sFieldName) != 0;
}

void dsTable::SetFieldBool(const char *sFieldName, bool bValue) noexcept
{
    m_pSet->SetFieldInt32(sFieldName, bValue ? 1 : 0);
}

bool dsTable::GetFieldBool(const wchar_t *sFieldName) const noexcept
{
	return m_pSet->GetFieldInt32(sFieldName) != 0;
}

void dsTable::SetFieldBool(const wchar_t *sFieldName, bool bValue) noexcept
{
	m_pSet->SetFieldInt32(sFieldName, bValue ? 1 : 0);
}

time_t dsTable::GetFieldDateTime(const char *sFieldName) const noexcept
{
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
    return m_pSet->GetFieldDateTime(sFieldNameUTF16.c_str());
}

time_t dsTable::GetFieldDateTime(const wchar_t *sFieldName) const noexcept
{
	return m_pSet->GetFieldDateTime(sFieldName);
}

void dsTable::SetFieldDateTime(const wchar_t *sFieldName, time_t nValue) noexcept
{
	m_pSet->SetFieldDateTime(sFieldName, nValue);
}

void dsTable::SetFieldDateTime(const char *sFieldName, time_t nValue) noexcept
{
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
    m_pSet->SetFieldDateTime(sFieldNameUTF16.c_str(), nValue);
}

void dsTable::SetFieldNull(const char *sFieldName) noexcept
{
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
    m_pSet->SetFieldValueNull(sFieldNameUTF16.c_str());
}

void dsTable::SetFieldNull(const wchar_t *sFieldName) noexcept
{
	m_pSet->SetFieldValueNull(sFieldName);
}

bool dsTable::Delete() noexcept	        
{ 
    return m_pSet->Delete();         
}

void dsTable::AddNew() noexcept
{ 
	VERIFY(Open());
	m_pSet->AddNew();
}

void dsTable::Edit() noexcept		    
{ 
    m_pSet->Edit();	               
}

bool dsTable::Update() noexcept	       
{ 
    return m_pSet->Update();         
}

int32_t dsTable::GetColumnCount() noexcept
{
    VERIFY(Open());
    return m_pSet->GetColumnCount(); 
}

int32_t dsTable::GetRecordCount() noexcept
{ 
    VERIFY(Open());
    return m_pSet->GetRecordCount(); 
}

std::wstring dsTable::GetColumnName(int32_t nCol) noexcept
{
    VERIFY(Open());
    return m_pSet->GetColumnName(nCol); 
}

dsFieldType dsTable::GetColumnType(int32_t nCol) noexcept
{
    VERIFY(Open());
    return m_pSet->GetColumnType(nCol); 
}

bool dsTable::DoesFieldExist(const char *sFieldName) noexcept
{
    VERIFY(Open());
    const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
	return m_pSet->DoesFieldExist(sFieldNameUTF16.c_str());
}

bool dsTable::DoesFieldExist(const wchar_t *sFieldName) noexcept
{
    VERIFY(Open());
	return m_pSet->DoesFieldExist(sFieldName);
}

std::wstring dsTable::GetUniqueTextFieldValue(const wchar_t *sFieldName, const wchar_t *sPrefix, int width) noexcept
{
	VERIFY(Open());
	
    std::wstring sFormat  = L"%s%0";
                 sFormat += std::to_wstring(width);
                 sFormat += L"d";
    
	int index = 1;
	while (true)
	{
        int sz = std::swprintf(nullptr, 0, sFormat.c_str(), sPrefix, index);
        std::vector<wchar_t> buf(sz + 1); // note +1 for null terminator
        std::swprintf(&buf[0], buf.size(), sFormat.c_str(), sPrefix, index);
        std::wstring sTemp(buf.begin(), buf.end()-1); //#31407 "-1" to exclude \0

		if ( !SeekIndex(sFieldName, sTemp.c_str()) ) {
			return sTemp;
		}
		++index;
	}	

    ASSERT(FALSE);
    return L"";
}

dsDatabase *dsTable::GetDatabase() const noexcept
{
    return m_pDatabase; 
}

bool dsTable::DeleteAllByIndex(const char *sField, const char *sValue) noexcept
{
    VERIFY(Open());
    return m_pSet->DeleteAllByStringValueUTF8(sField, sValue);
}

bool dsTable::DeleteAllByIndex(const wchar_t *sField, const wchar_t *sValue) noexcept
{
    VERIFY(Open());
    return m_pSet->DeleteAllByStringValue(sField, sValue);
}

bool dsTable::DeleteAllByIndex(const char *sField, int32_t nValue) noexcept
{
    VERIFY(Open());
    return m_pSet->DeleteAllByLongValueUTF8(sField, nValue);
}

bool dsTable::DeleteAllByIndex(const wchar_t *sField, int32_t nValue) noexcept
{
    VERIFY(Open());
    return m_pSet->DeleteAllByLongValue(sField, nValue);
}

bool dsTable::DeleteByIndex(const wchar_t *sField, const wchar_t *sValue) noexcept
{
    VERIFY(Open());
    return m_pSet->DeleteByStringValue(sField, sValue);
}

bool dsTable::DeleteByIndex(const char *sField, const char *sValue) noexcept
{
    VERIFY(Open());
    return m_pSet->DeleteByStringValue(sField, sValue);
}

bool dsTable::DeleteByIndex(const wchar_t *sField, int32_t nValue) noexcept
{
    VERIFY(Open());
    return m_pSet->DeleteByLongValue(sField, nValue);
}

bool dsTable::DeleteByIndex(const char *sField, int32_t nValue) noexcept
{
    VERIFY(Open());
    return m_pSet->DeleteByLongValue(sField, nValue);
}

std::wstring dsTable::GetTableName() const noexcept
{
    return m_sTableName;
}

std::string dsTable::GetFieldStringUTF8(const char *sFieldName) const noexcept
{
    return m_pSet->GetFieldStringUTF8(sFieldName);
}

void dsTable::SetFieldStringUTF8(const char *sFieldName, const char *sValue) noexcept
{
    return m_pSet->SetFieldStringUTF8(sFieldName, sValue);
}