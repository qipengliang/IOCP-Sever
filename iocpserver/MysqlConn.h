#pragma once
#include<string>
//#include<mysql.h>
class MysqlConn
{
private:
	//MYSQL mysql;
	std::string user;
	std::string pass;
	bool connectmysql();

public:
	MysqlConn();
	~MysqlConn();
};

