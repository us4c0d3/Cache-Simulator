# Cache-Simulator
2022-2 COMP411 컴퓨터구조 Computer Architecture
과제 본문 복붙

이 과제에서는 캐시 시뮬레이터의 구현을 통해 캐시의 기본적인 동작을 이해하는 것을 목표로 한다.

[팀구성]
이 과제는 1인, 혹은 2인 팀으로 수행 가능함.
팀 구성원은 결과물 제출 시 설명칸에 적고 대표로 한명만 제출한다.
2인으로 수행한 경우 두 멤버는 동일한 점수를 받는 것을 원칙으로 함.
팀원 점수 관련해서 예외 상황이 발생하여 문제가 있을 시 개별 컨택할 것.

[요구기능]
과제의 결과물은 cachesim.c 파일로 구현한다. 리눅스환경에서 컴파일 및 동작가능하여야 한다.
구현하는 캐시의 종류는 set associative 캐시이다. 그리고, 이 캐시의 크기와 block 크기는 가변이다.
따라서, 프로그램 시작 시 두 값들을 파라미터로 입력받고 set associativity(=set size, 몇 way인지)도 입력받는다. 전체 파라미터는 다음과 같다.

-s=<cache size>
-b=<block size> 
-a=<set size>
-f=<trace file name>

캐시와 block의 크기는 2의 지수승배이어야 한다. Block의 최소 크기는 1 word이며 단위는 byte임.
지정 가능한 캐시의 크기는 32B~1MB이다.
지정 가능한 block의 크기는 4B~256B이다.
Set크기는 2의 지수승배임. 즉, 1,2,4,8 ... 

Trace 파일(첨부 참조)은 메모리 접근 기록이며 이 기록들을 사용하여 캐시를 동작시키고 캐시의 내용을 반영하여야 한다. 프로그램 시작 시 메모리와 캐시는 전부 0으로 초기화된 상태이어야 한다. Trace파일의 메모리 접근 기록들은 다음과 같은 형식을 가진다.

<Address> <R/W> <data>

예) 00010008 W 33
      0001000C W 4
      00010000 R
      ...

첨부파일: sample.trc

메모리주소는 16진수로 표기되어 있고 크기는 32bit이며 R/W은 read/write인지를 나타낸다. W의 경우는 어떤 값을 쓸지가 10진수로 3번째 자리에 나온다. R의 경우는 3번째 data 값은 없다.
하나의 메모리 접근은 항상 기본 단위를 1 word 크기를 가정한다.
메모리 주소의 크기가 32bit이므로 4G영역의 모든 곳에 접근 가능하다. 메모리주소는 항상 8 character로 주어진다. 하나의 16진수 character는 4bit이므로 4x8=32비트이다. 구현 시는 메모리 4GB는 커서 실제로 다 할당받아 잡아두기 어려우므로 변경된 값이 있는 메모리의 주소와 그 값들만을 따로 적절한 자료구조를 만들어서 관리한다. 동작 시 임의의 시점에서 그때까지 건드리지 않은 주소의 메모리의 초기값은 모두 0이라고 가정한다.
캐시의 write 정책은 Write-back을 사용한다.
Set의 크기가 2이상일 경우는 쫓아내야 할 block의 결정은 set 중에서 가장 오래된 것을 기준으로 한다. 
캐시의 구조는 Tag, Valid, Dirty 비트가 각 캐시 line에 있다고 설정한다. Dirty bit의 경우는 1bit이며 W를 통해 그 line의 값이 변경되었으면 1로 set되며 R만 해서 값이 변경되지 않은 상태이면 0을 가진다. Dirty bit가 0일 경우는 그 line을 쫓아낼 때 쫓아낼 값을 메모리로 쓸 필요가 없으므로 그냥 덮어써버리면 됨.

시뮬레이터의 목적은 주어진 trace 파일을 돌린 후 캐시의 내용을 출력하고 여러가지 통계치를 출력하는 것이다.
Trace파일은 하나의 간단한 예제파일로서 제공되지만 구현 과제의 올바름을 테스트하기 위하여 직접 다양한 trace파일들을 직접 생성하여 캐시가 최종적으로 어떠한 값을 가져야 하는지 예상하고 이를 사용하여 작성한 코드가 맞게 동작하는지를 디버깅 하여야 한다.
Trace파일을 작성할 때에는 결과를 예상할 수 있을 정도의 간단한 여러 trace를 만드는 것이 관건임. 두세줄의 매우 짧은 trace파일들 부터 만들어보는 것을 권장함. 짧은 것들이 문제없이 동작하면 조금 더 긴 trace파일들을 만드는데 동작이 규칙적이어서 결과가 쉽게 예상이 되는 trace파일을 만들어서 테스트해 볼 것. 너무 복잡하고 임의의 값들을 가진 trace파일들은 정답을 예측하기 어려우므로 크게 도움이 되지 않음.
임의의 값과 순서로 trace 파일을 만들면 디버깅이 어려울 것임. 첨부된 샘플 Trace에 해당하는 정답은 주어지지 않음.

캐시 시뮬레이터의 실행은 다음과 같다. 

// Simulate 64B 2-way set associative cache with a block size of 8B
$ ./cachesim -s=64 -a=2 -b=8 -f=sample.trc
 
Trace파일명은 임의의 것이어도 무방함. 파라미터들의 순서는 고정되어 있지 않고 임의의 순서로 올 수 있음.
리눅스 환경에서의 실행 결과는 다음과 같다.
(아래의 예시 결과는 출력 형식만을 보여주기 위한 무작위 값임. 아래의 값들을 정답이라 간주해서 자신의 코드의 결과와 맞는지 비교하지 말 것.)
출력 형태는 스페이스 개수, 위치, 대소문자 등이 예시와 완벽하게 일치하여야 함.
Syntax 불일치 개수만큼  -1점씩 감점.

$ ./cachesim -a=2 -s=64 -b=8 -f=sample.trc
0: 00000001 00000001 v:1 d:1
   00000002 FFFC1100 v:1 d:0
1: 00000003 00000001 v:1 d:1
   0005FD04 00000001 v:1 d:0
2: 00000000 00000001 v:0 d:1
   00000001 01100001 v:1 d:1
3: ABAB0022 00011111 v:1 d:0
   11100001 00000001 v:1 d:1

total number of hits: 234
total number of misses: 31
miss rate: 11.7%
total number of dirty blocks: 4
average memory access cycle: 29.4
$ 
 
캐시의 내용을 출력할 때는 각 캐시라인의 인덱스 번호와 그 자리에 있는 라인 데이터의 값을 16진수로 출력한다. 캐시 내용은 한 word를 8글자 16진수로 출력한다. 위의 예에서는 block 크기가 8이므로 2 word 형태로 출력되었다. 소수점은 한자리까지만 출력함. 소수점 두번째 자리에서 반올림. 그리고, 2-way이므로 0:, 1:  등의 인덱스하에 두개의 캐시라인이 있음.
 

Cache hit은 1 cycle 소모함.
Cache miss penalty는 200 cycle임.
 
[평가] 
평가시 테스트용 몇개의 평가용으로 준비된 trace파일을 사용해서 돌려보고 결과가 맞게 나오는지에 따라 점수를 부여한다. Trace파일들은 짧고 간단한 것부터 점점 복잡한 것들로 구성됨.

평가방법의 세부사항은 상황에 따라 소폭 변경될 수 있다.
컴파일 오류로 인해 실행 불가시 0점 처리 됨.
컴파일 시 Warning 개수당 -1점 처리 됨. 컴파일 시 -Wall 옵션을 적용해서 나오는 모든 warning은 다 제거할 것.

[제출]
cachesim.c 파일만을 제출한다. 설명문서, 스크린샷, 압축파일 등은 제출하지 않는다.
제출시 팀원의 명단을 적고 대표로 한명만 제출한다.
단, 2인이상의 팀으로 한 경우 멤버별로 담당한 역할을 상세히 적어서 첨부, 제출할 것.
멤버의 기여도에 문제가 있을 경우 컨택할 것.

팀 중 한명만 제출. 이중으로 제출하지 말 것.
지각 제출 불허.


[부정행위 관련]
다른 팀과 어떠한 형태로든지 코드를 공유하는 것은 금지함. 코드 유사도 검사기를 사용하여 전수검사 예정.
부정행위 적발시 과목F부여됨. (원인제공자도 동일하게 적용)