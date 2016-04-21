Import mojo
Import playniax.ignitionx.engine
Import simplexnoise
Import level

Global textures:Image
Global enemies:Image
'
'Scene for use in testing content generation using simplex noise generation
'as the main driving force
'
Class NoiseTestScene Extends iEngine

'Constants for use in identifying terrain type
    Const DEEP_WATER:Int = 0
    Const SHALLOW_WATER:Int = 1
    Const BEACH:Int = 2
    Const LIGHT_GRASS:Int = 3
    Const HEAVY_GRASS:Int = 4
    Const MOUNTAIN:Int = 5
    Const DARK_SNOW_MOUNTAIN:Int = 6
    Const LIGHT_SNOW_MOUNTAIN:Int = 7
    Const MAP_EDGE:Int = 8
    Const CAVE_ENTRANCE:Int = 9
    
'Constants for generation caps/quotas
    Const NUMBER_OF_CAVES:Int = 1

'Should be refactored into an object, noiseMap = elevation 
    Field noiseMap:Float[][]
    Field moisture:Float[][]
    Field biomes:Int[][]
    Field caves:Level[NUMBER_OF_CAVES]
    Field enemyPlacement:Int[][]
    
    Field mapWidth:Int
    Field mapHeight:Int
    Field playfieldN:iPlayfield
    Field backLayer:iLayer
    Field playerLayer:iLayer
    Field player:Hero
    
    Field chunks:Int[][]
 '
 'Overloads OnCreate Method from iEngine
 '   
    Method OnCreate ()
        Print "Creating Noise Test"
        Self.playfieldN=New iPlayfield
        Self.playfieldN.AttachLast()
        Self.playfieldN.AutoCls(0,0,0)
        Self.playfieldN.Width=600
        Self.playfieldN.Height=460
        Self.playfieldN.Position(25,15)
        Self.playfieldN.ZoomPointX(200)
        Self.playfieldN.ZoomPointY(128)
        
        textures = iLoadSprite("textures20.png", 20, 20, 9)
        enemies = iLoadSprite("enemies20.png", 20, 20, 9)
'         Self.backLayer = New iLayer()
'         Self.backLayer.AttachLast(Self.playFieldN)
'         
'         Self.playerLayer = New iLayer()
'         Self.player = New Hero()
'         Self.playerLayer.AttachLast(Self.player)
        mapWidth = 600
        mapHeight = 460
        
        chunks = setArray(30, 23)
    End
 '
 'Overloads OnRender Method from iEngine
 '   
    Method OnRender ()
    	drawNoiseMap(mapWidth, mapHeight)
    	drawEnemies(mapWidth, mapHeight)
    	  'Debugging Text
    	DrawText("CameraX: " + Self.playfieldN.CameraX, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+10)
    	DrawText("CameraY: " + Self.playfieldN.CameraY, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+20)
    	DrawText("MapX: " + Self.playfieldN.CameraX , Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+40)
    	DrawText("MapY: " + Self.playfieldN.CameraY, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+50)
      DrawText("MapXEnd: " + (Self.playfieldN.CameraX + 120), Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+70)
    	DrawText("MapYEnd: " + (Self.playfieldN.CameraY + 89), Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+80)
    End
 '
 'Overloads OnStart Method from iEngine
 '   
    Method OnStart ()
        Print "Starting Noise Test"
        Local n:SimplexNoise = New SimplexNoise()
        noiseMap = n.generateOctavedNoiseMap(mapWidth, mapHeight, 5, 0.5, 1)
        moisture = n.generateNoiseMap(mapWidth, mapHeight)
        biomes = setArray(mapWidth, mapHeight)
        determineBiomes(noiseMap, moisture, mapWidth, mapHeight)
        makeRivers(25)
        selectRandomStartPoint()
        makeCaves(1)
        placeEnemies(mapWidth, mapHeight) 
           
'         Local x:Int = noiseMap.Length
'         Local y:Int = noiseMap[0].Length
'         Print "X: " + x
'         Print "Y: " + y
'         Print "Top Left Corner: " + noiseMap[0][0]
'         Print "Top Right Corner: " + noiseMap[x-1][0]
'         Print "Bottom Left Corner: " + noiseMap[0][y-1]
'         Print "Bottom Right Corner: " + noiseMap[x-1][y-1]
        'noiseMap = n.generateNoiseMap(mapWidth, mapHeight)
    End
'
'Creates /total/ rivers
'
    Method makeRivers(total:Int)
      Local rivers:Int = 0
      Local randX:Int
      Local randY:Int
      Local randBiome:Int
      While rivers < total
        randX = Rnd(0, mapWidth)
        randY = Rnd(0, mapHeight)
        randBiome = biomes[randX][randY]
        If randBiome = MOUNTAIN
          makeRiver(randX, randY)
          Print "River at " + randX + ", " + randY
          rivers += 1
        End
      End
    End Method
'
'Create a river starting at the given coordinates
'
    Method makeRiver(startX:Int, startY:Int)
      Local currentX:Int = startX
      Local currentY:Int = startY
      Local localMinX:Int = startX
      Local localMinY:Int = startY
      Local lowestElevation:Float = noiseMap[currentX][currentY]
      Local riverEnd:Bool = False
      Local riverLength:Int = 0
      biomes[currentX][currentY] = SHALLOW_WATER
      
      While riverEnd = False And riverLength < 100
        
        If currentX > 0 And noiseMap[currentX-1][currentY] < lowestElevation
          localMinX = currentX-1
          localMinY = currentY
          lowestElevation = noiseMap[localMinX][localMinY]
        End
        If currentX < mapWidth - 2 And noiseMap[currentX+1][currentY] < lowestElevation
          localMinX = currentX+1
          localMinY = currentY
          lowestElevation = noiseMap[localMinX][localMinY]
        End
        If currentY > 0 And noiseMap[currentX][currentY-1] < lowestElevation
          localMinX = currentX
          localMinY = currentY-1
          lowestElevation = noiseMap[localMinX][localMinY]
        End
        If currentY < mapHeight - 2 And noiseMap[currentX][currentY+1] < lowestElevation
          localMinX = currentX
          localMinY = currentY+1
          lowestElevation = noiseMap[localMinX][localMinY]
        End
        
        If biomes[localMinX][localMinY] = DEEP_WATER
          'Print "River ends at " + localMinX + ", " + localMinY
          riverEnd = true
        Else If localMinX = currentX And localMinY = currentY
          makeLake(currentX, currentY)
          riverEnd = true
        Else    
          biomes[localMinX][localMinY] = SHALLOW_WATER
          currentX = localMinX
          currentY = localMinY
          riverLength += 1
        End
      End
      
    End Method
'
'Creates a randomly sized lake at the given coordinates
'
    Method makeLake(centerX:Int, centerY:Int)
        
        Local lakeWidth:Int = Rnd(1,5)
        Local lakeHeight:Int = Rnd(1,5)
        Print "Make a lake at " + centerX + ", " + centerY + ", Size: " + lakeWidth + "x" + lakeHeight
        Local tempX:Int = centerX - lakeWidth
        Local tempY:Int = centerY - lakeHeight
        
        Local xTarget:Int = tempX + (lakeWidth*2+1)
        Local yTarget:Int = tempY + (lakeHeight*2+1)
        
        biomes[centerX][centerY] = SHALLOW_WATER
        
        For Local i:Int = tempX Until xTarget
          For Local j:Int = tempY Until yTarget
            If i > 0 And i < mapWidth - 1 And j > 0 And j < mapHeight - 1 And Not (biomes[i][j] = DEEP_WATER)
              biomes[i][j] = SHALLOW_WATER
            End
          End
        End
        
    End Method
'
'Select position for /total/ caves and generate them.
'
    Method makeCaves(total:Int)
      Local lowX = Self.playfieldN.CameraX - 50
      Local highX = Self.playfieldN.CameraX + 100
      Local lowY = Self.playfieldN.CameraY - 50
      Local highY = Self.playfieldN.CameraY + 100
      
      Local randX:Int
      Local randY:Int
      
      Local cavesMade:Int = 0
      Local isReachable:Bool = false
      
      While cavesMade < total
          isReachable = false
          randX = Rnd(lowX, highX)
          randY = Rnd(lowY, highY)
          
          ' Cave entrances should be on the sides of mountains
          If biomes[randX][randY] = MOUNTAIN
          ' Check to see if the entrance is reachable
          ' Current code is insufficient, but will often be correct
            If (randX > 0 And biomes[randX-1][randY] = HEAVY_GRASS)
              isReachable = true
            End
            If (randX < mapWidth-2 And biomes[randX+1][randY] = HEAVY_GRASS)
              isReachable = true
            End
            If (randX > 0 And randY > 0 And biomes[randX-1][randY-1] = HEAVY_GRASS)
              isReachable = true
            End
            If (randX < mapWidth-2 And randY > 0 And biomes[randX+1][randY-1] = HEAVY_GRASS)
              isReachable = true
            End
            If (randY > 0 And biomes[randX][randY-1] = HEAVY_GRASS)
              isReachable = true
            End
            If (randY > mapHeight-2 And biomes[randX][randY+1] = HEAVY_GRASS)
              isReachable = true
            End
            If (randX > 0 And randY < mapHeight-2 And biomes[randX-1][randY+1] = HEAVY_GRASS)
              isReachable = true
            End
            If (randX < mapWidth-2 And randY < mapHeight - 2 And biomes[randX+1][randY+1] = HEAVY_GRASS)
              isReachable = true
            End
          End
          
          If isReachable
            Print "Cave entrance at " + randX + ", " + randY
            biomes[randX][randY] = MAP_EDGE
            caves[cavesMade] = New Level(randX, randY, 150, 100, "Cellular")
            cavesMade += 1
          End
      End
    End Method
'
'Selects a semi-random spot on the map for the player to start out at
'
    Method selectRandomStartPoint()
        Local x:Int
        Local y:Int
        Local startPointSet:Bool
        
        While Not startPointSet
          x = Rnd(200, 400)
          y = Rnd(130, 330)
          If Self.biomes[x][y] = LIGHT_GRASS Or Self.biomes[x][y] = HEAVY_GRASS
            Self.playfieldN.CameraX = x
            Self.playfieldN.CameraY = y
            startPointSet = true
          End
        End
    End Method
 '
 'Overloads OnStop Method from iEngine
 '   
    Method OnStop ()
        Print "Stopping Noise Test"
    End
 '
 'Overloads OnUpdate Method from iEngine
 '   
    Method OnUpdate ()
			If KeyDown(KEY_CONTROL)
        
            If KeyDown(KEY_A)
                Self.playfieldN.AlphaFade=Self.playfieldN.AlphaFade-0.01
                If Self.playfieldN.AlphaFade < 0 Self.playfieldN.AlphaFade=Self.playfieldN.AlphaFade+1
            End If
            
            If KeyDown(KEY_Z)
                Self.playfieldN.ZoomX=Self.playfieldN.ZoomX-0.01
                Self.playfieldN.ZoomY=Self.playfieldN.ZoomY-0.01
            End If
        
        Else
        
            If KeyDown(KEY_A)
                Self.playfieldN.AlphaFade=Self.playfieldN.AlphaFade+0.01
                If Self.playfieldN.AlphaFade > 1 Self.playfieldN.AlphaFade=Self.playfieldN.AlphaFade-1
            End If
            
            If KeyDown(KEY_Z)
                Self.playfieldN.ZoomX=Self.playfieldN.ZoomX+0.01
                Self.playfieldN.ZoomY=Self.playfieldN.ZoomY+0.01
            End If
        
        End If
			If KeyDown(KEY_LEFT)
            Self.playfieldN.CameraX=Self.playfieldN.CameraX-1
        End If
    
        If KeyDown(KEY_RIGHT)
            Self.playfieldN.CameraX=Self.playfieldN.CameraX+1
        End If
    
        If KeyDown(KEY_UP)
            Self.playfieldN.CameraY=Self.playfieldN.CameraY-1
        End If
    
        If KeyDown(KEY_DOWN)
            Self.playfieldN.CameraY=Self.playfieldN.CameraY+1
        End If
        checkCameraBounds()
		End
'
'Method for checking the boundaries of the game camera. Ensures camera
'stops at the edges of the map
'
		Method checkCameraBounds()
		
        If Self.playfieldN.CameraX < 0
          Self.playfieldN.CameraX = 0
        End
        If Self.playfieldN.CameraY < 0
          Self.playfieldN.CameraY = 0
        End
'         Print "Playfield CameraX: " + Self.playfieldN.CameraX
'         Print "Camera End: " + (mapWidth - 30)
'         If Self.playfieldN.CameraX + (mapWidth/20) > mapWidth
'           Self.playfieldN.CameraX = mapWidth - (mapWidth/20)
'         End
        If Self.playfieldN.CameraX + (Self.playfieldN.Width/20) > Self.playfieldN.Width
          Self.playfieldN.CameraX = Self.playfieldN.Width - (Self.playfieldN.Width/20)
        End
        If Self.playfieldN.CameraY + (Self.playfieldN.Height/20) > Self.playfieldN.Height
          Self.playfieldN.CameraY = Self.playfieldN.Height - (Self.playfieldN.Height/20)
        End
		End
		
'
'Method to draw the textures drawn from simplex noise map
'
		Method drawNoiseMap(w:Int, h:Int)
      Local xOffset:Int = Self.playfieldN.CameraX
      Local yOffset:Int = Self.playfieldN.CameraY
      Local xTarget:Int = xOffset + 30
      Local yTarget:Int = yOffset + 23
'       Local localTexture:Int = 0
'       Local counter:Int = 0

      For Local i:Int = xOffset Until xTarget
        For Local j:Int = yOffset Until yTarget
          DrawImage(textures, (i-xOffset)*20+xOffset,(j-yOffset)*20+yOffset, biomes[i][j])
        End
      End
      
      'Print "Counter: " + counter
'       DrawText("xOffset: " + xOffset, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+100)
'     	DrawText("yOffset: " + yOffset, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+110)
'     	DrawText("xTarget: " + xTarget, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+120)
'     	DrawText("yTarget: " + yTarget, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+130)
		End Method
		
'
'Method to draw enemies on screen
'
		Method drawEnemies(w:Int, h:Int)
      Local xOffset:Int = Self.playfieldN.CameraX
      Local yOffset:Int = Self.playfieldN.CameraY
      Local xTarget:Int = xOffset + 30
      Local yTarget:Int = yOffset + 23

      For Local i:Int = xOffset Until xTarget
        For Local j:Int = yOffset Until yTarget
          If Not (enemyPlacement[i][j] = -1)
            DrawImage(enemies, (i-xOffset)*20+xOffset,(j-yOffset)*20+yOffset, enemyPlacement[i][j])
          End
        End
      End
		End Method
		
'
'Method to read noise map and set the terrain type based on elevation and moisture
'
		Method determineBiomes(elevation:Float[][], moist:Float[][], width:Int, height:Int)
      Local localBiome:Int = 0
      
      For Local i:Int = 0 Until width
        For Local j:Int = 0 Until height
          If noiseMap [i][j] < -0.35
            localBiome = DEEP_WATER
           Else If noiseMap [i][j] < -0.3
            localBiome = SHALLOW_WATER
           Else If noiseMap [i][j] < -0.2
            localBiome = BEACH
           Else If noiseMap [i][j] < -0.1
            localBiome = LIGHT_GRASS
           Else If noiseMap [i][j] < 0.2
            localBiome = HEAVY_GRASS
           Else If noiseMap [i][j] < 0.5
            localBiome = MOUNTAIN
           Else If noiseMap [i][j] < 0.6
            localBiome = DARK_SNOW_MOUNTAIN
           Else If noiseMap [i][j] < 1
            localBiome = LIGHT_SNOW_MOUNTAIN
          End
          
          If i = 0 Or i = width-1 Or j = 0 Or j = height-1
            localBiome = MAP_EDGE
          End
          
          biomes[i][j] = localBiome
        
        End
      End
		End Method
'
'Method to determine which kind of enemy to create based on the terrain
'type at its starting location
'
    Method determineEnemyType(enemies:Int[][], habitat:Int[][])
      For Local i:Int = 0 Until enemies.Length
        For Local j:Int = 0 Until enemies[0].Length
          If Not (enemies[i][j] = -1)
              enemies[i][j] = habitat[i][j]
          End
        End
      End
		End Method
'
'Method to decide where to place enemies on the game map.
'
		Method placeEnemies(width:Int, height:Int)
      Self.enemyPlacement = setArray(width, height)
      randomlyAssignCells(Self.enemyPlacement, 1)
      determineEnemyType(Self.enemyPlacement, Self.biomes)
		End Method
End Class

Function randomlyAssignCells(cells:Int[][], threshold:Int)
		Seed = Millisecs()
		Local rand:Int = 0
		Local thresh:Int = threshold Mod 100
		
		For Local i:Int = 0 Until cells.Length
			For Local j:Int = 0 Until cells[0].Length
				rand = Rnd(0, 100)
				If rand < thresh
					cells[i][j] = 1	
				Else
					cells[i][j] = -1
				End
				If i = 0 Or j = 0 Or i = cells.Length - 1 Or j = cells[0].Length - 1 Or cells[i][j] = 9
					cells[i][j] = -1
				End

			End
		End
		
End Function

Function setArray:Int[][](i:Int, j:Int)
		Local result:Int[][] = New Int[i][]
		
		For Local index = 0 Until i
			result[index] = New Int[j]
		End
		
		Return result
End Function