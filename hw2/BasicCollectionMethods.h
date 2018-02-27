#pragma once

#define ADDING_METHOD(Collection, Method)                     \
template< class T >                                           \
void _AddingMethod(std::Collection< T >& vec, T&& value, int) \
{                                                             \
	vec.Method(std::forward<T>(value));                       \
}

template< class T, int N >
void _AddingMethod(std::array< T, N >& vec, T&& value, int ind)
{
	vec[ind] = std::move(value);
}

ADDING_METHOD(vector, push_back);
ADDING_METHOD(list, push_back);
ADDING_METHOD(deque, push_back);

ADDING_METHOD(queue, push);
ADDING_METHOD(stack, push);

ADDING_METHOD(set, insert);
ADDING_METHOD(multiset, insert);

ADDING_METHOD(forward_list, push_front);



template< class Collection >
Collection _ReverseIfNeedsTo(Collection col)
{
	return col;
}

template< class T >
std::forward_list< T > _ReverseIfNeedsTo(std::forward_list< T > col)
{
	std::forward_list< T > res;
	for (auto a : col) {
		res.push_front(std::move(col.front));
		col.pop_front();
	}
	return res;
}

template< class T >
std::stack< T > _ReverseIfNeedsTo(std::stack< T > col)
{
	std::stack< T > res;
	for (; !col.empty(); col.pop()) {
		res.push(std::move(col.top()));
	}
	return res;
}



template< class T >
T _ExtractingMethod(std::vector< Future< T > >& vec, int count)
{
	return vec[count].Get();
}

template< class T >
T _ExtractingMethod(std::list< Future< T > >& vec, int)
{
	T temp = std::move(vec.front().Get());
	vec.pop_front();
	return temp;
}

template< class T >
T _ExtractingMethod(std::forward_list< Future< T > >& vec, int)
{
	T temp = std::move(vec.front().Get());
	vec.pop_front();
	return temp;
}

template< class T >
T _ExtractingMethod(std::deque< Future< T > >& vec, int)
{
	T temp = std::move(vec.front().Get());
	vec.pop_front();
	return temp;
}

template< class T >
T _ExtractingMethod(std::queue< Future< T > >& vec, int)
{
	T temp = std::move(vec.back().Get());
	vec.pop();
	return temp;
}

template< class T >
T _ExtractingMethod(std::stack< Future< T > >& vec, int)
{
	T temp = std::move(vec.top().Get());
	vec.pop();
	return temp;
}

template< class T, int N >
T _ExtractingMethod(std::array< Future< T >, N >& vec, int ind)
{
	return vec[ind].Get();
}

template< class T >
T _ExtractingMethod(std::set< Future< T > >& vec, int)
{
	T temp = std::move(vec.begin()->Get());
	vec.erase(vec.begin());
	return temp;
}

template< class T >
T _ExtractingMethod(std::multiset< Future< T > >& vec, int)
{
	T temp = std::move(vec.begin()->Get());
	vec.erase(vec.begin());
	return temp;
}




template< template< class... > class Collection, class T, class... U >
struct collapse< Collection< Future< T >, U... > >
{
	using type = Collection< T >;
};

template< class T, int N >
struct collapse< std::array< Future< T >, N > >
{
	using type = std::array< T, N >;
};
