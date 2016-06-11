Import mojo
Import playniax.ignitionx.engine
Import monkey.list
Import math
Import point
Import monkey.stack

'
'A* search implementation for MonkeyX for use in intelligent procedural
'content generation.
'
Class AStarSearch
    Field visited:Stack<AStarNode>
    Field unvisited:Stack<AStarNode>
    
    Field gameMap:Int[][]
    Field maxDistance:Int
    
    Field nodes:Stack<Stack<AStarNode>>
    
    Field allowDiagonal:Bool
    
    Method New(gMap:Int[][], maxDist:Int, allowDiag:Bool)
        
        Self.gameMap = gMap
        Self.maxDistance = maxDist
        Self.allowDiagonal = allowDiag
        
        Self.visited = New Stack<AStarNode>
        Self.unvisited = New Stack<AStarNode>
        
        Self.nodes = New Stack<Stack<AStarNode>>
        Local idCount:Int = 0
        For Local i:Int = 0 Until gameMap.Length
            Self.nodes.Push(New Stack<AStarNode>())
            For Local j:Int = 0 Until gameMap[i].Length
                Self.nodes.Get(i).Push(New AStarNode(i, j, idCount))
                idCount += 1
            End For
        End For
    End Method

    '
    'Helper method to remove an element from a stack if it exists in the stack.
    '
    Method remIfStackHasElement:Bool(s:Stack<AStarNode>, n:AStarNode)
        'Print "Trying to rem..."
        For Local i:Int = 0 Until s.Length
        
            If s.Get(i).id = n.id
                'Print "Will return true, removing"
                s.Remove(i)
                'Print "Removed, Returning True"
                Return true
            End If
        End For
        
        Return false
    End Method


    '
    'Helper method that returns the node from a stack that has the lowest cost of movement,
    'given a stack to draw from.
    '
    Method getLowestCostNode:AStarNode(n:Stack<AStarNode>)
        Local index:Int = 0
        Local lowestCost:Float = n.Get(0).totalCost
        
        For Local i:Int = 1 Until n.Length
            If n.Get(i).totalCost < lowestCost
                index = i
                lowestCost = n.Get(i).totalCost
            End If
        End For
        
        Return n.Get(index)
    End Method
    
    '
    'Helper method that returns the node from a stack that has the lowest cost of movement,
    'with no parameters passed to it.
    '
    Method getLowestCostNode:AStarNode()
        Local index:Int = 0
        Local lowestCost:Float = Self.unvisited.Get(0).totalCost
        
        For Local i:Int = 1 Until Self.unvisited.Length
            If Self.unvisited.Get(i).totalCost < lowestCost
                index = i
                lowestCost = Self.unvisited.Get(i).totalCost
            End If
        End For
        
        Return Self.unvisited.Get(index)
    End Method
    
    '
    'Method to find a path between two points, if one exists. Used for cave generation.
    '
    Method findPath:Path(sx:Int, sy:Int, tx:Int, ty:Int)
        
        If Not isWalkableTile(Self.gameMap, tx, ty)
            Print "Treasure not walkable"
            Return Null
        End If
        
        For Local i:Int = 0 Until Self.nodes.Length
            For Local j:Int = 0 Until Self.nodes.Get(i).Length
                Self.nodes.Get(i).Get(j).setCost(i, j, tx, ty)
            End For
        End For
        
        Local startNode:AStarNode = Self.nodes.Get(sx).Get(sy)
        Local endNode:AStarNode = Self.nodes.Get(tx).Get(ty)
        
        startNode.cost = 0
        startNode.depth = 0
        
        Self.visited.Clear()
        Self.unvisited.Clear()
        
        Self.unvisited.Push(startNode)
        
        endNode.parent = Null
        
        Local depth:Int = 0
        Local iterations:Int = 0
        
        While depth < 300 And unvisited.Length > 0
            iterations += 1
            Local currentNode:AStarNode = getLowestCostNode()
            
             If endNode.id = currentNode.id
                Exit
             End If

             remIfStackHasElement(Self.unvisited, currentNode)

             Self.visited.Push(currentNode)
             
             For Local i:Int = -1 Until 2
                For Local j:Int = -1 Until 2
                    If i = 0 And j = 0
                        Continue
                    End If
                    
                    If Not Self.allowDiagonal
                        If Not i = 0 And Not j = 0
                            Continue
                        End If
                    End If

                    Local xpos = i + currentNode.x
                    Local ypos = j + currentNode.y
                    
                    If isWalkableTile(Self.gameMap, xpos, ypos)
       
                        Local neighbor:AStarNode = Self.nodes.Get(xpos).Get(ypos)
                        
                        
                        Local nextStepCost:Float = currentNode.cost + 1
                        
                        If nextStepCost < neighbor.cost
                            remIfStackHasElement(Self.unvisited, neighbor)
                            remIfStackHasElement(Self.visited, neighbor)
                        End If
                        
                        If (Not stackHasElement(Self.unvisited, neighbor)) And (Not stackHasElement(Self.visited, neighbor))
                            neighbor.cost = nextStepCost
                            neighbor.totalCost = nextStepCost + neighbor.heuristic
                            depth = math.Max(depth, neighbor.setParent(currentNode))
                            Self.unvisited.Push(neighbor)
                        End If
                    End If
                    
                End For
             End For
                
        End While
        
        If endNode.parent = Null
            Print "End node id: " + endNode.id
            Print "End node null"
            Return Null
        End If
        
        Print "Iterations to treasure: " + iterations
        
        Local foundPath:Path = New Path()
        Local target:AStarNode = Self.nodes.Get(tx).Get(ty)
        
        While Not ((target.x = startNode.x) And (target.y = startNode.y))
            foundPath.prependStep(target.x, target.y)
            target = target.parent
        End While
        foundPath.prependStep(sx, sy)
        
        Print "Path Length: " + foundPath.pathLength()
        
        
        Return foundPath
        
    End Method
    
    '
    'Method to determine if a given tile on the overworld map is one which the player
    'is alble to walk on. Lots of hard coded values that should probably be changed to
    'public, static Ints
    '
    Method isWalkableTileOverworld:Bool(m:Int[][], x:Int, y:Int)
        Local i:Int = m[x][y]
        
        If (i = 0) Or (i = 8) Or (i = 9) Or (i = 10) Or (i = 11) Or (i = 21) Or (i = 22)
            Return false
        Else
            Return true
        End If
    End Method
    
    '
    'Method to find the path between two points on the overworld map, if one exists.
    '
    Method findPathOverworld:Path(overworld:Int[][], sx:Int, sy:Int, tx:Int, ty:Int)
        
        If Not isWalkableTileOverworld(Self.gameMap, tx, ty)
            Print "Cave tile not walkable"
            Return Null
        End If
        
        For Local i:Int = 0 Until Self.nodes.Length
            For Local j:Int = 0 Until Self.nodes.Get(i).Length
                Self.nodes.Get(i).Get(j).setCost(i, j, tx, ty)
            End For
        End For
        
        Local startNode:AStarNode = Self.nodes.Get(sx).Get(sy)
        Local endNode:AStarNode = Self.nodes.Get(tx).Get(ty)
        
        startNode.cost = 0
        startNode.depth = 0
        
        Self.visited.Clear()
        Self.unvisited.Clear()
        
        Self.unvisited.Push(startNode)
        
        endNode.parent = Null
        
        Local depth:Int = 0
        Local iterations:Int = 0
        
        While depth < 300 And unvisited.Length > 0
            iterations += 1
            Local currentNode:AStarNode = getLowestCostNode()  
            
             If endNode.id = currentNode.id
                Exit
             End If

             remIfStackHasElement(Self.unvisited, currentNode)

             Self.visited.Push(currentNode)
             
             For Local i:Int = -1 Until 2
                For Local j:Int = -1 Until 2
                    If i = 0 And j = 0
                        Continue
                    End If
                    
                    If Not Self.allowDiagonal
                        If Not i = 0 And Not j = 0
                            Continue
                        End If
                    End If

                    Local xpos = i + currentNode.x
                    Local ypos = j + currentNode.y
                    
                    If isWalkableTileOverworld(Self.gameMap, xpos, ypos)
       
                        Local neighbor:AStarNode = Self.nodes.Get(xpos).Get(ypos)
                        
                        
                        Local nextStepCost:Float = currentNode.cost + getTileCostOverworld(Self.gameMap, xpos, ypos)
                        
                        If nextStepCost < neighbor.cost
                            remIfStackHasElement(Self.unvisited, neighbor)
                            remIfStackHasElement(Self.visited, neighbor)
                        End If
                        
                        If (Not stackHasElement(Self.unvisited, neighbor)) And (Not stackHasElement(Self.visited, neighbor))
                            neighbor.cost = nextStepCost
                            neighbor.totalCost = nextStepCost + neighbor.heuristic
                            depth = math.Max(depth, neighbor.setParent(currentNode))
                            Self.unvisited.Push(neighbor)
                        End If
                    End If
                    
                End For
             End For
                
        End While
        
        If endNode.parent = Null
            Print "End node id: " + endNode.id
            Print "End node null"
            Return Null
        End If
        
        Print "Iterations to cave: " + iterations
        
        Local foundPath:Path = New Path()
        Local target:AStarNode = Self.nodes.Get(tx).Get(ty)
        
        While Not ((target.x = startNode.x) And (target.y = startNode.y))
            foundPath.prependStep(target.x, target.y)
            target = target.parent
        End While
        foundPath.prependStep(sx, sy)
        
        Print "Path Length: " + foundPath.pathLength()
        
        
        Return foundPath
        
    End Method
    
    '
    'Method to calculate the cost of movement over a walkable tile on the overworld map.
    '
    Method getTileCostOverworld:Int(m:Int[][], x:Int, y:Int)
        Local i:Int = m[x][y]
        If (i > 22 And i < 30) Or (i > 11 And i < 18)
            Return 10
        Else
            Return 1
        End If
    End Method
    
    '
    'Method to determine if a tile in a cave is one which the player character can
    'walk on.
    '
    Method isWalkableTile(gameMap:Int[][], x:Int, y:Int)
        Return gameMap[x][y] = 5 Or gameMap[x][y] = 17
    End Method
End Class

'
'Custom nodes used for the A* search algorithm.
'
Class AStarNode
    Field x:Int
    Field y:Int
    Field id:Int
    
    Field parent:AStarNode
    Field heuristic:Float
    
    Field totalCost:Float
    
    Field cost:Float
    Field depth:Int
    
    Method New(xCoord:Int, yCoord:Int, nodeID:Int)
        Self.x = xCoord
        Self.y = yCoord
        Self.id = nodeID
        Self.cost = 200000
        Self.totalCost = cost
    End Method
    
    '
    'Method to assign a node being passed in as the current node's parent.
    '
    Method setParent:Int(mamaNode:AStarNode)
        Self.depth = mamaNode.depth + 1
        Self.parent = mamaNode
        
        Return Self.depth
    End Method
    
    '
    'Method to set the heuristic cost of traveling through the current node.
    '
    Method setCost(x:Int, y:Int, tx:Int, ty:Int)
        'Self.cost = 1
        Self.heuristic = getCost(x, y, tx, ty) * 2
        Self.totalCost = Self.cost + Self.heuristic
    End Method
    
    '
    'Method to compare current node's cost of movement to another node's cost
    'of movement.
    '
    Method compareCosts(other:AStarNode)
        Local thisCost:Float = Self.heuristic
        Local thatCost:Float = other.heuristic
        
        If thisCost < thatCost
            Return -1
        Else If thisCost > thatCost
            Return 1
        Else 
            Return 0
        End If
    End Method
End Class

'
'Class to keep track of a path between one point and another in the A* search algorithm
'
Class Path
    Field steps:List<PathStep>
    
    Method New()
        Self.steps = New List<PathStep>()
'           Local c:Float = getCost(1, 1, 9, 10)
'           Print "Cost: " + c
    End Method
    
    '
    'Method to return the length of the current Path.
    '
    Method pathLength:Int()
        Return steps.Count()
    End Method
    
    '
    'Method to add a step to the end of the current Path.
    '
    Method appendStep(xCoord:Int, yCoord:Int)
        steps.AddLast(New PathStep(xCoord, yCoord))
    End Method
    
    '
    'Method to add a step to the start of the current Path.
    '
    Method prependStep(xCoord:Int, yCoord:Int)
        steps.AddFirst(New PathStep(xCoord, yCoord))
    End Method
    
    '
    'Method to check if a coordinate already exists in the current Path.
    '
    Method contains:Bool(xCoord:Int, yCoord:Int)
        Return Self.steps.Contains(New PathStep(xCoord, yCoord))
    End Method
    
    '
    'Method to print the current path to the console. Used for debugging.
    '
    Method printPath()
        For Local i:= Eachin Self.steps
            'Print "Path Step: " + i.getX() + ", " + i.getY()
        End For
        'Print "Path Length: " + this.pathLength()
    End Method
End Class

'
'Class that represents one step within a path for A* search.
'
Class PathStep
    Field x:Int
    Field y:Int
    
    Method New(theX:Int, theY:Int)
        Self.x = theX
        Self.y = theY
    End Method
    
    '
    'Getter method for step's X coordinate
    '
    Method getX()
        Return Self.x
    End Method
    
    '
    'Getter method for step's Y coordinate.
    '
    Method getY()
        Return Self.y
    End Method

End Class

'
'Function to return the Euclidian distance between two points.
'
Function getCost:Float(sX:Int, sY:Int, tX:Int, tY:Int)
    Local dx = tX - sX
    Local dy = tY - sY
    
    Return math.Sqrt(dx * dx + dy * dy) 
End Function

'
'Function to determine if a stack contains a certain element
'
Function stackHasElement:Bool(s:Stack<AStarNode>, n:AStarNode)
    
        For Local i:Int = 0 Until s.Length
            If s.Get(i).id = n.id
                'Print "RETURNING TRUE"
                Return True
            End If
        End For
        
        Return false
End Function

'
'Function to remove an element from a stack
'
Function removeFromStack:Stack<AStarNode>(s:Stack<AStarNode>, n:AStarNode)
    For Local i:Int = 0 Until s.Length
        If s.Get(i).id = n.id
            s.Remove(i)
            Exit
        End If
    End For
    Return s
End Function
