//check for memory leaks
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//necessary for debugger to display the line of the memory leak
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include <queue>
#include <stack>
#include <assert.h>
#include <string>
#include <iostream>

//include these for Unicode
#include <io.h>
#include <fcntl.h>

#define POP_MISSIONARIES 3
#define POP_CANNIBALS 3
#define POP_DOCKS 2

#define L 0
#define R 1

#define verbose true

using std::string;

enum boatState {boatLeft, boatRight};

class ProblemState{
public:
	char missionaries[2]; //missionaries left and right
	char cannibals[2]; //cannibals left and right
	boatState boatSide;

	ProblemState::ProblemState(char mL,char mR,char cL,char cR, boatState b) : boatSide(b) {
		assert(mL+mR == POP_MISSIONARIES);
		assert(cL+cR == POP_CANNIBALS);
		missionaries[L] = mL;
		missionaries[R] = mR;
		cannibals[L] = cL;
		cannibals[R] = cR;
	}
	ProblemState::ProblemState() {
		missionaries[L] = POP_MISSIONARIES;
		missionaries[R] = 0;
		cannibals[L] = POP_CANNIBALS;
		cannibals[R] = 0;
		boatSide = boatState::boatLeft;
	}
};

enum ProblemAction {m2,m1,m1c1,c1,c2}; //all possible boat transfers
#define numActions 5

//use wstring throughout so that unicode can be printed
std::wstring problemString(ProblemAction pa) {
	switch(pa) {
	case m2:
		return L"2 missionaries";
	case m1:
		return L"1 missionary";
	case m1c1:
		return L"1 missionary & 1 cannibal";
	case c1:
		return L"1 cannibal";
	case c2:
		return L"2 cannibals";
	default:
		return L"drown";
	}
}

struct SearchNode {
	ProblemState nodeState;
	SearchNode* parent;
	ProblemAction action; //action to get here from parent
	int pathCost; //total cost so far
	int depth; //depth from initial node

	SearchNode(){} //default constructor
	SearchNode(ProblemState ns,SearchNode* p,ProblemAction a,int pc,int d)
		: nodeState(ns),parent(p),action(a),pathCost(pc),depth(d) {}
};

bool isGoal(SearchNode* n) {
	if(n->nodeState.missionaries[R] == POP_MISSIONARIES &&
		n->nodeState.cannibals[R] == POP_CANNIBALS)
		return true;
	else
		return false;
}

void generateChildren(SearchNode *n, std::queue<struct SearchNode*> &searchQueue) {
	//analyses acceptable actions for this state to generate child nodes
	char s = (char)n->nodeState.boatSide; //this side
	char o = s ^ (char)1; //other side (destination for the trip being prepared by this function)
	int familySize = searchQueue.size();
	for(char a = 0; a <= numActions; a++) {
		switch(a) {
		case m2:
			if(n->nodeState.missionaries[s] >= 2) {
				if(n->nodeState.missionaries[s]-2 == 0 ||
					(n->nodeState.missionaries[s]-2 >= n->nodeState.cannibals[s] &&
					n->nodeState.missionaries[o]+2 >= n->nodeState.cannibals[o])) {
						//add to queue
						ProblemState ns = ProblemState(n->nodeState.missionaries[L] + 2*s - 2*o,
							n->nodeState.missionaries[R] -2*s + 2*o,
							n->nodeState.cannibals[L],
							n->nodeState.cannibals[R],
							(boatState)o);
						SearchNode* nextNode = new SearchNode(ns, n,(ProblemAction)a,n->pathCost+1,n->depth+1);
						searchQueue.push(nextNode);
				}
			}
			break;
		case m1:
			if(n->nodeState.missionaries[s] >= 1) {
				if(n->nodeState.missionaries[s]-1 == 0 ||
					(n->nodeState.missionaries[s]-1 >= n->nodeState.cannibals[s] &&
					n->nodeState.missionaries[o]+1 >= n->nodeState.cannibals[o])) {
						//add to queue
						ProblemState ns = ProblemState(n->nodeState.missionaries[L] + s - o,
							n->nodeState.missionaries[R] -s + o,
							n->nodeState.cannibals[L],
							n->nodeState.cannibals[R],
							(boatState)o);
						SearchNode* nextNode = new SearchNode(ns, n,(ProblemAction)a,n->pathCost+1,n->depth+1);
						searchQueue.push(nextNode);
				}
			}
			break;
		case m1c1:
			if(n->nodeState.missionaries[s] >= 1 && n->nodeState.cannibals[s] >= 1) {
				if(n->nodeState.cannibals[o] <= n->nodeState.missionaries[o]) {
					//add to queue
					ProblemState ns = ProblemState(n->nodeState.missionaries[L] + s - o,
						n->nodeState.missionaries[R] -s + o,
						n->nodeState.cannibals[L] + s - o,
						n->nodeState.cannibals[R] -s + o,
						(boatState)o);
					SearchNode* nextNode = new SearchNode(ns, n,(ProblemAction)a,n->pathCost+1,n->depth+1);
					searchQueue.push(nextNode);
				}
			}
			break;
		case c1:
			if(n->nodeState.cannibals[s] >= 1) {
				if(n->nodeState.cannibals[o]+1 <= n->nodeState.missionaries[o] ||
					n->nodeState.missionaries[o] == 0) {
					//add to queue
					ProblemState ns = ProblemState(n->nodeState.missionaries[L],
							n->nodeState.missionaries[R],
							n->nodeState.cannibals[L] + s - o,
							n->nodeState.cannibals[R] -s + o,
							(boatState)o);
						SearchNode* nextNode = new SearchNode(ns, n,(ProblemAction)a,n->pathCost+1,n->depth+1);
						searchQueue.push(nextNode);
				}
			}
			break;
		case c2:
			if(n->nodeState.cannibals[s] >= 2) {
				if(n->nodeState.cannibals[o]+2 <= n->nodeState.missionaries[o] ||
					n->nodeState.missionaries[o] == 0) {
					//add to queue
					ProblemState ns = ProblemState(n->nodeState.missionaries[L],
							n->nodeState.missionaries[R],
							n->nodeState.cannibals[L] + 2*s - 2*o,
							n->nodeState.cannibals[R] -2*s + 2*o,
							(boatState)o);
						SearchNode* nextNode = new SearchNode(ns, n,(ProblemAction)a,n->pathCost+1,n->depth+1);
						searchQueue.push(nextNode);
				}
			}
		default:
			break;
		}
	}
	if(verbose)
		wprintf(L"%d %ls added to fringe (total %d)\n",searchQueue.size()-familySize,(searchQueue.size() == 1 ? L"child" : L"children"),
		searchQueue.size());
}

int main(int argc, char** argv) {
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); //show memory leaks when program exits
	_setmode(_fileno(stdout),_O_U16TEXT); //print unicode properly

	std::queue<struct SearchNode*> searchQueue; //use pointers so that nodes can be kept on heap
	SearchNode* start = new SearchNode();
	start->nodeState = ProblemState(3,0,3,0,boatLeft);
	start->parent = NULL;
	start->pathCost = 0;
	start->depth = 0;
	searchQueue.push(start);

	SearchNode* goal;

	bool visitedTable[POP_MISSIONARIES+1][POP_CANNIBALS+1][POP_DOCKS] = {false}; //cases could be [0-3][0-3][0/1]
	std::vector<SearchNode*> nodeMemory; //look after the heap

	while(searchQueue.size() >= 1) {
		SearchNode* n = searchQueue.front();
		searchQueue.pop();
		
		if(isGoal(n)) {
			goal = n;
			nodeMemory.push_back(n); //ensure it gets deleted with everything else
			break;
		}

		if(visitedTable[n->nodeState.missionaries[L]][n->nodeState.cannibals[L]][n->nodeState.boatSide] == true) {
			//skip this one
			if(verbose) {
				wprintf(L"déjà vu node: m(%d, %d), c(%d,%d), boat %ls\n",n->nodeState.missionaries[L],n->nodeState.missionaries[R],
					n->nodeState.cannibals[L], n->nodeState.cannibals[R], (n->nodeState.boatSide == boatLeft ? L"left" : L"right"));
			}
			delete n;
			n = NULL;
			continue;
		} else {
			nodeMemory.push_back(n); //keep it from being forgotten
			//mark n as visited
			visitedTable[n->nodeState.missionaries[L]][n->nodeState.cannibals[L]][n->nodeState.boatSide] = true;
			//generate children
			if(verbose) {
				wprintf(L"generating children for m(%d,%d), c(%d,%d), boat %ls\n",n->nodeState.missionaries[L],n->nodeState.missionaries[R],
					n->nodeState.cannibals[L],n->nodeState.cannibals[R],(n->nodeState.boatSide == boatLeft? L"left" : L"right"));
			}
			generateChildren(n,searchQueue);
		}
	}

	//goal reached
	//clear queue
	for(int q = 0; q < searchQueue.size();) {
		SearchNode* qn = searchQueue.front();
		searchQueue.pop();
		delete qn;
		qn = NULL;
	}

	//reverse path
	std::stack<SearchNode*,std::deque<SearchNode*>> routePlan;
	wprintf(L"Transporting the missionaries and cannibals involves %d trips:\n",goal->pathCost);
	do {
		routePlan.push(goal);
		goal = goal->parent;
	} while(goal->parent != NULL);
		
	//iterate through path
	for(int sn = 0; sn < routePlan.size();) {
		goal = routePlan.top();
		routePlan.pop();
		wprintf(L"%ls to %ls\n",problemString(goal->action).c_str(),(goal->nodeState.boatSide == boatLeft ? L"left" : L"right"));
	}

	//free memory
	for(int i=0; i < nodeMemory.size(); i++) {
		delete nodeMemory[i];
		nodeMemory[i] = NULL;
	}
	
	return 0;
}