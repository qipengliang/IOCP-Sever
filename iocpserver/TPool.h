#pragma once
//可作为库使用
#include<thread>
#include<vector>
#include<queue>
#include<functional>
#include<mutex>
#include<condition_variable>
#include<future>
#include<winsock2.h>
class TPool
{
	using Tasks = std::function<void()>;
private:
	int max_num_thread;
	std::vector<std::thread> pool;
	std::queue<Tasks> taskqueue;
	std::condition_variable cond;
	std::mutex que;
	Tasks take_task_from_queue();
	void emptytask();
	bool stopflag;
public:
	void stop();
	template<class Fn,class ...Arg>//在类中加入模板函数是可以的。写在.h文件中
    //其中...Arg为变长参数模板，意为可以不定参数长度，支持偏特化，具体见笔记
	auto task_ennqueue(Fn && fn, Arg && ...arg)->std::future<decltype(fn(arg...))>//&&意为右值引用，decltype意为获得返回值类型，arg...为变长参数
	//auto 为有系统推算返回值，自动填充，->用来指定返回类型，本例中由于希望在主线程中得到某一任务的返回值，为异步任务，所以使用future类型作为函数返回值
	{
		using result = decltype(fn(arg...));
		auto task = std::make_shared<std::packaged_task<result()>>(std::bind(std::forward<Fn>(fn), std::forward<Arg>(arg)...));
		//make_shared为建立智能指针，指针指向new出来的可调用对象，类型是由任务的返回值和参数决定的可调用对象，所以使用auto。
		//make_shared<T>()函数可以接受最多10个参数，然后把它们传递给类型T的构造函数，创建一个shared_ptr<T>的对象并返回,本例中的参数为一个可调用对象（使用bind创立）用来构造packeged_task类
        //packeged_task类与promise类的作用和机制相似，但处理的事件更加constom，允许用户通过一个可调用对象定制。
		//forward的用处见笔记
		std::future<result> fu = task->get_future();//绑定future和packeged_task
		std::unique_lock<std::mutex> lck1(que);
		taskqueue.emplace([task](){(*task)(); });//将任务转化为函数指针后写入lambda函数，此指针对应的函数已被make_shared特例化（传参），所以lambda为void()型的，允许放入queue
		lck1.unlock();
		cond.notify_one();
		std::cout << "inter" << std::endl;
		
		return fu;
	}//整体思路为利用参数新建特例化的可调用对象，在将其包裹在void（）型的lambda函数中，送入queue
	TPool(int numofthread);//构造即启动
	virtual ~TPool();
};

