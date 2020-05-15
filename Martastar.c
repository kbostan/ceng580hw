#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

#define ALFA 10
#define INF  1000000  

typedef enum dir_type
{
    MOVEMENT_UP,
    MOVEMENT_DOWN,
    MOVEMENT_LEFT,
    MOVEMENT_RIGHT,
    MOVEMENT_NONE
} movementDirectionType; 

typedef struct agent_type
{
    movementDirectionType movementDirection;
    int* cellVisited;
    int* localCosts;
    float repulsionRange;
    int h_si;
    int id;
    int x;
    int y;
}AGENT;

typedef struct cell_type
{
    int hasObstacle;
    int isGoal;
    int globalCost;
}CELL;

typedef struct child_type
{
    int available;
    int x;
    int y;
    int bestOne;
    int secondBestOne;
    int cost;
    movementDirectionType direction;
}CHILD;

char DirectionAsChar(movementDirectionType direction)
{
    char dir;
    if(direction == MOVEMENT_DOWN)
    {
        dir = 'D';
    }
    else if(direction == MOVEMENT_UP)
    {
        dir = 'U';
    }
    else if(direction == MOVEMENT_RIGHT)
    {
        dir = 'R';
    }
    else if(direction == MOVEMENT_LEFT)
    {
        dir = 'L';
    }
    return dir;
}

movementDirectionType GetAvailableChildren(int x,int y, int mazeEdge, CELL* maze, int* foundGoal,CHILD* children)
{
    *foundGoal = FALSE;
    movementDirectionType goalDir;
    children[0].x = x;
    children[0].y = y + 1;
    children[1].x = x;
    children[1].y = y - 1;
    children[2].x = x - 1;
    children[2].y = y;
    children[3].x = x + 1;
    children[3].y = y;
    children[0].direction = MOVEMENT_UP;
    children[1].direction = MOVEMENT_DOWN;
    children[2].direction = MOVEMENT_LEFT;
    children[3].direction = MOVEMENT_RIGHT;

    for(int i=0; i<4;i++)
    {
        children[i].bestOne = FALSE;
    }

    if(children[0].x == mazeEdge && children[0].y == mazeEdge)
    {
        *foundGoal = TRUE;
        children[0].available = TRUE;
        goalDir = MOVEMENT_UP;
    }
    else if(children[3].x == mazeEdge && children[3].y == mazeEdge)
    {
        *foundGoal = TRUE;
        children[3].available = TRUE;
        goalDir = MOVEMENT_RIGHT;
    }
    else
    {
        //Check down movement
        if(y>1)
        {
            children[1].available = maze[mazeEdge*(y-2) + x - 1].hasObstacle ? FALSE : TRUE;
        }
        else
        {
            children[1].available = FALSE;
        }
        //Check up movement
        if(y<mazeEdge)
        {
            children[0].available = maze[mazeEdge*y + x - 1].hasObstacle ? FALSE : TRUE;
        }
        else
        {
            children[0].available = FALSE;
        }
        //Check right movement
        if(x<mazeEdge)
        {
            children[3].available = maze[mazeEdge*(y-1) + x].hasObstacle ? FALSE : TRUE;
        }
        else
        {
            children[3].available = FALSE;
        }
        //Check left movement   
        if(x > 1)
        {
            children[2].available = maze[mazeEdge*(y-1) + x -2].hasObstacle ? FALSE : TRUE;
        }
        else
        {
            children[2].available = FALSE;
        }
    }

    return goalDir;
}

int CalculateManhattanDistance(int x_0, int y_0, int x_1, int y_1)
{
    int dist;
    int dist_x;
    int dist_y;
    dist_x = x_0 >= x_1 ? (x_0 - x_1) : (x_1 - x_0); 
    dist_y = y_0 >= y_1 ? (y_0 - y_1) : (y_1 - y_0); 
    dist = dist_x + dist_y;
    return dist;
}

float CalculateRepulsionRange(int agentId, AGENT* agents, int h_s0)
{
    float repulsionRange;
    repulsionRange = ALFA * agents[agentId -1].h_si / h_s0;
    return repulsionRange;
}

//Use repulsion for tiebreak
int CheckRepulsionRange(int agentId, AGENT* agents, int agentCount, CHILD* children, int h_s0, int bestCount)
{
    int chosenChild;
    int maxDistances[] = {0, 0, 0, 0};
    int outsideRange[] = {FALSE,FALSE,FALSE,FALSE};
    int maxDistance = 0;
    int countOfOutsideRange = 0;
    //Get closest agents
    for(int i=0; i<agentCount; i++)
    {
        if((agentId -1) != i)
        {
            for(int j=0; j<4; j++)
            {
                if(children[j].bestOne && children[j].available)
                {
                    int dist = CalculateManhattanDistance(children[j].x,children[j].y,agents[i].x,agents[i].y);
                    if(dist <= agents[i].repulsionRange)
                    {
                        maxDistances[j] = maxDistances[j] > dist ? maxDistances[j] : dist;
                    }
                    else
                    {
                       maxDistances[j] = -1;
                    }
                    
                }
            }
        }
    }
    for(int j=0; j<4; j++)
    {
        if(children[j].bestOne && children[j].available)
        {
            //Outside repulsive range of all agents
            if(maxDistances[j] == -1)
            {
                outsideRange[j] = TRUE;
                countOfOutsideRange++;
            }
            else
            {
                maxDistance = maxDistance > maxDistances[j] ? maxDistance : maxDistances[j];
            }
        }
    }

    //Choose random
    if(countOfOutsideRange != 0)
    {
        int chosen = random() % countOfOutsideRange;
        int count = 0;
        for(int j=0; j<4; j++)
        {
            if(outsideRange[j] && (count==chosen) && children[j].available)
            {
                chosenChild = j;
                break;
            }
            else if (outsideRange[j] && children[j].available)
            {
                count = count + 1;
            }
        }   
    }
    else
    {
        int numOfAvailable = 0;
        for(int j=0; j<4; j++)
        {
            if (children[j].available)
            {
                numOfAvailable = numOfAvailable + 1;
            }
        }
        int chosen = random() % numOfAvailable;
        int count = 0;
        for(int j=0; j<4; j++)
        {
            if((count==chosen) && children[j].available)
            {
                chosenChild = j;
                break;
            }
            else if (children[j].available)
            {
                count = count + 1;
            }
        }
    }
    
    return chosenChild;
}

void GetMinCosts(CHILD* children,int* min_cost,int* second_min_cost, int* count_of_best, int* count_of_2nd_best)
{

    //Find mincost
    for(int i=0; i<4; i++)
    {   if(children[i].available)
        *min_cost = *min_cost <= children[i].cost ? *min_cost : children[i].cost;
    }

    for(int i=0; i<4; i++)
    {
        if((children[i].cost == *min_cost) && children[i].available)
        {
            *count_of_best = *count_of_best + 1;
            children[i].bestOne = TRUE;
        }
    }

    for(int i=0; i<4; i++)
    {
        if((children[i].cost > *min_cost) && (children[i].cost < *second_min_cost) && children[i].available)
        {
            *second_min_cost = children[i].cost;
        }
    }

    for(int i=0; i<4; i++)
    {
        if((children[i].cost == *second_min_cost) && children[i].available)
        {
            *count_of_2nd_best = *count_of_2nd_best + 1;
            children[i].secondBestOne = TRUE;
        }
    }
    
}
void SetChildCosts(int agentId, AGENT* agents,CHILD* children, int mazeEdge, CELL* maze)
{
    float repulsionRange = 0;
    int delta = 0;
    int max_delta = 0;

    int agent_x = agents[agentId -1].x;
    int agent_y = agents[agentId -1].y;


    //Look ahead search
    if(children[0].available == TRUE)
    {
        //Up
        if(agents[agentId -1].cellVisited[agent_y*mazeEdge + agent_x - 1])
        {
            children[0].cost = 1 + agents[agentId -1].localCosts[agent_y*mazeEdge + agent_x - 1];
        }
        else
        {
            //Not calculated yet
            if(maze[agent_y*mazeEdge + agent_x - 1].globalCost == -1)
            {
                children[0].cost = 1 + CalculateManhattanDistance(children[0].x,children[0].y,mazeEdge,mazeEdge);
            }
            else
            {
                children[0].cost = 1 + maze[agent_y*mazeEdge + agent_x - 1].globalCost;
            }
        }
    }
    else
    {
        children[0].cost = INF;
    }
    if(children[1].available == TRUE)
    {
        //Down
        if(agents[agentId -1].cellVisited[(agent_y-1)*mazeEdge + agent_x - 1])
        {
            children[1].cost= 1 + agents[agentId -1].localCosts[(agent_y-2)*mazeEdge + agent_x - 1];
        }
        else
        {
            //Not calculated yet
            if(maze[(agent_y-1)*mazeEdge + agent_x - 1].globalCost == -1)
            {
                children[1].cost = 1 + CalculateManhattanDistance(children[1].x,children[1].y,mazeEdge,mazeEdge);
            }
            else
            {
                children[1].cost = 1 + maze[(agent_y-1)*mazeEdge + agent_x - 1].globalCost;
            }
        }
    }
    else
    {
        children[1].cost = INF;
    }
    if(children[2].available == TRUE)
    {
        if(agents[agentId -1].cellVisited[(agent_y-1)*mazeEdge + agent_x - 2])
        {
            children[2].cost = 1 + agents[agentId -1].localCosts[(agent_y-1)*mazeEdge + agent_x - 2];
        }
        else
        {
            //Not calculated yet
            if(maze[(agent_y-1)*mazeEdge + agent_x - 2].globalCost == -1)
            {
                children[2].cost = 1 + CalculateManhattanDistance(children[2].x,children[2].y,mazeEdge,mazeEdge);
            }
            else
            {
                children[2].cost = 1 + maze[(agent_y-1)*mazeEdge + agent_x - 2].globalCost;
            }
        }
    }
    else
    {
        children[2].cost = INF;
    }
    if(children[3].available == TRUE)
    {
        //Right
        if(agents[agentId -1].cellVisited[(agent_y-1)*mazeEdge + agent_x])
        {
            children[3].cost = 1 + agents[agentId -1].localCosts[(agent_y-1)*mazeEdge + agent_x];
        }
        else
        {
            //Not calculated yet
            if(maze[(agent_y-1)*mazeEdge + agent_x].globalCost == -1)
            {
                children[3].cost = 1 + CalculateManhattanDistance(children[3].x,children[3].y,mazeEdge,mazeEdge);
            }
            else
            {
                children[3].cost = 1 + maze[(agent_y-1)*mazeEdge + agent_x].globalCost;
            }
        }
    }
    else
    {
        children[3].cost = INF;
    }


}

int main()
{
    /*ENVIRONMENT SETTING START*/
    int mazeEdge;
    int mazeSize;
    int numOfAgents;
    int numOfObstacles;
    int goalReached;
    int agentIdAtGoal;
    int h_s0;
    int index;
    int temp0;
    int temp1;
    int step;
    AGENT* agents;
    CELL* maze; 
    struct timeval t1,t2;
    double elapsedTime;
    FILE *fp;
    char* line = NULL;
    size_t len = 0;
    srandom(time(NULL));
    fp = fopen("input.txt","r");
    if(NULL == fp)
    {
        return 1;
    }
    else if(getline(&line, &len, fp) != -1)
    {
        mazeEdge = atoi(strtok(line, " ")); 
        mazeSize =  mazeEdge * mazeEdge;
        numOfAgents = atoi(strtok(NULL, " "));
        numOfObstacles = atoi(strtok(NULL, " "));
    }

    agents = (AGENT*)malloc(sizeof(AGENT) * numOfAgents);
    maze = (CELL*)malloc(sizeof(CELL) * mazeSize);
    
    h_s0 = CalculateManhattanDistance(1,1,mazeEdge,mazeEdge);
    
    //Set initial settings for all agents
    for(index = 0; index < numOfAgents; index++)
    {
        agents[index].id = index + 1;
        agents[index].x = 1;
        agents[index].y = 1;
        agents[index].movementDirection = MOVEMENT_LEFT;
        agents[index].cellVisited = (int*)malloc(sizeof(int) * mazeSize);
        agents[index].localCosts = (int*)malloc(sizeof(int) * mazeSize);
        agents[index].h_si = h_s0;
        agents[index].repulsionRange = CalculateRepulsionRange(index + 1,agents,h_s0);
        for(int i = 1; i < mazeSize; i++)
        {
            agents[index].cellVisited[i] = FALSE; 
        }
        agents[index].cellVisited[0] = TRUE;

        for(int i = 1; i < mazeSize; i++)
        {
            agents[index].localCosts[i] = -1; 
        }
    }

    for(index = 1; index < mazeSize; index++)
    {
        maze[index].hasObstacle = FALSE;
        maze[index].isGoal = FALSE;
        maze[index].globalCost = -1;
    }
    maze[0].globalCost = 1 + CalculateManhattanDistance(1,1,mazeEdge,mazeEdge);
    maze[mazeSize -1].isGoal = TRUE;

    for(index = 0; index < numOfObstacles; index++)
    {
        getline(&line, &len, fp);
        int temp_x = atoi(strtok(line, " "));
        int temp_y = atoi(strtok(NULL, " "));
        maze[(temp_y - 1) * mazeEdge + temp_x -1].hasObstacle = TRUE;
    }

    goalReached = FALSE;
    int goalReachedOnce = FALSE;
    movementDirectionType goalDirection;
    step = 1;
    gettimeofday(&t1,NULL);

    //Main Loop
    while(!goalReachedOnce)
    {
        printf("Step:%d\n",step);
        //Print state
        for(index = 0; index < numOfAgents; index++)
        {
            //Look for child
            CHILD children[4];
            int chosenChild = -1;
            int minCost = INF;
            goalDirection = GetAvailableChildren(agents[index].x,agents[index].y,mazeEdge,maze,&goalReached,children);
            //If goal is reached then go there and stop
            if(goalReached)
            {
                goalReachedOnce = TRUE;
                agentIdAtGoal = agents[index].id;
                if(goalDirection == MOVEMENT_UP)
                {
                    chosenChild = 0;
                }
                else if(goalDirection == MOVEMENT_RIGHT)
                {
                    chosenChild = 3;
                }
            }
            else
            {
                int secondMinCost = INF;
                int countOfBest = 0;
                int countOf2ndBest = 0;
                
                //Look ahead search and Choose best child
                SetChildCosts(agents[index].id,agents,children,mazeEdge,maze);
                GetMinCosts(children,&minCost,&secondMinCost,&countOfBest,&countOf2ndBest);
                if(countOfBest == 1)
                {
                    for(int i=0;i<4;i++)
                    {
                        if(children[i].bestOne == TRUE)
                        {
                            chosenChild = i;
                        }
                    }
                }
                else
                {
                    //Check repulsion
                    chosenChild = CheckRepulsionRange(agents[index].id,agents,numOfAgents,children,h_s0,countOfBest);
                }
                maze[(agents[index].y-1)*mazeEdge + agents[index].x  -1].globalCost = minCost;
                if(countOf2ndBest == 1)
                {
                    agents[index].localCosts[(agents[index].y -1) * mazeEdge + agents[index].x - 1] = secondMinCost;
                }
                else
                {
                    agents[index].localCosts[(agents[index].y -1) * mazeEdge + agents[index].x - 1] = INF;
                }
            }
            //Go to best child
            if(chosenChild != -1)
            {
                agents[index].x = children[chosenChild].x;
                agents[index].y = children[chosenChild].y;
                agents[index].movementDirection = children[chosenChild].direction;
                agents[index].cellVisited[(children[chosenChild].y - 1)*mazeEdge + children[chosenChild].x - 1] = TRUE;
                agents[index].h_si = CalculateManhattanDistance(agents[index].x,agents[index].y,mazeEdge,mazeEdge);
                agents[index].repulsionRange = CalculateRepulsionRange(agents[index].id,agents,h_s0);

                printf("AGENT%d: %c\t(%d,%d)\n",agents[index].id, DirectionAsChar(agents[index].movementDirection),agents[index].x,agents[index].y);
            }
            else
            {
                printf("Error: Can't chose a child");
                return 1;
            }
            
        }
        step++;
        printf("\n");
    }

    gettimeofday(&t2,NULL);
    elapsedTime = (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
    //Print the agent that reached the goal and elapsed time 
    printf("AGENT%d reached the goal in %f milliseconds.\n",agentIdAtGoal,elapsedTime);

    return 0;
}