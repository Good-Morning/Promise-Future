#include "pch.h"

std::mutex cout_mutex;
std::map< std::thread::id, std::shared_ptr< ThreadPool > > ThreadPool::_ConnectedPool;
std::shared_ptr< ThreadPool > ThreadPool::_DefaultPool(new ThreadPool(4));
