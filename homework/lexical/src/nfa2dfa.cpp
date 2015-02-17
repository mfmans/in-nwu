/**
	$ Lexical   (C) 2005-2013 MF
	$ nfa2dfa.cpp
*/


#include "class.h"



/* NFA 转换为 DFA */
char * lexical::convert_nfa_to_dfa (void) {
	// 字母表数量
	size_t	count	= this->alphabet.size();

	// 获取生成的 DFA 自动机结构
	automata_t *	object	= this->stack_automata.top();

	// 起始状态集转换结构
	convert_t *		start_convert	= new convert_t;
	// 起始状态集 ε 闭包
	array_state_t *	start_state		= new array_state_t;

	// 求起始状态集的 ε 闭包
	this->nfa_epsilon_closure (object->start, start_state);
	// 对起始状态集的 ε 闭包进行排序
	this->nfa_state_sort(start_state);

	// 设置起始状态集转换结构
	start_convert->number	= 0;
	start_convert->state	= start_state;

	// 将起始状态集转换结构添加到转换矩阵
	this->matrix.push_back(start_convert);

	// 遍历转换矩阵
	for(size_t i = 0; i < this->matrix.size(); i ++) {
		// 当前转换结构
		convert_t *		convert	= this->matrix[i];
		// 当前转换结构包含的状态集闭包
		array_state_t *	closure	= convert->state;

		// 查询 NFA 自动机结束状态是否在状态集, 设置是否为终态
		convert->accept	= this->nfa_state_exist(closure, object->end);
		// 创建转换方向数组
		convert->to		= new convert_t * [count];

		// 遍历字母集
		for(size_t j = 0; j < count; j ++) {
			char byte	= this->alphabet[j];

			// 读入字母后转移到的新状态集
			array_state_t * to	= new array_state_t;

			// 对状态集进行推导, 计算从当前状态集的 ε 闭包接收字符 byte 后转移到的新状态集
			this->nfa_derivation (byte, closure, to);

			// 如果新状态集为空, 表示不接收当前字符
			if(to->empty() == true) {
				// 回收内存
				delete to;

				// 设置该字符对应的转换路径为 NULL
				convert->to[j] = NULL;

				continue;
			}

			// 遍历新状态集中的每一个状态
			for(size_t k = 0; k < to->size(); k ++) {
				// 计算状态的 ε 闭包
				this->nfa_epsilon_closure (to->at(k), to);
			}

			// 对新状态集的 ε 闭包进行排序
			this->nfa_state_sort(to);

			// 获取新状态集的 ε 闭包对应的转换结构
			convert_t * next = this->nfa_convert (to);

			// 如果转换结构不存在
			if(next == NULL) {
				// 创建新的转换结构
				next	= new convert_t;

				// 设置序号
				next->number	= this->matrix.size();
				// 保存闭包信息
				next->state		= to;

				// 保存到转换矩阵
				this->matrix.push_back(next);
			} else {
				// 清理数据
				delete to;
			}

			// 设置接收了 byte 字符后转换到新的转换结构
			convert->to[j] = next;
		}
	}

	// 清理转换矩阵
	this->nfa_clear();

	return NULL;
}




/* 清理状态集, 即转换结构中不再保留 NFA 状态集 */
void lexical::nfa_clear (void) {
	// 遍历转换矩阵
	for(size_t i = 0; i < this->matrix.size(); i ++) {
		delete this->matrix[i]->state;

		// 重置指针
		this->matrix[i]->state = NULL;
	}
}



/* 对状态集进行推导, 即计算一个状态集读取接收一个字符后生成的新状态集 */
void lexical::nfa_derivation (char byte, array_state_t *from, array_state_t *to) {
	// 遍历当前状态集
	for(size_t i = 0; i < from->size(); i ++) {
		state_t * state = from->at(i);

		// 遍历以当前状态为起点的所有连线
		for(size_t j = 0; j < from->at(i)->out.size(); j ++) {
			line_t * line = from->at(i)->out[j];

			// 判断当前连线的条件是否为读入指定字符
			if(line->byte != byte) {
				continue;
			}

			// 将连线的终点添加到新状态集中
			if(find(to->begin(), to->end(), line->to) == to->end()) {
				to->push_back(line->to);
			}
		}
	}
}



/* 计算状态的 ε 闭包 */
void lexical::nfa_epsilon_closure (state_t *from, array_state_t *result) {
	// 查询队列
	queue<state_t *> query;

	// 将第一个状态入队列
	query.push(from);

	// 遍历队列直到没有需要处理的状态
	while(query.empty() == false) {
		state_t * state = query.front();

		// 删除队首元素
		query.pop();

		// 将当前状态加入到结果状态集
		if(find(result->begin(), result->end(), state) == result->end()) {
			result->push_back(state);
		}

		// 遍历所有以当前状态为起点的连线
		for(size_t i = 0; i < state->out.size(); i ++) {
			line_t * line = state->out[i];

			// 只处理 ε 弧
			if(line->byte != 0) {
				continue;
			}
			// 判断连线终点是否为到自身状态
			if(line->to == state) {
				continue;
			}

			// 判断连线终点是否已经存在于结果状态集
			if(find(result->begin(), result->end(), line->to) == result->end()) {
				// 不存在的话加入队列
				query.push(line->to);
			}
		}
	}
}




/* 状态集排序 */
void lexical::nfa_state_sort (array_state_t *set) {
	sort(set->begin(), set->end());
}


/* 求状态是否存在于状态集 */
bool lexical::nfa_state_exist (array_state_t *set, state_t *state) {
	if(find(set->begin(), set->end(), state) == set->end()) {
		return false;
	}

	return true;
}



/* 获取状态集对应的转换结构 */
convert_t * lexical::nfa_convert (array_state_t *set) {
	// 遍历转换矩阵
	for(size_t i = 0; i < this->matrix.size(); i ++) {
		// 当前转换结构的状态集
		array_state_t * closure = this->matrix[i]->state;

		// 比较状态集大小
		if(closure->size() != set->size()) {
			continue;
		}

		// 遍历比较状态集内容
		for(size_t j = 0; j < set->size(); j ++) {
			if(set->at(j) != closure->at(j)) {
				goto next;
			}
		}

		// 比较成功
		return this->matrix[i];

next:
		// 比较失败
		continue;
	}

	return NULL;
}

