#pragma once
#include "pch.h"

template<class T> class Promise;
template<class T> class MultiPromise;
template<class T> class Future;

template<class T>
void print(T str) 
{ 
	std::cout << str << std::endl; 
}


namespace FutureNS
{
	template<class T> class _Promise;
	template<class T> class _Future;

	template<class T>
	class place_holder 
	{
		bool valid;
		int8_t data[sizeof(T)];
	
	public:
		place_holder() 
		: valid(false) 
		{}

		template<class... Args>
		place_holder(Args&&... args) 
		{ 
			set(std::forward<Args>(args)...); 
		}
		
		template<class... Args>
		void set(Args&&... args) 
		{ 
			valid = true; 
			new (data) T(std::forward<Args>(args)...); 
		}
		
		bool is_valid() 
		{ 
			return valid; 
		}

		operator T&& () 
		{
			if (is_valid()) 
			{ 
				return std::move(*(T*)data); 
			}

			throw std::exception("placeholder is empty");
		}

		~place_holder() 
		{ 
			if (is_valid()) 
			{ 
				((T*)data)->~T(); 
			} 
			
			valid = false; 
		}
	};

	template <class T>
	struct shared_state 
		: public shared_state<void> 
	{ 
		place_holder<T> data; 
	};
	
	template<>
	struct shared_state<void>
	{
		std::mutex mutex;
		std::atomic<bool> ready;
		std::exception_ptr exception;
		std::condition_variable cond;
	};

	template<class T>
	class _Future
	{
	protected:
#if THREAD_POOL
		std::shared_ptr<ThreadPool::Barrier> barrier;
		
		_Future() 
		{}
		
		_Future(std::shared_ptr<shared_state<T>>& data) 
		: data(data) 
		{}
		
		void _Destroy() 
		{ 
			if (barrier) 
			{ 
				barrier->sync(); 
				barrier.reset();
			} 
		}
		
		_Future(_Future<T>&& that) 
		{ 
			operator=(std::move(that)); 
		}

#else
		std::shared_ptr< std::thread > thread;
		
		_Future()
		{}
		
		_Future(std::shared_ptr< shared_state< T > >& data) 
		: data(data) 
		{}
	
		void _Destroy() 
		{ 
			if (thread) 
			{ 
				thread->join(); 
				thread.~shared_ptr();
			} 
		}

		_Future(_Future<T>&& that)
		{ 
			operator=(std::move(that)); 
		}

#endif
		std::shared_ptr<shared_state<T>> data;
		
		_Future              (const _Future<T>&) = delete;
		_Future<T>& operator=(const _Future<T>&) = delete;
		
		void _ExceptionHandle() const 
		{ 
			if (data->exception) 
			{ 
				std::rethrow_exception(data->exception);
			} 
		}
		
		void _PointerCheck() const 
		{ 
			if (data == 0) 
			{ 
				throw std::exception("Future is invalid"); 
			} 
		}

	public:

#if THREAD_POOL
		void SetBarrier(std::shared_ptr< ThreadPool::Barrier > _barrier)
		{
			barrier = _barrier;
		}
#else
		void SetThread(std::shared_ptr< std::thread >& _thread)
		{
			thread = _thread;
		}
#endif

		bool operator<(const _Future& that) const 
		{
			return data < that.data; 
		}

		_Future& operator=(_Future<T>&& that)
		{
			that._PointerCheck();
			
			if (data) 
			{ 
				_Destroy(); 
			}

			data = std::move(that.data);

#if THREAD_POOL
			barrier = std::move(that.barrier);
#else
			thread = std::move(that.thread);
#endif
			return *this;
		}

		bool IsReady() const 
		{ 
			_PointerCheck(); 
			return data->ready; 
		}

		operator bool() const
		{ 
			return IsReady(); 
		}

		void Wait() const 
		{
			_PointerCheck();
			
			std::unique_lock<std::mutex> 
				lock(data->mutex);
			
			data->cond.wait(
				lock, 
				[this] 
				{
					return IsReady(); 
				}
			);
			
			_ExceptionHandle();
		}

		~_Future() 
		{ 
			_Destroy(); 
		}
	};

	template<class T>
	class _Promise
	{
	protected:

		std::shared_ptr<shared_state<T>> data;
		
		_Promise           (const _Promise&) = delete;
		_Promise& operator=(const _Promise&) = delete;
		
		bool validSet;
		
		void _Error(const char* str)
		{ 
			SetException(std::make_exception_ptr(std::exception(str))); 
		}

		void _PointerCheck()
		{
			if (!data) 
			{
				throw std::exception("Promise is invalid");
			}
		}

		bool _ReadyCheck()
		{ 
			_PointerCheck(); 
			
			if (validSet) 
			{ 
				return true; 
			}
			else 
			{ 
				_Error("Duplicated setting Promise's value"); 
				return false; 
			}
		}

		void _InValidate()
		{ 
			validSet = false; 
		}

		void _Destroy() 
		{ 
			if (data) 
			{ 
				if (!data->ready && !data.unique()) 
				{ 
					_Error("No hope to hachiko a value"); 
				} 
			} 
		}

	public:
		_Promise() 
		:	validSet(true), 
			data(new shared_state<T>) 
		{ 
			data->ready = false; 
		}

		_Promise<T>(_Promise<T>&& promise)
		{ 
			operator=(std::move(promise)); 
		}

		_Promise<T>& operator=(_Promise<T>&& promise)
		{
			promise._PointerCheck();
			
			if (data) 
			{ 
				_Destroy(); 
			}
			
			validSet = promise.validSet;
			data = std::move(promise.data);
			promise._InValidate();
			
			return *this;
		}

		void SetException(const std::exception_ptr& exception)
		{
			_PointerCheck();
			
			try 
			{ 
				std::rethrow_exception(exception); 
			}
			catch (const std::exception& e)	
			{
				std::lock_guard<std::mutex> lock(data->mutex);
				data->exception = std::make_exception_ptr(
					*(new std::exception(e))
				);
			}

			data->ready = true;
			data->cond.notify_all();
		}

		Future<T> GetFuture() 
		{  
			_PointerCheck(); 
			return Future<T>(data); 
		}

		~_Promise() 
		{ 
			_Destroy(); 
		}
	};
}