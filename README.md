# IOCP-服务器
step1:利用windows的IOCP机制的服务端,实现客户端到服务器的高并发连接发送和接受数据  
step2：实现p2p功能，客户端连接服务器后可不经过服务器实现端对端传输  
step3:添加webserver功能  

参考：https://blog.csdn.net/PiggyXP/article/details/6922277?spm=1001.2014.3001.5501  
https://github.com/gaojs/IOCPServer

# 更新日志
2021-08-05 实现iocp并发服务器于客户端的基本通讯和并发（未测试）  
2021-08-15 实现web服务器的基本功能改造，即向网页发送html，但存在最后发送的文件在closesocket后丢失的问题，暂时用sleep解决，原因待查。  
2021-09-03 实现了web服务器的get功能  
2021-09-06 增加vector<Socontext*> solist 管理创建的socket上下文资源，并添加了必要的注释  
2021-09-10 修复了一部分内存泄漏的问题，将CreateIoCompletionPort中(DWORD)listen_socket_context改为(ULONG_PTR)listen_socket_context以满足32位和63位环境。添加了表单信息提交模块，将提交的表单信息写入mysql数据库  
2021-9-11 添加了数据库传输模块，当接受用户名和密码后，将查询事件放入线程池，查询语句用store procedure实现。前提，使用mysql创建名为userpassword的数据库，并创建名为user的表。

