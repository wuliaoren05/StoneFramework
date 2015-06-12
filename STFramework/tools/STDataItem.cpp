#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include "tools/STDataItem.h"
#include "tools/STBase64.h"


class STDataItem::PrivateFunc
{
public:

    static STString itemToStr(const STDataItem &item)
    {
        STString ret;

        if (!item.isValid() ) {
            return ret;
        }

        ret += "<" + item.tagName() + ">";
        if (item.type() == Type_Parent) {
            STDataItems childs = item.childs();
            for (unsigned int i=0; i<childs.size(); ++i) {
                ret += itemToStr(childs[i]);
            }
        }
        else {
            switch (item.type() ) {
            case Type_Bool:
                ret += "<bool>" + boolToString(item.toBool() ) + "</bool>";
                break;
            case Type_Int:
                ret += "<int>" + intToString(item.toInt() ) + "</int>";
                break;
            case Type_String:
                ret += "<string>" + item.toString() + "</string>";
                break;
            case Type_Binary:
                ret += "<binaryAsBase64>" + STBase64::encode(item.toBinary()) + "</binaryAsBase64>";
                break;
            default:
                break;
            }
        }
        ret += "</" + item.tagName() + ">";

        return ret;
    }

    static STDataItem stringToItem(const STString &str)
    {
        STString itemTag = getFirstTagName(str);
        if (itemTag.empty() ) {
            return STDataItem("");
        }

        STString interTagStr = getTagStr(str, itemTag, false);
        if (interTagStr.empty() ) {
            return STDataItem("");
        }

        STDataItem item(itemTag);
        if (isValueStr(interTagStr)) {
            STString valueTag = getFirstTagName(interTagStr);
            STString valueStr = getTagStr(interTagStr, valueTag);
            if ("bool" == valueTag) {
                item = stringToBool(valueStr);
            }
            else if ("int" == valueTag) {
                item = stringToInt(valueStr);
            }
            else if ("string" == valueTag) {
                item = valueStr;
            }
            else if ("binaryAsBase64" == valueTag) {
                item = STBase64::decode(valueStr);
            }
        }
        else {
            STString childStr = interTagStr;
            int childPos = -1;
            STString childTag = getFirstTagName(childStr, childPos);
            while (!childTag.empty()) {
                STString realChildStr = getTagStr(childStr, childTag, true);
                STDataItem child = stringToItem(getTagStr(childStr, childTag, true));
                if ( child.isValid() ) {
                    item.addChild(child);
                }

                childStr = childStr.substr(childPos+realChildStr.size()
                                           , childStr.size()-childPos-realChildStr.size());
                childTag = getFirstTagName(childStr, childPos);
            }
        }

        return item;
    }

    static STString getFirstTagName(const STString &str)
    {
        int pos = -1;
        return getFirstTagName(str, pos);
    }

    static STString getFirstTagName(const STString &str, int &position)
    {
        STString ret;

        int beginPos = -1;
        int endPos  = -1;

        beginPos = str.find("<") + 1;
        endPos = str.find(">");
        if (findError(beginPos) || findError(endPos)) {
            return ret;
        }
        else {
            ret = str.substr(beginPos, endPos - beginPos);
            position = beginPos -1;
        }

        return ret;
    }

    static STString getTagStr(const STString &str, const STString &tagName, bool withTag = false)
    {
        STString ret;
        if(str.empty() || tagName.empty() ) {
            return ret;
        }

        const STString startTag("<" + tagName + ">");
        const STString endTag("</" + tagName + ">");

        if (str.size() < startTag.size() + endTag.size()) {
            return ret;
        }

        int beginPos = str.find(startTag);
        int endPos = beginPos;
        int startTagInStack = 1;
        for (unsigned int i=beginPos+startTag.size(); i<str.size(); ++i) {
            if (str[i] == startTag[0]) {
                bool isStartTag = true;
                for (unsigned int j=1; j<startTag.size(); ++j) {
                    if (str[i+j] != startTag[j]) {
                        isStartTag = false;
                        break;
                    }
                }

                if (isStartTag) {
                    ++startTagInStack;
                }
            }

            if (str[i] == endTag[0]) {
                bool isEndTag = true;
                for (unsigned int j=1; j<endTag.size(); ++j) {
                    if (str[i+j] != endTag[j]) {
                        isEndTag = false;
                        break;
                    }
                }

                if (isEndTag) {
                    --startTagInStack;
                }
            }

            if (startTagInStack == 0) {
                endPos = i;
                break;
            }
        }


        if (findError(beginPos) || findError(endPos) || beginPos == endPos ) {
            return ret;
        }
        else {
            int charCount = 0;
            if (!withTag) {
                beginPos += startTag.size();
                charCount = endPos - beginPos;
            }
            else {
                charCount = endPos - beginPos + endTag.size();
            }
            ret = str.substr(beginPos, charCount);
        }

        return ret;
    }

    static bool isValueStr(const STString &str)
    {
        bool ret = false;
        if( str.empty() ) {
            return ret;
        }

        STString valueTag = getFirstTagName(str);
        if ("bool" != valueTag && "int" != valueTag && "string" != valueTag && "binaryAsBase64" != valueTag) {
            return ret;
        }
        STString startTag("<" + valueTag + ">");
        STString endTag("</" + valueTag + ">");

        int beginPos = str.find(startTag);
        int endPos = str.find_last_of(endTag);

        if (findError(beginPos) || findError(endPos) ) {
            return ret;
        }
        else {
            ret = true;
        }

        return ret;
    }

    static bool findError(STString::size_type pos)
    {
        return STString::npos == pos;
    }

    static STString intToString(int value)
    {
        std::ostringstream intStream;
        intStream << value;
        return intStream.str();
    }

    static int stringToInt(const STString &str)
    {
        std::stringstream strStream(str);
        int outInt = -1;
        strStream >> outInt;
        return outInt;
    }

    static STString boolToString(bool value)
    {
        return value ? "true" : "false";
    }

    static bool stringToBool(const STString &str)
    {
        return str=="true" ? true : false;
    }

private:
    PrivateFunc();
};


STDataItem::STDataItem(const STString &tagName)
    : m_data(new RealData())
{
    m_data->m_tagName = tagName;
    m_data->m_type = Type_Parent;
}

STDataItem::STDataItem(const STString &tagName, bool value)
    : m_data(new RealData())
{
    m_data->m_tagName = tagName;
    m_data->m_type = Type_Bool;
    m_data->m_value.boolValue = value;
}

STDataItem::STDataItem(const STString &tagName, int value)
    : m_data(new RealData())
{
    m_data->m_tagName = tagName;
    m_data->m_type = Type_Int;
    m_data->m_value.intValue = value;
}

STDataItem::STDataItem(const STString &tagName, const char *value)
    : m_data(new RealData())
{
    m_data->m_tagName = tagName;
    m_data->m_type = Type_String;
    m_data->m_value.strValue = value;
}

STDataItem::STDataItem(const STString &tagName, const STString &value)
    : m_data(new RealData())
{
    m_data->m_tagName = tagName;
    m_data->m_type = Type_String;
    m_data->m_value.strValue = value;
}

STDataItem::STDataItem(const STString &tagName, const STBuffer &value)
    : m_data(new RealData())
{
    m_data->m_tagName = tagName;
    m_data->m_type = Type_Binary;
    m_data->m_value.binValue = value;
}

STDataItem::STDataItem(const RealDataPtr &realDataPtr)
    : m_data(realDataPtr)
{
}

bool STDataItem::isValid() const
{
    return !tagName().empty();
}

STDataItem::Type STDataItem::type() const
{
    Type ret = Type_Count;

    if (NULL != m_data.get()) {
        ret = m_data->type();
    }
    else {
        ret = Type_Count;
    }

    return ret;
}

STString STDataItem::tagName() const
{
    return m_data->m_tagName;
}

bool STDataItem::toBool(bool &transSuccess) const
{
    bool ret = false;
    if (type() == Type_Bool) {
        transSuccess = true;
        ret = m_data->boolValue();
    }
    else {
        transSuccess = false;
    }

    return ret;
}

bool STDataItem::toBool() const
{
    bool transSuccess = false;
    return toBool(transSuccess);
}

int STDataItem::toInt(bool &transSuccess) const
{
    int ret = -1;

    if (type() == Type_Int) {
        transSuccess = true;
        ret = m_data->intValue();
    }
    else {
        transSuccess = false;
    }

    return ret;
}

int STDataItem::toInt() const
{
    bool transSuccess = false;
    return toInt(transSuccess);
}

STString STDataItem::toString(bool &transSuccess) const
{
    STString ret("");

    if (type() == Type_String) {
        transSuccess = true;
        ret = m_data->strValue();
    }
    else {
        transSuccess = false;
    }

    return ret;
}

STString STDataItem::toString() const
{
    bool transSuccess = false;
    return toString(transSuccess);
}

STBuffer STDataItem::toBinary(bool &transSuccess) const
{
    STBuffer ret(0);

    if (type() == Type_Binary) {
        transSuccess = true;
        ret = m_data->binValue();
    }
    else {
        transSuccess = false;
    }

    return ret;
}

STBuffer STDataItem::toBinary() const
{
    bool transSuccess = false;
    return toBinary(transSuccess);
}

void STDataItem::operator =(bool value)
{
    if (!m_data->m_childs.empty() ) {
        m_data->m_childs.clear();
    }

    if (m_data.get() == NULL) {
        m_data = new RealData();
        m_data->changeValue(value);
    }
    else {
        m_data->changeValue(value);
    }
}

void STDataItem::operator =(const STBuffer &value)
{
    if (!m_data->m_childs.empty() ) {
        m_data->m_childs.clear();
    }

    if (m_data.get() == NULL) {
        m_data = new RealData();
        m_data->changeValue(value);
    }
    else {
        m_data->changeValue(value);
    }
}

void STDataItem::operator =(int value)
{
    if (!m_data->m_childs.empty() ) {
        m_data->m_childs.clear();
    }

    if (m_data.get() == NULL) {
        m_data = new RealData();
        m_data->changeValue(value);
    }
    else {
        m_data->changeValue(value);
    }
}

void STDataItem::operator =(const STString &value)
{
    if (!m_data->m_childs.empty() ) {
        m_data->m_childs.clear();
    }

    if (m_data.get() == NULL) {
        m_data = new RealData();
        m_data->changeValue(value);
    }
    else {
        m_data->changeValue(value);
    }
}

int STDataItem::childCount(const STString &tagName) const
{
    return m_data->m_childs.count(tagName);

}

STDataItems STDataItem::childs(const STString &tagName) const
{
    STDataItems ret;

    if (!tagName.empty() ) {
        std::multimap<STString, RealDataPtr>::const_iterator itor = m_data->m_childs.find(tagName);
        int count = childCount(tagName);
        for (int i=0; i != count; ++i, ++itor) {
            ret.push_back(itor->second);
        }
    }
    else {
        std::multimap<STString, RealDataPtr>::const_iterator itor = m_data->m_childs.begin();
        for (; itor != m_data->m_childs.end(); ++itor) {
            ret.push_back(itor->second);
        }
    }

    return ret;
}

STDataItem STDataItem::child(const STString &tagName)
{
    STDataItem ret("");

    if (childCount(tagName) > 0) {
        ret = STDataItem(m_data->m_childs.find(tagName)->second);
    }

    return ret;
}

void STDataItem::addChild(const STDataItems &childs)
{
    for (std::size_t i = 0; i<childs.size(); ++i) {
        m_data->m_childs.insert( std::make_pair(childs[i].tagName(), childs[i].m_data) );
    }
}

void STDataItem::addChild(const STDataItem &child)
{
    m_data->m_childs.insert( std::make_pair(child.tagName(), child.m_data) );
}

bool STDataItem::loadFromStr(const STString &str)
{
    bool ret = false;

    STDataItem item = PrivateFunc::stringToItem(str);
    if ( item.isValid() ) {
        this->clear();
        *this = item;
        ret = true;
    }

    return ret;
}

bool STDataItem::loadFromFile(const STString &fileFullPath)
{
    bool ret = false;
    if (!fileFullPath.empty() ) {
        std::ifstream in(fileFullPath.c_str(), std::ios::in);
        std::istreambuf_iterator<char> begin(in), end;
        STString configStr(begin, end);
        in.close();

        ret = loadFromStr(configStr);
    }
    else {
        ret = false;
    }

    return ret;
}

void STDataItem::clear()
{
    *this = STDataItem("");
}

bool STDataItem::saveToFile(const STString &filePath)
{
    bool ret = false;

    if (!filePath.empty()) {
        std::ofstream outPut;
        outPut.open(filePath.c_str());
        outPut << serialToStr().c_str();
        outPut.close();

        ret = true;
    }
    else {
        ret = false;
    }
    return ret;
}

STString STDataItem::serialToStr()
{
    return PrivateFunc::itemToStr(*this);
}







//for class STDataItem::RealData
STDataItem::RealData::RealData()
    : m_type(Type_Parent)
    , m_tagName("")
    , m_refCount(0)
{}

STDataItem::Type STDataItem::RealData::type() const
{
    return m_type;
}

void STDataItem::RealData::changeValue(bool value)
{
    m_type = Type_Bool;
    m_value.boolValue = value;
}

void STDataItem::RealData::changeValue(int value)
{
    m_type = Type_Int;
    m_value.intValue = value;
}

void STDataItem::RealData::changeValue(const STString &value)
{
    m_type = Type_String;
    m_value.strValue = value;
}

void STDataItem::RealData::changeValue(const STBuffer &value)
{
    m_type = Type_Binary;
    m_value.binValue = value;
}

bool STDataItem::RealData::boolValue()
{
    return m_value.boolValue;
}

int STDataItem::RealData::intValue()
{
    return m_value.intValue;
}

STString STDataItem::RealData::strValue()
{
    return m_value.strValue;
}

STBuffer STDataItem::RealData::binValue()
{
    return m_value.binValue;
}

//for class STDataItem::RealData end




