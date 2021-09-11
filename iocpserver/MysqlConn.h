#pragma once
#include<string>
#include<mysql.h>
#include<iostream>
class MysqlConn
{
private:
	std::string user;
	std::string pass;
	std::string host;
	std::string DB_name;
	int port;
	BOOL connectmysql();

public:
	MYSQL mysql;
	static void Query(MYSQL* mysql,std::string query);
	MysqlConn();
	~MysqlConn();
};

