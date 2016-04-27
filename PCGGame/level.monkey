Import mojo
Import playniax.ignitionx.engine
Import point

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
    Const LAVA_NORTH_SOUTH = 6
    Const LAVA_NORTH_WEST = 7
    Const LAVA_NORTH_EAST = 8
    Const LAVA_SOUTH_WEST = 9
    Const LAVA_SOUTH_EAST = 10
    Const LAVA_EAST_WEST = 11
    Const LAVA = 12
    
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
		
		caveTextures = iLoadSprite("cave_texture40.png", 40, 40, 13)
		
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
		Self.generated = true
	End Method
	
	Method makeLavaRivers()
        Self.counter = 0
        Local total:Int = walkways.Length / 200
        Print "Total rivers to make: " + total
        
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
                    Print "Lava River starting at: " + randX + ", " + randY
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
            riverEnd = True
        End If
'           Else 'If layout[nextX][nextY] = CAVE_WALKWAY
'           
'             If direction = up
'               If previousDirection = left
'                   riverType = LAVA_NORTH_EAST
'               Else If previousDirection = right
'                   riverType = LAVA_NORTH_WEST
'               Else
'                   riverType = LAVA_NORTH_SOUTH
'               End If
'             Else If direction = down
'               If previousDirection = left
'                   riverType = LAVA_SOUTH_EAST
'               Else If previousDirection = right
'                   riverType = LAVA_SOUTH_WEST
'               Else
'                   riverType = LAVA_NORTH_SOUTH
'               End If
'             Else If direction = left
'               If previousDirection = up
'                   riverType = LAVA_SOUTH_WEST
'               Else If previousDirection = down
'                   riverType = LAVA_NORTH_WEST
'               Else
'                   riverType = LAVA_EAST_WEST
'               End If
'             Else If direction = right
'               If previousDirection = up
'                   riverType = LAVA_SOUTH_EAST
'               Else If previousDirection = down
'                   riverType = LAVA_NORTH_EAST
'               Else
'                   riverType = LAVA_EAST_WEST
'               End If
'             End If
'           End If
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
        Print "Initial size: " + Self.walkways.Length
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
        Print "New length: " + Self.walkways.Length
        Print "Final point: " + Self.walkways[Self.walkways.Length - 1].getX()
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
      Local xOffset:Int = xOffsetG
      Local yOffset:Int = yOffsetG
      Local xTarget:Int = xOffset + 16
      Local yTarget:Int = yOffset + 13
'       Local localTexture:Int = 0
'       Local counter:Int = 0

      For Local i:Int = xOffset Until xTarget
        For Local j:Int = yOffset Until yTarget
            If i > 0 And i < width - 1 And j > 0 And j < height - 1
                If Not (layout[i][j] = CAVE_CENTER Or layout[i][j] = CAVE_WALKWAY)
                    DrawImage(caveTextures, (i-xOffset)*40+xOffset,(j-yOffset)*40+yOffset, CAVE_WALKWAY)
                End If
                DrawImage(caveTextures, (i-xOffset)*40+xOffset,(j-yOffset)*40+yOffset, layout[i][j])
            End If
        End
      End
	End Method
End Class