#include <iostream>
#include <tuple>
#include <thread>
#include <chrono>
#include <map>
#include <vector>
#include <random>
#include <time.h>
#include <algorithm>

using namespace std;
using namespace std::chrono_literals;

/**
 * an enumeration to determine the type of the piece
 */
enum Type{
    ROCK,PAPER,SCISSORS,MOUNT,FLAG
};

const int TIMEOUT = 400; // maximum number of milliseconds that a player is allowed to take

/**
 * an enumeration to determine the owner of the piece
 */
enum Owner{
    ZERO,ONE,NA
};


/** a class to give the position of a piece in a tuple
 */
class Position{
private:
    std::tuple<int, int> pos;
public:
    Position() {}
    Position(int r,int c) {
        pos=std::make_tuple(r,c);
    }
    int getAt(int i) const {
        if (i==0) return get<0>(pos);
        if (i==1) return get<1>(pos);
    }
    friend bool operator==(const Position &p1,const Position &p2);
    friend bool operator<(const Position &p1,const Position &p2);
};

bool operator==(const Position &p1,const Position &p2) {
    return p1.pos==p2.pos;
}

/**
 * order by row then by column
 */
bool operator<(const Position &p1,const Position &p2) {
    if (p1.getAt(0)==p2.getAt(0)) {
        return p1.getAt(1)<p2.getAt(1);
    } else {
        return p1.getAt(0)<p2.getAt(0);
    }
}

/**
 * a class to store the pieces on the board of the game
 * @tparam value contained in the class
 * ITEM 3.b.i ITEM 3.b.ii
 */
template<class T>
class Piece {
private:
    T value;
    Type type;
    Position pos;
    Owner owner;
public:
    Piece() {}
    Piece(Type t,Owner o,T val,Position p) {
        value=val;
        type=t;
        pos=p;
        owner=o;
    }
    Type getType() {
        return type;
    }
    T getVal() {
        return value;
    }
    Position getPos() {
        return pos;
    }
    void setPos(Position p) {
        this->pos=p;
    }
    Owner getOwner() {
        return owner;
    }
};

/** for moving a piece from a position to a position
 */
class Action {
private:
    Position from;
    Position to;
public:
    Action() {}
    Action(Position f,Position t) {
        from=f;
        to=t;
    }
    Position getFrom() {
        return from;
    }
    Position getTo() {
        return to;
    }
    Position setFrom(Position f) {
        this->from=f;
    }
    Position setTo(Position t) {
        this->to=t;
    }
};

/** the world that contains all the objects and pieces used in the world
 */
class World {
public:
    // ITEM 3.b shared pointers because there will be more than one,
    // one pointing in the map and one in the vector for each object
    vector<shared_ptr<Piece<char>>> mountains;
    // ITEM 1.1.a ITEM 1.1.b ITEM 3.a.1
    vector<shared_ptr<Piece<char>>> units0;
    vector<shared_ptr<Piece<char>>> units1;
    map<Position,shared_ptr<Piece<char>>> grid;

    World() {
        // ITEM 1.2 ITEM 3.a.2 this constructor contains the initial setup
        // the map of the mountains
        bool m[15][15] = {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,1,0,1,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0},
            {0,0,0,0,0,0,0,0,0,0,1,1,1,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,1,1,1,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,1,0,1,0,1,0,0,0,0,0,0,0,0},
            {0,0,1,0,1,0,1,0,0,0,0,0,0,0,0},
            {0,0,1,0,1,1,1,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
        };
        for (int i=0;i<15;i++) {
            for (int j=0;j<15;j++) {
                if (m[i][j]) {
                    Position p(i+1,j+1);
                    grid[p]=std::make_shared<Piece<char>>(Piece<char>(MOUNT,NA,'M',p));
                    mountains.push_back(grid[p]);
                }
            }
        }
        // add units of player zero
        for (int i=1;i<=15;i++) {
            for (int j=2;j<=15;j++) {
                Position p(i,j);
                if (i>6 || j>6) continue;
                if (i%3==0) {
                    grid[p]=std::make_shared<Piece<char>>(Piece<char>(SCISSORS,ZERO,'s',p));
                }
                if (i%3==1) {
                    grid[p]=std::make_shared<Piece<char>>(Piece<char>(ROCK,ZERO,'r',p));
                }
                if (i%3==2) {
                    grid[p]=std::make_shared<Piece<char>>(Piece<char>(PAPER,ZERO,'p',p));
                }
                units0.push_back(grid[p]);
            }
        }
        // add units of player one
        for (int i=1;i<=15;i++) {
            for (int j=2;j<=15;j++) {
                if (i>6 || j>6) continue;
                int ii=15-i+1;
                int jj=15-j+1;
                Position p(ii,jj);
                if (i%3==0) {
                    grid[p]=std::make_shared<Piece<char>>(Piece<char>(SCISSORS,ONE,'S',p));
                }
                if (i%3==1) {
                    grid[p]=std::make_shared<Piece<char>>(Piece<char>(ROCK,ONE,'R',p));
                }
                if (i%3==2) {
                    grid[p]=std::make_shared<Piece<char>>(Piece<char>(PAPER,ONE,'P',p));
                }
                units1.push_back(grid[p]);
            }
        }
        // add flags
        Position fPos(1,1);
        Position FPos(15,15);
        grid[fPos]=std::make_shared<Piece<char>>(Piece<char>(FLAG,ZERO,'f',fPos));
        grid[FPos]=std::make_shared<Piece<char>>(Piece<char>(FLAG,ONE,'F',FPos));
    }
    //show the grid
    void show() {
        for (int i=1;i<=15;i++) {
            for (int j=1;j<=15;j++) {
                Position p(i,j);
                if (grid[p]==NULL)
                    cout<<". ";
                else
                    cout<<grid[p]->getVal()<<' ';
            }
            cout<<endl;
        }
        cout<<endl;
    }
};

/**
 * the strategy of this player is random. It chooses the last piece of his pieces and
 * and checks the possible directions and picks one randomly.
 * @param world World& the given world
 * @return Action the chosen action
 * ITEM 3.c the random guy
 */
Action actionPlayerZero(World& world) {
    if (world.units0.size()==0) return Action(Position(1,1),Position(1,1));
    shared_ptr<Piece<char>> chosen = world.units0[world.units0.size()-1];
    vector<Position> directories;
    Position p=chosen->getPos();
    int dir[]={-1,1};
    for (int i=0;i<2;i++) {
        Position newPosition(p.getAt(0)+dir[i],p.getAt(1));
        if (world.grid[newPosition]==NULL)
            directories.push_back(newPosition);
        else if (world.grid[newPosition]->getOwner()!=ZERO &&
                 world.grid[newPosition]->getType()!=MOUNT)
            directories.push_back(newPosition);
    }
    for (int i=0;i<2;i++) {
        Position newPosition(p.getAt(0),p.getAt(1)+dir[i]);
        if (world.grid[newPosition]==NULL)
            directories.push_back(newPosition);
        else if (world.grid[newPosition]->getOwner()!=ZERO &&
                 world.grid[newPosition]->getType()!=MOUNT)
            directories.push_back(newPosition);
    }
    srand(time(0));
    int d=directories.size();
    Action action(p,directories[rand()%d]);
    return action;
}


/** @brief
 * the strategy of this player is to first make a defending wall around the flag.
 * then it picks the type that is most available in its units and moves it until
 * it dies or reaches the flag and wins.
 * @param world World&
 * @return Action
 * ITEM 3.c the strategy guy
 */
Action actionPlayerOne(World& world) {
    Action action;
    bool found=false;
    for (int i=0;i<world.units1.size();i++) {
        Position here=world.units1[i]->getPos();
        if (here.getAt(0)<13) continue;
        else {
            Position right(here.getAt(0),here.getAt(1)+1);
            if (world.grid[right]==NULL && here.getAt(1)+1<=15) {
                found=true;
                action.setFrom(here);
                Position there(here.getAt(0),here.getAt(1)+1);
                action.setTo(there);
                break;
            }
        }
    }
    if (found) {
        return action;
    }
    else {
        int r=0,p=0,s=0;
        for (int i=0;i<world.units1.size();i++) {
            Position here=world.units1[i]->getPos();
            if (here.getAt(0)>=13) continue;
            else {
                if (world.units1[i]->getType()==ROCK) r++;
                if (world.units1[i]->getType()==PAPER) p++;
                if (world.units1[i]->getType()==SCISSORS) s++;
            }
        }
        int a[3]={r,p,s};
        sort(a,a+3);
        reverse(a,a+3);
        Type desired;
        if (a[0]==r) desired=ROCK;
        else if (a[0]==p) desired=PAPER;
        else desired=SCISSORS;
        shared_ptr<Piece<char>> chosen;
        for (int i=1;i<=15;i++) {
            for (int j=1;j<=15;j++) {
                Position here(i,j);
                if (world.grid[here]==NULL) continue;
                if (world.grid[here]->getOwner()==ZERO) continue;
                if (world.grid[here]->getType()==desired) {
                    chosen=world.grid[here];
                    found=true;
                    break;
                }
            }
            if (found) break;
        }
        Position pos=chosen->getPos();
        int row=pos.getAt(0),col=pos.getAt(1);
        Position dir(row,col-1);
        if (col-1<1) {
            dir=Position(row-1,col);
        } else if (world.grid[dir]!=NULL && (world.grid[dir]->getOwner()==ONE || world.grid[dir]->getType()==MOUNT)) {
            dir=Position(row-1,col);
        }
        action=Action(pos,dir);
        return action;
    }
}

/**
 * The return is a pair: action and a boolean whether a timeout happened
 */
std::tuple<Action, bool> waitPlayer(Action (*f)(World&), World &world) {
    auto start = std::chrono::high_resolution_clock::now();
    Action action = f(world);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    if (elapsed.count() > TIMEOUT)
        return {action, true};
    else return {action, false};
}

/** @brief
 * check if a given position is within the grid
 * @param p Position to check
 * @return bool true if is valid
 *
 */
bool checkBounds(Position p) {
    if (p.getAt(0)>=1 && p.getAt(0)<=15 &&
        p.getAt(1)>=1 && p.getAt(1)<=15) return true;
    else return false;
}

/** @brief
 * check if the piece at a certain position is owned or not by the  given owner
 * @param p Position
 * @param o Owner
 * @param world World&
 * @param flip bool to flip the result. it's used to make the function useful in checking
 * if a piece is owned by owner, and to check if a piece is NOT owned by the owner
 * @return bool depends on flip
 *
 */
bool checkOwner(Position p, Owner o, World& world, bool flip) {
    if (world.grid[p]==NULL) return false^flip;
    if (world.grid[p]->getOwner()==o) return true^flip;
    else return false^flip;
}

/** @brief
 * check if at position is a mountain
 * @param p Position
 * @param world World&
 * @return bool
 *
 */
bool checkNotMount(Position p, World& world) {
    if (world.grid[p]==NULL) return true;
    if (world.grid[p]->getType()==MOUNT) return false;
    return true;
}

/** @brief
 * check if a position contains the flag of the owner
 * @param p Position
 * @param owner Owner
 * @param world World&
 * @return bool
 *
 */
bool checkNotYourFlag(Position p,Owner owner,World& world) {
    if (world.grid[p]==NULL) return true;
    if (world.grid[p]->getType()==FLAG && world.grid[p]->getOwner()==owner) return false;
    return true;
}

/** @brief
 * check if the action moves only to orthogonally adjacent postions
 * @param action Action
 * @return bool
 *
 */
bool checkDistance(Action action){
    Position from=action.getFrom();
    Position to=action.getTo();
    if (from==to) return false;
    if (abs(from.getAt(0)-to.getAt(0))>1) return false;
    if (abs(from.getAt(1)-to.getAt(1))>1) return false;
    if (abs(from.getAt(1)-to.getAt(1))==1 && abs(from.getAt(0)-to.getAt(0))==1) return false;
    return true;
}

/** @brief
 * check if an action is valid known that it's owned an owner
 * @param action Action
 * @param owner Owner
 * @param world World&
 * @return bool
 * ITEM 3.a.4 ITEM 1.4 all the movement rules in the update function and validateAction function
 */
bool validateAction(Action action, Owner owner, World& world) {
    bool valid=true;
    // don't go outside the maze ITEM 3.a.4.b ITEM 1.4.b
    valid&=checkBounds(action.getFrom());
    valid&=checkBounds(action.getTo());
    // ITEM 3.a.4.a ITEM 1.4.a move only your units and don't move to one of your units
    // ITEM 3.a.2.c ITEM 1.2.c
    valid&=checkOwner(action.getFrom(),owner,world,false);
    valid&=checkOwner(action.getTo(),owner,world,true);
    // ITEM 3.a.4.c ITEM 1.4.c moving to a mount is illegal
    valid&=checkNotMount(action.getTo(),world);
    valid&=checkNotYourFlag(action.getFrom(),owner,world);
    // ITEM 3.a.4.b ITEM 1.4.b to an orthogonally adjacent position
    valid&=checkDistance(action);
    return valid;
}

bool operator>(Type t1,Type t2) {
    int x1=t1,x2=t2;
    if (t1==SCISSORS && t2==ROCK) return false;
    return x1>x2;
}

/** @brief
 * update the world given the actions of the two players and return the winner
 * @param world World&
 * @param action0 Action
 * @param action1 Action
 * @param tie bool& becomes true if they tied
 * @return Owner the winner if someone won
 * ITEM 3.a.4 ITEM 1.4 all the movement rules in the update function and validateAction function
 */
Owner update(World& world, Action action0 ,Action action1,bool &tie) {
    // ITEM 3.a.3.d ITEM 1.3.d first check if one of the players or both of them reached the flags
    Position f(1,1);
    Position F(15,15);
    if (action0.getTo()==F && action1.getTo()==f){
        tie=true;
        return NA;
    }
    if (action0.getTo()==F) return ZERO;
    if (action1.getTo()==f) return ONE;
    // in case both of the players moved to the same position
    if (action0.getTo()==action1.getTo()) {
        // ITEM 3.a.4.e ITEM 1.4.e if they have the same type do nothing (bounce back in other words)
        if (world.grid[action0.getFrom()]->getType()==
                world.grid[action1.getFrom()]->getType()) {
            return NA;
        }
        // if the player0's unit is stronger kill the other one and move the
        // first to to the desired position
        // ITEM 3.a.4.g ITEM 1.4.g (killing)
        else if (world.grid[action0.getFrom()]->getType()>
                 world.grid[action1.getFrom()]->getType()) {
            world.grid[action0.getFrom()]->setPos(action0.getTo());
            world.grid[action0.getTo()]=world.grid[action0.getFrom()];
            world.grid[action0.getFrom()]=NULL;
            for (int i=0;i<world.units1.size();i++) {
                if (world.units1[i]==world.grid[action1.getFrom()]) {
                    world.units1.erase(world.units1.begin()+i);
                }
            }
            world.grid[action1.getFrom()]=NULL;
            // else make the player1's piece kill and move
        } else {
            world.grid[action1.getFrom()]->setPos(action1.getTo());
            world.grid[action1.getTo()]=world.grid[action1.getFrom()];
            world.grid[action1.getFrom()]=NULL;
            for (int i=0;i<world.units0.size();i++) {
                if (world.units0[i]==world.grid[action0.getFrom()]) {
                    world.units0.erase(world.units0.begin()+i);
                }
            }
            world.grid[action0.getFrom()]=NULL;
        }
    } else {
        // each one goes to a different position
        if (world.grid[action0.getTo()]==NULL) {
            world.grid[action0.getFrom()]->setPos(action0.getTo());
            world.grid[action0.getTo()]=world.grid[action0.getFrom()];
            world.grid[action0.getFrom()]=NULL;
        }
        else {
            if (world.grid[action0.getFrom()]->getType()==
                    world.grid[action0.getTo()]->getType()) {

            }
            else if (world.grid[action0.getFrom()]->getType()>
                     world.grid[action0.getTo()]->getType()) {
                world.grid[action0.getFrom()]->setPos(action0.getTo());
                world.grid[action0.getTo()]=world.grid[action0.getFrom()];
                world.grid[action0.getFrom()]=NULL;
                for (int i=0;i<world.units1.size();i++) {
                    if (world.units1[i]==world.grid[action0.getTo()]) {
                        world.units1.erase(world.units1.begin()+i);
                    }
                }
            } else {
                for (int i=0;i<world.units0.size();i++) {
                    if (world.units0[i]==world.grid[action0.getFrom()]) {
                        world.units0.erase(world.units0.begin()+i);
                    }
                }
                world.grid[action0.getFrom()]=NULL;
            }
        }

        if (world.grid[action1.getTo()]==NULL) {
            world.grid[action1.getFrom()]->setPos(action1.getTo());
            world.grid[action1.getTo()]=world.grid[action1.getFrom()];
            world.grid[action1.getFrom()]=NULL;
        }
        else {
            if (world.grid[action1.getFrom()]->getType()==
                    world.grid[action1.getTo()]->getType()) {

            }
            else if (world.grid[action1.getFrom()]->getType()>
                     world.grid[action1.getTo()]->getType()) {
                world.grid[action1.getFrom()]->setPos(action1.getTo());
                world.grid[action1.getTo()]=world.grid[action1.getFrom()];
                world.grid[action1.getFrom()]=NULL;
                for (int i=0;i<world.units0.size();i++) {
                    if (world.units0[i]==world.grid[action1.getTo()]) {
                        world.units0.erase(world.units0.begin()+i);
                    }
                }
            } else {
                for (int i=0;i<world.units1.size();i++) {
                    if (world.units1[i]==world.grid[action1.getFrom()]) {
                        world.units1.erase(world.units1.begin()+i);
                    }
                }
                world.grid[action1.getFrom()]=NULL;
            }
        }
    }
    // no one won
    return NA;
}

/** @brief
 * show which player has more advantage using a bar,
 * depending on the manhattan distance between the flag
 * and the closest opponent.
 * @param world World&
 * @return void
 * ITEM 3.d the advantage of each player using a bar
 */
void advantage(World& world) {
    int mn0=30,mn1=30;
    // mn0 is the manhattan distance between the flag0 and the closest piece of one
    // same for mn1
    for (int i=1;i<=15;i++) {
        for (int j=1;j<=15;j++) {
            Position cur(i,j);
            if (world.grid[cur]!=NULL) {
                if (world.grid[cur]->getOwner()==ONE) {
                    mn0=min(mn0,i-1+j-1);
                }
                if (world.grid[cur]->getOwner()==ZERO) {
                    mn1=min(mn1,15-i+15-j);
                }
            }
        }
    }
    // calculate the fraction on a scale from 1 to 20
    int sum=mn0+mn1;
    int adv0=(1.0*mn0/sum)*20;
    cout<<"The advantage bar:\n";
    cout<<"\tPlayer zero ";
    for (int i=0;i<20;i++) {
        if (i<adv0) cout<<">";
        else cout<<"<";
    }
    cout<<" Player one\n\n";
}

int main() {
    World world;
    world.show();
    bool endGame = false;
    while (!endGame) {
        // ITEM 1.3 ITEM 3.a.3
        auto[action0, timeout0] = waitPlayer( actionPlayerZero, world);
        auto[action1, timeout1] = waitPlayer( actionPlayerOne , world);
        if (timeout0 || timeout1) {
            endGame = true;
            if (timeout0 && !timeout1) {
                cout<<"\n\tTime is over for player zero\n";
                cout<<"\tPlayer one won\n";
            }
            if (!timeout0 && timeout1) {
                cout<<"\n\tTime is over for player one\n";
                cout<<"\tPlayer zero won\n";
            }
            if (timeout0 && timeout1) {
                cout<<"\n\tTIE\n";
                cout<<"\tBoth of the players used all their times\n";
            }
        } else {
            // ITEM 3.a.4.f ITEM 1.4.f A player immediately loses if attempted to make an illegal move
            bool invalid0=!validateAction(action0,ZERO,world);
            bool invalid1=!validateAction(action1,ONE,world);
            if (invalid0 || invalid1) {
                endGame=true;
                if (invalid0 && !invalid1) {
                    cout<<"\n\tPlayer zero played illegal move\n";
                    cout<<"\tPlayer one won\n";
                }
                if (!invalid0 && invalid1) {
                    cout<<"\n\tPlayer one played illegal move\n";
                    cout<<"\tPlayer zero won\n";
                }
                if (invalid0 && invalid1) {
                    cout<<"\n\tTIE\n";
                    cout<<"\tBoth of the players played illegal moves\n";
                }
            }
            bool tie=false;
            Owner winner = update(world,action0,action1,tie);
            if (tie) {
                cout<<"\n\tTie\tBoth players captured the flag at the same time\n";
                endGame=true;
            }
            else if (winner==ZERO) {
                cout<<"\n\tPlayer zero won by capturing the flag\n";
                endGame=true;
            }
            else if (winner==ONE) {
                cout<<"\n\tPlayer one won by capturing the flag\n";
                endGame=true;
            }
            else {
                world.show();
                advantage(world);
                // ITEM 3.a.3 ITEM 1.3 once per second
                this_thread::sleep_for(1000ms);
            }
        }
    }
    return 0;
}
