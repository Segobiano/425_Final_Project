Pressing Space bar selects a random location for each orge to use Astar to travel too.

The MoveTo function takes a goal node only as the input.  It then calls the Astar Function is grid that takes a start and a goal position.  This is a design choice so that astar was not limited to only agents.  With it in grid its more general purpose for something like a tower defense that will been to check to see if the player placing a tower is blocking the path (or the aStar is returning NULL)

if in the agent MoveTo the agents current location is given then its doesnt call grids Astar because its already at that location.  This is to save from doing the function call.  


The orge if blocked from the goal will not move.  If the goal is the ogres current postion he will not move.  Or if the random node from the space bar press is occupied the ogre will not move.  