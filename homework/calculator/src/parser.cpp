/**
	$ Calculator   (C) 2005-2014
	$ parser.cpp
*/


#include "base.h"



/* 下推栈 */
std::vector <node_t *> *	parser::stack	= NULL;





/* 移进 */
void parser::shift (char symbol, double value) {
	// 分配新的节点
	node_t *	node	= new node_t;

	// 保存节点数据
	node->symbol	= symbol;
	node->value		= value;

	// 节点进栈
	parser::stack->push_back (node);
}



/* 规约 */
bool parser::reduce (void) {
	// 必须有不少于 3 个节点进行规约
	if(parser::stack->size ( ) < 3) {
		return false;
	}

	// 依次获取 3 个元素
	node_t *	node_1	= parser::stack->back ( );		parser::stack->pop_back ( );
	node_t *	node_2	= parser::stack->back ( );		parser::stack->pop_back ( );
	node_t *	node_3	= parser::stack->back ( );		parser::stack->pop_back ( );

	// S -> (S)
	if((node_3->symbol == '(') && (node_2->symbol == 0) && (node_1->symbol == ')')) {
		parser::shift (0, node_2->value);

		return true;
	}

	// S -> S op S
	if((node_3->symbol == 0) && (node_1->symbol == 0)) {
		switch(node_2->symbol) {
			case '+':	parser::shift (0, node_3->value + node_1->value);	break;
			case '-':	parser::shift (0, node_3->value - node_1->value);	break;
			case '*':	parser::shift (0, node_3->value * node_1->value);	break;

			case '/':
				if(node_1->value == 0) {
					return false;
				} else {
					parser::shift (0, node_3->value / node_1->value);
				}
				break;

			default:	return false;
		}

		return true;
	}

	return false;
}




/* 执行计算 */
double parser::execute (int *error) {
	// 初始化错误
	* error	= -1;

	// 循环读取单词
	while(true) {
		// 运算符
		char	symbol	= 0;
		// 数值
		double	value	= 0.;

		// 读取单词
		if(lexical::next (& symbol, & value, error) == false) {
			// 判断是否由错误导致的结束
			if(* error != -1) {
				// 词法错误
				(* error) ++;

				// 返回失败
				return 0.;
			}

			goto found_symbol;
		}

		// 判断当前是否读到运算符
		if(symbol != 0) {
			goto found_symbol;
		}

		// 数值直接进栈, 移进
		parser::shift (symbol, value);

		continue;


found_symbol:
		while(true) {
			// 栈顶符号
			char	symbol_top	= parser::top ( );

			// 如果栈顶无运算符, 不需要规约
			if(symbol_top == 0) {
				break;
			}

			// 根据符号优先级判断是否需要规约
			if(parser::priority (symbol_top, symbol) != '>') {
				break;
			}

			// 如果需要规约, 执行规约
			if(parser::reduce ( ) == false) {
				goto failed;
			}
		}

		if(symbol == 0) {
			// 如果读取完毕, 结束循环
			break;
		} else {
			// 如果没有读取完毕, 移进运算符
			parser::shift (symbol, value);
		}
	}

	// 栈中此时应该只剩下一个元素
	if(parser::stack->size ( ) == 1) {
		// 直接返回结果
		return parser::stack->back ( )->value;
	}


failed:
	// 语法错误
	* error	= 0;

	// 返回失败
	return 0.;
}




/* 栈顶运算符 */
char parser::top (void) {
	// 遍历起点
	int	i	= parser::stack->size ( ) - 1;

	// 遍历栈
	while(i >= 0) {
		// 遍历节点
		node_t *	node	= parser::stack->at (i);

		// 查找是否为符号
		if(node->symbol > 0) {
			return node->symbol;
		}

		i --;
	}

	return 0;
}



/* 比较优先级 */
char parser::priority (char top, char read) {
	switch(top) {
		case '+':
		case '-':
			switch(read) {
				case  0 :
				case '+':
				case '-':
					return '>';
				case '*':
				case '/':
				case '(':
					return '<';
				case ')':
					return '>';
			}
			break;

		case '*':
		case '/':
			switch(read) {
				case  0 :
				case '+':
				case '-':
				case '*':
				case '/':
					return '>';
				case '(':
					return '<';
				case ')':
					return '>';
			}
			break;

		case '(':
			switch(read) {
				case '+':
				case '-':
				case '*':
				case '/':
				case '(':
					return '<';
			}
			break;

		case ')':
			switch(read) {
				case  0 :
				case '+':
				case '-':
				case '*':
				case '/':
				case ')':
					return '>';
			}
			break;
	}

	return 0;
}




/* 清理栈 */
void parser::flush (void) {
	// 遍历栈直至没有节点
	while(true) {
		// 判断栈空
		if(parser::stack->size ( ) <= 0) {
			break;
		}

		// 栈顶
		node_t *	node	= parser::stack->back ( );

		// 释放节点
		delete node;

		// 弹出栈顶
		parser::stack->pop_back ( );
	}
}





/* 运行主循环 */
void parser::loop (void) {
	// 初始化栈
	parser::stack	= new std::vector <node_t *>;

	// 主循环
	while(true) {
		// 输入提示
		fprintf	(stdout, "请输入算术表达式：");
		// 清除输入缓冲区
		fflush	(stdin);

		// 调用词法分析器准备
		lexical::read (stdin);

		// 错误位置
		int		error	= 0;
		// 计算结果
		double	result	= parser::execute (& error);

		// 判断是否输入错误
		if(error == -1) {
			// 输出运算结果
			fprintf (stdout, "运算结果：%lf", result);
		} else {
			// 输出错误信息
			if(error == 0) {
				fprintf (stdout, "运算错误：输入的表达式格式不正确");
			} else {
				fprintf (stdout, "运算错误：第 %d 个字符出现了预料之外的字符", error);
			}
		}

		// 输出换行
		fprintf (stdout, "\n\n\n");

		// 清理栈
		parser::flush ( );
	}
}

