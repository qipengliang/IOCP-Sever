#include "MysqlConn.h"


BOOL MysqlConn::connectmysql()
{
    MYSQL* conn=mysql_init(&mysql);//��ʼ��mysql
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
	user="root";//�û���
	pass="l3013599";//����
	host="localhost";//����ip
	DB_name="namepassword";//���ݿ���
	port=3306;//Ĭ��port
	connectmysql();//����mysql
}


MysqlConn::~MysqlConn()
{
}

void MysqlConn::Query(MYSQL* mysql,std::string query)
{
	int result = mysql_query(mysql, query.c_str());
	std::cout << "qury result" << std::endl;
}
