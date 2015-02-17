/**
	$ Lexical   (C) 2005-2013 MF
	$ main.cpp
*/


#include "class.h"



int main (int argc, char *argv[]) {
	// 使用标准输入流创建对象
	lexical lex (stdin);

	// 读入并转换
	if(lex.read() == false) {
		// 转换失败, 输出换行
		cout << endl << endl << endl;

		// 暂停程序让用户可以清晰看到错误
		system("pause");

		// 停止程序
		return 0;
	}

	// 输出提示
	cout << endl << endl << "状态转换矩阵：" << endl;

	// 输出矩阵
	lex.output();
	// 进入匹配模式
	lex.match();

	return 0;
}

