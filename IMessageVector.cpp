#include <algorithm>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename IMsg>
class IMessageVector {
	public:
		using size_type = typename std::vector<int>::size_type;
		using predicateI = std::function<bool(const IMsg&)>;

		template <typename Return, typename Cont, bool reverse>
		class Iterator final : public std::iterator<std::random_access_iterator_tag, Return, int> {
			public:
				using reference = typename std::iterator<std::random_access_iterator_tag, Return, int>::reference;
				using pointer = typename std::iterator<std::random_access_iterator_tag, Return, int>::pointer;

				Iterator() = delete;
				Iterator(Cont& container, int offset) : vp{&container}, index{offset} {}
				Iterator(const Iterator<Return, Cont, reverse>&) = default;
				Iterator(Iterator<Return, Cont, reverse>&&) = default;
				Iterator& operator = (const Iterator<Return, Cont, reverse>&) = default;
				Iterator& operator = (Iterator<Return, Cont, reverse>&&) = default;
				~Iterator() = default;
				pointer operator -> () const { return &((*vp)[index]); }
				reference operator * () const { return (*vp)[index]; }
				Iterator& operator += (int offset);
				Iterator& operator -= (int offset) { return (*this) += -offset; }
				Iterator& operator ++ () { return (*this) += 1; }
				Iterator& operator -- () { return (*this) -= 1; }
				bool operator == (const Iterator<Return, Cont, reverse>& rhs) const;
				bool operator != (const Iterator<Return, Cont, reverse>& rhs) const { return !(*this == rhs); }

			private:
				Cont* vp;
				int index;
		};

		using iterator = Iterator<IMsg, IMessageVector<IMsg>, false>;
		using reverse_iterator = Iterator<IMsg, IMessageVector<IMsg>, true>;
		using const_iterator = Iterator<const IMsg, const IMessageVector<IMsg>, false>;

		virtual ~IMessageVector() = default;

		virtual size_type size() const = 0;
		virtual size_type capacity() const = 0;
		virtual bool empty() const = 0;
		virtual void clear() = 0;
		virtual void reserve(const size_type numOfElements) = 0;

		virtual IMsg& operator [] (size_type idx) = 0; 
		virtual const IMsg& operator [] (size_type idx) const = 0;
		virtual void push_back(const IMsg& rhs) = 0;
		virtual void push_back(IMsg&& rhs) = 0;
		virtual IMsg& find_if(const predicateI&); 
		//virtual const IMsg& find_if(const predicateI&) const;

		iterator begin() { return iterator(*this, 0); }
		iterator end() { return iterator(*this, size()); }
		const_iterator cbegin() const { return const_iterator(*this, 0); }
		const_iterator cend() const { return const_iterator(*this, size());}
		const_iterator begin() const { return cbegin(); }
		const_iterator end() const { return cend(); }
};

template <typename IMsg, typename DerivedMsg>
class MessageVector final : public IMessageVector<IMsg> {
	public:
		static_assert(std::is_base_of<IMsg, DerivedMsg>::value, "DerivedMsg not derived from IMsg");

		using size_type = typename std::vector<int>::size_type;
		using predicateI = typename IMessageVector<IMsg>::predicateI;
		using predicateD = std::function<bool(const DerivedMsg&)>;

		MessageVector() = default;
		~MessageVector() override = default;
		explicit MessageVector(const std::vector<DerivedMsg>& rhs) : data(rhs) {}
		explicit MessageVector(std::vector<DerivedMsg>&& rhs) : data(rhs) {}

		void assign(const IMessageVector<IMsg>& rhs);

		size_type size() const override { return data.size(); }
		size_type capacity() const override { return data.capacity(); }
		bool empty() const override { return data.empty(); }
		void clear() override { return data.clear(); }
		void reserve(const size_type numOfElements) override { return data.reserve(numOfElements); }

		DerivedMsg& operator [] (size_type idx) override { return data[idx]; }
		const DerivedMsg& operator [] (size_type idx) const override { return data[idx]; }
		void push_back(const IMsg& rhs) override { data.push_back(static_cast<const DerivedMsg&>(rhs)); }
		void push_back(IMsg&& rhs) override { data.push_back(static_cast<DerivedMsg&>(rhs)); }

		std::vector<DerivedMsg> data;
};


//--------------------------------------------------------------------
//---- inline definitions IMessageVector<IMsg>::Iterator-------------
//--------------------------------------------------------------------

template <typename IMsg>
template <typename Return, typename Cont, bool reverse>
inline IMessageVector<IMsg>::Iterator<Return, Cont, reverse>& IMessageVector<IMsg>::Iterator<Return, Cont, reverse>::
operator += (int offset) {
	if (reverse) {
		offset = -offset;
	}
	this->index++;
	return *this;
}

template <typename IMsg>
template <typename Return, typename Cont, bool reverse>
inline bool IMessageVector<IMsg>::Iterator<Return, Cont, reverse>::
operator == (const Iterator<Return, Cont, reverse>& rhs) const {
	if (this->vp != rhs.vp) {
		return false;
	}
	return (this->index == rhs.index);
}

//--------------------------------------------------------------------
//---- inline definitions IMessageVector<IMsg>-----------------------
//--------------------------------------------------------------------


template<typename IMsg>
inline IMsg& IMessageVector<IMsg>::find_if(const predicateI& pred) {
	const auto endIT = cend();
	const auto it = std::find_if(cbegin(), endIT, pred); 
	if ( it == endIT ) {
		throw std::runtime_error("Element not found");
	}
	return *it;
}

#if 0
template<typename IMsg>
inline const IMsg& IMessageVector<IMsg>::find_if(const predicateI& pred) const {
	const auto it = std::find_if(begin(), end(), pred);
	if ( it == end() ) {
		throw std::runtime_error("Element not found");
	}
	return *it;
}

#endif
//--------------------------------------------------------------------
//---- inline definitions MessageVector<IMsg, DerivedMsg>-------------
//--------------------------------------------------------------------
template<typename IMsg, typename DerivedMsg>
inline void MessageVector<IMsg, DerivedMsg>::assign(const IMessageVector<IMsg>& rhs) {
	try {
		data = dynamic_cast<const MessageVector<IMsg, DerivedMsg>&>(rhs).data;
	}
	catch(std::bad_cast& e) {
		std::vector<DerivedMsg> v;
		v.reserve(rhs.size());
		for (const auto& element : rhs) {
			v.push_back(static_cast<const DerivedMsg&>(element));
		}
		data = std::move(v);
	}
}

class A { 
	public:
		A() = default;
		virtual ~A() = default;
		int value;
};
 
class B : public A {
	public:
		B() = default;
		~B() = default;
};

int main() {
	
	MessageVector<A, B> vec;
	B b1, b2;
	b2.value = 1;

	vec.push_back(b1);
	vec.push_back(b2);
	vec.size();
}