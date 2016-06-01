Import mojo
Import playniax.ignitionx.engine
Import point
Import pathfinder
Import monkey.stack
Import math


Global caveTextures:Image
'Procedurally generated representation of a cave/dungeon.
'
'
Class Level

    Const CAVE_CENTER:Int = 0
    Const CAVE_UP_LEFT:Int = 1
    Const CAVE_UP_RIGHT:Int = 2
    Const CAVE_DOWN_LEFT:Int = 3
    Const CAVE_DOWN_RIGHT:Int = 4
    Const CAVE_WALKWAY:Int = 5
    Const LAVA_NORTH_SOUTH:Int = 6
    Const LAVA_NORTH_WEST:Int = 7
    Const LAVA_NORTH_EAST:Int = 8
    Const LAVA_SOUTH_WEST:Int = 9
    Const LAVA_SOUTH_EAST:Int = 10
    Const LAVA_EAST_WEST:Int = 11
    Const LAVA:Int = 12
    Const TRAPDOOR_CLOSED:Int = 13
    Const TRAPDOOR_OPEN:Int = 14
    Const SPIKES:Int = 15
    Const ENTRANCE:Int = 16
    Const TREASURE:Int = 17
    Const FOUND_PATH:Int = 18
    
    'Temporary value. texture not drawn yet
    Const CAVE_BORDER:Int = CAVE_CENTER
    
	Field layout:Int[][]
	Field width:Int
	Field height:Int
	Field generated:Bool
	Field walkways:Point[]
	Field lava:Point[]
	
	Field counter:Int
	
	'Location on World Map
	Field xCoord:Int
	Field yCoord:Int
	
	Field treasureX:Int
	Field treasureY:Int
	
	Field treasure2X:Int
	Field treasure2Y:Int
	
	Field treasures:Stack<Point>
	Field entranceX:Int
	Field entranceY:Int
	
	
	'Constructor for Level
	'
	'Takes in x/y coordinate pair for overworld location, width/height of cave, and a 
	'string literal stating generation method.
	Method New(x:Int, y:Int, w:Int, h:Int, type:String)
		Self.generated = False
		Self.xCoord = x
		Self.yCoord = y
		
		Self.width = w
		Self.height = h
		Self.layout = setArray(Self.width, Self.height)
		
		caveTextures = iLoadSprite("cave_texture40.png", 40, 40, 19)
		
		If type = "Cellular"
			randomlyAssignCells(Self.layout)
			layout = generateCellularly(Self.layout)
			layout = generateCellularly(Self.layout)
			layout = generateCellularly(Self.layout)
			layout = generateCellularly(Self.layout)
			layout = generateCellularly(Self.layout)
			smoothEdges()
		Else If type = "Drunk"
			fillCells(Self.layout)
			drunkWalk(Self.layout)
		End If
		
		countWalkways()
		lava = New Point[1000]
		makeLavaRivers()
		
		addSpikesAndTraps()
		
		makeStartingPoint()
		
		Self.treasureX = 0
		Self.treasureY = 0
		
		treasures = New Stack<Point>()
		setTreasure(10)
		
		'Local isReachable:Bool = False
		'While Not isReachable
            'setTreasure()
         '   isReachable = validatePath()
        'End While
		'Print "Treasure X: " + treasureX
        'Print "Treasure Y: " + treasureY
		
		Self.generated = true
	End Method
	
	Method setTreasure:Bool(total:Int)
        Local successCount:Int = 0;
        Local failCount:Int = 0;
        Local searcher:AStarSearch = New AStarSearch(Self.layout, 300, False)
        Local currentPath:Path
        
        While successCount < 10 And failCount < 10
            Print "Fails: " + failCount + ", Success: " + successCount
            setTreasure()
            Local closestIndex:Int = -1
            Print "Last treas: " + treasures.Get(treasures.Length-1).getX() + ", " + treasures.Get(treasures.Length-1).getY()
            Local closestDist:Float = getDist(entranceX, entranceY, treasures.Get(treasures.Length-1).getX(), treasures.Get(treasures.Length-1).getY())
            For Local t:Int = 0 Until treasures.Length - 1
                Local localDist:Float = getDist(treasures.Get(treasures.Length-1).getX(), treasures.Get(treasures.Length-1).getY(), treasures.Get(t).getX(), treasures.Get(t).getY())
                If localDist < closestDist
                    closestDist = localDist
                    closestIndex = t
                End If
            End For
            If closestIndex = -1
                currentPath = searcher.findPath(entranceX, entranceY, treasures.Get(treasures.Length-1).getX(), treasures.Get(treasures.Length-1).getY())
            Else
                currentPath = searcher.findPath(treasures.Get(closestIndex).getX(), treasures.Get(closestIndex).getY(), treasures.Get(treasures.Length-1).getX(), treasures.Get(treasures.Length-1).getY())
            End If
            
            If currentPath = Null
                Print "FAIL"
                failCount += 1
                treasures.Pop()
            Else
                successCount += 1
            End If
        End While
        
	End Method
    Method debugPath(p:Path)
        For Local i:= Eachin p.steps
            If Self.layout[i.getX()][i.getY()] = CAVE_WALKWAY
                Self.layout[i.getX()][i.getY()] = FOUND_PATH
            End If
        End For
    End Method
	Method addSpikesAndTraps()
        Local spikeCount:Int = walkways.Length / 30
        Local counter:Int = 0
        Local randTile:Int = Rnd(0, walkways.Length)
        
        Local x:Int = 0
        Local y:Int = 0
        While counter < spikeCount
            x = walkways[randTile].getX()
            y = walkways[randTile].getY()
            If layout[x][y] = CAVE_WALKWAY
                layout[x][y] = SPIKES
                counter += 1
            End If
            randTile = Rnd(0, walkways.Length)
        End While
        
        Local trapCount:Int = walkways.Length / 50
        counter = 0
        randTile = Rnd(0, walkways.Length)
        
        x = 0
        y = 0
        While counter < trapCount
            x = walkways[randTile].getX()
            y = walkways[randTile].getY()
            If layout[x][y] = CAVE_WALKWAY
                layout[x][y] = TRAPDOOR_CLOSED
                counter += 1
            End If
            randTile = Rnd(0, walkways.Length)
        End While
        
	End Method
	
	Method makeStartingPoint()
        Local rng = Rnd(0, walkways.Length)
        Local isSet = False
        
        Local x:Int
        Local y:Int
        
        'Local tries:Int = 0
        
        While Not isSet
            'tries += 1
            x = walkways[rng].getX()
            y = walkways[rng].getY()
            
            If layout[x][y] = CAVE_WALKWAY
                isSet = True
            Else
                rng = Rnd(0, walkways.Length)
            End If
            
        End While
        'Print "Tries for start pos: " + tries
        layout[x][y] = ENTRANCE
        entranceX = x
        entranceY = y
        
        'Print "X: " + x
        'Print "Y: " + y
	End Method
	
	Method setTreasure()
        Local rng = Rnd(0, walkways.Length)
        Local isSet = False
        
        Local x:Int
        Local y:Int
        While Not isSet
            x = walkways[rng].getX()
            y = walkways[rng].getY()
            
            If layout[x][y] = CAVE_WALKWAY
                isSet = True
            Else
                rng = Rnd(0, walkways.Length)
            End If
        End While
        
        layout[x][y] = TREASURE
        Self.treasureX = x
        Self.treasureY = y
        treasures.Push(New Point(x, y))
'              layout[Self.entranceX+40][Self.entranceY+40] = TREASURE
'           Self.treasureX = Self.entranceX + 40
'           Self.treasureY = Self.entranceY + 40
        
	End Method
	
	Method setTreasure2()
        Local rng = Rnd(0, walkways.Length)
        Local isSet = False
        
        Local x:Int
        Local y:Int
        While Not isSet
            x = walkways[rng].getX()
            y = walkways[rng].getY()
            
            If layout[x][y] = CAVE_WALKWAY
                isSet = True
            Else
                rng = Rnd(0, walkways.Length)
            End If
        End While
        
        layout[x][y] = TREASURE
        Self.treasure2X = x
        Self.treasure2Y = y
'              layout[Self.entranceX+40][Self.entranceY+40] = TREASURE
'           Self.treasureX = Self.entranceX + 40
'           Self.treasureY = Self.entranceY + 40
        
	End Method
	
	Method validatePath:Bool()
        Local walkable:Int[][] = setArray(layout.Length, layout[0].Length)
        For Local i:Int = 0 Until walkable.Length
            For Local j:Int = 0 Until walkable[0].Length
                If layout[i][j] = CAVE_WALKWAY
                    walkable[i][j] = 1
                Else
                    walkable[i][j] = 0
                End
            End For
        End For
        
        Local x:Int = Self.entranceX
        Local y:Int = Self.entranceY
        
        Local isHope:Bool = True
        walkable[x][y] = 2
        
        While isHope
            walkable[x-1][y] = 2
            
        End While
        Return true
	End Method
	
	Method checkWalkable:Bool(x:Int, y:Int, path:Int[][])
        
        If path[x][y] = 1
            
        Else
        End If
	End Method
	
	Method makeLavaRivers()
        Self.counter = 0
        Local total:Int = walkways.Length / 200
        'Print "Total rivers to make: " + total
        
        Local randX:Int
        Local randY:Int
        Local randTile:Int
        
        Local index:Int = 0
        While index < total
            randX = Rnd(0, Self.width)
            randY = Rnd(0, Self.height)
            randTile = layout[randX][randY]
            If randTile = CAVE_WALKWAY 
                If layout[randX-1][randY] = CAVE_CENTER Or layout[randX+1][randY] = CAVE_CENTER Or layout[randX][randY-1] = CAVE_CENTER Or layout[randX][randY+1] = CAVE_CENTER
                    makeLavaRiver(randX, randY)
                    'Print "Lava River starting at: " + randX + ", " + randY
                    index += 1
                End If
            End If
        End While
        lava = lava.Resize(counter)
	End Method
	
	Method makeLavaRiver(startX:Int, startY:Int)
      Local currentX:Int = startX
      Local currentY:Int = startY
      Local nextX:Int = startX
      Local nextY:Int = startY
'         Local localMinX:Int = startX
'         Local localMinY:Int = startY
'         Local lowestElevation:Float = noiseMap[currentX][currentY]

      Local randDirection:Int
      Local riverEnd:Bool = False
      Local riverLength:Int = 0
      
      Local up:Int = 0
      Local right:Int = 1
      Local down:Int = 2
      Local left:Int = 3
      
      
      Local previousDirection:Int = -1
      Local direction:Int = 0
      
      Local riverType:Int
      
      While riverEnd = False And riverLength < 20
        direction = Rnd(0, 4)
        If direction = up
            nextY -= 1
        Else If direction = down
            nextY += 1
        Else If direction = left
            nextX -= 1
        Else If direction = right
            nextX += 1
        End If

        If layout[nextX][nextY] = CAVE_CENTER
            Local rng:Int = Rnd(0, 100)
            If rng < 10
                riverEnd = True
            Else
                nextX = currentX
                nextY = currentY
            End If
        End If
          layout[currentX][currentY] = LAVA
          lava[Self.counter] = New Point(currentX, currentY)
          Self.counter += 1
          
          If Self.counter = lava.Length
            lava = lava.Resize(lava.Length + 1000)
          End If
          'riverTiles[Self.tileCounter] = New Point(currentX, currentY)
          'Self.tileCounter += 1
          'If Self.tileCounter = riverTiles.Length
          '  riverTiles = riverTiles.Resize(riverTiles.Length + 1000)
          'End If
          currentX = nextX
          currentY = nextY
          riverLength += 1
          previousDirection = direction
        
      End While
	End Method
	'
	'
	'
	Method countWalkways()
        Self.walkways = New Point[1]
        'Print "Initial size: " + Self.walkways.Length
        Local index:Int = 0;
        For Local i:Int = 0 Until layout.Length
            For Local j:Int = 0 Until layout[0].Length
                If layout[i][j] = CAVE_WALKWAY
                    Self.walkways[index] = New Point(i,j)
                    Self.walkways = Self.walkways.Resize(index + 2)
                    index += 1
                End
                
            End
        End
        Self.walkways = Self.walkways.Resize(Self.walkways.Length-1)
        'Print "New length: " + Self.walkways.Length
        'Print "Final point: " + Self.walkways[Self.walkways.Length - 1].getX()
	End Method
	
	'
	'
	'
    Method smoothEdges()
        Local cave:Int[][] = Self.layout
        Local up:Bool = False
        Local down:Bool = False
        Local left:Bool = False
        Local right:Bool = False
        
        For Local i:Int = 0 Until cave.Length
            For Local j:Int = 0 Until cave[0].Length
                up = false 
                down = false
                left = false
                right = false
                
                If cave[i][j] = CAVE_WALKWAY
                    If i > 0 And cave[i-1][j] = CAVE_CENTER
                        up = true
                    End If
                    If i < cave.Length - 2 And cave[i+1][j] = CAVE_CENTER
                        down = true
                    End If
                    If j > 0 And cave[i][j-1] = CAVE_CENTER
                        left = true
                    End If
                    If j < cave[0].Length - 2 And cave[i][j+1] = CAVE_CENTER
                        right = true
                    End If
                    
                    If up = True And down = false
                        If left = true And right = false
                            cave[i][j] = CAVE_UP_LEFT
                        Else If right = True And left = false
                            cave[i][j] = CAVE_DOWN_LEFT
                        End If
                    Else If down = True And up = false
                        If left = true And right = false
                            cave[i][j] = CAVE_UP_RIGHT
                        Else If right = true And left = false
                            cave[i][j] = CAVE_DOWN_RIGHT
                        End If
                    End If  
                End If
            End For
        End For
	End Method
	'Getter method for isGenerated. Should be used to ensure Level object is not
	'accessed until all generation is finished.
	'
	'Returns true is generation has finished.
	Method isGenerated:Bool()
		Return generated
	End Method
	
	#Rem
			setArray
			:Helper method for allocating a 2D array of a given size
	#End
	Method setArray:Int[][](i:Int, j:Int)
		Local result:Int[][] = New Int[i][]
		
		For Local index = 0 Until i
			result[index] = New Int[j]
		End
		
		Return result
	End Method
	#Rem
			randomlyAssignCells
			:Randomly sets values for Cellular automata generation's initial state
	#End
	Method randomlyAssignCells(design:Int[][])
		Seed = Millisecs()
		Local rand:Int = 0
		
		For Local i:Int = 0 Until design.Length
			For Local j:Int = 0 Until design[0].Length
				rand = Rnd(0, 100)
				If rand < 45
					design[i][j] = CAVE_CENTER	
				Else
					design[i][j] = CAVE_WALKWAY
				End If
				If i = 0 Or j = 0 Or i = design.Length - 1 Or j = design[0].Length - 1
					design[i][j] = CAVE_CENTER
				End If

			End
		End
		
	End Method
	#Rem
			generateCellularly
			:One iteration of a cellular automata generation algorithm
	#End
	Method generateCellularly:Int[][](design:Int[][])
		Local adjacentWalls:Int
		Local result:Int[][] = setArray(design.Length, design[0].Length)
		
		For Local i:Int = 0 Until design.Length
			For Local j:Int = 0 Until design[0].Length
				adjacentWalls = checkWalls(design, i, j)
				
				If design[i][j] = CAVE_CENTER
					If adjacentWalls > 3
						result[i][j] = CAVE_CENTER
					Else
						result[i][j] = CAVE_WALKWAY
					End If
				Else If design[i][j] = CAVE_WALKWAY
					If adjacentWalls > 4
						result[i][j] = CAVE_CENTER
					Else
						result[i][j] = CAVE_WALKWAY
					End If
				End If
				
				If i = 0 Or j = 0 Or i = design.Length - 1 Or j = design[0].Length - 1
					result[i][j] = CAVE_BORDER
				End If
			End
		End
		
		Return result
	End Method
	
	#Rem
	#End
	Method checkWalls:Int(design:Int[][], i:Int, j:Int)
		Local total:Int = 0
		If i > 0 And design[i-1][j] = CAVE_CENTER
			total += 1
		End If
		If i < design.Length - 1 And design[i+1][j] = CAVE_CENTER
			total += 1
		End If
		If j > 0 And design[i][j-1] = CAVE_CENTER
			total += 1
		End If
		If j < design[0].Length - 1 And design[i][j+1] = CAVE_CENTER
			total += 1
		End If
		If i > 0 And j < design[0].Length - 1 And design[i-1][j+1] = CAVE_CENTER
			total += 1
		End If
		If i > 0 And j > 0 And design[i-1][j-1] = CAVE_CENTER
			total += 1
		End If
		If i < design.Length - 1 And j < design[0].Length - 1 And design[i+1][j+1] = CAVE_CENTER
			total += 1
		End If
		If i < design.Length - 1 And j > 0 And design[i+1][j-1] = CAVE_CENTER
			total += 1
		End If

		Return total 
	End Method
	#Rem
	#End
	Method printLevel(design:Int[][])

		Local line:String
	
		For Local i:Int = 0 Until design.Length

			line = ""
			For Local j:Int = 0 Until design[1].Length
				line += design[i][j]
			End

			Print line
		End
	End Method
	#Rem
	#End
	Method fillCells(design:Int[][])
		For Local i:Int = 0 Until design.Length
			For Local j:Int = 0 Until design[0].Length
				design[i][j] = CAVE_CENTER
			End
		End
	End Method
	#Rem
	#End
	Method drunkWalk(design:Int[][])
		Local target:Float = (design.Length * design[0].Length) * 0.35
		Local cleared:Int = 1
		
		Local tempX:Int
		Local tempY:Int
		Local direction:Int
		
		Local followBias:Int = 0
		tempX = Rnd(1, design.Length - 1)
		tempY = Rnd(1, design[0].Length - 1)
		
		design[tempX][tempY] = CAVE_WALKWAY
		
		While cleared < target
			If followBias < 40
				direction = Rnd(0, 4)
			End If
			If direction = 0 And tempY > 1
				tempY -= 1
			Else If direction = 1 And tempY < design[0].Length - 2
				tempY += 1
			Else If direction = 2 And tempX > 1
				tempX -= 1
			Else If direction = 3 And tempX < design.Length - 2
				tempX += 1
			End If
			
			If design[tempX][tempY] = CAVE_CENTER
				design[tempX][tempY] = CAVE_WALKWAY
				cleared += 1
			End If
			followBias = Rnd(0, 100)
		End
	End Method
	
	#Rem
	#End
	Method Draw(xOffsetG:Int, yOffsetG:Int)
'           Local currentTexture:Int
'           
'   		For Local i:Int = 0 Until Self.layout.Length
'   			For Local j:Int = 0 Until Self.layout[0].Length
'   				If Self.layout[i][j] = CAVE_CENTER
'   					SetColor(0,0,0)
'   				Else If Self.layout[i][j] = CAVE_WALKWAY
'   					SetColor(255,255,255)
'                   Else
'                       SetColor(125,0,0)
'   				End If
'   				DrawRect(i * 40, j * 40, 40, 40)
'   			End
'   		End
'         Local xOffset:Int = xOffsetG
'         Local yOffset:Int = yOffsetG
'         Local xTarget:Int = xOffset + 16
'         Local yTarget:Int = yOffset + 13
'   '       Local localTexture:Int = 0
'   '       Local counter:Int = 0
'   
'         For Local i:Int = xOffset Until xTarget
'           For Local j:Int = yOffset Until yTarget
'               If i > 0 And i < width - 1 And j > 0 And j < height - 1
'                   If Not (layout[i][j] = CAVE_CENTER Or layout[i][j] = CAVE_WALKWAY)
'                       DrawImage(caveTextures, (i-xOffset)*40+xOffset,(j-yOffset)*40+yOffset, CAVE_WALKWAY)
'                   End If
'                   DrawImage(caveTextures, (i-xOffset)*40+xOffset,(j-yOffset)*40+yOffset, layout[i][j])
'               End If
'           End
'         End
        Local xTile:Int = xOffsetG / 40 - 1
        Local yTile:Int = yOffsetG / 40 - 1
        Local xOffset:Int = xOffsetG Mod 40
        Local yOffset:Int = yOffsetG Mod 40
        
        For Local i:Int = xTile Until xTile + 18
            For Local j:Int = yTile Until yTile + 14
                If i > -1 And i < layout.Length And j > -1 And j < layout[i].Length
                    DrawImage(caveTextures, i * 40 , j * 40, layout[i][j])
                End If
            End For
        End For
	End Method
End Class

Function getDist:Float(sX:Int, sY:Int, tX:Int, tY:Int)
    Local dx = tX - sX
    Local dy = tY - sY
    
    Return math.Sqrt(dx * dx + dy * dy) 
End Function