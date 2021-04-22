#include <stdio.h>
#include <stdlib.h>

//  Process 를 구조체로 정의  
typedef struct Process {          
    int PID;                      // identification number of the process
    int queue;                    // initial queue
    int at;                    // arrival time
    int n_cycle;				// 입력받은 사이클  
    int burst_arr_index;			// burst arr 를 차례로 참조하기 위한 index  
    int sum_burst_time;			// burst time 의 총합  
    int * burst_arr;               // int array for storing burst times
} Process;

// Ready Queue 를 정의  
// linked list 를 활용한 priority queue  
typedef struct RQ {
    struct RQ* next;   
    Process* data;      
} RQ;



Process    **  job_queue;           // processes before arriving ready queue
RQ       *   ready_queue0;      	// Q0, RR(time quantum = 2)
RQ       *   ready_queue1;      	// Q1, RR(time quantum = 4)
RQ    *  ready_queue2;        		// Q2, RR(time quantum = 8)
RQ       *   ready_queue3;       	// Q3, FCFS
Process    **  sleep_queue;         // processes requesting I/O system call
int        **  process_table;       // burst time 결과값 테이블 
int            process_num;			// 프로세스 숫자  

int global_time = 0; //while 문이 진행됨에 따라 1씩 증가  
int time_quantum; // RR 스케줄링 기법에 포함되는 TIME QUANTUM  



// func
// 초기 입력값을 ready queue 에 넣어주는 함수  
int push_queue(Process* process);

// 추가적인 process 가 있는지, job queue 를 이용한 체크  
int arrival_check(void);

// burst time 이 남았는지 확인하는 함수 
int burst_check(Process* process, int queue, int PID);

// fcfs 스케줄링  
Process *fcfs(int type);

// burst time 을 모두 소진한 프로세스 삭제  
void delete_process(Process* process, int global_time);

// 입력받은 정보를 queue 에 넣어주는 함수  
Process * init_process(int PID, int queue , int at, int n_cycle); 

// CHART 출력  
void print_table(void); 


int main(int argc, char *argv[]) {
	Process* new_process;
	int i, j, k;
    int pid, init_q, at, cycle, size_arr, burst_value;
	
	
	
    Process *current_process;	// 현재  진행중인 프로세스  
    int current_process_id; 	
    int current_queue;
    int prev_process_id;
    int remain_process;
    int line;
    int arrival_result;
    int break_check;
    int burst_time;
	
	FILE *file;	
	
	file = fopen("input.txt", "r");
	if(file == NULL){
		printf("error 0 : check your file path. \n");
		return 0;
	}
	
	fscanf(file, "%d", &process_num);
//	printf("process _ num  = %d \n", process_num);
	if(process_num < 0){
		printf("error 1 : process number should be positive. \n");
		return 0;
	}
	
	
	// 사용할 큐를 동적할당 한다.  
	job_queue = (Process**)malloc(sizeof(Process) * process_num);
	
	ready_queue0 = (RQ*)malloc(sizeof(RQ));
	ready_queue0->data = NULL;
    ready_queue0->next = NULL;
	
	
	ready_queue1 = (RQ*)malloc(sizeof(RQ));
	ready_queue1->data = NULL;
    ready_queue1->next = NULL;
	
	
	ready_queue2 = (RQ*)malloc(sizeof(RQ));
	ready_queue2->data = NULL;
    ready_queue2->next = NULL;
    
    ready_queue3 = (RQ*)malloc(sizeof(RQ));
    ready_queue3->data = NULL;
    ready_queue3->next = NULL;
    
    sleep_queue = (Process**)malloc(sizeof(Process) * process_num);
    for (i = 0; i < process_num; i++) {
        sleep_queue[i] = NULL;
    }    
    
    process_table = (int**)malloc(sizeof(int*) * process_num);
    for (i = 0; i < process_num; i++) 
	process_table[i] = (int*)malloc(sizeof(int) * 2);
    
    
    
    // 읽어온 정보를 동적할당한 큐들에 집어넣기
    for (i = 0; i < process_num; i++) {
        int total = 0;
        burst_value = 0;
        
		fscanf(file, "%d %d %d %d", &pid, &at, &init_q, &cycle);
        
        size_arr = (cycle * 2) - 1;        
		
		// 입력받은 정보에 따른 process 초기화 함수  
        new_process = init_process(pid, init_q, at, cycle);
        
        // burst arr 사이클에 따른 배열  사이즈 만큼 입력 받기  
        for (j = 0; j < size_arr; j++) {
            if(fscanf(file, "%d", &burst_value) == -1){
            	
            	printf("\nerror 3 : burst arr must follow cycle number constraints. \n");
				return 0 ;
			}
            
            total += burst_value;
            new_process->burst_arr[j] = burst_value;
        	
        }    
        new_process->sum_burst_time = total;
        job_queue[i] = new_process;
    }
	
	
	// 초기화  
    current_process = NULL;
    current_process_id = 0;
    current_queue = -1;
    prev_process_id = -1;
    remain_process = 1;
    line = 0;

    printf("  Process ID\n");
    
    
    // while 무한루프를 돌며 할당되는 프로세스를 수행한다.  
    while (1) {
    	
    	// 1. global time 과 각 프로세스의 at 를 arrival_check 함수를 통하여 비교하여
		// 추가적으로 할당해야하는 프로세스의 유무를 확인한다.  
    	if (remain_process == 1) {
            arrival_result = arrival_check();
            if (arrival_result == 0) {
                remain_process = 0;
            }
        }
        
        
        // 2. I/O 를 요청한 프로세스가 종료됨을 확인하고
        // ready queue 에 넣어준다.  
		for (i = 0; i < process_num; i++) {
        	if (sleep_queue[i] != NULL) {
        	
        		// sleep queue [i] 에 저장된 burst 시간 정보 가져오기  
				int time = sleep_queue[i]->burst_arr[sleep_queue[i]->burst_arr_index];
            	if (time == 0) {
            	
            		// 모두 소진 된 경우  
            		// cycle index 를 높여서 다음 burst time 을 탐색한다.  
              	  sleep_queue[i]->burst_arr_index += 1;
                
               		// queue 의 우선순위를 새로 조정한다.					    
					// 만약 0 ready queue 에 있는 경우 그대로 둔다.  
					// 만약 3 ready queue 에 있는 경우도 그대로 둔다.  
                	int queue = sleep_queue[i]->queue;
                	if( queue != 3) queue = queue > 0 ? queue - 1 : 0;
                	
                
                	// 갱신된 queue 값을 sleep_queue 에 들어간 process 에 전달 
					sleep_queue[i]->queue = queue;
                	push_queue(sleep_queue[i]); 
                	sleep_queue[i] = NULL;
            	}
        	}
    	}
		
		
        // 3. 실행가능한 프로세스를 확인   
        // current_process 에 값이 없는 경우 스케줄링을 시작한다.  
        if (current_process == NULL) {  
		        
			Process * tmp; 
			
			// 4개의 큐를 우선순위대로 돌면서 
			// 스케줄링 할 수 있는 큐를 찾아서 fcfs  를 수행한다. 
			// queue 0, 1, 2 의 경우 time quantum 이 존재하는 RR 기법이며.  
			// queue 3 의 경우 time FCFS 기법이다.  
			for(k=0; k<4; k++){
				tmp = fcfs(k);
				if(tmp != NULL){
					if(k != 3) time_quantum = 2*(k+1);
					else time_quantum = -1;
					
					current_process = tmp;
					break;
				}
			}            
            
            // 스케줄을 시도했음에도 실패한 경우 
			 
            // Fail to schedule
            if (current_process == NULL) {
                
				// break 인 상황인지 체크한다. 
				// break : 1 / not break : 0 이다.  
				break_check = 0;                
                for(k =0; k<process_num; k++){
                	if(sleep_queue[k] != NULL) break_check =1;
				}			
                
                
                // Finish if all processes are terminated
                // break 중인 프로세스도 없고, 
				// 아직 시작하지 못한 프로세스도 없다면, 종료한다.  
                if (remain_process == 0 && break_check == 0) {
                    for(i=0; i<line; i++) {
                        printf("|      | \n");
                    }
                    
                    line = 0;
                    printf("|  %d   | \n", prev_process_id);
                    printf("+-------------- %d\n\n", global_time);
                    break;
                }
                
                
                // Wait if the I/O requesting process remains
                current_process_id = 0;
                current_queue = -1;
            } else {
                current_process_id = current_process->PID;
                current_queue = current_process->queue;
                
            }
        }
        
        
        // 만약 현재 진행중인 프로세스가 있다면, (burst 중이다.)
		// 새로운 프로세스인지, 이전의 프로세스와 같은지 구분한다.    
        // 3-2. Run an existing process
        if(prev_process_id != current_process_id) {
            
            for(i=0; i<line; i++) {
                printf("|      | \n");
            }
            line = 0;
            if(prev_process_id == 0) {
                printf("| .... | \n");
            } 
            printf("+--------------clock--- %d\n", global_time);
            
            
            if(current_process_id == 0) {
                printf("| .... | \n");
            } 
			else {
				printf("|  %d   | \n", current_process_id);
            }                        
            prev_process_id = current_process_id;
            
        } else {
            line++;
        }
        
         
        // 4. Increase global time by 1
        // while 반복문 안에서 진행하며, global time 을 1 증가
		// 스케줄링 시에 받은 time quantum 또한 1 소모를 한다. 
		 
        global_time += 1;
        time_quantum -= 1;
        for (i = 0; i < process_num; i++) {
            if (sleep_queue[i] != NULL) {
                int index = sleep_queue[i]->burst_arr_index;
                sleep_queue[i]->burst_arr[index] -= 1;
            }
        }
        
    
        // current_process 에 NULL 인 경우, 즉 모든 프로세스의 cpu burst 가 소모되고
		// 오직 io burst 만 남은 상황을 말한다. 
		// 이 경우 while 문 처음으로 돌아간다.  
        if( current_process == NULL) continue;
        
        // 5. Consume allocated time quantum 1        
        // 남아있는 cpu burst 를 사용한다. 
		
		
		// 해당 index 위치의  burst_arr 에서 burst time 을 모두 소모한 경우 
		// case 1. process burst time 을 모두 소모한 경우  return -1
		// case 2. cpu burst 를 다 소모해서 io burst 를 요청한 경우 return 0  
		// case 3. 아직 cpu burst time 이 남아있는 경우 return 1  
        int check = burst_check(current_process, current_queue, current_process_id);
        
        
        // case 1, 2
        if (check != 1) {
            current_process = NULL;
            current_process_id = 0;
            current_queue = -1;
        } 
        
        // case 3 
		else {
            if (time_quantum == 0) {
            	// 0,1,2 ready queue 에 있는 경우, 우선순위를 하나 떨어뜨린다. 
                current_queue = current_queue < 3 ? current_queue + 1 : 3 ;
                current_process->queue = current_queue;
				                
                if (push_queue(current_process) == -1)
                    return (-1);
                    
                current_process = NULL;
                current_process_id = 0;
                current_queue = -1;
            }
        }
    }   
    
    fclose(file); 
    
	
	free(job_queue);
    free(ready_queue0);
    free(ready_queue1);
    free(ready_queue2);
    free(ready_queue3);
    free(sleep_queue);
    free(process_table);
    
    
    print_table();
    
    return (0);
}


// allocate memory for each process and initiate
Process* init_process(int _PID, int _queue, int _at, int _n_cycle) {
    int arr_size;

    Process* new_process = (Process*)malloc(sizeof(Process));
    if (new_process == NULL)
        return (new_process);
    new_process->PID = _PID;
    new_process->queue = _queue;
    new_process->at = _at;
    new_process->n_cycle = _n_cycle;
    new_process->burst_arr_index = 0;
    new_process->sum_burst_time = 0;
    arr_size = (_n_cycle * 2) - 1;
    new_process->burst_arr = (int*)malloc(sizeof(int) * arr_size);

    return (new_process);
}

void print_table(void) {
	int i;
    double AverageTT = 0;
    double AverageWT = 0;
    printf("process table:\n");
    printf("+-------+-------------------+----------------+\n");
    printf("|  PID  |  Turnaround Time  |  Waiting Time  |\n");
    printf("+-------+-------------------+----------------+\n");
    for(i=0; i<process_num; i++) {
        AverageTT += process_table[i][0];
        AverageWT += process_table[i][1];
        printf("| %5d | %17d | %14d |\n", i+1, process_table[i][0], process_table[i][1]);
    }
    printf("+-------+-------------------+----------------+\n");
    printf("\n");
    AverageTT /= process_num;
    AverageWT /= process_num;
    printf("Average Turnaround Time : %.2lf\nAverage Wating Time     : %.2lf\n", AverageTT, AverageWT);
}




// push the process proper ready queue
int push_queue(Process* process) {
    RQ* head;
    RQ* rq;
    int queue_num;
    int pid;

    queue_num = process->queue;
     
	pid = process->PID - 1;
    switch (queue_num) {
    case 0:
        head = ready_queue0;
        break;
    case 1:
        head = ready_queue1;
        break;
    case 2:
        head = ready_queue2;
        break;
    case 3:
        head = ready_queue3;
        break;
    default:
        break;
    }
    
    rq = (RQ*)malloc(sizeof(RQ));
    rq->data = process;
    rq->next = NULL;
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = rq;
    return (0);
}

int arrival_check(void) {
    int result, i;

    result = 0;
    for (i = 0; i < process_num; i++) {
        if (job_queue[i] != NULL) {
            if (job_queue[i]->at == global_time) {
                push_queue(job_queue[i]);
                job_queue[i] = NULL;
            } else {
                result = 1;
            }
        }
    }
    return (result);
}

int burst_check(Process* process, int queue, int PID) {
    int result;
    int remain;
    int n_cycle;
    int index;
    int arr_size;
    
    remain = process->burst_arr[process->burst_arr_index] - 1 ;
    process->burst_arr[process->burst_arr_index] = remain;
    
    // 해당 index 위치의  burst_arr 에서 burst time 을 모두 소모한 경우 
	// case 1. process burst time 을 모두 소모한 경우  return -1
	// case 2. cpu burst 를 다 소모해서 io burst 를 요청한 경우 return 0  
    if (remain == 0) {
        n_cycle = process->n_cycle;
        index = process->burst_arr_index;
        arr_size = (n_cycle * 2) - 1;
        index += 1;
        process->burst_arr_index = index;
        if (arr_size == index) {
            delete_process(process, global_time);
            result = -1;
            return result;
        }
        result = 0;
        PID -= 1;
        sleep_queue[PID] = process;
        return result; 
    }
    
    // 아직 burst time 이 남아있는 경우  
    result = 1;
    return result;
}

// schedule a process from receiving ready queue (= type)
// if there's nothing to schedule, then return (NULL)
// else, return scheduled process pointer

Process* fcfs(int type) {
    Process* result;
    RQ* head;
    RQ* remove;

    switch (type) {
    case 0:
        head = ready_queue0;        
        break;
    case 1:
        head = ready_queue1;
        break;
    case 2:
    	head = ready_queue2;
    	break;
    case 3:
        head = ready_queue3;
        break;
    default:
        break;
    }
        
    if (head == NULL || head->next == NULL) {
        return (NULL);
    } 
    
    remove = head->next;
    result = remove->data;
    head->next = remove->next;
    free(remove);
    return (result);
}


void delete_process(Process* process, int global_time) {
    int pid;
    int arrival_time;
    int total_burst_time;
    int TT;
	
    pid = process->PID - 1;
    arrival_time = process->at;
    total_burst_time = process->sum_burst_time;
    TT = global_time - arrival_time;
    
    process_table[pid][0] = TT;
    process_table[pid][1] = TT - total_burst_time;
    
	free(process->burst_arr);
    free(process);
    return;
}

