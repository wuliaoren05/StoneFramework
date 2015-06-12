#ifndef ST_DATA_ITEM_H
#define ST_DATA_ITEM_H

#include <vector>
#include <map>

#include "base/STCommonDefine.h"
#include "base/STPtr.h"
#include "base/STBuffer.h"


class STDataItem;
typedef std::vector<STDataItem> STDataItems;


class STDataItem
{
public:
    enum Type
    {
        Type_Parent = 0,
        Type_Bool,
        Type_Int,
        Type_String,
        Type_Binary,

        Type_Count
    };

public:
    STDataItem(const STString &tagName);
    STDataItem(const STString &tagName, bool value);
    STDataItem(const STString &tagName, int value);
    STDataItem(const STString &tagName, const char* value);
    STDataItem(const STString &tagName, const STString &value);
    STDataItem(const STString &tagName, const STBuffer &value);

    bool isValid() const;
    Type type() const;
    STString tagName() const;

    bool toBool(bool &transSuccess) const;
    bool toBool() const;

    int toInt(bool &transSuccess) const;
    int toInt() const;

    STString toString(bool &transSuccess) const;
    STString toString() const;

    STBuffer toBinary(bool &transSuccess) const;
    STBuffer toBinary() const;


    void operator =(bool value);
    void operator =(int value);
    void operator =(const std::string &value);
    void operator =(const STBuffer &value);

    //const NGConfigItem(const NGConfigItem &other);//not need, default is enough

    int childCount(const std::string& tagName) const;
    STDataItems childs(const STString &tagName = "") const;
    STDataItem child(const STString& tagName);

    void addChild(const STDataItems &childs);
    void addChild(const STDataItem &child);

    bool loadFromStr(const STString &str);
    bool loadFromFile(const STString &fileFullPath);
    void clear();
    bool saveToFile(const STString &filePath);
    STString serialToStr();

private:
    class RealData;
    typedef STSharePtr<RealData> RealDataPtr;
    STDataItem(const RealDataPtr &realDataPtr);//for vector

    class PrivateFunc;

    RealDataPtr m_data;
};


class STDataItem::RealData
{
public:
    RealData();
    Type type() const;
    void changeValue(bool value);
    void changeValue(int value);
    void changeValue(const STString &value);
    void changeValue(const STBuffer &value);
    bool boolValue();
    int intValue();
    STString strValue();
    STBuffer binValue();

private:
    friend class STDataItem;
    Type                                    m_type;
    struct
    {
        bool boolValue;
        int intValue;
        STString strValue;
        STBuffer binValue;
    }                                       m_value;

    STString                                m_tagName;
    std::multimap<STString, RealDataPtr>    m_childs;

    int m_refCount;
 };


#endif // ST_DATA_ITEM_H








