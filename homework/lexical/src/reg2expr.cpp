/**
	$ Lexical   (C) 2005-2013 MF
	$ reg2expr.cpp
*/


#include "class.h"



/* 正规式转换为后缀表达式 */
char * lexical::convert_regular_to_expression (void) {
	char	byte;

	// 已读取字符数量
	size_t	count	= 0;
	// 字符转义模式
	bool	escape	= false;

	// 上一个读入的字符信息
	bool	prev_alphabet	= false;
	char	prev_byte		= 0;

	// 输入提示
	cout << "请输入正规表达式：" << endl;

	while(1) {
		// 读取字符
		cin >> noskipws >> byte;

		// 已读字符数量
		count ++;

		// 不允许输入 NUL
		if(byte == 0) {
			return "输入了无效的字符";
		}
		// 读到换行结束
		if((byte == '\r') || (byte == '\n')) {
			break;
		}

		// 处理转义模式
		if(escape == false) {
			// 如果读取到 \, 下一个字符应该视为字母
			if(byte == '\\') {
				// 打开转义模式
				escape = true;

				// 直接结束本次循环, 进入下一轮读取
				continue;
			}
		}

		// 当前字符是否属于字母
		bool	alphabet	= false;
		// 处理错误
		char *	error		= NULL;

		// 如果转义模式已经打开, 无论读入什么都视为字母
		if(escape == true) {
			// 设置字母标记
			alphabet	= true;
		} else {
			// 处理运算符
			switch(byte) {
				// 左括号, 括号前按情况增加一个连接
				case '(':			this->regular_concat		(prev_alphabet, prev_byte);
							error =	this->regular_bracket_left	();

							break;

				// 右括号
				case ')':	error =	this->regular_bracket_right	();		break;
				// 或
				case '|':	error =	this->regular_or			();		break;
				// 闭包
				case '*':	error =	this->regular_closure		();		break;

				// 其它字符视为字母
				default:
					// 设置字母标记
					alphabet	= true;
			}
		}

		// 处理字母
		if(alphabet == true) {
			// 每个字母前都增加一个连接
			this->regular_concat(prev_alphabet, prev_byte);

			// 添加当前字符到字母表中
			error = this->regular_alphabet (byte);
		}

		// 出现错误
		if(error != NULL) {
			// 错误信息长度
			size_t	length	= strlen(error);
			// 分配空间, 预留额外空间
			char *	message	= new char [length + 64];

			// 生成包括字符位置的错误信息
			sprintf(message, "%s （在第 %d 个字符处）", error, count);

			return message;
		}

		// 关闭转义模式
		escape	= false;

		// 保存本次处理的字符
		prev_alphabet	= alphabet;
		prev_byte		= byte;
	}

	// 整个正规式结束后转义应该关闭, 否则表明正规式以 \ 结束
	if(escape == true) {
		return "无效的转义";
	}

	// 将栈中剩余的所有运算符放回表达式数组
	while(this->stack_byte.empty() == false) {
		// 栈顶运算符
		byte = this->stack_byte.top();

		// 如果遇到了 '(', 括号不匹配
		if(byte == '(') {
			return "括号不匹配";
		}

		// 弹出当前运算符
		this->stack_byte.pop();
		// 生成表达式对象
		this->regular_byte(false, byte);
	}

	// 判断最后表达式数组中对象数目
	if(this->expression.size() == 0) {
		return "没有读入任何规则";
	}

	return NULL;
}




/* 处理字符 */
char * lexical::regular_byte (bool alphabet, char byte) {
	// 创建表达式对象
	expression_t *	object	= new expression_t;

	object->alphabet	= alphabet;
	object->byte		= byte;

	// 添加到表达式对象数组
	this->expression.push_back(object);

	return NULL;
}



/* 处理字母字符 */
char * lexical::regular_alphabet (char byte) {
	// 连接字符
	lexical::regular_byte(true, byte);

	return NULL;
}



/* 处理左括号 */
char * lexical::regular_bracket_left (void) {
	// 左括号直接压栈
	this->stack_byte.push('(');

	return NULL;
}


/* 处理右括号 */
char * lexical::regular_bracket_right (void) {
	// 弹出所有运算符
	while(this->stack_byte.empty() == false) {
		// 读取栈顶字符
		char byte = this->stack_byte.top();

		// 弹出栈顶
		this->stack_byte.pop();

		// 遇到左括号, 结束循环
		if(byte == '(') {
			return NULL;
		}

		this->regular_byte (false, byte);
	}

	// 未读到左括号就结束, 说明括号不匹配
	return "括号不匹配";
}



/* 处理连接 */
char * lexical::regular_concat (bool alphabet, char byte) {
	// 如果读入的是第一个字符, 不添加连接
	if(byte == 0) {
		return NULL;
	}

	// 只有上一个字符为字母 ')' 或闭包才添加连接
	if(alphabet == false) {
		if((byte != ')') && (byte != '*')) {
			return NULL;
		}
	}

	// 弹出闭包和连接运算
	while(this->stack_byte.empty() == false) {
		char byte = this->stack_byte.top();

		// 只有读到 '*' 或 '&' 才弹出
		if((byte == '*') || (byte == '&')) {
			this->stack_byte.pop();
			this->regular_byte(false, byte);

			continue;
		}

		// 否则视为遇到左括号或优先级更低的字符
		break;
	}

	// 当前运算符压栈
	this->stack_byte.push('&');

	return NULL;
}


/* 处理或 */
char * lexical::regular_or (void) {
	while(this->stack_byte.empty() == false) {
		char byte = this->stack_byte.top();

		// 只要读到不是 '(' 都弹出并添加到线性表
		if(byte != '(') {
			this->stack_byte.pop();
			this->regular_byte(false, byte);

			continue;
		}

		break;
	}

	this->stack_byte.push('|');

	return NULL;
}


/* 处理闭包 */
char * lexical::regular_closure (void) {
	while(this->stack_byte.empty() == false) {
		char byte = this->stack_byte.top();

		// 只有读到 '*' 才弹出并添加到线性表
		if(byte == '*') {
			this->stack_byte.pop();
			this->regular_byte(false, byte);

			continue;
		}

		break;
	}

	this->stack_byte.push('*');

	return NULL;
}

