
#ifndef _L_TASK_

#define _L_TASK_

class LTask;

extern "C" LTask *TaskQueue;

extern "C" UINT16 TaskQueueSelectLength;

extern "C" void DoEvents();
extern "C" void AddTask(LTask *);

class LTask {
public:
	UINT16 Priority = 0;
	LTask *Left = NULL, *Right = NULL;
	
	LTask() {
	}
	
	LTask(UINT16 _Priority) : Priority (_Priority) {
	}
	
	~LTask() {
	}
	
	bool IsRunning() {
		return Left != NULL || Right != NULL || TaskQueue == this;
	}
	
	void Enqueue() {
		LTask *Prior = TaskQueue;
		
		TaskQueue = this;
		
		this->Right = Prior;
		
		if (Prior) {
			Prior->Left = this;
		}
	}
	
	void Dequeue() {
		LTask *L = this->Left;
		LTask *R = this->Right;
		
		if (L) {
			L->Right = R;
		}
		
		if (R) {
			R->Left = L;
		}
		
		this->Left = this->Right = NULL;
		
		if (this == TaskQueue) {
			TaskQueue = R;
		}
	}
	
	virtual void Run() {
	}
};

#endif
