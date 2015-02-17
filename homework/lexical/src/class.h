/**
	$ Lexical   (C) 2005-2013 MF
	$ class.h
*/


#pragma warning (disable: 4996)



#ifndef _CLASS_H_
#define _CLASS_H_



// 基本库
#include <iostream>

// 容器类
#include <vector>
#include <stack>
#include <queue>



// for std
using namespace std;




/* 状态节点 */
struct state_t;
/* 连线节点 */
struct line_t;


/* 状态节点数组 */
typedef vector <struct state_t *>	array_state_t;
/* 连线节点数组 */
typedef vector <struct line_t *>	array_line_t;

/* 布尔数组 */
typedef vector <bool>	array_bool_t;



/* 状态节点 */
typedef struct state_t {
	array_line_t	in;				// 以该节点作为终点的连线的数组
	array_line_t	out;			// 以该节点作为起点的连线的数组
} state_t;


/* 连线节点 */
typedef struct line_t {
	char	byte;					// 转换字节, 如果为 0 表示 ε 弧

	state_t *	from;				// 起点节点
	state_t *	to;					// 终点节点
} line_t;



/* 表达式对象 */
typedef struct {
	bool	alphabet;				// 是否为字母, 否为运算符
	char	byte;					// 接收字节
} expression_t;


/* 自动机结构 */
typedef struct {
	state_t *	start;				// 开始节点
	state_t *	end;				// 结束节点
} automata_t;



/* 状态集转换结构 */
typedef struct convert_t {
	int		number;					// 状态序号
	bool	accept;					// 是否为终态

	array_state_t *		state;		// 对应状态集

	struct convert_t **	to;			// 转换方向数组
} convert_t;




/* 词法分析器类 */
class lexical {
	public:
		explicit	 lexical	(FILE *stream);
					~lexical	(void);


		// 读入正规式并进行转换, 返回是否成功
		bool	read		(void);
		// 输出矩阵, 即转换的结果
		void	output		(void);
		// 进入匹配模式, 等待用户输入字符串并告知匹配结果
		void	match		(void);



	private:
		FILE *	in;										// 输入流

		vector	<char>				alphabet;			// 字母表
		vector	<convert_t *>		matrix;				// 状态集转换矩阵
		vector	<expression_t *>	expression;			// 表达式对象数组

		stack	<char>				stack_byte;			// 正规式转后缀表达式字符读入栈
		stack	<automata_t *>		stack_automata;		// 自动机生成栈


		// 正规式转换为后缀表达式
		char *	convert_regular_to_expression	(void);
		// 后缀表达式转换为 NFA
		char *	convert_expression_to_nfa		(void);
		// NFA 转换为 DFA
		char *	convert_nfa_to_dfa				(void);
		// DFA 转换为最简 DFA
		char *	convert_dfa_to_simpdfa			(void);


		// 处理字符
		char *	regular_byte			(bool alphabet, char byte);
		char *	regular_alphabet		(char byte);
		// 处理括号
		char *	regular_bracket_left	(void);
		char *	regular_bracket_right	(void);
		// 处理运算符
		char *	regular_concat			(bool alphabet, char byte);
		char *	regular_or				(void);
		char *	regular_closure			(void);


		// 处理表达式对象
		char *	expression_alphabet		(char byte);
		char *	expression_concat		(void);
		char *	expression_or			(void);
		char *	expression_closure		(void);


		// 清理状态集
		void	nfa_clear				(void);
		// 对状态集进行推导
		void	nfa_derivation			(char byte, array_state_t *from, array_state_t *to);
		// 计算状态的 ε 闭包
		void	nfa_epsilon_closure		(state_t *from, array_state_t *result);
		// 状态集排序
		void	nfa_state_sort			(array_state_t *set);
		// 求状态是否存在于状态集
		bool	nfa_state_exist			(array_state_t *set, state_t *state);
		// 获取状态集对应的转换结构
		convert_t *	nfa_convert			(array_state_t *set);


		// 生成节点矩阵
		array_state_t *	dfa_state_build		(array_bool_t **state_end);
		// 重置转换结构矩阵
		char *	dfa_state_convert		(vector <array_state_t *> *list, array_bool_t *end);
		// 初始化状态集
		void	dfa_set_initialize		(array_state_t *list, array_bool_t *end, vector <array_state_t *> *set);
		// 分割状态集
		void	dfa_set_split			(size_t byte, vector <array_state_t *> *list, array_bool_t *end);
};



#endif   /* CLASS */