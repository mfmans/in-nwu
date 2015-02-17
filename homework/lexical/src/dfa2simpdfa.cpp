/**
	$ Lexical   (C) 2005-2013 MF
	$ dfa2simpdfa.cpp
*/


#include "class.h"



/* DFA 转换为最简 DFA */
char * lexical::convert_dfa_to_simpdfa (void) {
	// 处理错误
	char * error;

	// 状态数量
	size_t	count_matrix	= this->matrix.size();
	// 字母数量
	size_t	count_alphabet	= this->alphabet.size();

	// 终止状态数组
	array_bool_t *	state_end		= NULL;
	// 状态转换矩阵
	array_state_t *	state_matrix	= this->dfa_state_build (& state_end);

	// 状态集
	vector <array_state_t *>	state_set;

	// 初始化状态集
	this->dfa_set_initialize (state_matrix, state_end, & state_set);

	// 遍历字母表
	for(size_t i = 0; i < count_alphabet; i ++) {
		// 根据不同的字符对状态集进行切割
		this->dfa_set_split(i, & state_set, state_end);
	}

	// 根据合并后的状态集生成新的转换结构矩阵
	if(error = this->dfa_state_convert(& state_set, state_end)) {
		return error;
	}

	return NULL;
}




/* 生成节点矩阵, 即将转换结构矩阵转换为节点矩阵 */
array_state_t * lexical::dfa_state_build (array_bool_t **state_end) {
	// 数量
	size_t	count_matrix	= this->matrix.size();
	size_t	count_alphabet	= this->alphabet.size();

	// 终止状态数组
	*state_end	= new array_bool_t;

	// 状态矩阵
	array_state_t *	states	= new array_state_t;

	// 初始化矩阵
	for(size_t i = 0; i < count_matrix; i ++) {
		// 增加一个新状态
		states->push_back(new state_t);

		// 判断是否为终止状态
		(*state_end)->push_back(this->matrix[i]->accept);
	}

	// 初始化矩阵中每一个状态
	for(size_t i = 0; i < count_matrix; i ++) {
		// 遍历字母表
		for(size_t j = 0; j < count_alphabet; j ++) {
			// 转换结构中指定字符的转换方向
			convert_t * convert = this->matrix[i]->to[j];

			if(convert == NULL) {
				// 不接收当前字符
				states->at(i)->out.push_back(NULL);
			} else {
				// 创建新的连线
				line_t * line	= new line_t;

				// 指向目的转换结构对应的状态节点
				line->byte	= this->alphabet[j];
				line->from	= states->at(i);
				line->to	= states->at(convert->number);

				// 保存目的节点
				states->at(i)->out.push_back(line);
				// 保存源节点
				states->at(convert->number)->in.push_back(line);
			}
		}
	}

	return states;
}



/* 重置节点矩阵, 由新的状态集合并生成 */
char * lexical::dfa_state_convert (vector <array_state_t *> *list, array_bool_t *end) {
	// 清空当前矩阵
	this->matrix.clear();

	size_t	count_list		= list->size();
	size_t	count_alphabet	= this->alphabet.size();

	// 初始化状态集矩阵
	for(size_t i = 0; i < count_list; i ++) {
		// 当前状态集
		array_state_t * set = list->at(i);

		// 创建一个转换结构
		convert_t * convert = new convert_t;

		convert->accept		= end->at(i);
		convert->number		= this->matrix.size();
		convert->state		= NULL;
		convert->to			= new convert_t * [count_alphabet];

		// 添加到矩阵中
		this->matrix.push_back(convert);
	}

	// 转换状态集数组为矩阵
	for(size_t i = 0; i < count_list; i ++) {
		// 从当前状态集中任意取一个状态
		state_t *		state	= list->at(i)->at(0);
		// 转换目的状态数组
		array_line_t *	lines	= & state->out;

		// 遍历状态的转换目的状态数组
		for(size_t j = 0; j < count_alphabet; j ++) {
			// 如果没有目的状态, 表明不接收这个字符
			if(lines->at(j) == NULL) {
				this->matrix[i]->to[j] = NULL;

				continue;
			}

			// 结果状态集序号
			size_t index = 0;

			// 再次遍历状态集, 查找目的状态所在的状态集序号
			for(; index < count_list; index ++) {
				if(find(list->at(index)->begin(), list->at(index)->end(), lines->at(j)->to) != list->at(index)->end()) {
					goto found;
				}
			}

			// 没有找到
			return "没有找到目的状态集";

found:
			// 保存目的状态
			this->matrix[i]->to[j] = this->matrix[index];
		}
	}

	return NULL;
}




/* 初始化状态集, 即将状态矩阵转换为状态集数组 */
void lexical::dfa_set_initialize (array_state_t *list, array_bool_t *end, vector <array_state_t *> *set) {
	// 判断是否有非终态
	bool	exist	= false;

	// 终态状态集索引
	size_t	index_end	= 0;
	// 非终态状态集索引
	size_t	index_unend	= 0;

	// 检测是否有非终态的状态
	if(find(end->begin(), end->end(), false) != end->end()) {
		exist = true;
	}

	// 计算状态集索引
	if(end->at(0) == true) {
		// 非终态为第 2 项
		index_unend	= 1;
	} else {
		// 终态为第 2 项
		index_end	= 1;
	}

	// 为状态集数组添加一个状态集
	set->push_back(new array_state_t);

	// 如果存在非终态, 再添加一个状态集, 此时第 0 项为非终态状态集, 第 1 项为终态状态集
	if(exist == true) {
		set->push_back(new array_state_t);
	}

	// 遍历所有状态
	for(size_t i = 0; i < list->size(); i ++) {
		// 判断是否为终态
		if(end->at(i) == true) {
			// 如果状态是终止状态, 直接加入终态状态集
			set->at(index_end)->push_back(list->at(i));
		} else {
			// 加入非终态状态集
			set->at(index_unend)->push_back(list->at(i));
		}
	}

	// 清空所有终止状态信息
	end->clear();

	if(exist == true) {
		// 如果存在非终态
		if(index_end == 1) {
			end->push_back(false);
			end->push_back(true);
		} else {
			end->push_back(true);
			end->push_back(false);
		}
	} else {
		// 如果不存在非终态, 只有一项且为非终态状态集
		end->push_back(true);
	}
}



/* 分割状态集, 即根据读入的字符对状态集进行分裂 */
void lexical::dfa_set_split (size_t byte, vector <array_state_t *> *list, array_bool_t *end) {
	// 分裂开始前状态集数量
	size_t	count_set		= list->size();
	// 状态数目
	size_t	count_matrix	= this->matrix.size();

	// 遍历状态集
	for(size_t i = 0; i < count_set; i ++) {
		// 当前状态集
		array_state_t *	set	= list->at(i);

		// 需要分割出去的状态的目的状态集
		array_state_t *	split_set	= new array_state_t [count_matrix + 1];

		// 是否已经读到第一个状态的目的状态序号
		bool	split_read		= false;
		// 是否需要分割状态集
		bool	split_require	= false;
		// 期望中的目的状态序号
		size_t	split_target	= 0;

		// 创建新的状态集, 旧的状态集已经通过 set 进行保存
		list->at(i) = new array_state_t;

		// 遍历状态集中的状态
		for(size_t j = 0; j < set->size(); j ++) {
			// 当前状态
			state_t *	state	= set->at(j);

			// 当前状态与读入字符后转换到的目的状态连线
			line_t *	line	= state->out[byte];

			// 目的状态所在状态集序号, 0 表示不接收字符
			size_t		target	= 0;

			// 判断当前状态是否接收当前字符
			if(line != NULL) {
				// 目的状态
				state_t * to = line->to;

				// 遍历状态集数组
				for(size_t k = 0; k < list->size(); k ++) {
					// 查询目的状态是否在当前状态集中
					if(find(list->at(k)->begin(), list->at(k)->end(), to) == list->at(k)->end()) {
						continue;
					}

					// 保存状态集序号
					target = k + 1;

					break;
				}

				// 如果找不到, 表明落在当前状态集
				if(target == 0) {
					target = i + 1;
				}
			}

			// 如果这是当前状态集中第一个状态
			if(split_read == false) {
				// 设置保存状态
				split_read		= true;
				// 直接保存目的状态集序号
				split_target	= target;

				// 将当前状态插入到新的状态集
				list->at(i)->push_back(state);

				// 结束本次循环, 继续处理下一个状态
				continue;
			}

			// 如果与这个状态集应该的目的状态集相同, 直接保存到新状态集
			if(split_target == target) {
				list->at(i)->push_back(state);

				continue;
			}

			// 保存到需要分割出去的目的状态集数组
			split_set[target].push_back(state);

			// 设置需要切割
			split_require	= true;
		}

		// 如果没有需要分割出去的状态, 循环结束
		if(split_require == false) {
			goto done;
		}

		// 遍历分割出去的状态
		for(size_t k = 0; k <= count_matrix; k ++) {
			// 如果没有任何状态落入这个状态集, 跳过
			if(split_set[k].size() == 0) {
				continue;
			}

			// 对于每一个结果都创建一个新的状态集
			array_state_t * state_set = new array_state_t (split_set[k]);

			// 向终态数组复制当前状态集状态
			end->push_back(end->at(i));
			// 向状态集数组添加新的状态集
			list->push_back(state_set);
		}

done:
		// 释放旧的状态集
		delete		set;
		// 释放分割状态集数组
		delete []	split_set;
	}
}

