
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

#include <ltask.hpp>

LTask *TaskQueue = NULL;

UINT16 TaskQueueSelectLength = 4;

void DoEvents() {
	LTask *selectedTask = NULL;
	LTask *taskQueue = TaskQueue;
	
	for (int i = 0; i < TaskQueueSelectLength && taskQueue; ++i, taskQueue = taskQueue->Right) {
		if (!selectedTask || selectedTask->Priority > taskQueue->Priority) {
			selectedTask = taskQueue;
		}
	}
	
	if (selectedTask) {
		selectedTask->Run();
		
		selectedTask->Dequeue();
		
		selectedTask->Destroy();
		
		delete selectedTask;
	}
}

void AddTask (LTask *task) {
	if (!task->IsRunning()) {
		task->Enqueue();
	}
}
