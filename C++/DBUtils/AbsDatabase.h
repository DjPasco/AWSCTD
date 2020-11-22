#ifndef __ABS_DATABASE_H__
#define __ABS_DATABASE_H__
#pragma once

#ifndef __DS_TYPES_H__
	#include "dsTypes.h"
#endif

#include "unordered_map"
class dsTableFieldInfo : public std::unordered_map<std::wstring, dsFieldType, std::hash<std::basic_string<wchar_t> > > { };

class CAbsRecordset;

class CAbsDatabase
{
// Construction/Destruction
public:
	CAbsDatabase() { }
	virtual ~CAbsDatabase() { }

// Overrides
public:
	virtual bool BeginTrans()  = 0; 
	virtual bool CommitTrans() = 0; 
	virtual bool Rollback()    = 0; 

	virtual bool Execute(const wchar_t *lpszSQL) = 0; 

    virtual bool OpenDB(const wchar_t *sPath, bool bReadOnly, const wchar_t *szPsw) = 0;
    
	virtual dsDBType GetType() = 0;

	virtual bool IsReadOnly() const = 0;
	virtual bool IsOpen()     const = 0;

	virtual std::wstring GetName() = 0;

	virtual bool DoesTableExist(const wchar_t *sTable) = 0;

	virtual CAbsRecordset *CreateRecordset() = 0;

    virtual void CommitDatabase() = 0;

    virtual bool CompactDatabase() = 0;

	virtual void DeleteRelation(const wchar_t *sRelation) = 0;
	virtual bool CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, long lAttr,
								const wchar_t *sField, const wchar_t *sForeignField) = 0;

    virtual bool GetTableFieldInfo(const wchar_t *sTable, dsTableFieldInfo &info) = 0;

    typedef void (*dbErrorHandler)(const wchar_t *msg); 
    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) = 0;
};

namespace ds_table_field_info_util
{
    inline void fields_union(dsTableFieldInfo &union_info, const dsTableFieldInfo &src_info, const dsTableFieldInfo &dst_info)
    {
        auto end_it = src_info.end();
        for (auto it = src_info.begin(); it != end_it; ++it) 
        {
            if ( dst_info.find(it->first.c_str()) == dst_info.end() ) {
                continue;
            }
            union_info[it->first] = it->second;
        }
    }    

    inline bool fields_union(dsTableFieldInfo &union_info, CAbsDatabase *pSrcDB, const wchar_t *sTableNameSrc, CAbsDatabase *pDstDB, const wchar_t *sTableNameDst) 
    {
        dsTableFieldInfo dst_info;
        if ( !pDstDB->GetTableFieldInfo(sTableNameDst, dst_info) ) {
            return false;
        }

        dsTableFieldInfo src_info;
        if ( !pSrcDB->GetTableFieldInfo(sTableNameSrc, src_info) ) {
            return false;
        }

        ds_table_field_info_util::fields_union(union_info, src_info, dst_info);
        return true;
    }
};

#endif