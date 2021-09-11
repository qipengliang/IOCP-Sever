#include "MysqlConn.h"


BOOL MysqlConn::connectmysql()
{
    MYSQL* conn=mysql_init(&mysql);//初始化mysql
	if (nullptr != mysql_real_connect(&mysql, host.c_str(), user.c_str(), pass.c_str(), DB_name.c_str(), port, NULL, 0))
	{
		std::cout << "mysql connect success" << std::endl;
	}
	else
	{
		std::cout << "mysql connect fail" << std::endl;
		return 0;
	}


    return false;
}

MysqlConn::MysqlConn()
{
	user="root";//用户名
	pass="l3013599";//密码
	host="localhost";//本地ip
	DB_name="namepassword";//数据库名
	port=3306;//默认port
	connectmysql();//连接mysql
}


MysqlConn::~MysqlConn()
{
}

void MysqlConn::Query(MYSQL* mysql,std::string query)
{
	int result = mysql_query(mysql, query.c_str());
	std::cout << "qury result" << std::endl;
}
