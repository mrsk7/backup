#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>
#include<limits.h>
#include <stdio.h>
#define BSIZE 1<<15

using namespace std;

char buffer[BSIZE];
long bpos = 0L, bsize = 0L;

long readLong() 
{
	long d = 0L, x = 0L;
	char c;

	while (1)  {
		if (bpos >= bsize) {
			bpos = 0;
			if (feof(stdin)) return x;
			bsize = fread(buffer, 1, BSIZE, stdin);
		}
		c = buffer[bpos++];
		if (c >= '0' && c <= '9') { x = x*10 + (c-'0'); d = 1; }
		else if (d == 1) return x;
	}
	return -1;
}

struct Edge;

struct Node{
    long long int id, key;
    unsigned long pos;
    vector<Edge*> neigbors;
    bool inHeap, isGas;    
};

struct Edge{
    Node *point1, *point2;
    long long int weight;
};

vector<Node*> nodes;
vector<Edge*> edges;
vector<Node*> myHeap;
vector<Node*> track;

bool isLeaf(Node * s) {
    return ((s -> pos * 2) > myHeap.size());
}

unsigned long parent(Node *source){
    return (source -> pos) / 2;
}

unsigned long left_child(Node *source){
    return (source -> pos) * 2;
}


unsigned long right_child(Node *source){
    return (source -> pos) * 2 + 1;
}

void swap_Nodes(int i, int j){
    swap(myHeap.at(i) -> pos, myHeap.at(j) -> pos);
    Node * first = myHeap.at(i);
    myHeap.at(i) = myHeap.at(j); 
    myHeap.at(j) = first;
}

void decreaseKey(Node * s){
    while((parent(s) != 0) && (s -> key < myHeap.at(parent(s)-1) -> key)){
        swap_Nodes(s-> pos-1,parent(s) - 1);
    }
}

void increaseKey(Node * s){
    bool end =false;
    while(! isLeaf(s) && !end){
        
        Node * left = myHeap.at(left_child(s) -1);
        
        Node * minimal;
        if(right_child(s) < myHeap.size()){
            Node * right = myHeap.at(right_child(s) -1);
            minimal = (left -> key < right -> key) ? left : right;
            if(minimal -> key < s-> key)
                swap_Nodes((s -> pos) -1, (minimal -> pos) -1);
            else
                end = true;
        }else{
           minimal = left;
           if(minimal -> key > s-> key){
                end = true;    
            }else
            swap_Nodes((s -> pos) -1, (minimal -> pos) -1);
        }
    }    
}

Node * extractMin(){
    swap_Nodes(0,myHeap.size()-1);
    Node * out = myHeap.at(myHeap.size()-1);
    myHeap.pop_back();
    if(myHeap.size() > 0)
        increaseKey(myHeap.at(0));
    return out;
}

void insertInHeap(Node* s){
    myHeap.push_back(s);
    s -> pos = myHeap.size();
    decreaseKey(s);
}


void Dijkstra(long N){
    
    for(long i =0; i < N; i++){
        Node* tmp = nodes.at(i);
        if(tmp -> isGas ){
            tmp -> key = 0;
            insertInHeap(tmp);
        }else{
            insertInHeap(tmp);
        }
    }
    
    for(long i =0 ; i < N; i++){
        Node * minimum = extractMin();
        minimum -> inHeap = false;
        long long int d = minimum -> key;
        for(unsigned long j =0; j < minimum->neigbors.size(); j++ ){
            Node* point1 = minimum -> neigbors.at(j) -> point1;
            Node* point2 = minimum -> neigbors.at(j) -> point2;
            Node* next = (point1 -> id == minimum -> id) ? point2 : point1;
            int next_d = d + minimum -> neigbors.at(j) -> weight;
            if(next_d < next -> key && next -> inHeap){
                next -> key = next_d;
                decreaseKey(next);
            }
        }
    }
}

long long int lengthTrack(long L){
    Node* start = track.at(0);
    long  long int sum =0; 
    for(long i =1; i < L; i++){
        Node* end = track.at(i);
        bool stop = false;
        for(unsigned long j=0; j < end-> neigbors.size() && !stop; j++){
            Edge * cur = end -> neigbors.at(j);
            if(cur -> point1 == start || cur -> point2 == start){
                sum += cur -> weight;
                stop = true;
            }
        }
        start = end;  
    }
    return sum;
} 

bool compFun(Node* a, Node *b){
    return (a -> key < b -> key);
}

int main(int args, char** argv) {
    long int N = readLong();
    //cout << N << endl;
    long int M = readLong(); 
    long int K = readLong();
    long int L = readLong(); 
    long int B = readLong();
        
    for(long i =0;i < N; i++){
        Node* tmp = new Node;
        tmp -> id = (long long int) i+1;
        tmp -> inHeap = true;
        tmp -> key = LLONG_MAX;
        tmp -> isGas = false;
        nodes.push_back(tmp);
    }
    
    for(long i = 0; i < M ;i++){
        long int point1 = readLong();
        long int point2 = readLong();
        long int weight = readLong();
        
        Edge * tmp = new Edge;
        Node* node1 = nodes.at(point1-1);
        Node* node2 = nodes.at(point2-1);
        tmp -> point1 = node1;
        tmp -> point2 = node2;
        tmp -> weight = weight;
        node1->neigbors.push_back(tmp);
        node2->neigbors.push_back(tmp); 
        edges.push_back(tmp);
    }
    
    for(long i = 0; i < K; i++){
        long int j = readLong();
        track.push_back(nodes.at(j-1));
    }
    
    for(long i = 0; i < B; i++){
        long int j = readLong();
        nodes.at(j-1) -> isGas = true;
    }
    
    Dijkstra(N);
    
    long long int result = lengthTrack(K);
    sort(track.begin()+1,track.end()-1,compFun);
    
    for(long i =1; i <= L ; i++){
        result += track.at(i) -> key;
    }
    
    cout << result << endl;
    
    nodes.clear();
    edges.clear();
    track.clear();
}