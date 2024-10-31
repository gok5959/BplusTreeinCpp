CSE3207 Project #2
Implementation of a Disk Based B+-Tree

컴퓨터공학과
12191701 권혁준

- What you have implemented and what you have not 
 과제에서 요구하는 모든 기능(create, insert, search, range-search, print)를 모두 구현하였습니다.

- Brief explanation of your implementation (Avoid any fancy designs and make it less than 1 page)
Create – 새로운 파일을 생성하고, 헤더가 담고 있는 세가지 정수인 block_size, root_bid, depth를 파일에 16진수 형태로 저장합니다.

Insert – 초기에 아무 블록도 없을 때는 리프 노드를 생성해서 root로 만드는 것으로 처리를 해주었습니다. 블록이 있다면 해당 key에 해당하는 말단 노드 위치를 찾는 find_leaf_node 함수를 실행합니다. 삽입 정렬 형태로 블록을 노드에 삽입하였고, 이 때 b+트리의 노드 조건을 만족하는지 확인하여서 만족하면 파일에 쓰고, 아니라면 split을 진행합니다. 

Split – split은 leaf노드 일 때와 non-leaf 노드 일 때를 나누어서 진행하였습니다. 왼쪽보다 오른쪽에 더 많은 엔트리를 위치하게 하였습니다. 그리고 find_leaf_node 함수를 진행할때, 해당 노드의 부모 노드를 map 자료구조에 매핑하며 진행하는데, 이를 해당 노드의 부모로 split이 전이될 때 활용해줍니다. 만약 해당 노드가 root였다면 새로운 root 노드를 만들어서 파일에 작성하여주고, 아니었다면 non_leaf_node에 대한 split을 진행합니다. Non_leaf_node의 split은 재귀적으로 실행하는데, 다른 구조는 leaf_node_split과 유사하나, leaf 노드와 non-leaf 노드의 split 방법이 다르기 때문에 그 점만 반영하여 주었습니다.

Search – search는 해당 key에 해당하는 말단 노드의 위치를 찾고, 그 위치에서 선형으로 엔트리들을 탐색해서 탐색에 성공하면, 그 key에 해당하는 value를 반환하는 형태로 구현하였습니다.

Range search – range search는 search와 마찬가지로 해당 Key에 해당하는 말단 노드의 위치를 찾고, 그 자리에서부터 선형으로 범위에 해당하는 노드의 key와 value를 벡터에 담고, 범위 내 탐색을 모두 마치면 그 벡터를 반환하는 형태로 구현하였습니다.

Print – print 함수는 root_id를 통해 파일에서 root 노드를 가져와서 해당 노드와 담겨있는 엔트리들의 next_bid를 통해 depth = 0, 1인 노드들을 꺼내와서 엔트리의 key를 모두 출력하도록 구현하였습니다. 추가적으로, 발생할 수 있는 예외들에 대비하여 depth = 0일 때, depth가 1 이상일 때를 나누어서 구현해주었고, queue에 해당 노드의 depth를 담아서, non-leaf 노드인지, leaf 노드인지를 구별하여 파일에서 읽어오게 해주었습니다. 

그리고 현재 트리가 b+ 트리 구조를 만족하는지 확인하기 위해(디버깅용) bfs를 수행하는 cur_tree_bfs() 함수와 말단 노드의 엔트리들이 정렬 상태를 유지하는지 확인하는 all_the_leaf 함수를 개인적으로 추가 구현하였습니다.
- How to compile and run
M1 macbook(터미널 -> zsh), Clion 2021.3.3, C++14
생성 : ./btree.exe c btree.bin 36
삽입 : ./btree.exe i btree.bin sample_insertion_input.txt
탐색 : ./btree.exe s btree.bin sample_search_input.txt my_search_output.txt
범위탐색 : ./btree.exe r btree.bin sample_range_search.txt my_range_search_output.txt
Print() : ./btree.exe p btree.bin my_print_output.txt

- Talk about your experience of doing this project
사실 수업을 듣고, 시험 공부를 하면서 b+트리의 구조에 대해 완벽히 이해하고 있었다고 생각했었습니다. 하지만 이번 과제를 하면서 개념에 혼동이 있던 부분들을 발견하였고, 구현 과정에서 이를 완벽히 습득하였습니다. 그리고 구현을 하면서, 지금은 엔트리에 실제 value를 담아서 구현하는데, 실제 파일에 있는 것을 색인화 하는 b+트리는 어떤 식으로 구현될지에 대해 의문점이 생기기도 하였습니다. 실제 데이터베이스에서 사용하는 것을 제가 실제로 사용하는 언어로 구현하는 과제였다 보니, 큰 성취감이 느껴지기도 했습니다. 파일 입출력이나 문자열 파싱, 특정 알고리즘에 대한 구현 등 데이터 베이스를 넘어서 종합적으로 코딩에 대한 실력을 늘리는 좋은 과제였던 것 같습니다. 또한, 3학년에 들어 5대 과목 등 이론 위주의 과목만 수강하다보니, 자연스럽게 실제 코딩에 대한 감이 떨어져 있었는데, 이번 과제를 계기로 다시 그 감을 되찾아 더욱 유익한 방학을 보낼 수 있게 될 것 같습니다.

- Write your available contact information such as phone number (just in case)

