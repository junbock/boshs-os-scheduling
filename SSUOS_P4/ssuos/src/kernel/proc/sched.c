#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <proc/switch.h>
#include <interrupt.h>
#include <device/console.h>

extern struct list level_que[QUE_LV_MAX];
extern struct list plist;
extern struct list slist;
extern struct process procs[PROC_NUM_MAX];
extern struct process *idle_process;

struct process *latest;

struct process* get_next_proc(struct list *rlist_target);
void proc_que_levelup(struct process *cur);
void proc_que_leveldown(struct process *cur);
struct process* sched_find_que(void);

int scheduling;

/*
	linux multilevelfeedback queue scheduler
	level 1 que policy is FCFS(First Come, First Served)
	level 2 que policy is RR(Round Robin).
*/

//sched_find_que find the next process from the highest queue that has proccesses.
struct process* sched_find_que(void) {
	int i,j;
	struct process * result = NULL;
	 
	proc_wake();

		/*TODO :check the queue whether it is empty or not  
		 and find each queue for the next process.
		*/
	while(result == NULL){
		if(!list_empty(&level_que[1]))//1번 큐 찾기
			result = get_next_proc(&level_que[1]);
		else if(!list_empty(&level_que[2]))//2번 큐 찾기
			result = get_next_proc(&level_que[2]);	
	}
	return result;
}

struct process* get_next_proc(struct list *rlist_target) {
	struct list_elem *e;

	for(e = list_begin (rlist_target); e != list_end (rlist_target);
		e = list_next (e))
	{
		struct process* p = list_entry(e, struct process, elem_stat);
		
		if(p->state == PROC_RUN)
		{
			list_remove(e);//리스트에서 뺌
			return p;
		}
	}
	return NULL;
}

void schedule(void)
{
	struct process *cur;
	struct process *next;
	struct process *tmp;
	struct list_elem *ele;
	int i = 0, printed = 0;

	scheduling = 1;	
	cur = cur_process;
	/*TODO : if current process is idle_process(pid 0), schedule() choose the next process (not pid 0).
	when context switching, you can use switch_process().  
	if current process is not idle_process, schedule() choose the idle process(pid 0).
	complete the schedule() code below.
	*/

	if ((cur -> pid) != 0) {
		//0번 프로세스가 아니면 0번 프로세스로 바꿈
		ele = list_begin(&level_que[0]);
		next = list_entry(ele, struct process, elem_stat);
		cur_process = next;
		switch_process(cur, next);
		return;
	}
		
		switch (latest -> que_level){
			case 1:
				//이전 프로세스가 1번큐였으면 2번큐로 바꿈
				proc_que_leveldown(latest);
				break;
			case 2:
				//이전 프로세스가 2번큐였으면 2번큐로 다시 푸쉬
				if(latest->state!=PROC_ZOMBIE&&latest->state!=PROC_STOP)
					list_push_back(&level_que[2], &latest->elem_stat);
				break;
		}
	proc_wake(); //wake up the processes 
	
	//print the info of all 10 proc.
	for (ele = list_begin(&plist); ele != list_end(&plist); ele = list_next(ele)) {
		tmp = list_entry (ele, struct process, elem_all);
		if ((tmp -> state == PROC_ZOMBIE) || 
			//(tmp -> state == PROC_BLOCK) || 
			//	(tmp -> state == PROC_STOP) ||
					(tmp -> pid == 0)) 	continue;
			if (!printed) {	
				printk("#=%2d t=%3d u=%3d ", tmp -> pid, tmp -> time_slice, tmp -> time_used);
				printk("q=%3d\n", tmp->que_level);
				printed = 1;			
			}
			else {
				printk(", #=%2d t=%3d u=%3d ", tmp -> pid, tmp -> time_slice, tmp->time_used);
				printk("q=%3d\n", tmp->que_level);
				}
			
	}
	//printk("que 1 size %d\n", list_size(&level_que[1]));
	//printk("que 2 size %d\n", list_size(&level_que[2]));
	if (printed)
		printk("\n");
	if ((next = sched_find_que()) != NULL) {
		printk("Selected process : %d\n", next -> pid);	
		//프로세스를 next 프로세스로 바꿈		
		intr_enable();	
		next->time_slice = 0;
		latest = next;
		scheduling = 0;
		cur_process = next;
		switch_process(cur, next);
		//프로세스를 0번 프로세스로 바꿈
		ele = list_begin(&level_que[0]);
		next = list_entry(ele, struct process, elem_stat);
		cur_process = next;
		switch_process(cur, next);
		intr_disable();
		return;
	}
	return;
}

void proc_que_levelup(struct process *cur)
{
	/*TODO : change the queue lv2 to queue lv1.*/
	//큐레벨을 1로 바꿔준 다음 1번큐에 푸쉬
	printk("proc%d ", cur->pid);
	printk("change the queue (2->1)\n");
	cur->que_level = 1;
	list_push_back(&level_que[1], &cur->elem_stat);
}

void proc_que_leveldown(struct process *cur)
{
	/*TODO : change the queue lv1 to queue lv2.*/
	//큐레벨을 2로 바꿔준 다음 2번큐에 푸쉬	
	printk("proc%d ", cur->pid);
	printk("change the queue (1->2)\n");
	cur->que_level = 2;
	if(cur->state!=PROC_ZOMBIE&&cur->state!=PROC_STOP)
		list_push_back(&level_que[2], &cur->elem_stat);
}
