#pragma once
//����Ϊ��ʹ��
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
	template<class Fn,class ...Arg>//�����м���ģ�庯���ǿ��Եġ�д��.h�ļ���
    //����...ArgΪ�䳤����ģ�壬��Ϊ���Բ����������ȣ�֧��ƫ�ػ���������ʼ�
	auto task_ennqueue(Fn && fn, Arg && ...arg)->std::future<decltype(fn(arg...))>//&&��Ϊ��ֵ���ã�decltype��Ϊ��÷���ֵ���ͣ�arg...Ϊ�䳤����
	//auto Ϊ��ϵͳ���㷵��ֵ���Զ���䣬->����ָ���������ͣ�����������ϣ�������߳��еõ�ĳһ����ķ���ֵ��Ϊ�첽��������ʹ��future������Ϊ��������ֵ
	{
		using result = decltype(fn(arg...));
		auto task = std::make_shared<std::packaged_task<result()>>(std::bind(std::forward<Fn>(fn), std::forward<Arg>(arg)...));
		//make_sharedΪ��������ָ�룬ָ��ָ��new�����Ŀɵ��ö���������������ķ���ֵ�Ͳ��������Ŀɵ��ö�������ʹ��auto��
		//make_shared<T>()�������Խ������10��������Ȼ������Ǵ��ݸ�����T�Ĺ��캯��������һ��shared_ptr<T>�Ķ��󲢷���,�����еĲ���Ϊһ���ɵ��ö���ʹ��bind��������������packeged_task��
        //packeged_task����promise������úͻ������ƣ���������¼�����constom�������û�ͨ��һ���ɵ��ö����ơ�
		//forward���ô����ʼ�
		std::future<result> fu = task->get_future();//��future��packeged_task
		std::unique_lock<std::mutex> lck1(que);
		taskqueue.emplace([task](){(*task)(); });//������ת��Ϊ����ָ���д��lambda��������ָ���Ӧ�ĺ����ѱ�make_shared�����������Σ�������lambdaΪvoid()�͵ģ��������queue
		lck1.unlock();
		cond.notify_one();
		std::cout << "inter" << std::endl;
		
		return fu;
	}//����˼·Ϊ���ò����½��������Ŀɵ��ö����ڽ��������void�����͵�lambda�����У�����queue
	TPool(int numofthread);//���켴����
	virtual ~TPool();
};

