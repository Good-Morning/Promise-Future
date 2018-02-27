#pragma once
#include "pch.h"

template< class T >
struct collapse 
{ 
	using type = T; 
};

template< class T >
struct collapse< Future< T > > 
{ 
	using type = T; 
};

template< class T >
using collapse_t = typename collapse< T >::type;

template< class T >
struct collapse< Future< Future< T > > > 
{ 
	using type = collapse_t< Future< T > >; 
};

template<class Any>
Any _PlanarFlatten(Any x) { return x; }

template< class T >
T _PlanarFlatten(Future< T > future) 
{ 
	return future.Get(); 
}

template< class T >
collapse_t< Future< T > > 
	_PlanarFlatten(Future< Future< T > > future) 
{
	return _PlanarFlatten(
		std::move(future.Get())
	);
}

template< class T >
Future< collapse_t< Future< T > > >
	Flatten(Future< T > future)
{
	return MakeFuture(
		[] (Future< T > future)
		{
			return _PlanarFlatten(
				std::move(future)
			); 
		},

		std::move(future)
	);
}

#include "BasicCollectionMethods.h"

template< 
	template<class...> class Collection, 
	class T, 
	class... U, 
	class Checker = std::enable_if_t<
		!std::is_same_v<
			Collection< Future< T >, U... >,
			Future    < Future< T > >
		>
		&&
		!std::is_same_v<
			Collection< Future< T >, U... >,
			std::tuple< Future< T >, U... >
		>,
		
		int
	> 
> 
Future< collapse_t< Collection< Future< T >, U... > > > 
	Flatten(Collection< Future< T >, U... > a) 
{ 
	return MakeFuture( 
		[] (Collection< Future< T >, U... > collection)
		{ 
			collapse_t< Collection< Future< T >, U... > > res; 
			int size = collection.size();
			for (int count = 0; !collection.empty() && count != size; count++)
			{
				_AddingMethod(
					res,
					std::move(
						_ExtractingMethod(collection, count)
					),
					count
				);
			}
			
			return _ReverseIfNeedsTo(std::move(res)); 
		},

		std::move(a)
	); 
}

template<class T, class Key, class... U> 
Future< std::map< Key, T > > 
	Flatten(std::map< Key, Future< T >, U... > a) 
{
	auto storage = std::shared_ptr< std::map< Key, Future< T > > >(
		new std::map< Key, Future< T > >(
			std::move(a)
		));

	return MakeFuture(
		[storage] 
		{ 
			std::map< Key, T > res; 
			for (auto& i : *storage) 
			{ 
				res[i.first] = std::move(i.second.Get()); 
			} 
			
			return res; 
		}
	); 
}


#if FLATTEN_TUPLE

template< class Tuple, size_t... I >
auto _Flatten(Tuple tuple, std::index_sequence< I... >)
{
	return std::make_tuple(
		_PlanarFlatten(std::move(
			std::get< I >(tuple)
		))...
	);
}

template< class... Args >
auto Flatten(std::tuple< Args... > tuple)
{
	return MakeFuture(
		[](std::tuple< Args... > tuple)
		{
			return _Flatten(
				std::move(tuple), 
				std::make_index_sequence<
					std::tuple_size_v<std::tuple< Args... >>
				>()
			);
		},

		std::move(tuple)
	);
}

#endif

