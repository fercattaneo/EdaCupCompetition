#include "proximityMap.h"

using namespace std;

#define DIST(x,x0,y,y0)  (sqrt((x-x0)*(x-x0) + (y-y0)*(y-y0)))

void proximityMap::start()
{
    for(int x = 0 ; x < cols ; x++)
        for(int y = 0 ; y < rows ; y++)
        {
            pMap[x][y].botInfo.type = 0;
            pMap[x][y].botInfo.x = x;
            pMap[x][y].botInfo.y = y;
            
            for(int relX = -1 ; relX < 2 ; relX++)
                for(int relY = -1 ; relY < 2 ; relY++)
                {
                    if(!((relX == 0) && (relY == 0)))
                    {
                        try{
                            sNode * newNeighbour = &pMap.at(x+relX).at(y+relY);
                            pMap[x][y].neighbours.push_back(newNeighbour);
                        }
                        catch(const out_of_range& oor)
                        {
                        };
                    }
                }
        }      
}

void proximityMap::restart()
{
    for(int x = 0 ; x < cols ; x++)
        for(int y = 0 ; y < rows ; y++)
        {
            pMap[x][y].beenVisited = false;
            pMap[x][y].globalGoal = sqrt(cols*cols + rows*rows) + 1;
            pMap[x][y].localGoal = sqrt(cols*cols + rows*rows) + 1;
            pMap[x][y].parent = NULL;
            pMap[x][y].botInfo.value = 0;
        }
}

void proximityMap::update(vector<Players *> allyTeam)
{
    restart();

    int tilex;
    int tiley;

    for(auto i : allyTeam)
    {
        //cout << i->getPosition().x << endl << i->getPosition().y << endl;

        tilex = (int) ((acc) * (4.5F + i->getPosition().x));
        tiley = (int) ((acc) * (3.0F + i->getPosition().y));


        try
        {
/* to do cambiar*/
            pMap.at(tilex).at(tiley).botInfo.type = 1;
        }
        catch(const std::out_of_range& oor)
        {
            cout << "player near corner" << endl;
        }       
    }
}

void proximityMap::update(vector<Players *> allyTeam, Vector2 ball)
{
    restart();

    int tilex;
    int tiley;

    for(auto i : allyTeam)
    {
        tilex = (int) ((acc) * (4.5F + i->getPosition().x));
        tiley = (int) ((acc) * (3.0F + i->getPosition().y));

        try
        {
/* to do cambiar*/
            pMap.at(tilex).at(tiley).botInfo.type = 1;
        }
        catch(const std::out_of_range& oor)
        {
            cout << "player near corner" << endl;
        }       
    }
    tilex = (int) ((acc) *(4.5F + ball.x));
    tiley = (int) ((acc) *(3.0F + ball.y));
        
    try
    {
/*to do cambiar*/
        pMap.at(tilex).at(tiley).botInfo.type = 2;}
    catch(const std::out_of_range& oor)
    {
        cout << "ball near corner" << endl;
    } 
}

void proximityMap::update(vector<Players *> allyTeam, Vector2 ball,
                          vector<Players *> enemyTeam)
{
    restart();

    int tilex;
    int tiley;

    for(auto i : allyTeam)
    {
        tilex = (int) ((acc) * (4.5F + i->getPosition().x));
        tiley = (int) ((acc) * (3.0F + i->getPosition().y));

        try
        {
            pMap.at(tilex).at(tiley).botInfo.type = 1;
        }
        catch(const std::out_of_range& oor)
        {
            cout << "player near corner" << endl;
        }       
    }

    tilex = (int) (acc *(4.5F + ball.x));
    tiley = (int) (acc *(3.0F + ball.y));
        
    try
    {
        pMap.at(tilex).at(tiley).botInfo.type = 2;}
    catch(const std::out_of_range& oor)
    {
        cout << "ball near corner" << endl;
    }

    for(auto j : enemyTeam)
    {
        tilex = (int) ((acc) * (4.5F + j->getPosition().x));
        tiley = (int) ((acc) * (3.0F + j->getPosition().y));

        try
        {
            pMap.at(tilex).at(tiley).botInfo.type = 3;
        }
        catch(const std::out_of_range& oor)
        {
            cout << "enemy near corner" << endl;
        }       
    }
}

void proximityMap::printhm()
{
    for(int y = 0 ; y < rows ; y++)
        for(int x = 0 ; x < cols ; x++)
        {
            cout << "|" << (int) pMap[x][y].botInfo.type;
            if(x == (cols-1))
                cout << "|" << endl;
        }
}


void proximityMap::printParen()
{
    for(int y = 0 ; y < rows ; y++)
        for(int x = 0 ; x < cols ; x++)
        {
            if(pMap[x][y].botInfo.type == 4)
                cout << "|" <<  1;
            else
                cout << "|" <<  0;
            if(x == (cols-1))
                cout << "|" << endl;
        }
}

void proximityMap::solveAStar(Vector2 initPos, Vector2 endPos)
{
    restart();

    int initTileX = (int) (acc * (4.5F + initPos.x));
    int initTileY = (int) (acc * (3.0F + initPos.y));

    int endTileX = (int) (acc * (4.5F + endPos.x));
    int endTileY = (int) (acc * (3.0F + endPos.y));

    sNode *nodeStart = &pMap[initTileX][initTileY];
    sNode *nodeCurrent = nodeStart;
    sNode *nodeEnd = &pMap[endTileX][endTileY];
    nodeCurrent->localGoal = 0.0F;
    nodeCurrent->globalGoal = DIST(endTileX,initTileX,endTileY,initTileY);

    list<sNode *> notTestedNodes;
    notTestedNodes.push_back(nodeStart);

    while(!notTestedNodes.empty() && nodeCurrent != nodeEnd)
    {
        notTestedNodes.sort([](const sNode* lhs, const sNode* rhs){ return lhs->globalGoal < rhs->globalGoal; } );

        while(!notTestedNodes.empty() && notTestedNodes.front()->beenVisited)
				notTestedNodes.pop_front();
        
        if (notTestedNodes.empty())
				break;
        
        nodeCurrent = notTestedNodes.front();
		nodeCurrent->beenVisited = true;

        for (auto nodeNeighbour : nodeCurrent->neighbours)
        {
            if (!nodeNeighbour->beenVisited && nodeNeighbour->botInfo.type == 0)
                notTestedNodes.push_back(nodeNeighbour);

            float possiblyLowerGoal = nodeCurrent->localGoal + DIST(nodeCurrent->botInfo.x,nodeNeighbour->botInfo.x,nodeCurrent->botInfo.y,nodeNeighbour->botInfo.y);


            if (possiblyLowerGoal < nodeNeighbour->localGoal)
            {
                nodeNeighbour->parent = nodeCurrent;
                nodeNeighbour->localGoal = possiblyLowerGoal;

                nodeNeighbour->globalGoal = nodeNeighbour->localGoal
                    + DIST(nodeNeighbour->botInfo.x,nodeEnd->botInfo.x,nodeNeighbour->botInfo.y,nodeEnd->botInfo.y);
            }
        }	
	}
    /*test purposes*/
    
}

void proximityMap::shortest(Vector2 initPos, Vector2 endPos)
{
    int initTileX = (int) (acc * (4.5F + initPos.x));
    int initTileY = (int) (acc * (3.0F + initPos.y));

    int endTileX = (int) (acc * (4.5F + endPos.x));
    int endTileY = (int) (acc * (3.0F + endPos.y));

    pMap[endTileX][endTileY].botInfo.type = 4;
    sNode *path = pMap[endTileX][endTileY].parent;
    
    while(path != &pMap[initTileX][initTileY])
    {
        path->botInfo.type = 4;
        path = path->parent;
    }
}

/* GETTERS */
//podria poner un getter to dolist

