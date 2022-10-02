
// Sample Run: 5 6 8 3 4 2 1 7 0
// Sample Run 2: 6 7 3 1 8 4 2 0 5
// Sample Run 3: 1 2 3 4 5 6 7 0 8

/*
 * Summary:
 * g-score is the number of moves we took since the initial state
 * h-score is either the number of misplaced pieces (0) or the total sum of
 * distance of piece from its correct location
 * we evaluate and choose the next move based on the lower f-score (h-score + g-score)
 */

#include <iostream>
#include <unordered_map>
#include <set>
#include <utility>

using namespace std;

typedef pair<int, int> pii;

const int SIZE = 3;
const int SQR_SIZE = SIZE * SIZE;

const string moveStr[] = {"UP", "DOWN", "LEFT", "RIGHT"};

enum Move
{
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
};

enum TypeOfH
{
    BY_MISPLACED = 0,
    BY_DISTANCE = 1
};

TypeOfH DEFAULT_H = BY_DISTANCE;

// save the state's key and the pair has the parent's key + direction
unordered_map<int, pii> parent;

// keep tracks of the states are currently evaluating, store them in the ascending order of f-score
set<class State> Processing;

// visited map to keep track of the child states
unordered_map<int, bool> inOpen;

unordered_map<int, bool> inClose;

#pragma region State
class State
{
public:
    int board[SQR_SIZE];
    int g; // g-score
    int h; // h-score

    State()
    {
        g = 0;
        h = 0;
    }

    State(int board[SQR_SIZE], int g = 0, int h = 0)
    {
        for (int i = 0; i < SQR_SIZE; i++)
            this->board[i] = board[i];
        this->g = g;
        this->h = h;
    }

    // comparator to determine which state is better based on f-score
    bool operator<(const State &other) const
    {
        if (g + h == other.g + other.h)
            return getKey() < other.getKey();
        return g + h < other.g + other.h;
    }

    // comparator to check if 2 states are equal
    bool operator==(const State &other) const
    {
        return this->getKey() == other.getKey();
    }

    // returns a copy of the current state
    State *clone()
    {
        State *newState = new State();
        for (int i = 0; i < SQR_SIZE; i++)
            newState->board[i] = board[i];
        newState->g = g;
        newState->h = h;
        return newState;
    }

    // print the board
    void print()
    {
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
                cout << board[i * SIZE + j] << ' ';
            cout << endl;
        }
    }

    // convert the array into the integer number that represents it and
    // use that integer as a key for the state
    int getKey() const
    {
        int res = 0;
        for (int i = 0; i < SQR_SIZE; i++)
            res = res * 10 + board[i];
        return res;
    }
};
#pragma endregion State

#pragma region Operator

// manage the move
class Operator
{
public:
    int dir;

    Operator(int dir)
    {
        this->dir = dir;
    }

    State *up(State *s)
    {
        pair<int, int> pos = findPos(s);
        int x = pos.first;
        int y = pos.second;
        if (x == SIZE - 1)
            return NULL;
        return swap(s, x, y, dir);
    }

    State *down(State *s)
    {
        pair<int, int> pos = findPos(s);
        int x = pos.first;
        int y = pos.second;
        if (x == 0)
            return NULL;
        return swap(s, x, y, dir);
    }

    State *left(State *s)
    {
        pair<int, int> pos = findPos(s);
        int x = pos.first;
        int y = pos.second;
        if (y == SIZE - 1)
            return NULL;
        return swap(s, x, y, dir);
    }

    State *right(State *s)
    {
        pair<int, int> pos = findPos(s);
        int x = pos.first;
        int y = pos.second;
        if (y == 0)
            return NULL;
        return swap(s, x, y, dir);
    }

    State *swap(State *s, int x, int y, int dir)
    {
        State *ns = s->clone();
        int nx = x;
        int ny = y;
        if (dir == UP)
        {
            nx = x + 1;
            ny = y;
        }
        else if (dir == DOWN)
        {
            nx = x - 1;
            ny = y;
        }
        else if (dir == LEFT)
        {
            nx = x;
            ny = y + 1;
        }
        else
        {
            nx = x;
            ny = y - 1;
        }
        ns->board[x * SIZE + y] = ns->board[nx * SIZE + ny];
        ns->board[nx * SIZE + ny] = 0;
        return ns;
    }

    State *move(State s)
    {
        if (dir == UP)
            return up(&s);
        else if (dir == DOWN)
            return down(&s);
        else if (dir == LEFT)
            return left(&s);
        else
            return right(&s);
    }

    // return the position of 0 in 2d array
    pair<int, int> findPos(State *s)
    {
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (s->board[i * SIZE + j] == 0)
                    return make_pair(i, j);
        return make_pair(0, 0);
    }
};

#pragma endregion Operator

#pragma region Hfunctions
// calculate h score by # of misplaced pieces
int calHByMisplaced(State s, State g)
{
    int res = 0;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (s.board[i * SIZE + j] != g.board[i * SIZE + j])
                res++;
    return res;
}
// calculate h score by the distance of all pieces from its correct place
int calHByDistance(State s, State g)
{
    int res = 0;
    for (int x = 1; x < SQR_SIZE; x++)
    {
        int sx, sy;
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (s.board[i * SIZE + j] == x)
                    sx = i, sy = j;
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (g.board[i * SIZE + j] == x)
                    res += abs(sx - i) + abs(sy - j);
    }
    return res;
}

int Hx(State s, State g)
{
    if (DEFAULT_H == BY_MISPLACED)
        return calHByMisplaced(s, g);
    else
        return calHByDistance(s, g);
}

#pragma endregion Hfunctions

bool checkInOpen(State s)
{
    return !(inOpen.find(s.getKey()) == inOpen.end());
}

bool checkInClose(State s)
{
    return !(inClose.find(s.getKey()) == inClose.end());
}

void print(int s)
{
    int data[SQR_SIZE];
    for (int i = SQR_SIZE - 1; i >= 0; i--)
    {
        data[i] = s % 10;
        s /= 10;
    }
    cout << "\nBoard:\n";
    for (int i = 0; i < SIZE; i++)
    {
        cout << '\t';
        for (int j = 0; j < SIZE; j++)
            cout << data[i * SIZE + j] << ' ';
        cout << endl;
    }
}

// print the path to goal
void path(int s)
{
    if (parent[s].first != 0)
    {
        path(parent[s].first);
        cout << "\nMove: " << moveStr[parent[s].second];
    }
    print(s);
}

int main()
{
    char playAgain = 'y';
    while (playAgain == 'y')
    {
        Processing.clear();
        inOpen.clear();
        inClose.clear();
        parent.clear();
        int initBoard[SQR_SIZE];

        /*
         * Goal State:
         * 1 2 3
         * 4 5 6
         * 7 8 0
         */
        int goalBoard[SQR_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 0};

        cout << "Enter the initial board: " << endl;
        for (int i = 0; i < SQR_SIZE; i++)
            cin >> initBoard[i];

        cout << "Choose the type of heuristic evaluation function." << endl;
        cout << "Enter 0 for evaluating based on the number of misplaced pieces. " << endl;
        cout << "Enter 1 for evaluating based on the sum of distance of pieces from their "
             << "correct position." << endl;
        int typeH;
        cin >> typeH;
        DEFAULT_H = typeH == 0 ? BY_MISPLACED : BY_DISTANCE;

        State S(initBoard);
        State G(goalBoard);

        // print initial state and goal state
        cout << "Initial State: " << endl;
        S.print();
        cout << "Goal State:" << endl;
        G.print();

        Processing.insert(S);
        S.h = Hx(S, G);
        inOpen[S.getKey()] = true;
        int cntVisited = 1;
        bool finished = false;
        while (!finished)
        {
            if (Processing.size() == 0)
            {
                cout << "Can not find a solution!" << endl;
                finished = true;
            }
            State O = *Processing.begin();
            Processing.erase(O);

            inOpen.erase(O.getKey());

            // mark the state as visited
            inClose[O.getKey()] = true;

            if (O == G)
            {
                cout << "Found a solution!" << endl;
                cout << "Number of step: " << O.g << endl;
                cout << "Number of visited nodes: " << cntVisited << endl;
                cout << "Here's the path to get to the goal:" << endl;
                path(O.getKey());
                finished = true;
            }
            // check the 4 (max) possible next moves with 4 directions
            for (int i = 0; i < 4; i++)
            {
                // try the move
                Operator op(i);
                State *child = op.move(O);
                if (child == NULL)
                    continue;
                bool isInOpen = checkInOpen(*child);
                bool isInClose = checkInClose(*child);
                cntVisited++;
                // if the child state is never visited before
                if (!isInOpen && !isInClose)
                {
                    // save the parent info
                    parent[child->getKey()] = make_pair(O.getKey(), i);
                    child->g = O.g + 1;
                    child->h = Hx(*child, G);
                    Processing.insert(*child);
                    inOpen[child->getKey()] = true;
                }
                else if (isInOpen)
                {
                    if (child->g > O.g + 1)
                    {
                        Processing.erase(*child);
                        child->g = O.g + 1;
                        Processing.insert(*child);
                    }
                }
                else if (isInClose)
                {
                    if (child->g > O.g + 1)
                    {
                        Processing.insert(*child);
                        child->g = O.g + 1;
                        inClose[child->getKey()] = false;
                        inOpen[child->getKey()] = true;
                    }
                }
            }
        }
        cout << "Do you want to play again? (y/n)" << endl;
        cin >> playAgain;
    }
    return 0;
}
