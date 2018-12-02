
#ifndef   EXLIB_CORE_REFCOUNT_HPP
#define   EXLIB_CORE_REFCOUNT_HPP

#include <stdint.h>

namespace exlib
{


/**
 * @brief  参照カウント基底クラス
 * IntrusivePtrなどに用いるための基底クラス
 **/ 
class RefCount
{
public:
    
    RefCount() : mCount( 0 ) {}
    virtual ~RefCount() throw() {}
    
    //! 参照カウントを増やす
    void addref() { ++mCount; }
    /**
     * @brief  参照カウントを減らす
     * @return   参照カウントが0以下ならtrue、それ以外はfalse
     */
    bool release()
    {
        --mCount;
        return mCount == 0;
    }
    
private:
    
    int32_t mCount;  //! 参照カウント
};


}  // end of namespace exlib

#endif   // EXLIB_CORE_REFCOUNT_HPP

