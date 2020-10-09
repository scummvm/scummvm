// Some compiler only have partial support for C++11 and we provide replacements for reatures not available.
#ifdef USE_CXX11

#ifdef NO_CXX11_INITIALIZER_LIST
namespace std {
	template<class T> class initializer_list {
	public:
		typedef T value_type;
		typedef const T& reference;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef const T* iterator;
		typedef const T* const_iterator;

		constexpr initializer_list() noexcept = default;
		constexpr size_t size() const noexcept { return m_size; };
		constexpr const T* begin() const noexcept { return m_begin; };
		constexpr const T* end() const noexcept { return m_begin + m_size; }

	private:
		// Note: begin has to be first or the compiler gets very upset
		const T* m_begin = { nullptr };
		size_t m_size = { 0 };

		// The compiler is allowed to call this constructor
		constexpr initializer_list(const T* t, size_t s) noexcept : m_begin(t) , m_size(s) {}
	};

	template<class T> constexpr const T* begin(initializer_list<T> il) noexcept {
		return il.begin();
	}

	template<class T> constexpr const T* end(initializer_list<T> il) noexcept {
		return il.end();
	}
}

#else

#include <initializer_list>

#endif // NO_CXX11_INITIALIZER_LIST

#endif // USE_CXX11
