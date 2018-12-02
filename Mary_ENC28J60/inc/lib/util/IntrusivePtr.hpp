
#ifndef   EXLIB_CORE_INTRUSIVE_PTR_HPP
#define   EXLIB_CORE_INTRUSIVE_PTR_HPP

namespace exlib
{

/**
 * @brief   侵入式参照カウントスマートポインタ
 **/
template <class T>
class IntrusivePtr
{
public:
    
    IntrusivePtr()
        : mPtr(0)
    {}

    /**
     * @param[in] T           管理するポインタ
     * @param[in] need_add_ref  参照カウントを増やすか
     **/
    explicit IntrusivePtr( T* p, bool need_add_ref = true )
        : mPtr( p )
    {
        if( need_add_ref && mPtr ){
            mPtr->addref();
        }
    }
    
    IntrusivePtr( const IntrusivePtr& p )
        : mPtr( p.mPtr )
    {
        if( mPtr ){
            mPtr->addref();
        }
    }
    
    template <class U>
    IntrusivePtr( const IntrusivePtr<U>& p )
        : mPtr( static_cast<T*>( p.mPtr ) )
    {
        if( mPtr ){
            mPtr->addref();
        }
    }
    
    ~IntrusivePtr() throw()
    {
        if( mPtr && mPtr->release() ){
            destroy();
        }
    }
    
    IntrusivePtr& operator=( const IntrusivePtr& r )
    {
        // まず引数側の参照カウントをインクリメントする
        // 先に代入される側を消すと、自己代入したとき悲惨になる
        if( r.mPtr ){
            r.mPtr->addref();
        }
        
        if( mPtr && mPtr->release() ){
            destroy();
        }
        mPtr = r.mPtr;
        
        return *this;
    }

    template <class U>
    IntrusivePtr& operator=( const IntrusivePtr<U>& r )
    {
        if( r.mPtr ){
            r.mPtr->addref();
        }
        
        if( mPtr && mPtr->release() ){
            destroy();
        }
        mPtr = static_cast<T*>( r.mPtr );
        
        return *this;
    }
        
    T& operator*() const { return *mPtr; }
    T* operator->() const { return mPtr; }
    T* get() const { return mPtr; }
    bool isNull() const { return mPtr == nullptr; }
    
private:
        
    void destroy()
    {
        delete mPtr;
        mPtr = 0;
    }
    
    T* mPtr;
};


}   // end of namespace exlib

#endif   // EXLIB_CORE_INTRUSIVE_PTR_HPP
