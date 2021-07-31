## IPC



Sockets interface = Socket programming related APIs = Socket system call







**Unix Domain Sockets**

=> running on the same System 



**Network Sockets** 

=> different physical machines 



>중요포인트
>
>1. How Sockets based communication state machine works in general, 
>
>2. How socket APIs can be used 
>
>





-------

#### 번외 : Computer layer architecture 



- Hw layer

- Os / kernel layer

— system call interface. — =>. ***socket. layer***   /////////  Ex) socket(), accept(), connect(), sendto(), … malloc(), free()

- Applications  layer



By using system call applications ask some service to OS.

malloc() : program 이 OS 에 메모리를 요청하는 system call.



-------





### Socket message types

1. connection initiation request messages (CIR)

\- connection 이 성립되어야 client 가 server 에 Service request 를 보낼 수 있다. 



2. Service Request Messages (SRM)

\- connection 이 fully established 되어 있어야 메시지를 보낼 수 있다. 

\- Client request server to provide a service 



Server 는 CIR, SRM 을 다르게 인식 & 처리한다. 



-------



#### Client Server Communication 

State machine : 상태 기계, 컴퓨터 프로그램 설계하는데 쓰이는 수학적 모델, 한번에 한가지 상태만 가진다.  

State machine of Client Server communication 



1. Servers boots up, creates connection socket. 

socket() : “master socket file descriptor”, M 이라 하자 



Master 와 각 process 들이 connection 을 맺으면 

Server 안에 이를 저장한다. (Client handle)



2. M 이 Client handles 의 조상이다. 

Client handle 은 “data_sockets” 이라고 한다. 

- 연결이 된 Client 에서 server 에 service request message 를 보낸다. 

- OS 가 client 의 요청을 diverse(전환) 해서, client handle 로 전달한다. 

- server 에서 service response 를 client 로 다시 보낸다. 





3. Client handle 이 생성되면, (actual data exchange) server 는 Client handle (M 이 아니다!)을 사용해서 client 와 통신한다. 



4. Server 는 연결된 client handles(data socket) 과 DataBase 를 유지한다. 



5. M 은 connection 만들때만 사용, 실제 통신은 client handle 이 한다. 



6. accept() 은 server 에서 client handle 을 만들기 위한 system call 이다. 



7. linux 에서 Handles 은 “file descriptor” 로 불린다. 

M : “master socket file descriptor” , “Connection socket”

Client handle : “communication file descriptors”, “Data sockets”








>+) file descriptor 란
>
>- 시스템으로부터 할당 받은 파일을 대표하는 0 이 아닌 정수 값,
>
>- 프로세스에서 열린 파일의 목록을 관리하는 테이블의 인덱스 









