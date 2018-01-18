#pragma once

namespace CppTools
{
	/// WARNING - due to BUG in MS Visual Studio (https://stackoverflow.com/questions/12701469/why-empty-base-class-optimization-is-not-working#12714226) multiple inheritance that includes this
	///  class increases size of the child class.
	/// CRTP for classes that represent non copyable objects. Why this is CRTP? See: https://en.wikibooks.org/wiki/Talk:More_C%2B%2B_Idioms/Non-copyable_Mixin.
	/// Usage: class Type : NonCopyable<Type> { };
	template <typename CrtpType> class NonCopyable
	{
		/// Deleted copy constructor.
		NonCopyable(const NonCopyable&) = delete;

		/// Deleted copy assignment operator.
		CrtpType& operator = (CrtpType const& other) = delete;

	protected:
		/// Protected default ctor.
		NonCopyable() { }

		/// Protected non-virtual destructor.
		~NonCopyable() noexcept { }
	};
}
