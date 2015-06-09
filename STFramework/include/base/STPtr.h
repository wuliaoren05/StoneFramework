#ifndef ST_PTR_H
#define ST_PTR_H

#include "STAtom.h"
#include "STCommonDefine.h"

#ifndef NULL
    #define NULL 0
#endif

template <class T>
class STScopePtr
{
public:
    explicit STScopePtr(T* obj)
        : m_obj(obj)
    {}
    ~STScopePtr()
    {
        deleteObj();
    }

    void deleteObj()
    {
        if (NULL != m_obj) {
            delete m_obj;
            m_obj = NULL;
        }
    }

    T* get() const
    {
        return m_obj;
    }

    T* operator->() const
    {
        return m_obj;
    }

    operator bool()
    {
        return (m_obj != NULL);
    }
private:
    STScopePtr(){};
    STScopePtr(const STScopePtr& other){};
    STScopePtr& operator=(const STScopePtr& other){return this;};
    bool operator==(const STScopePtr& other){return false;};

private:
    T* m_obj;
};


//thread safe
template <class T>
class STSharePtr
{
public:
    explicit STSharePtr(T* obj)
        : m_objRef(new ObjRef(obj))
    {}
    STSharePtr(const STSharePtr& other)
    {
        if (this != &other) {
            this->m_objRef = other.m_objRef;
            addRef();
        }
    }

    ~STSharePtr()
    {
        removeRef();
    }

    T* get() const
    {
        return m_objRef->obj;
    }

    T* operator->() const
    {
        return m_objRef->obj;
    }

    STSharePtr& operator=(const STSharePtr& other)
    {
        if (this != &other) {
            this->m_objRef = other.m_objRef;
            addRef();
        }
        return *this;
    }
    STSharePtr& operator=(T* obj)
    {
        if (this->m_objRef->obj != obj) {
            removeRef();

            this->m_objRef = new ObjRef(obj);
        }
        return *this;
    }
    bool operator==(const STSharePtr& other)
    {
        return (m_objRef->obj = other.m_objRef->obj);
    }
    operator bool()
    {
        return (NULL != m_objRef->obj);
    }

private:
    struct ObjRef
    {
        int count;
        T*  obj;
        ObjRef(T* obj)
            : count(1)
            , obj(obj)
        {}
    };

private:
    void addRef()
    {
        STAtom::Inc(&(m_objRef->count));
    }
    void removeRef()
    {
        STDAssert(m_objRef->count > 0);
        STAtom::Dec(&(m_objRef->count));
        if (0 == m_objRef->count && m_objRef->obj != NULL) {
            delete m_objRef->obj;
            m_objRef->obj = NULL;
        }
    }

private:
    ObjRef *m_objRef;
};


/*
//not thread safe
//T* must implement addRef and removeRef function. obj must delete it's self in removeRef when ref count decline to 0
template <class T>
class STIntrusiveSharePtr
{
public:
    explicit STIntrusiveSharePtr(T* obj)
    {
        m_obj = obj;
        addRef();
    }
    STIntrusiveSharePtr(const STIntrusiveSharePtr& other)
    {
        if (this != &other) {
            this->m_obj = other.m_obj;
            addRef();
        }
    }
    ~STIntrusiveSharePtr()
    {
        removeRef();
    }

    void addRef()
    {
        m_obj->addRef();
    }
    void removeRef()
    {
        m_obj->removeRef();
    }

    T* get() const
    {
        return m_obj;
    }

    T* operator->() const
    {
        return m_obj;
    }

    STIntrusiveSharePtr& operator=(const STIntrusiveSharePtr& other)
    {
        if (this != &other) {
            this->m_obj = other.m_obj;
            addRef();
        }
        return *this;
    }
    STIntrusiveSharePtr& operator=(T* obj)
    {
        if (this->m_obj != obj) {
            removeRef();

            this->m_obj = obj;
        }
        return *this;
    }
    bool operator==(const STIntrusiveSharePtr& other)
    {
        return (m_obj = other.m_obj);
    }
private:
    T* m_obj;
};
*/

#endif // ST_PTR_H















