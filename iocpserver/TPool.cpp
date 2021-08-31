#include "TPool.h"
#include<thread>
#include<vector>
#include<queue>
#include<functional>
#include<iostream>
#include<mutex>
#include<condition_variable>
#include<future>

TPool::TPool(int numofthread) :max_num_thread(numofthread), stopflag(1)
{
	//�����̣߳���ʼ�������б�Ϊ�����񣨹����̣߳�
	for (int i = 1; i <= max_num_thread; i++)
	{
		if (taskqueue.empty() == 1)
			pool.push_back(std::thread(std::bind(&TPool::emptytask,this), this));
		else
		{
			Tasks task = take_task_from_queue();
			taskqueue.pop();
			pool.push_back(std::thread(task));
		}
	}
}

TPool::Tasks TPool::take_task_from_queue()
{
	Tasks task;
	if (taskqueue.empty() == 1)
	{
		task = std::bind(&TPool::emptytask,this);
		std::cout << "����Ϊ��" << std::endl;
	}
	else
	{
		task = taskqueue.front();
		taskqueue.pop();
	}
	return task;
}

void TPool::emptytask()
{
	Tasks task;
	while (stopflag)
	{
		std::unique_lock<std::mutex> lck1(que);
		//ʹ�̹߳���
		while (taskqueue.empty() == 1)
		{
			std::cout << "thread.id=" << std::this_thread::get_id() << "����" << std::endl;
			cond.wait(lck1);
			std::cout << "thread.id=" << std::this_thread::get_id() << "wakeup" << std::endl;
		}
		task = taskqueue.front();
		taskqueue.pop();
		lck1.unlock();
		std::cout << "task" << std::endl;
		task();
	}
}


void TPool::stop()
{
	stopflag=0;
}

TPool::~TPool()
{
	for (std::thread &worker : pool)
		worker.join();
}
