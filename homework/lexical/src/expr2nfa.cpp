/**
	$ Lexical   (C) 2005-2013 MF
	$ expr2nfa.cpp
*/


#include "class.h"



/* 后缀表达式转换为 NFA */
char * lexical::convert_expression_to_nfa (void) {
	// 遍历表达式数组
	for(int i = 0, j = this->expression.size(); i < j; i++) {
		// 处理错误
		char *			error	= NULL;
		// 当前项
		expression_t *	object	= this->expression[i];

		if(object->alphabet == true) {
			// 处理字母
			error = this->expression_alphabet (object->byte);
		} else {
			// 处理运算符
			switch(object->byte) {
				// 连接
				case '&':	error = this->expression_concat		();		break;
				// 或
				case '|':	error = this->expression_or			();		break;
				// 闭包
				case '*':	error = this->expression_closure	();		break;
			}
		}

		// 处理出现错误
		if(error) {
			return error;
		}
	}

	// 结束后栈中应该只有一个元素
	if(this->stack_automata.size() != 1) {
		return "结构不正确";
	}

	// 对字母表进行排序
	sort(this->alphabet.begin(), this->alphabet.end());

	return NULL;
}




/* 处理字母 */
char * lexical::expression_alphabet (char byte) {
	// 创建自动机结构
	automata_t *	object	= new automata_t;

	// 开始节点和结束节点
	state_t *	start	= new state_t;
	state_t *	end		= new state_t;

	// 创建自动机开始和结束节点的连线
	line_t *	line	= new line_t;

	// 开始节点读入 byte 后转移到结束节点
	line->byte		= byte;
	line->from		= start;
	line->to		= end;

	// 保存连线
	start->out.push_back	(line);
	end  ->in .push_back	(line);

	// 自动机从 start 开始, end 结束
	object->start	= start;
	object->end		= end;

	// 将自动机结构入栈
	this->stack_automata.push(object);

	// 查找字母表中是否存在当前读入的字符
	if(find(this->alphabet.begin(), this->alphabet.end(), byte) == this->alphabet.end()) {
		// 添加字符到字母表中
		this->alphabet.push_back(byte);
	}

	return NULL;
}



/* 处理连接 */
char * lexical::expression_concat (void) {
	// 连接运算需要两个元素
	if(this->stack_automata.size() < 2) {
		return "连接运算错误，至少要有 2 个元素";
	}

	// 弹出后一个自动机结构
	automata_t *	object_2	= this->stack_automata.top();	this->stack_automata.pop();
	// 弹出前一个自动机结构
	automata_t *	object_1	= this->stack_automata.top();	this->stack_automata.pop();

	// 创建连线
	line_t * line	= new line_t;

	// ε 弧连接前一个结构的结束节点和后一个结构的开始节点
	line->byte		= 0;
	line->from		= object_1->end;
	line->to		= object_2->start;

	// 保存连线
	object_1->end  ->out.push_back	(line);
	object_2->start->in .push_back	(line);

	// 合并两个结构, 重置前一个结构的结束节点为后一个结构的结束节点
	object_1->end	= object_2->end;

	// 删除后一个结构
	delete object_2;

	// 将合并后的自动机结构入栈
	this->stack_automata.push(object_1);

	return NULL;
}


/* 处理或 */
char * lexical::expression_or (void) {
	// 或运算需要两个元素
	if(this->stack_automata.size() < 2) {
		return "或运算错误，至少要有 2 个元素";
	}

	// 第一个结构
	automata_t *	object_1	= this->stack_automata.top();	this->stack_automata.pop();
	// 第二个结构
	automata_t *	object_2	= this->stack_automata.top();	this->stack_automata.pop();

	// 创建新的起始节点和结束节点
	state_t *	state_start	= new state_t;
	state_t *	state_end	= new state_t;

	// 新的起始节点连线
	line_t *	line_start_1	= new line_t;
	line_t *	line_start_2	= new line_t;
	// 新的终止节点连线
	line_t *	line_end_1		= new line_t;
	line_t *	line_end_2		= new line_t;

	// 第一条 ε 弧连接新的起始节点到第一个结构的起始节点
	line_start_1->byte		= 0;
	line_start_1->from		= state_start;
	line_start_1->to		= object_1->start;

	// 第二条 ε 弧连接第一个结构的结束节点和新的结束节点
	line_end_1->byte		= 0;
	line_end_1->from		= object_1->end;
	line_end_1->to			= state_end;

	// 第三条 ε 弧连接新的起始节点到第二个结构的起始节点
	line_start_2->byte		= 0;
	line_start_2->from		= state_start;
	line_start_2->to		= object_2->start;

	// 第四条 ε 弧连接第二个结构的结束节点和新的结束节点
	line_end_2->byte		= 0;
	line_end_2->from		= object_2->end;
	line_end_2->to			= state_end;

	// 保存第一条弧
	state_start->out.push_back		(line_start_1);
	object_1->start->in.push_back	(line_start_1);

	// 保存第二条弧
	object_1->end->out.push_back	(line_end_1);
	state_end->in.push_back			(line_end_1);

	// 保存第三条弧
	state_start->out.push_back		(line_start_2);
	object_2->start->in.push_back	(line_start_2);

	// 保存第四条弧
	object_2->end->out.push_back	(line_end_2);
	state_end->in.push_back			(line_end_2);

	// 合并新的节点和两个结构
	object_1->start		= state_start;
	object_1->end		= state_end;

	// 删除第二个结构
	delete object_2;

	// 将合并后的元素入栈
	this->stack_automata.push(object_1);

	return NULL;
}


/* 处理闭包 */
char * lexical::expression_closure (void) {
	// 闭包运算需要一个元素
	if(this->stack_automata.size() < 1) {
		return "闭包运算错误，至少要有 1 个元素";
	}

	// 获取栈顶自动机结构
	automata_t *	object	= this->stack_automata.top();

	// 创建新的起始节点和结束节点
	state_t *	state_start	= new state_t;
	state_t *	state_end	= new state_t;

	line_t *	line_1	= new line_t;
	line_t *	line_2	= new line_t;
	line_t *	line_3	= new line_t;
	line_t *	line_4	= new line_t;

	// 第一条 ε 弧连接新的起始节点和结构起始节点
	line_1->byte	= 0;
	line_1->from	= state_start;
	line_1->to		= object->start;

	// 第二条 ε 弧连接新的起始节点和新的结束节点
	line_2->byte	= 0;
	line_2->from	= state_start;
	line_2->to		= state_end;

	// 第三条 ε 弧连接结构结束节点和新的结束节点
	line_3->byte	= 0;
	line_3->from	= object->end;
	line_3->to		= state_end;

	// 第四条 ε 弧连接新的结束节点和新的起始节点
	line_4->byte	= 0;
	line_4->from	= state_end;
	line_4->to		= state_start;

	// 保存第一条弧
	state_start->out.push_back		(line_1);
	object->start->in.push_back		(line_1);

	// 保存第二条弧
	state_start->out.push_back		(line_2);
	state_end->in.push_back			(line_2);

	// 保存第三条弧
	object->end->out.push_back		(line_3);
	state_end->in.push_back			(line_3);

	// 保存第四条弧
	state_end->out.push_back		(line_4);
	state_start->in.push_back		(line_4);

	// 重置元素开始和结束节点
	object->start	= state_start;
	object->end		= state_end;

	return NULL;
}

