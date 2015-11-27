//------------------------------------------------------------------------
//�߳�����
//------------------------------------------------------------------------
#ifndef THREADTASK_H_
#define THREADTASK_H_

#include "task.h"

namespace thread
{
/*
	�̳߳ص��̻߳���
*/
class TPTask : public Task
{
public:
	enum TPTaskState
	{
		/// һ�������Ѿ����
		TPTASK_STATE_COMPLETED = 0,

		/// ���������߳�ִ��
		TPTASK_STATE_CONTINUE_MAINTHREAD = 1,

		// ���������߳�ִ��
		TPTASK_STATE_CONTINUE_CHILDTHREAD = 2,
	};

	/**
		����ֵ�� thread::TPTask::TPTaskState�� ��ο�TPTaskState
	*/
	virtual thread::TPTask::TPTaskState presentMainThread(){ 
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}
};

}
#endif
