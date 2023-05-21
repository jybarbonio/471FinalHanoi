// AUTHOR: JOHN BARBONIO
// Professor Yoshii
// CS 471 Introduction to AI
// 18 May 2023

/*
NOTES AND NOTICES README:
	- The letter order is reversed from the example in the final word docx so that code is like a stack, so [SML| | ] is [LMS| | ], [ML| |S] is [LM| | S]
	-	the reverse letter order is for clarity and for easier mental conveyance and comprehension that disks == a FIFO-like stack data structure 
	- the tracePath function recursively prints back to the start state in reverse display order
	-	i.e. printing first the current state and the last printline being the original state
	-	I could not figure out the recursive functionality to where I could reverse it and did not want to cause errors for something that was already working.
*/

#include <vector>
#include <regex>
#include <iostream>
#include <stdlib.h>
#include <iterator>

using namespace std;

void displayVector(vector<vector<char>>);	// function prototype to use displayVector above its func coded position but keep organization


// FUNCTION "State"
// Purpose: data structure for saving necessary node data, augments 2d vector storing towers with traversal value and lookback
class State
{
public:
	// using vector for pop_back and ease of index read functionality, could have used stack but that had less functionality
	vector<vector<char>> towers;		// 2D vector, outer vector are the three towers, while inner represents the plate spots on the vector, 3x3
	// char to represent plates as characters or alternatively numbers
	vector<vector<char>> camefrom;		// saves the state before the current state vector for traceback purposes
	int g;	// the moves so far (cost so far)
	int h;	// estimated cost to goal
	int f;	// g + h or subjective goodness value
};

vector<State>frontier;		// stores valid states generated ahead of current State node
vector<State>been_there;	// stores already visited states to avoid repeat traversal

// FUNCTION "addtoBeenThere"
// Purpose: access func that takes traversed states and adds them to the been_there vector as traversal history
void addtoBeenThere(State x)
{
	been_there.push_back(x);
}

// FUNCTION "addtoFrontier"
// Purpose: access func for saving possible safe states ahead of current to the possible paths to take
void addtoFrontier(State x)
{
	frontier.push_back(x);
}

// FUNCTION "removefromFrontier"
// Purpose: upon traversal removes a state from Frontier storage and sets to current
void removefromFrontier(State x)
{
	vector<State> newfrontier;
	for (int k = 0; k < frontier.size(); k++)
		if (frontier[k].towers != x.towers)
			newfrontier.push_back(frontier[k]);

	frontier = newfrontier;
}

// FUNCTION "tracePath"
// Purpose: recursively parses through been_there, printing path from init state [SML| | ] to current state
void tracePath(State goal)
{
	State prev_elem = goal;

	// conditional to trace back all the way to [LMS||] initial condition
	// recursively traceback all the way to beginning, stopping at first state as it's the end of the iterator, first state camefrom being empty anyway
	if (goal.towers[0].size() < 3 || goal.towers[0][0] != 'L' && goal.towers[0][1] != 'M' && goal.towers[0][2] != 'S')
	{
		for (vector<State>::iterator i = been_there.begin(); i != been_there.end(); ++i)
		{
			if (i->towers == goal.camefrom) 
			{
				prev_elem.camefrom = i->camefrom;
				prev_elem.towers = i->towers;
			}
		}
		cout << "came from ";
		displayVector(goal.camefrom);	// display the camefrom, recursively
		cout << endl;
		tracePath(prev_elem);	// recursive using iterator pointer to save recursion state
	}
}

// FUNCTION "inFrontier"
// Purpose: checks if a potential next state was already stored in the frontier
bool inFrontier(State next)
{
	for (vector<State>::iterator i = frontier.begin(); i != frontier.end(); ++i)
	{
		// if a matching state already exists in the frontier, find better cost f (g+h)
		if (i->towers == next.towers)
		{
			// if next f is better than current f, replace it with that better performer or only return true as there is no performance optimization change
			if (i->f > next.f)
			{
				i->towers = next.towers;		
				i->camefrom = next.camefrom;
				i->f = next.f;		// replace with lower total g + h value
				i->g = next.g;
				i->h = next.h;
				return true;
			}
			else
			{
				return true;
			}
		}
	}
	cout << "Not in the frontier yet" << endl;
	return false;		// return false and report as eligible to add to frontier 
}

// FUNCTION generate
// Purpose: tests a possible path for the next state, catching if it's unsafe or already traversed
void generate(State next)
{
	cout << "Trying to add: ";
	displayVector(next.towers);

	if (next.towers[2].size() == 3 && next.towers[2][0] == 'L' && next.towers[2][1] == 'M' && next.towers[2][2] == 'S')  // the goal is reached
	{
		cout << "reached goal: ";
		displayVector(next.towers);
		cout << endl;
		tracePath(next);  // display the solution path

		exit(0);
	}

	// *** if been there before, do not add to frontier.
	// You may write a utility function check been-there and call it
	for (vector<State>::iterator i = been_there.begin(); i != been_there.end(); ++i)
	{
		// if next towers equals towers in a previous beenThere Node, then don't add it and return
		if (next.towers == i->towers)		// compare towers vectors, wow I'm surprised I did not have to create a 2D vector overload == for this
		{
			cout << "Already Visited" << endl;
			return;
		}
	}

	// checking for unsafe orders i.e. bigger plates above smaller ones
	// theoretically there is only ever one stack where the plates are > 1, so really only check the stack > 1
	for (int i = 0; i < 3; i++)
	{
		if (next.towers[i].size() > 1)	// theoretically only one disk should have a stack > 1
		{
			if (next.towers[i][0] == 'S' && next.towers[i][1] == 'M')		// unsafe if a medium is above a small
			{
				cout << "Unsafe" << endl;
				return;
			}
			if (next.towers[i][0] == 'S' && next.towers[i][1] == 'L')		// unsafe if a large is above a small
			{
				cout << "Unsafe" << endl;
				return;
			}
			if (next.towers[i][0] == 'M' && next.towers[i][1] == 'L')		// unsafe if a medium is under a large
			{
				cout << "Unsafe" << endl;
				return;
			}
		}
	}

	// if the state is safe, then set the h, and the g which was set already
	// approximation of needed moves to completion i.e. an educated guess or moves to completion
	int h = 3 - next.towers[2].size();		// how many plates there are from 3 in the right disk [ | |SML], that is the estimate to completion
	//int h = next.towers[2].size();		// alternative heuristic

	// updating h and f
	next.h = h;		// update heuristic estimate
	next.f = next.g + next.h;		// get a rough "goodness" indicator based on total moves so far + heuristic (lower is better)

	if (!inFrontier(next))		// if state candidate is not already in saved frontiers
		addtoFrontier(next);	// then add the new approved next to the frontier
}

// FUNCTION generateAll
// Purpose: given all possible move conditions, generate possible future state move candidates, check if safe and not duplicated
//		this should only ever check one tower
void generateAll(State current) 
{
	State next;		// store next as a variable to be manipulated into the next state, preserving current for recurring reassignment of base state

	current.g = current.g + 1;		// increment moves so far
	current.camefrom = current.towers;		// save current (now past) state as the coming from

	// check for ALL possibilities
	next = current;		// store current to be converted and tested by single generate func
	char t;		// t init for holding disk that is moved to another spot
	if (current.towers[0].size() > 0)
	{
		t = next.towers[0].back();		// save the 'rear' plate first, it will be popped and put elsewhere
		next.towers[0].pop_back();		// pop from rear or "top", this was my preferred data structure hence the letter order reversal for visual/mental clarity
		next.towers[1].push_back(t);	// add plate to new tower
		generate(next);					// test resulting state

		next = current;				// reset next and modify again
		t = next.towers[0].back();		// save plate to place elsewhere
		next.towers[0].pop_back();		// pop plate from tower
		next.towers[2].push_back(t);	// put on different tower
		generate(next);					// test resulting state
	}
	next = current;					// reset next and modify again
	if (current.towers[1].size() > 0)
	{
		t = next.towers[1].back();		// save plate to place elsewhere
		next.towers[1].pop_back();		// pop plate from tower
		next.towers[0].push_back(t);	// put on different tower
		generate(next);					// test resulting state

		next = current;					// reset next and modify again
		t = next.towers[1].back();		// save plate to place elsewhere
		next.towers[1].pop_back();		// pop plate from tower
		next.towers[2].push_back(t);	// put on different tower
		generate(next);					// test resulting state
	}
	next = current;					// reset next and modify again
	if (current.towers[2].size() > 0)
	{
		t = next.towers[2].back();		// save plate to place elsewhere 
		next.towers[2].pop_back();		// pop plate from tower
		next.towers[0].push_back(t);	// put on different tower
		generate(next);					// test resulting state

		next = current;				// reset next and modify again
		t = next.towers[2].back();		// save plate to place elsewhere
		next.towers[2].pop_back();		// pop plate from tower
		next.towers[1].push_back(t);	// put on different tower
		generate(next);					// test resulting state
	}
}

// FUNCTION bestofFrontier
// Purpose: compare and replace up the frontier state canditate vector looking for lowest combination of heuristic moves and total moves combination
State bestofFrontier()
{
	State lowest = frontier[0];		// init lowest to front of frontier for a start

	// start at beginning of vector, going up and replacing with lowest, simple linear search assuming few possible move states per branching factor of 3
	for (vector<State>::iterator i = frontier.begin(); i != frontier.end(); ++i)
	{
		if (i->f < lowest.f)
		{
			// replacing lowest values with whatever is lower that is encountered the frontier
			lowest.camefrom = i->camefrom;
			lowest.f = i->f;
			lowest.g = i->g;
			lowest.h = i->h;
			lowest.towers = i->towers;
		}
	}
	cout << "Best of frontier is: ";
	displayVector(lowest.towers);		// show lowest frontier for the current state's next
	cout << endl;

	return lowest;		// return so program moves to it
}

// FUNCTION displayFrontier
// Purpose: display the possible states saved in the frontier (called from main)
void displayFrontier()
{
	// displays the possible frontiers ahead of a given current state
	for (int k = 0; k < frontier.size(); k++)
	{
		displayVector(frontier[k].towers);
		cout << "g = " << frontier[k].g << " ";
		cout << "h = " << frontier[k].h << " ";
		cout << "f = " << frontier[k].f << endl;
	}
}

// FUNCTION displays 2D vector by looping through 3x3 vector chars
// utility function for towers vectors/stacks
void displayVector(vector<vector<char>> next)
{
	int inner = 0;		// inner index so that for loop j does not exceed plates in tower, avoiding error
	cout << "[";
	for (int i = 0; i < 3; i++)
	{
		inner = next[i].size();		// inner index so that for loop j does not exceed plates in tower, avoiding error
		for (int j = 0; j < inner; j++) 
		{
			cout << next[i][j];		// print existing char elem at that area
		}
		if (i < 2) 
		{
			cout << "|";		// divider visualizing towers
		}
	}
	cout << "] ";
	//cout << endl;		// removed this since it overly clrf spaces the information displayed
}

// MAIN METHOD
int main() 
{
	// initial [SML| | ] code block
	vector<vector<char>> towers;			// towers, used as size 3 (i.e. left, middle, right or tower 1, 2, 3)
	vector<vector<char>> init_camefrom;		// saves towers state prior to current state, except for init state which is blanked
	vector<char> disk;						// discs/plates that make up the elem spots of the tower, used as if size = 3 constrained
	towers.push_back(disk);				// tower 1
	towers.push_back(disk);				// tower 2
	towers.push_back(disk);				// tower 3 (we want LMS here)

	// push LMS to tower 1, completing initial state of towers
	towers[0].push_back('L');
	towers[0].push_back('M');
	towers[0].push_back('S');

	State current = {towers, init_camefrom, 0, 3, 3}; // redundant assignment, but follows guideline layout of fwdc.cpp
	addtoFrontier(current);		// add current state to frontier

	char ans = 'c';		// cin to pause looping

	// keep going until final state is achieved
	while (ans != 'n')
	{
		removefromFrontier(current);		// current is current, so it is no longer a frontier candidate
		addtoBeenThere(current);			// current has been visited

		cout << ">>>Expand: ";				
		displayVector(current.towers);		// show the towers status so far
		cout << endl;					
		generateAll(current);			// new candidate move states are tested then added to frontier

		cout << "Frontier is:" << endl;
		displayFrontier();		// display the possible frontiers and their viability as efficient move candidates

		current = bestofFrontier(); // pick the lowest f, or predicted best performer out of frontier
		cin >> ans;		// wait for user input before continuing to next state processing

	}
}