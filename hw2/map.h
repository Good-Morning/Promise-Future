#pragma once
#include "pch.h"

template<class T, class Function>
Future<	std::result_of_t< Function(T) > >
	Map(Future< T > future, Function functor)
{
	auto storageF = std::shared_ptr< Future< T > >(
		new Future< T >(std::move(future))
	);
	auto storageFn = std::shared_ptr< Function >(
		new Function(std::move(functor))
	);

	return MakeFuture(
		[storageF, storageFn]
		{
			return (*storageFn)(storageF->Get());
		}
	);
}