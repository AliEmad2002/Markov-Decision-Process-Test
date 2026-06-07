/*
 * main.cpp
 * Author: Ali Emad
 * Date: 7 Jun 2026
 * This program simulates an MDP evaluation using the Monte-Carlo method
 * Reference: https://www.youtube.com/watch?v=VnpRp7ZglfA
 */

#include <stdio.h>
#include <iostream>

#include <vector>
#include <map>
#include <set>
#include <ranges>

#include <conio.h>
#include <iomanip>

#include <chrono>
#include <thread>

/******************************************************************************
 * Configurations
 *****************************************************************************/
/*  Number of episodes to be ran in order for the model to learn    */
 #define N_EPISODES      200000

/*  Maximum number of samples (i.e.: movements) in a single episode */
#define MAX_SAMPLES_PER_EPISODE     200

#define GAMMA       0.8f


/******************************************************************************
 * Position class:
 * - Represents a 2D position by its x, y values
 * - Provides points comparison via operator overloading
 *****************************************************************************/
class CPosition{
private:
    /*  Position coordenates    */
    int m_x;
    int m_y;

public:
    /*  Ctor and default ctor    */
    CPosition(int x=0, int y=0): m_x(x), m_y(y) {   }

    /*  Copy ctor   */
    CPosition(const CPosition& other): m_x(other.m_x), m_y(other.m_y)   {   }

    /*  '<' operator overloading. lexographic comparison between "this" and "other"    */
    bool operator<(const CPosition& other) const
    {
        if (this->m_x != other.m_x)
            return this->m_x < other.m_x;
        return this->m_y < other.m_y;
    }

    /*  '>' operator overloading. lexographic comparison between "this" and "other"    */
    bool operator>(const CPosition& other) const
    {
        if (this->m_x != other.m_x)
            return this->m_x > other.m_x;
        return this->m_y > other.m_y;
    }

    /*  '==' operator overloading. Checks if points "this" and "other" are identical    */
    bool operator==(const CPosition& other) const
    {
        return (this->m_x == other.m_x) && (this->m_y == other.m_y);
    }

    /*  Checks whether "this" is with the rectangle {(0, 0), "other"} or not    */
    bool isWithin(const CPosition& other) const
    {
        return 
            (this->m_x >= 0)            &&
            (this->m_y >= 0)            &&
            (this->m_x <= other.m_x)    &&
            (this->m_y <= other.m_y);
    }

    /*  Sets 'x' value  */
    void setX(const int x)
    {
        m_x = x;
    }

    /*  Sets 'y' value  */
    void setY(const int y)
    {
        m_y = y;
    }

    /*  Gets 'x' value  */
    int getX() const
    {
        return m_x;
    }

    /*  Gets 'y' value  */
    int getY() const
    {
        return m_y;
    }

    /*  Prints the point   */
    void print() const
    {
        std::cout << '{' << m_x << ",\t" << m_y << '}';
    }
};

/******************************************************************************
 * Action enumurator:
 * - Numerically defines environment's possible actions
 *****************************************************************************/
enum EAction{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    EAction_MAX
};

/******************************************************************************
 * Environment class:
 * - Represents a 2D map by its border point and a set of restricted points
 * - Represents agent (i.e.: dot) movement on that map
 *****************************************************************************/
class CEnvironment{
private:
    /*  Dimentions / borders of the environment */
    CPosition m_border;

    /*  Initial position of the dot */
    CPosition m_InitialDotPosition;

    /*  Current position of the dot */
    CPosition m_dotPosition;

    /*  Target position for the dot */
    CPosition m_targetDotPosition;

    /*  Set of restricted positions  */
    std::set<CPosition> m_restrictedPositionSet;

    /*  Helper function: Checks whether a point is of environment's borders or not */
    bool isPositionOutOfBorder(const CPosition& p) const
    {
        return !p.isWithin(m_border);
    }

    /*  Helper function: Checks whether a point is restricted or not */
    bool isPositionRestricted(CPosition p) const
    {
        return m_restrictedPositionSet.find(p) != m_restrictedPositionSet.end();
    }

public:
    /*  Ctor*/
    CEnvironment(
        const CPosition& border,
        const CPosition& initialDotPosition,
        const CPosition& targetDotPosition,
        const std::set<CPosition>& restrictedPositionSet
    ):
        m_border(border),
        m_InitialDotPosition(initialDotPosition),
        m_dotPosition(initialDotPosition),
        m_targetDotPosition(targetDotPosition),
        m_restrictedPositionSet(restrictedPositionSet)
    {

    }

    /*  Resets the agent (i.e.: dot) to its initial position    */
    void reset()
    {
        m_dotPosition = m_InitialDotPosition;
    }

    /*  Moves the agent (i.e.: dot) based on a certain action. Movement is ignored if it is out of border   */
    void move(const EAction a)
    {
        int x = m_dotPosition.getX();
        int y = m_dotPosition.getY();

        switch (a)
        {
            case UP:    y--;  break;
            case DOWN:  y++;  break;
            case LEFT:  x--;  break;
            case RIGHT: x++;  break;
        }

        CPosition newPos(x, y);

        if (!isPositionOutOfBorder(newPos) && !isPositionRestricted(newPos))
        {
            m_dotPosition = newPos;
        }
    }

    /*  Returns current agent (i.e.: dot) position  */
    CPosition getDotPosition() const
    {
        return m_dotPosition;
    }

    /*  Returns environment's border point  */
    CPosition getBorder() const
    {
        return m_border;
    }

    /*  Checks whether the agent (i.e.: dot) have reached the target point or not   */
    bool isTargetReached() const
    {
        return m_dotPosition == m_targetDotPosition;
    }

    /*  Prints the environment  */
    void print() const
    {
        int xMax = m_border.getX();
        int yMax = m_border.getY();
        for (int y = 0; y <= yMax; y++)
        {
            for (int x = 0; x <= xMax; x++)
            {
                CPosition current(x, y);

                if (current == m_dotPosition)
                    std::cout << " D ";
                else if (current == m_targetDotPosition)
                    std::cout << " T ";
                else if (isPositionRestricted(current))
                    std::cout << " X ";
                else
                    std::cout << " . ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    /*  
     * Moves the agent (i.e.: dot) manually via {w, a, s, d} key presses and
     * prints map on terminal. exits if any other key is pressed
     */
    void manualControl()
    {
        while(1)
        {
            EAction a;
            char ch = _getch();
            switch (ch)
            {
                case 'w':   a = UP;     break;
                case 's':   a = DOWN;   break;
                case 'a':   a = LEFT;   break;
                case 'd':   a = RIGHT;  break;

                default:    return;
            }

            this->move(a);

            std::cout << "\033[2J\033[H"; // clear screen
            this->print();
        }        
    }
};

/******************************************************************************
 * Action value function class:
 * - Represents the action value function "Q(S, a)" via a map indexed by 'S' and each index has an array of 'a's
 * - Evaluates the statistically optimal action given certain state (i.e.: Exploitation), with a configurable probability of randomness (Exploration)
 *****************************************************************************/
class CActionValueFunction{
private:
    /*  A map representing the action value function "Q(S, a)". indexed by 'S' and each index has an array of 'a's  */
    std::map<CPosition, std::array<float, EAction_MAX>> m_q;

public:
    /*  Gets reference to "Q(S, a)"   */
    float& at(const CPosition& s, EAction a)
    {
        return m_q[s][a];
    }

    /*  Evaluates the statistically optimal action given certain state (i.e.: Exploitation), with a configurable probability of randomness (Exploration)    */
    EAction bestAt(const CPosition& dotPosition, float epsilon = 0.2f) // todo: make this function const on "this"
    {
        // Explore: pick a random action
        if ((rand() / (float)RAND_MAX) < epsilon)
        {
            return (EAction)(rand() % EAction_MAX);
        }

        // Exploit: pick the greedy best action
        EAction best = UP;
        float bestVal = this->at(dotPosition, UP);

        for (int a = 1; a < EAction_MAX; a++)
        {
            if (this->at(dotPosition, (EAction)a) > bestVal)
            {
                bestVal = this->at(dotPosition, (EAction)a);
                best    = (EAction)a;
            }
        }

        return best;
    }

    /*  Prints all values of the action value function in a table format */
    void print(const CPosition& border) // todo: make this function const on "this"
    {
        // Calculate column width based on EAction_MAX floats
        const int precision  = 2;
        const int floatWidth = 5; // "-0.00"
        const int colWidth   = EAction_MAX * (floatWidth + 2) + 4; // +2 for ", " +4 for "{}  "

        int xMax = border.getX();
        int yMax = border.getY();

        for (int y = 0; y <= yMax; y++)
        {
            for (int x = 0; x <= xMax; x++)
            {
                CPosition pos(x, y);
                std::ostringstream cell;
                cell << "{";
                for (int a = 0; a < EAction_MAX; a++)
                {
                    cell << std::fixed << std::setprecision(precision)
                        << std::setw(floatWidth) << m_q[pos][a];
                    if (a < EAction_MAX - 1) cell << ", ";
                }
                cell << "}";

                std::cout << std::left << std::setw(colWidth) << cell.str();
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
};

/******************************************************************************
 * Episode sample structure.
 * A sample has each of the following:
 *  - state     (S),
 *  - action    (a),
 *  - reward    (r),
 *  - return    (G)
 *****************************************************************************/
struct SEpisodeSample{
    CPosition m_s;
    EAction m_a;
    float m_r;
    float m_g;
};

/******************************************************************************
 * Episode definition
 * - It is simply a vector of episode samples
 *****************************************************************************/
typedef std::vector<SEpisodeSample> Episode;

int main (int argc, char* argv[])
{
    /*  Create an environment   */
    CEnvironment env(
        {8, 5},                                 /*  Border: 9 wide, 6 tall      */
        {0, 0},                                 /*  Initial position: top-left  */
        {8, 5},                                 /*  Target: bottom-right        */
        {                                       /*  Walls: a winding corridor   */
            {2, 0}, {2, 1}, {2, 2}, {2, 3},    /* vertical wall, gap at bottom */
            {4, 2}, {4, 3}, {4, 4}, {4, 5},    /* vertical wall, gap at top    */
            {6, 0}, {6, 1}, {6, 2}, {6, 3},    /* vertical wall, gap at bottom */
        }
    );
    
    /*  Give user chance to have a look at the environment and move in it   */
    env.manualControl();

    /*  Create an action value function (initially all its values are zeros)    */
    CActionValueFunction q;

    /*  Total number of successfull episodes    */
    int nSuccess = 0;

    /*  Run the configured amount of episodes   */
    for (int i = 0; i < N_EPISODES; i++)
    {
        /*  Create an empty episode */
        Episode episode;

        /*  Reset the environment (todo: instead of resetting, let's put the agent at a random start position, which is better for RL BTW?)  */
        env.reset();
        
        /*  
         * Exploration Vs Eploitation tradeoff: epsilon starts large
         * (High exploration - Low exploitation) and trends downwards
         * (Low exploration - High exploitation) as the model learns
         */
        float epsilon = std::max(0.05f, 0.8f - (static_cast<float>(i) / static_cast<float>(N_EPISODES/2)) * 0.75f);

        /*  For every sample in the episode */
        for (int j = 0; j < MAX_SAMPLES_PER_EPISODE; j++)
        {
            /*  Get agent's position (State)    */
            CPosition dotPosition = env.getDotPosition();

            /*  Evaluate best action given that state   */
            EAction a = q.bestAt(dotPosition, epsilon);

            /*  Apply that action on the environment   */
            env.move(a);

            /*  Store (i.e.: push) this sample in the episode   */
            SEpisodeSample sample = {
                .m_s = dotPosition,
                .m_a = a,
                .m_r = env.isTargetReached() ? 0.0f : -1.0f,
                .m_g = 0
            };

            episode.push_back(sample);

            /*  Give user chance to stop the simulation */
            if (_kbhit())
            {
                return 0;
            }

            /*  If target is reached, there's no need for next samples / movements  */
            if (env.isTargetReached())
            {
                nSuccess++;
                break; // end episode
            }
        }

        /*  Evaluate returns (i.e.: G's) of this episode */
        float gPrev = 0.0f;
        for (auto& sample : std::ranges::reverse_view(episode))
        {
            sample.m_g = gPrev * GAMMA + sample.m_r;
            gPrev = sample.m_g;

            q.at(sample.m_s, sample.m_a) = q.at(sample.m_s, sample.m_a) + (sample.m_g - q.at(sample.m_s, sample.m_a)) * 0.1f;
        }

        if (i%100 == 0)
            std::cout << "Episode#: " << i << "\tTotal Successes: " << nSuccess << "\tCurrent Episode Length: " << episode.size() << std::endl;
    }

    return 0;
}
