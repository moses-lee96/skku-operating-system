#include<stdio.h>
#include<stdlib.h>

/*
 process 가 갖는 page 개수 = np
 할당된 page frame 개수 = nf 
 window size 크기 = ws 
 page reference string 길이 = npf 
 
 page reference string 배열 = rf 
  
*/

int np, nf, ws, npf, hit_cnt, miss;
int rf[100001] = {0,};
int * pf;
int * ref_bit;
FILE *file;	



// if there is no error, it returns 1. 
int initialize(){
	int i;
	
	file = fopen("input2.txt", "r");
	if(file == NULL){
		printf("error 0 : check your file path. \n");
		return -1;
	}
	
	if(fscanf(file, "%d %d %d %d", &np, &nf, &ws, & npf) == -1){
		printf("error -1 : check your input format. \n");
		return -1;
	}
	
	for(i=0; i<npf; i++){
		if(fscanf(file,"%d", &rf[i]) == -1){
			printf("error -1 : check your input format. \n");
			return -1;
		}
	}
	
	// page frame 은  최대 20  
	pf = (int*) malloc(sizeof(int) * nf);
	if(pf ==NULL){
		printf("error 1 : memory allocation failed. \n");
		return -1;
	}
	
	
	for(i=0; i<nf; i++){
		pf[i] = 999;
	}
	
	return 1;
}

// page frame : Pf 에 데이터가 있는지 체크한다.  
int isHit(int data)
{
	int i, hit;
    hit=0;
    for(i=0; i<nf; i++){    	
        if(pf[i]==data){
            hit=1;
            break;
        } 
    }
 
    return hit;
}

// page frame 안에서 입력받은 데이터와 일치하는 요소의 index 
int getHitIndex(int data){
    int hitind, i;
    for(i=0; i<nf; i++){
        if(pf[i]==data){
        	
            hitind=i;
            break;
        }
    }
    return hitind;
}
 
void printState(){	
	int i;
    for (i=0; i<nf; i++){
        if(pf[i]!=999)
            printf(" %d",pf[i]);
        else printf("  ");
    }

}
 
void performance(int hit_cnt, int miss){

	printf("\n hitratio = %.2f ", (double)hit_cnt/(hit_cnt+miss));
	printf(" fault count = %d ", miss);
	
}

void free_alloc(){
	free(pf);
}

void MIN(){
	printf("\n -- MIN -- ");
	int i, j, k;
	hit_cnt = 0;
    miss = 0;
	
    if(initialize() == -1){
    	return 0;
	}
    
    // 최대 길이 20 인,  
    int near[21];
    for(i=0; i<npf; i++){
 
        printf("\n input %d :",rf[i]); 
        if(isHit(rf[i])==0){
 
            for(j=0; j<nf; j++){
            	
                int flag1=0;
                for(k=i; k<npf; k++){
                    if(pf[j]==rf[k]){
                        near[j]=k;
                        flag1=1;
                        break;
                    }
                    else
                        flag1=0;
                }
                
                // 찾지 못한 경우  
				// page reference 길이보다 큰 값을 할당한다. 
                if(!flag1)
                    near[j]=100002;
            }
            
            int max=-100002;
            int remote_idx;
            
            for(j=0; j<nf; j++){
            	
                if(near[j]>max){
                    max=near[j];
                    remote_idx=j;
                }
            }
            
            pf[remote_idx]=rf[i];
            miss ++;
 
            printState();
            printf(" %s ", "---FAULT");
        }
        
        else{
        	hit_cnt ++;
        	printState();
		}
            
    }
    performance(hit_cnt, miss);
}

void fifo(){	

	printf("\n -- FIFO -- ");
    if(initialize() == -1){
    	return 0;
	}
    int i,k;
    hit_cnt = 0;
    miss = 0;
    
    for(i=0; i<npf; i++){
    	
        printf("\n input %d :",rf[i]);
 
        if(isHit(rf[i])==0){
            for(k=0; k<nf-1; k++)
                pf[k]=pf[k+1];
 
            pf[k]=rf[i];
			miss ++;
            printState();
            printf(" %s ", "---FAULT");
        }
        
        else{
        	hit_cnt ++;
        	printState();
		}
            
    }
	performance(hit_cnt, miss);
}

void LRU(){
	
	printf("\n -- LRU -- ");
    if(initialize() == -1){
    	return 0;
	}
	
 	int repindex, min, flag1;
 	int i, j, k;
 	hit_cnt = 0;
	miss = 0;
	
    int recency_list[21];
    for(i=0; i<npf; i++){
 
        printf("\n input %d :",rf[i]);

        if(isHit(rf[i])==0){
 
            for(j=0; j<nf; j++){
                flag1=0;
                for(k=i-1; k>=0; k--){
                    if(pf[j]==rf[k]){
                        recency_list[j]=k;
                        flag1=1;
                        break;
                    }
                    else
                        flag1=0;
                }
                if(!flag1)
                    recency_list[j]=-999;
            }
            min=999;
            
            for(j=0; j<nf; j++){
            	
                if(recency_list[j]<min){
                    min=recency_list[j];
                    repindex=j;
                }
            }
            
            pf[repindex]=rf[i];
            miss ++; 
            printState();
            
            printf(" %s ", "---FAULT");
        }
        else{
        	hit_cnt ++;
        	printState();
		}
            
    }
    performance(hit_cnt, miss);
}

// tie 가 있는지 확인하는 함수  
int multiple_min(int * ar, int size, int min){
	int i;
	int min_cnt = 0;
	 
	for(i=0; i<size; i++){
		if(ar[i] == min) min_cnt ++;
	}
	return min_cnt;
}

void LFU(){
	
	printf("\n -- LFU -- ");
    int freq_list[21];
    int recency_list[101];
    int least,repin,sofarcnt, least_recency;
    int i, j, k;
    miss = 0; 
    hit_cnt = 0;
    
    if(initialize() == -1){
    	return 0;
	}
    for(i=0; i<nf; i++)
        freq_list[i]=0;
 
    for(i=0; i<npf; i++){
 
        printf("\n input %d :",rf[i]);
        
        // recency list 에 page 가 언제 참조되었는지 저장한다.  
        recency_list[rf[i]] = i;
        
        
        if(isHit(rf[i])){
            int hitind=getHitIndex(rf[i]);
            freq_list[hitind]++;
            hit_cnt ++;
            printState();
        }
        else{
            miss ++;
            
            // page frame 이 아직 차지 않은 경우  
            if( i < nf){
            	pf[i] = rf[i];
            	freq_list[i] = freq_list[i]+1;
			}
            
            // page frame 이 모두 찬 경우
			// * tie 가 발생할 경우, LRU 기법 사용  
            else{           	
                least=100002;                
                for(k=0; k<nf; k++){
                	// freq list 중에 가장 작은 값을 least 에 저장
					// repin 에 freq list 의 작은 값이 해당되는 index 저장  
					// TIE 가 발생하는 경우, LRU 기법 사용	
					
                    if(freq_list[k]<least){
						least = freq_list[k];
						repin = k;
                    }                   

				}
				
				// tie 처리 
				int tie_check = multiple_min(freq_list, nf, least);
				if(tie_check >1){
					int l, min_idx;
					int min = 1000002;
					
					for(l=0; l<nf; l++){
						if(min > recency_list[pf[l]]){
							min = recency_list[pf[l]];
							min_idx = l;
						}
					}
					repin = min_idx;
				}
				
                
                // 가장 작은 값의 page frame 값을 교체  
				pf[repin]=rf[i];
                sofarcnt=0;
                
                for(k=0; k<=i; k++)
                    if(rf[i]==rf[k])
                        sofarcnt=sofarcnt+1;
                freq_list[repin]=sofarcnt;
            }
 
            printState();
            printf(" %s ", "---FAULT");
        }
 
    }
//    for(i=0; i<npf; i++){
//    	printf(" %d ", recency_list[i]);
//	}
    performance(hit_cnt, miss);
}


void CLOCK(){
	printf("\n -- CLOCK -- ");
    int usedbit[21];
    int i, j, k;
    int victimptr=0;
    miss = 0;
    hit_cnt = 0;
    
    if(initialize() == -1){
    	return 0;
	}
    
    for(i=0; i<nf; i++)
        usedbit[i]=0;
    for(i=0; i<npf; i++){
    	
        printf("\n input %d:",rf[i]);
        if(isHit(rf[i])){
        	
			hit_cnt ++;
            printState();
            
            int hitindex=getHitIndex(rf[i]);
            if(usedbit[hitindex]==0)
                usedbit[hitindex]=1;
        }
        else{
            if(usedbit[victimptr]==1){
            	
                do{
                    usedbit[victimptr]=0;
                    victimptr++;
                    if(victimptr==nf)
                        victimptr=0;
                }while(usedbit[victimptr]!=0);
            }
            if(usedbit[victimptr]==0){
            	
                pf[victimptr]=rf[i];
                usedbit[victimptr]=1;
                victimptr++;
            }
            printState();
            printf(" %s ", "---FAULT");
            miss ++;
 
        }
        if(victimptr==nf)
            victimptr=0;
    }
    performance(hit_cnt, miss);
}



int isHit_ws(int data, int idx){
	int i, hit;
	hit = 0;	
	
	if(idx > ws){
		for(i=idx - ws; i<idx; i++){
			if(ref_bit[i] != 0){
				hit = 1;
				break;
			}
		}
	}
	
	
	return hit;
}

void printState_ws(){	
	int i;
    for (i=0; i<np; i++){
        if(ref_bit[i]!=-1)
            printf(" %d", i);
        else printf("  ");
    }

}

void WS(){
	
	printf("\n -- WS -- ");
	
    if(initialize() == -1){
    	return 0;
	}
    int i,j,k;
    hit_cnt = 0;
    miss = 0;   
   	
   	// page 개수 만큼 ref bit 배열을 선언해 준다 . 
    ref_bit = (int*) malloc(sizeof(int) * np);
    if(ref_bit == NULL){
    	printf("error 1 memory allocation failed\n");
    	return 0;
	}
    for(i=0; i<np; i++){
    	ref_bit[i] = -1;
	}
    
    for(i=0; i<npf; i++){
    	int flag = 0;
        printf("\n input %d :",rf[i]);
        

        int flag2= 0;
        if(i > ws){
        	for(j = i-ws; j < i; j ++){
        		if(rf[i-ws-1] == rf[j]){
        			flag2= 1;
				}
			}
			if( flag2 == 0 ){
				ref_bit[ rf[i-ws-1] ] = -1;
			}
		}
        
		if( i > ws){
			for(j = i -ws; j<i; j++ ){
				if(rf[i] == rf[j]){
					hit_cnt ++;
					printState_ws();	
					flag = 1;
					break;			
				}
			}
		}
		
		if(flag == 0){
			miss++;
			ref_bit[rf[i]] = 1;
				
			printState_ws();
			printf(" %s ", "---FAULT");
		}
	            
    }
	performance(hit_cnt, miss);
	
	free(ref_bit);
	
	
}


int main(){
	int i, j;

	MIN();
	free_alloc();
	
	fifo();
	free_alloc();
	
	LRU();
	free_alloc();
	
	LFU();
	free_alloc();
	
	CLOCK();
	free_alloc();
	
	WS();
	free_alloc();
	

	fclose(file);
	return 0;
	
}
