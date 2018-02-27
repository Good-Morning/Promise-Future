#include "pch.h"

using Worker = ThreadPool::Worker;
using Barrier = ThreadPool::Barrier;

using Guard = std::lock_guard< std::mutex >;
using Lock = std::unique_lock< std::mutex >;

using ThreadPoolPtr = std::shared_ptr< ThreadPool >;
using WorkerPtr = std::shared_ptr< Worker >;
using BarrierPtr = std::shared_ptr< Barrier >;

ThreadPool::ThreadPool(size_t num_threads)
:	workers(num_threads),
	taskSygnal(false) 
{
	for (auto &i : workers) 
	{
		i = Worker::NewWorker(
			std::move(ThreadPoolPtr(this))
		);
	}
}

WorkerPtr Worker::NewWorker(ThreadPoolPtr&& argument) 
{
	return WorkerPtr(
		new Worker(
			std::move(argument)
		)
	);
}

BarrierPtr Barrier::NewBarrier()
{
	return BarrierPtr(new Barrier);
}

void Worker::waitForCmd() 
{
	parent->cond.wait(
		Lock(parent->taskQueueMutex),

		[this] 
		{
			if (parent->taskSygnal || terminated)
			{
				parent->taskSygnal = false;
				return true;
			}
			return false; 
		}
	);
}

void Worker::routine()
{
	bool waitForJob;
	BarrierPtr barrier;

	{
		Guard guard(parent->taskQueueMutex);
		std::queue< FunctionNode >& queue = parent->taskQueue;

		if (!queue.empty())
		{
			task = queue.front().first;
			barrier = queue.front().second;
			queue.pop();
		}
		else
		{
			task.~function();
			barrier.reset();
		}
		waitForJob = queue.empty();
	}

	if (task)
	{
		task();
		barrier->release();
	}

	if (waitForJob)
	{
		waitForCmd();
	}
}

Worker::Worker(ThreadPoolPtr&& _parent) 
:	parent(std::move(_parent)),
	terminated(false) 
{
	thread = std::thread(
		[this] 
		{
			ConnectPool(std::this_thread::get_id(), parent);
			
			waitForCmd();

			while (!terminated) 
			{
				routine();
			}
			
			//ConnectPool(std::this_thread::get_id(), ThreadPool::_DefaultPool);
		}
	);
}

ThreadPool::~ThreadPool() 
{
	for (auto i : workers) 
	{
		i->Destructor(); 
	}
	cond.notify_all();
	for (auto i : workers)
	{
		i->DestructorWaiter();
	}
}

void Worker::Destructor() 
{
	Guard guard(mutex);
	terminated = true;
}

Worker::~Worker()
{}

BarrierPtr ThreadPool::execute(const std::function<void()>& func)
{
	Guard guard(taskQueueMutex);
	
	auto barrier = Barrier::NewBarrier();
	taskQueue.push(
		FunctionNode{ func, barrier	}
	);
	
	taskSygnal = true;
	cond.notify_one();
	return barrier;
}

void Worker::DestructorWaiter() 
{ 
	thread.join(); 
}

Barrier::Barrier() 
:	ready(false) 
{}

void Barrier::release() 
{ 
	Guard guard(mutex);

	ready = true; 
	cond.notify_one(); 
}

void Barrier::sync() 
{ 
	cond.wait(
		Lock(mutex),

		[this] 
		{ 
			return ready; 
		}
	);
}

std::shared_ptr< ThreadPool >
	ThreadPool::ConnectedPool(std::thread::id ID)
{
	if (_ConnectedPool.find(ID) == _ConnectedPool.end())
	{
		_ConnectedPool[ID] = _DefaultPool;
	}

	return _ConnectedPool[ID];
}

void ThreadPool::ConnectPool(std::thread::id ID, ThreadPoolPtr pool)
{
	_ConnectedPool[ID] = pool;
}

void ThreadPool::buddy()
{
	Lock lock(taskQueueMutex);
	
	while (!taskQueue.empty())
	{ 
		std::function<void()> task;
		BarrierPtr barrier;
		std::tie(task, barrier) = taskQueue.front();
		taskQueue.pop();
		lock.unlock();

		task();
		barrier->release();

		lock.lock();
	}
}

