'PathFinding

Import mojo
Import playniax.ignitionx.engine
Import monkey.list
Import math
Import point
Import monkey.stack



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


    Method getLowestCostNode:AStarNode(n:Stack<AStarNode>)
        Local index:Int = 0
        'Local lowNode:AStarNode = Self.unvisited.Get(0)
        Local lowestCost:Float = n.Get(0).totalCost
        
        For Local i:Int = 1 Until n.Length
            If n.Get(i).totalCost < lowestCost
                index = i
                lowestCost = n.Get(i).totalCost
            End If
        End For
        
        Return n.Get(index)
    End Method
    
    
    Method getLowestCostNode:AStarNode()
        Local index:Int = 0
        'Local lowNode:AStarNode = Self.unvisited.Get(0)
        Local lowestCost:Float = Self.unvisited.Get(0).totalCost
        
        For Local i:Int = 1 Until Self.unvisited.Length
            If Self.unvisited.Get(i).totalCost < lowestCost
                index = i
                lowestCost = Self.unvisited.Get(i).totalCost
            End If
        End For
        
        Return Self.unvisited.Get(index)
    End Method
    
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
            
'               If currentNode.x = sx-1 And currentNode.y = sy
'                   Print "Starting left neighbor visited!: Iteration " + iterations
'               End If
'               If currentNode.x = sx+1 And currentNode.y = sy
'                               Print "Starting right neighbor visited!: Iteration " + iterations
'   
'               End If
'               If currentNode.x = sx And currentNode.y = sy-1
'                               Print "Starting up neighbor visited!: Iteration " + iterations
'   
'               End If
'               If currentNode.x = sx And currentNode.y = sy+1
'                               Print "Starting down neighbor visited!: Iteration " + iterations
'   
'               End If
            
             If endNode.id = currentNode.id
                Exit
             End If

             remIfStackHasElement(Self.unvisited, currentNode)

'                If stackHasElement(Self.unvisited, currentNode)
'   
'                   Print "Did not remove at start"
'                End If

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
                            'neighbor.heuristic = getCost(xpos, ypos, tx, ty)
                            neighbor.totalCost = nextStepCost + neighbor.heuristic
                            'neighbor.totalCost = nextStepCost
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
    
    Method isWalkableTileOverworld:Bool(m:Int[][], x:Int, y:Int)
        Local i:Int = m[x][y]
        
        If (i = 0) Or (i = 8) Or (i = 9) Or (i = 10) Or (i = 11) Or (i = 21) Or (i = 22)
            Return false
        Else
            Return true
        End If
    End Method
    
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
            
'               If currentNode.x = sx-1 And currentNode.y = sy
'                   Print "Starting left neighbor visited!: Iteration " + iterations
'               End If
'               If currentNode.x = sx+1 And currentNode.y = sy
'                               Print "Starting right neighbor visited!: Iteration " + iterations
'   
'               End If
'               If currentNode.x = sx And currentNode.y = sy-1
'                               Print "Starting up neighbor visited!: Iteration " + iterations
'   
'               End If
'               If currentNode.x = sx And currentNode.y = sy+1
'                               Print "Starting down neighbor visited!: Iteration " + iterations
'   
'               End If
            
             If endNode.id = currentNode.id
                Exit
             End If

             remIfStackHasElement(Self.unvisited, currentNode)

'                If stackHasElement(Self.unvisited, currentNode)
'   
'                   Print "Did not remove at start"
'                End If

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
                            'neighbor.heuristic = getCost(xpos, ypos, tx, ty)
                            neighbor.totalCost = nextStepCost + neighbor.heuristic
                            'neighbor.totalCost = nextStepCost
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
    
    Method getTileCostOverworld:Int(m:Int[][], x:Int, y:Int)
        Local i:Int = m[x][y]
        If (i > 22 And i < 30) Or (i > 11 And i < 18)
            Return 10
        Else
            Return 1
        End If
    End Method
    
    
    Method isWalkableTile(gameMap:Int[][], x:Int, y:Int)
        Return gameMap[x][y] = 5 Or gameMap[x][y] = 17
    End Method
End Class

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
    
    Method setParent:Int(mamaNode:AStarNode)
        Self.depth = mamaNode.depth + 1
        Self.parent = mamaNode
        
        Return Self.depth
    End Method
    
    Method setCost(x:Int, y:Int, tx:Int, ty:Int)
        'Self.cost = 1
        Self.heuristic = getCost(x, y, tx, ty) * 2
        Self.totalCost = Self.cost + Self.heuristic
    End Method
    
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

Class Path
    Field steps:List<PathStep>
    
    Method New()
        Self.steps = New List<PathStep>()
'           Local c:Float = getCost(1, 1, 9, 10)
'           Print "Cost: " + c
    End Method
    
    Method pathLength:Int()
        Return steps.Count()
    End Method
    
    Method appendStep(xCoord:Int, yCoord:Int)
        steps.AddLast(New PathStep(xCoord, yCoord))
    End Method
    
    Method prependStep(xCoord:Int, yCoord:Int)
        steps.AddFirst(New PathStep(xCoord, yCoord))
    End Method
    
    Method contains:Bool(xCoord:Int, yCoord:Int)
        Return Self.steps.Contains(New PathStep(xCoord, yCoord))
    End Method
    
    Method printPath()
        For Local i:= Eachin Self.steps
            'Print "Path Step: " + i.getX() + ", " + i.getY()
        End For
        'Print "Path Length: " + this.pathLength()
    End Method
End Class

Class PathStep
    Field x:Int
    Field y:Int
    
    Method New(theX:Int, theY:Int)
        Self.x = theX
        Self.y = theY
    End Method
    
    Method getX()
        Return Self.x
    End Method
    
    Method getY()
        Return Self.y
    End Method

End Class

Function getCost:Float(sX:Int, sY:Int, tX:Int, tY:Int)
    Local dx = tX - sX
    Local dy = tY - sY
    
    Return math.Sqrt(dx * dx + dy * dy) 
End Function


Function stackHasElement:Bool(s:Stack<AStarNode>, n:AStarNode)
    
        For Local i:Int = 0 Until s.Length
            If s.Get(i).id = n.id
                'Print "RETURNING TRUE"
                Return True
            End If
        End For
        
        Return false
End Function

Function removeFromStack:Stack<AStarNode>(s:Stack<AStarNode>, n:AStarNode)
    For Local i:Int = 0 Until s.Length
        'Print "s.get(i) = " + s.Get(i).id
        'Print "n.id = " + n.id
        If s.Get(i).id = n.id
            'Print "Removeing : " + n.id
            s.Remove(i)
            Exit
        End If
    End For
    Return s
End Function
