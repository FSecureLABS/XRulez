#pragma once

namespace CppTools
{
	/// Smart pointer for objects that needs to call Release() method on destruction.
	/// Usage: UniqueReleasePtr<IMsgStore> p; WinApiGetMessageStore(0, &p); DoSomethingWithStoreThenAutomaticReleseIsCalledOnScopeEnd();
	template <typename Interface> struct UniqueReleasePtr : public PointerObject<UniqueReleasePtr<Interface>, Interface>
	{
		/// Public ctor.
		/// @param pointer initial value of underlying pointer.
		UniqueReleasePtr(Interface* pointer = nullptr) : PointerObject(pointer) { }

		/// Move constructor.
		/// @param source object to move from.
		UniqueReleasePtr(UniqueReleasePtr&& source) : PointerObject(source.m_Pointer) { source.m_Pointer = nullptr; }

		/// Move assignment operator.
		/// @param source object to copy from.
		UniqueReleasePtr& operator = (UniqueReleasePtr&& source) { return swap(source), *this; }

		/// Destructor.
		~UniqueReleasePtr() { Reset(); }

		/// Resets state of this pointer object.
		/// @param newValue value to assign to this pointer object.
		void Reset(Interface* newValue = nullptr)
		{
			if (!m_Pointer)
				return;

			// Use a third variable to ensure that object leaves in a valid state whatever happens.
			Interface* ptr = m_Pointer;
			m_Pointer = newValue;
			ptr->Release();
		}
	};
}
