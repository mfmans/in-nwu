/**
	$ Lexical   (C) 2005-2013 MF
	$ call.cpp
*/


#include "class.h"



/* 构造 */
lexical:: lexical	(FILE *stream) : in (stream)	{ }
/* 析构 */
lexical::~lexical	(void)	{ }



/* 读入正规式并进行转换 */
bool lexical::read (void) {
	// 转换函数列表
	char * (lexical:: * handler [])(void) = {
		& lexical::convert_regular_to_expression,
		& lexical::convert_expression_to_nfa,
		& lexical::convert_nfa_to_dfa,
		& lexical::convert_dfa_to_simpdfa
	};

	// 依次调用转换函数
	for(int i = 0; i < (sizeof(handler) / sizeof(void *)); i ++) {
		// 执行错误
		char *	error	 = (this->* handler[i])();

		// 如果出现错误, 输出错误并结束
		if(error != NULL) {
			cout << endl << "转换出现错误：" << endl << error;

			return false;
		}
	}

	return true;
}



/* 输出矩阵 */
void lexical::output (void) {
	// 字母表元素数量
	size_t count	= this->alphabet.size();

	// 输出第一列的空格
	cout << "\t";

	// 第一行为遍历并输出字母表
	for(size_t i = 0; i < count; i ++) {
		cout << "\t" << this->alphabet[i];
	}

	// 遍历状态集转换矩阵
	for(size_t i = 0; i < this->matrix.size(); i ++) {
		// 输出换行
		cout << endl;

		// 当前转换结构
		convert_t *	convert = this->matrix[i];

		// 输出状态序号
		if(convert->accept == true) {
			// 如果为终态, 在序号前后加上括号
			cout << "\t(" << convert->number << ")";
		} else {
			// 如果为非终态, 直接输出序号
			cout << "\t"  << convert->number;
		}

		// 遍历当前转换方向数组
		for(size_t j = 0; j < count; j ++) {
			// 判断当前转换结构是否接收当前字符
			if(convert->to[j] == NULL) {
				// 如果不接收, 输出 -
				cout << "\t-";
			} else {
				// 如果接收, 输出新状态序号
				cout << "\t"  << convert->to[j]->number;
			}
		}
	}

	// 最后一行后输出换行
	cout << endl;
}



/* 进入匹配模式 */
void lexical::match (void) {
	while(1) {
		// 清除输入流缓冲
		fflush(stdin);

		// 输出换行
		cout << endl << endl;
		// 输出提示
		cout << "输入需要匹配字符串：";

		// 最后读入的字节
		char	byte	= 0;
		// 当前处在的转换结构
		convert_t *	convert	= this->matrix[0];

		while(convert != NULL) {
			// 读入字符
			cin >> noskipws >> byte;

			// 遇到换行, 结束循环
			if((byte == '\n') || (byte == '\r')) {
				break;
			}

			// 查找读入的字符在字母表中的位置
			vector<char>::iterator iterator = find(this->alphabet.begin(), this->alphabet.end(), byte);

			// 判断是否读入了不在字母表中的字符
			if(iterator == this->alphabet.end()) {
				// 设置无法转换
				convert = NULL;
			} else {
				// 计算读入字符在字母表中实际索引
				size_t index = distance(this->alphabet.begin(), iterator);

				// 更新转换结构
				convert = convert->to[index];
			}
		}

		// 判断终止时的转换状态
		if((convert == NULL) || (convert->accept == false)) {
			cout << "输入的字符串不匹配。";
		} else {
			cout << "输入的字符串成功匹配。";
		}
	}
}

