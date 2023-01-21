#pragma once
#include <Misc/LazySingleton.h>
#include "Auxiliary.h"

//Expand TLazySingleton implemented by UE to make it work across modules.
//We will get the same instance when calling TSingleton<T>::Get() from any modules.
//TSingleton<T>::Get() keeps Thread-safe.

namespace FRAMEWORK
{
namespace PRIVATE
{
    template<typename T>
    struct CrossModuleWrapper {
        using Type = T;
    };
}
    template<typename T>
    using TSingleton = TLazySingleton<PRIVATE::CrossModuleWrapper<T>>;

    FRAMEWORK_API TMap<FString,FLazySingleton*>& GetSharedInstanceMap(const FString& TypeName);
    FRAMEWORK_API FCriticalSection& GetSingleTonCS();
}

template<typename T>
class TLazySingleton<FRAMEWORK::PRIVATE::CrossModuleWrapper<T>> final : public FLazySingleton
{
public:
    static T& Get()
    {
        return GetLazy(Construct<T>).GetValue();
    }

    static void TearDown()
    {
        return GetLazy(nullptr).Reset();
    }

    static T* TryGet()
    {
        return GetLazy(Construct<T>).TryGetValue();
    }

private:
    static TLazySingleton& GetLazy(void(*Constructor)(void*))
    {
        auto& SingletonCS = FRAMEWORK::GetSingleTonCS();
        
        FScopeLock ScopeLock(&SingletonCS);
        auto& SharedInstanceMap = FRAMEWORK::GetSharedInstanceMap(TypeName);
        FLazySingleton** SingleTon = SharedInstanceMap.Find(TypeName);
        if(!SingleTon) {
            static TLazySingleton StaticSingleton(Constructor);
            SharedInstanceMap.Add(TypeName,&StaticSingleton);
            return StaticSingleton;
        }
        return *static_cast<TLazySingleton*>(*SingleTon);
    }

    static inline FString TypeName = FRAMEWORK::AUX::TTypename<T>::Value;
    alignas(T) unsigned char Data[sizeof(T)];
    T* Ptr;

    TLazySingleton(void(*Constructor)(void*))
    {
        if (Constructor)
        {
            Constructor(Data);
        }

        Ptr = Constructor ? (T*)Data : nullptr;
    }

#if (!defined(DISABLE_LAZY_SINGLETON_DESTRUCTION) || !DISABLE_LAZY_SINGLETON_DESTRUCTION)
    ~TLazySingleton()
    {
        Reset();
    }
#endif

    T* TryGetValue()
    {
        return Ptr;
    }

    T& GetValue()
    {
        check(Ptr);
        return *Ptr;
    }

    void Reset()
    {
        if (Ptr)
        {
            Destruct(Ptr);
            Ptr = nullptr;
        }
    }
};
