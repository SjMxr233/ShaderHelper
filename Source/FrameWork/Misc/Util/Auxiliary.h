#pragma once
#include "CommonHeader.h"

namespace FRAMEWORK {
	namespace AUX {
		
		template<typename LeftSeq, typename RightSeq>
		struct Op;

		template<uint32 ... LeftIndexes, uint32 ... RightIndexes>
		struct Op<TIntegerSequence<uint32, LeftIndexes...>, TIntegerSequence<uint32, RightIndexes...>> {

			static_assert(sizeof...(LeftIndexes) == sizeof...(RightIndexes), "Op just support same size sequence.");
			
			template<typename RetType, typename LType, typename RType>
			static RetType Sum(LType& lhs, RType& rhs) { return RetType(lhs[LeftIndexes] + rhs[RightIndexes]...); }
			template<typename RetType, typename LType, typename RType>
			static RetType Mul(LType& lhs, RType& rhs) { return RetType(lhs[LeftIndexes] * rhs[RightIndexes]...); }
			template<typename RetType, typename LType, typename RType>
			static RetType Sub(LType& lhs, RType& rhs) { return RetType(lhs[LeftIndexes] - rhs[RightIndexes]...); }
			template<typename RetType, typename LType, typename RType>
			static RetType Div(LType& lhs, RType& rhs) { return RetType(lhs[LeftIndexes] / rhs[RightIndexes]...); }

		};

		template<typename Smart, typename Pointer>
		struct OutPtrImpl;
		
		template<typename T, typename D, typename Pointer>
		struct OutPtrImpl<TUniquePtr<T, D>, Pointer> {
			using Smart = TUniquePtr<T, D>;
			using Storage = typename Smart::ElementType*;
		public:
			OutPtrImpl(Smart& smart) : SmartPtr(smart), TargetPtr(nullptr) {}
			operator Pointer* () {
				return &TargetPtr;
			}
			~OutPtrImpl() {
				SmartPtr.Reset(static_cast<Storage>(TargetPtr));
			}
		private:
			Smart& SmartPtr;
			Pointer TargetPtr;
		};

		template<typename T, ESPMode Mode, typename Pointer>
		struct OutPtrImpl<TSharedPtr<T, Mode>, Pointer> {
			using Smart = TSharedPtr<T, Mode>;
			using Storage = typename Smart::ElementType*;
		public:
			OutPtrImpl(Smart& smart) : SmartPtr(smart), TargetPtr(nullptr) {}
			operator Pointer* () {
				return &TargetPtr;
			}
			~OutPtrImpl() {
				Smart Temp = Smart(static_cast<Storage>(TargetPtr));
				Swap(Temp, SmartPtr);
			}
		private:
			Smart& SmartPtr;
			Pointer TargetPtr;
		};

		template<typename T, ESPMode Mode, typename Pointer>
		struct OutPtrImpl<TSharedRef<T, Mode>, Pointer> {
			using Smart = TSharedRef<T, Mode>;
			using Storage = typename Smart::ElementType*;
		public:
			OutPtrImpl(Smart& smart) : SmartPtr(smart), TargetPtr(nullptr) {}
			operator Pointer* () {
				return &TargetPtr;
			}
			~OutPtrImpl() {
				Smart Temp = Smart(static_cast<Storage>(TargetPtr));
				Swap(Temp, SmartPtr);
			}
		private:
			Smart& SmartPtr;
			Pointer TargetPtr;
		};

		//c++14
		//template<typename Smart, typename Pointer,
		//	std::enable_if_t<std::is_same_v<Pointer, void>, bool> = true
		//>
		//decltype(auto) BaseOutPtr(Smart& s) {
		//	return OutPtrImpl<Smart, typename Smart::ElementType*>(s);
		//}

		//template<typename Smart, typename Pointer,
		//	std::enable_if_t<!std::is_same_v<Pointer, void>, bool> = true
		//>
		//decltype(auto) BaseOutPtr(Smart& s) {
		//	return OutPtrImpl<Smart, Pointer>(s);
		//}

		//c++23 OutPtr for UE smart pointer.
		template<typename Pointer = void, typename Smart>
		decltype(auto) OutPtr(Smart& s) {
			if constexpr (std::is_same_v<Pointer, void>) {
				return OutPtrImpl<Smart, typename Smart::ElementType*>(s);
			}
			else {
				return OutPtrImpl<Smart, Pointer>(s);
			}
		}

		template<typename T, int32 Min, T Cur, T... Seq>
		struct RangeIntegerSequenceImpl {
			struct LazyInitIntSeq {
				using Type = TIntegerSequence<T, Cur, Seq...>;
			};
			struct LazyInitRangeIntSeq {
				using Type = typename RangeIntegerSequenceImpl<T, Min, Seq...>::Type;
			};
			using Type = typename TChooseClass< Cur >= Min, LazyInitIntSeq, LazyInitRangeIntSeq >::Result::Type;
		};

		template<typename T, int32 Min, typename Seq>
		struct RangeIntegerSequence;

		template<typename T, int32 Min, T... Seq>
		struct RangeIntegerSequence<T, Min, TIntegerSequence<T, Seq...>> {
			static_assert(Min >= 0 && Min <= sizeof...(Seq) - 1, "Min must greater than 0 and less than Max");
			using Type = typename RangeIntegerSequenceImpl<T, Min, Seq...>::Type;
		};
		
		//Make TIntegerSequence[min,max], 0 <= min <= max
		template<typename T,int32 Min, int32 Max>
		using MakeRangeIntegerSequence = typename RangeIntegerSequence<T, Min, TMakeIntegerSequence<T, Max + 1>>::Type;

		//Pass a run-time var with known range to template
		//You need to move invocation to a function object.
		template<template<int> typename Caller, typename T>
		struct RunCaseWithIntImpl;

		template<template<int> typename Caller, typename T, T... Seq>
		struct RunCaseWithIntImpl<Caller, TIntegerSequence<T, Seq...>> {
			static decltype(auto) Invoke(int arg) {
				return Iter<Seq...>(arg);
			}
			template<int x, int... SubSeq>
			static decltype(auto) Iter(int arg) {
				if (arg == x) {
					return Caller<x>{}();
				}
				else if constexpr (sizeof...(SubSeq)) {
					return Iter<SubSeq...>(arg);
				}
			}
		};

		template<int Min, int Max, template<int> typename Caller>
		decltype(auto) RunCaseWithInt(int arg) {
			checkf(arg >= Min && arg <= Max, TEXT("arg must be [Min,Max]"));
			return RunCaseWithIntImpl<Caller, MakeRangeIntegerSequence<int,Min,Max>>::Invoke(arg);
		}
		
	}	
}