#pragma once
#include "pch.h"

template< class T >
class Future 
	: public FutureNS::_Future< T >
{
protected:
	Future(std::shared_ptr< FutureNS::shared_state< T > >& data) 
	: FutureNS::_Future< T >(data) 
	{}

public:

	Future(Future&& that) 
	: FutureNS::_Future< T >(
		std::move(
			(FutureNS::_Future< T >&&)that
		)
	  ) 
	{}

	Future& operator=(Future&& that) 
	{ 
		FutureNS::_Future< T >::operator=(
			std::move(
				(FutureNS::_Future< T >&&)that
			)
		); 

		return *this; 
	}

	T&& Get() const 
	{ 
		Wait(); 
		_ExceptionHandle(); 
		return std::forward< T >(data->data); 
	}

	friend FutureNS::_Promise< T >;
};

template<>
class Future< void > 
	: public FutureNS::_Future< void >
{
	Future(std::shared_ptr< FutureNS::shared_state< void > >& data)
	: FutureNS::_Future< void >(data) 
	{}

public:

	Future(Future&& that) 
	: FutureNS::_Future< void >(
		std::move(
			(FutureNS::_Future< void >&&)that
		)
	  ) 
	{}
	
	Future& operator=(Future&& that) 
	{ 
		FutureNS::_Future< void >::operator=(
			std::move(
				(FutureNS::_Future< void >&&)that
			)
		); 
		return *this; 
	}

	void Get() const 
	{ 
		Wait(); 
		_ExceptionHandle(); 
	}

	friend FutureNS::_Promise< void >;
};

template< class T >
class Future< T& > 
	: public Future< T* >
{
	Future(std::shared_ptr< FutureNS::shared_state< T* > >& data) 
	: Future< T* >(data) 
	{}

public:

	Future(Future&& that) 
	: FutureNS::_Future< T* >(
		std::move(
			(FutureNS::_Future< T >&&)that
		)
	  ) 
	{}

	Future& operator=(Future&& that) 
	{ 
		FutureNS::_Future< T* >::operator=(
			std::move(
				(FutureNS::_Future< T >&&)that
			)
		); 
		
		return *this; 
	}
	
	T& Get() const 
	{ 
		return *Future<T*>::Get(); 
	}

	friend MultiPromise< T& >;
};

template< class T >
class MultiPromise 
	: public FutureNS::_Promise< T >
{
public:

	template<class... Args>
	void Set(Args&&... args)
	{
		if (_ReadyCheck()) 
		{
			std::unique_lock< std::mutex > lock(data->mutex);
			data->data.set(std::forward< Args >(args)...);
		}
		validSet = false;

		data->ready = true;
		data->cond.notify_all();
	}
};

template<>
class MultiPromise< void > 
	: public FutureNS::_Promise< void >
{
public:

	void Set()
	{
		validSet = false;
		data->ready = true;
		data->cond.notify_all();
	}
};

template< class T >
class MultiPromise< T& > 
	: public MultiPromise< T* >
{
public:

	void Set(T& arg) 
	{ 
		MultiPromise< T* >::Set(&arg); 
	}

	Future< T& > GetFuture() 
	{ 
		_PointerCheck();
		return Future< T& >(data); 
	}
};

template< class T >
class Promise 
	: public MultiPromise< T > 
{
	bool validGetFuture;
	
	void _InValidate() 
	{ 
		MultiPromise< T >::_InValidate(); 
		validGetFuture = false; 
	}

public:

	Promise() = default;
	Promise(Promise&& that) 
	{
		operator=(std::move(that)); 
	}

	Promise& operator=(Promise&& that) 
	{ 
		MultiPromise< T >::operator=(std::move(that));
		validGetFuture = that.validGetFuture; 
		that._InValidate(); 
		return *this; 
	}

	Future< T > GetFuture() {
		if (!validGetFuture) 
		{ 
			throw std::exception("Duplicated getting Future"); 
		}
		
		validGetFuture = false;
		return MultiPromise< T >::GetFuture();
	}
};


#if THREAD_POOL

template< class T, class Functor, class... Args >
void _PromiseInvoke(Promise< T >& promise, Functor function, std::tuple<Args...> args)
{
	promise.Set(
		std::forward< T >(
			apply(function, std::move(args))
		)
	);
}

template< class Functor, class... Args >
void _PromiseInvoke(Promise< void >& promise, Functor function, std::tuple<Args...> args) 
{
	apply(function, std::move(args));
	promise.Set();
}

template< class F, class Tuple, std::size_t... I >
constexpr decltype(auto) 
	_Apply(F&& f, Tuple t, std::index_sequence< I... >)
{
	return std::invoke(
		std::forward< F >(f), 
		
		std::get< I >(
			std::forward< Tuple >(t)
		)...
	);
}

template<class F, class Tuple>
constexpr decltype(auto) 
	apply(F&& f, Tuple t)
{
	return _Apply(
		std::forward< F >(f), 
		std::forward< Tuple >(t),
		std::make_index_sequence<
			std::tuple_size_v< Tuple >
		>()
	);
}

template<
	class Functor, 
	class... Args, 
	class ReturnType = std::result_of_t< Functor(Args...) >
>
Future< ReturnType > MakeFuture(Functor functor, Args... args) 
{
	Promise< ReturnType > promise;
	Future< ReturnType > future = std::move(promise.GetFuture());
	
	auto ThreadID = std::this_thread::get_id();
	std::shared_ptr< ThreadPool > 
		pool = ThreadPool::ConnectedPool(ThreadID);
	

	auto _args = std::shared_ptr< std::tuple< Args... > >(
		new std::tuple<	Args... >(
			std::forward< Args >(args)...
		));

	auto pr = std::shared_ptr< Promise< ReturnType > >(
		new Promise< ReturnType >(
			std::move(promise)
		));

	future.SetBarrier(
		pool->execute(
			[functor, pr, _args] 
			{
				try {
					_PromiseInvoke(std::move(*pr), functor, std::move(*_args));
				}
				catch (...) 
				{ 
					pr->SetException(std::current_exception()); 
				}
			}
		) 
	);
	return future;
}
#else

template< class T, class Functor, class... Args >
void _PromiseInvoke(Promise< T >& promise, Functor function, Args... args)
{
	promise.Set(
		std::forward< T >(
			function(std::forward<Args>(args)...)
		)
	);
}

template< class Functor, class... Args >
void _PromiseInvoke(Promise< void >& promise, Functor function, Args... args)
{
	function(std::forward<Args>(args)...);
	promise.Set();
}

template<
	class Functor, 
	class... Args,
	class ReturnType = std::result_of_t< Functor(Args...) >
>
Future< ReturnType > MakeFuture(Functor functor, Args... args)
{
	Promise< ReturnType > promise;
	Future< ReturnType > future = promise.GetFuture();
	future.SetThread(
		std::shared_ptr< std::thread >(
			new std::thread(
				[](Functor functor, Promise< ReturnType > promise, Args... args)
				{
					try 
					{ 
						_PromiseInvoke(
							promise, 
							std::forward<Functor>(functor), 
							std::forward<Args>(args)...
						);
					}
					catch (...) 
					{ 
						promise.SetException(std::current_exception()); 
					}
				},

				std::forward< Functor >(functor), 
				std::move(promise), 
				std::forward< Args >(args)...
			)
		)
	);
	return future;
}


#endif








