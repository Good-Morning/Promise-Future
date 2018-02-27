#pragma once
#include "pch.h"

template< 
	class IteratorType, 
	class Function, 
	class Checker = std::result_of_t<
		Function(IteratorType)
	>
>
void Parallel(
	IteratorType begin, 
	IteratorType end, 
	std::shared_ptr< ThreadPool > tp, 
	Function functor)
{
	for (IteratorType iterator = begin; iterator != end; ++iterator)
	{
		tp->execute(
			[iterator, functor]
			{
				functor(iterator);
			}
		);
		tp->buddy();
	}
}