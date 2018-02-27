#pragma once
#include "pch.h"

class ThreadPool
{
public:
	class Barrier;
	class Worker;

private:
	using FunctionNode = std::pair
	<
		std::function< void() >, 
		std::shared_ptr< Barrier >
	>;

	std::condition_variable cond;
	std::vector< 
		std::shared_ptr< Worker > 
	> workers;
	
	bool taskSygnal;
	std::mutex taskQueueMutex;
	std::queue< FunctionNode > taskQueue;

	static std::map
	<
		std::thread::id, 
		std::shared_ptr< ThreadPool >
	> _ConnectedPool;
	static std::shared_ptr< ThreadPool > _DefaultPool;
public:

	static std::shared_ptr< ThreadPool >
		ConnectedPool(std::thread::id ID);
	static void ConnectPool(
		std::thread::id ID, 
		std::shared_ptr< ThreadPool > pool
	);

	ThreadPool(size_t num_threads);
	std::shared_ptr< Barrier > 
		execute(const std::function< void() >& func);

	void buddy();

	~ThreadPool();
};

class ThreadPool::Barrier
{
	bool ready;
	std::mutex mutex;
	std::condition_variable cond;

public:
	static std::shared_ptr< Barrier > NewBarrier();

	Barrier();
	void release();
	void sync();
};

class ThreadPool::Worker
{
	std::shared_ptr< ThreadPool > parent;
	std::thread thread;
	std::function< void() > task;
	std::mutex mutex;
	std::atomic<bool> terminated;

	void waitForCmd();
	void routine();

public:
	static std::shared_ptr< Worker > 
		NewWorker(std::shared_ptr< ThreadPool >&& argument);

	Worker(std::shared_ptr< ThreadPool >&& parent);
	void Destructor();
	void DestructorWaiter();
	~Worker();

	friend ThreadPool;
};
