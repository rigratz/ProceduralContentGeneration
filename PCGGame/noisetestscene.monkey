Import mojo
Import playniax.ignitionx.engine
Import simplexnoise
Import level
Import point
Import pathfinder
Import gameplayscene

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
    Const SWAMP:Int = 5
    Const FOREST:Int = 6
    Const DESERT:Int = 7
    Const MOUNTAIN:Int = 8
    Const DARK_SNOW_MOUNTAIN:Int = 9
    Const LIGHT_SNOW_MOUNTAIN:Int = 10
    Const MAP_EDGE:Int = 11
    Const RIVER_NORTH_SOUTH = 12
    Const RIVER_NORTH_WEST = 13
    Const RIVER_NORTH_EAST = 14
    Const RIVER_SOUTH_WEST = 15
    Const RIVER_SOUTH_EAST = 16
    Const RIVER_EAST_WEST = 17
    Const CAVE_ENTRANCE:Int = 18
    Const CACTUS:Int = 19
    Const BEACH_LIFE:Int = 20
    Const MOUNTAIN_MOSSY:Int = 21
    Const MOUNTAIN_SNOWY:Int = 22
    Const LAKE_TOP_LEFT:Int = 23
    Const LAKE_TOP:Int = 24
    Const LAKE_TOP_RIGHT:Int = 25
    Const LAKE_LEFT:Int = 26
    Const LAKE_RIGHT:Int = 27
    Const LAKE_BOTTOM_LEFT:Int = 28
    Const LAKE_BOTTOM:Int = 29
    Const LAKE_BOTTOM_RIGHT:Int = 30
    Const LIGHT_GRASS_ROCKS:Int = 31
    
    Const TOTAL_TEXTURES:Int = 32
    
'Constants for generation caps/quotas
    Const NUMBER_OF_CAVES:Int = 1

'Terrain arrays for pre-processing
    Field deepWaterTiles:Point[]
    Field shallowWaterTiles:Point[]
    Field beachTiles:Point[]
    Field lightGrassTiles:Point[]
    Field heavyGrassTiles:Point[]
    Field swampTiles:Point[]
    Field forestTiles:Point[]
    Field desertTiles:Point[]
    Field mountainTiles:Point[]
    Field darkSnowMountainTiles:Point[]
    Field lightSnowMountainTiles:Point[]
    Field riverTiles:Point[]
    Field caveEntranceTiles:Point[]
    
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
    
    Field sprite1:iLayerSprite
    Field layer:iLayer
    
    Field bBox:BoundingRect
    
    Field caveX:Int
    Field caveY:Int
    
    Field startX:Int
    Field startY:Int
    
    Field tileCounter:Int
    
    Field chunks:Int[][]
 '
 'Overloads OnCreate Method from iEngine
 '   
    Method OnCreate ()
        'Print "Creating Noise Test"
        Self.playfieldN=New iPlayfield
        Self.playfieldN.AttachLast()
        Self.playfieldN.AutoCls(0,0,0)
        Self.playfieldN.Width=600
        Self.playfieldN.Height=460
        Self.playfieldN.Position(25,15)
        Self.playfieldN.ZoomPointX(200)
        Self.playfieldN.ZoomPointY(128)
        
        textures = iLoadSprite("textures40.png", 40, 40, TOTAL_TEXTURES)
        enemies = iLoadSprite("enemies40.png", 40, 40, 9)
'         Self.backLayer = New iLayer()
'         Self.backLayer.AttachLast(Self.playFieldN)
'         
'         Self.playerLayer = New iLayer()
'         Self.player = New Hero()
'         Self.playerLayer.AttachLast(Self.player)
        mapWidth = 600
        mapHeight = 460
        
        Self.layer=New iLayer
        Local img:Image = iLoadSprite("char_walk_down.png",69,102,4)
        Self.layer.AttachLast(Self.playfieldN)

        Self.sprite1=New iLayerSprite
        Self.sprite1.AttachLast(Self.layer)
        Self.sprite1.ImagePointer(img)
        Self.sprite1.Position(300,275)
        
        'chunks = setArray(30, 23)
        Self.deepWaterTiles = New Point[1000]
        Self.shallowWaterTiles = New Point[1000]
        Self.beachTiles = New Point[1000]
        Self.lightGrassTiles = New Point[1000]
        Self.heavyGrassTiles = New Point[1000]
        Self.swampTiles = New Point[1000]
        Self.forestTiles = New Point[1000]
        Self.desertTiles = New Point[1000]
        Self.mountainTiles = New Point[1000]
        Self.darkSnowMountainTiles = New Point[1000]
        Self.lightSnowMountainTiles = New Point[1000]
        Self.riverTiles = New Point[1000]
        Self.caveEntranceTiles = New Point[1000]
    End Method
 '
 'Overloads OnRender Method from iEngine
 '   
    Method OnRender ()
    	drawNoiseMap(mapWidth, mapHeight)
    	drawEnemies(mapWidth, mapHeight)
    	  'Debugging Text
    	DrawText("CameraX: " + Self.playfieldN.CameraX, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+10)
    	DrawText("CameraY: " + Self.playfieldN.CameraY, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+20)
    	'DrawText("MapX: " + Self.playfieldN.CameraX , Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+40)
    	'DrawText("MapY: " + Self.playfieldN.CameraY, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+50)
        'DrawText("MapXEnd: " + (Self.playfieldN.CameraX + 120), Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+70)
    	'DrawText("MapYEnd: " + (Self.playfieldN.CameraY + 89), Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+80)
    	'DrawRect(Self.bBox.x, Self.bBox.y, Self.bBox.width, Self.bBox.height)
    End Method
 '
 'Overloads OnStart Method from iEngine
 '   
    Method OnStart ()
        'Print "Starting Noise Test"
        Local n:SimplexNoise = New SimplexNoise()
        noiseMap = n.generateOctavedNoiseMap(mapWidth, mapHeight, 5, 0.5, 1)
        moisture = n.generateOctavedNoiseMap(mapWidth, mapHeight, 5, 0.5, 1)
        biomes = setArray(mapWidth, mapHeight)
        determineBiomes(noiseMap, moisture, mapWidth, mapHeight)
        processBiomes()
        detailBiomes()
        'Print "Make rivers"
        makeRivers(40)
        'Print "Select Start"
        selectRandomStartPoint()
        'Print "Make Caves"
        makeCaves(1)
        placeEnemies(mapWidth, mapHeight)
        Print "Enemy array: " + enemyPlacement.Length + ", " + enemyPlacement[0].Length
        
        Local overworldSearch:AStarSearch = New AStarSearch(biomes, 300, false)
        Local pathToCave:Path = overworldSearch.findPathOverworld(biomes, Self.startX, Self.startY, Self.caveX, Self.caveY)
        If pathToCave
            For Local i:= Eachin pathToCave.steps
                If Self.biomes[i.getX()][i.getY()] = CAVE_ENTRANCE
                    Continue
                Else If Self.biomes[i.getX()][i.getY()] > 11 And Self.biomes[i.getX()][i.getY()] < 18
                    Print "MAKING BRIDGE! at: " + i.getX()*40 + ", " + i.getY()*40
                    Self.biomes[i.getX()][i.getY()] = SHALLOW_WATER
                End If
            End For
        Else
            Print "No path"
        End If
    End Method
    
    Method detailBiomes()
        Local cacti:Int = desertTiles.Length / 20
        Local counter:Int = 0
        Local randTile:Int = Rnd(0, desertTiles.Length)
        
        Local x:Int = 0
        Local y:Int = 0
        While counter < cacti
            x = desertTiles[randTile].getX()
            y = desertTiles[randTile].getY()
            If biomes[x][y] = DESERT
                biomes[x][y] = CACTUS
                counter += 1
            End If
            randTile = Rnd(0, desertTiles.Length)
        End While
        
        Local beachItems:Int = beachTiles.Length / 30
        counter = 0
        randTile = Rnd(0, beachTiles.Length)
        
        x = 0
        y = 0
        While counter < beachItems
            x = beachTiles[randTile].getX()
            y = beachTiles[randTile].getY()
            If biomes[x][y] = BEACH
                biomes[x][y] = BEACH_LIFE
                counter += 1
            End If
            randTile = Rnd(0, beachTiles.Length)
        End While
        
        Local rocks:Int = lightGrassTiles.Length / 40
        counter = 0
        randTile = Rnd(0, lightGrassTiles.Length)
        
        x = 0
        y = 0
        
        While counter < rocks
            x = lightGrassTiles[randTile].getX()
            y = lightGrassTiles[randTile].getY()
            If biomes[x][y] = LIGHT_GRASS
                biomes[x][y] = LIGHT_GRASS_ROCKS
                counter += 1
            End If
            randTile = Rnd(0, lightGrassTiles.Length)
        End While
    End Method
'
'Creates /total/ rivers
'
    Method makeRivers(total:Int)
        Self.tileCounter = 0
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
                rivers += 1
            End If
        End While
        riverTiles = riverTiles.Resize(Self.tileCounter)
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
      
        Local up:Int = 0
        Local right:Int = 1
        Local down:Int = 2
        Local left:Int = 3
      
      
        Local previousDirection:Int = -1
        Local direction:Int = 0
      
        Local riverType:Int
      
        While riverEnd = False And riverLength < 100
        
            If currentX > 0 And noiseMap[currentX-1][currentY] < lowestElevation
                localMinX = currentX-1
                localMinY = currentY
                lowestElevation = noiseMap[localMinX][localMinY]
                direction = left
            End If
            If currentX < mapWidth - 2 And noiseMap[currentX+1][currentY] < lowestElevation
                localMinX = currentX+1
                localMinY = currentY
                lowestElevation = noiseMap[localMinX][localMinY]
                direction = right
            End If
            If currentY > 0 And noiseMap[currentX][currentY-1] < lowestElevation
                localMinX = currentX
                localMinY = currentY-1
                lowestElevation = noiseMap[localMinX][localMinY]
                direction = up
            End
            If currentY < mapHeight - 2 And noiseMap[currentX][currentY+1] < lowestElevation
                localMinX = currentX
                localMinY = currentY+1
                lowestElevation = noiseMap[localMinX][localMinY]
                direction = down
            End If

            If localMinX = currentX And localMinY = currentY
                Local decision:Int = Rnd(0, 100)
                If Not (currentX -3 > - 1 And currentY - 3 > -1 And currentX + 3 < noiseMap.Length And currentY + 3 < noiseMap[0].Length)
                    riverEnd = true
                Else If decision < 15
                    makeLake(currentX, currentY)
                    riverEnd = true
                Else
                    If currentX -4 > 0 And noiseMap[currentX-3][currentY] < lowestElevation
                        localMinX = currentX-3
                        localMinY = currentY
                        lowestElevation = noiseMap[localMinX][localMinY]
                        direction = left
                    End If
                    If currentX < mapWidth - 5 And noiseMap[currentX+3][currentY] < lowestElevation
                      localMinX = currentX+3
                      localMinY = currentY
                      lowestElevation = noiseMap[localMinX][localMinY]
                      direction = right
                    End If
                    If currentY > 0 And noiseMap[currentX][currentY-3] < lowestElevation
                      localMinX = currentX
                      localMinY = currentY-3
                      lowestElevation = noiseMap[localMinX][localMinY]
                      direction = up
                    End If
                    If currentY < mapHeight - 5 And noiseMap[currentX][currentY+3] < lowestElevation
                      localMinX = currentX
                      localMinY = currentY+3
                      lowestElevation = noiseMap[localMinX][localMinY]
                      direction = down
                    End If
                End If
            Else
                If direction = up
                    If previousDirection = left
                        riverType = RIVER_NORTH_EAST
                    Else If previousDirection = right
                        riverType = RIVER_NORTH_WEST
                    Else
                        riverType = RIVER_NORTH_SOUTH
                    End If
                Else If direction = down
                    If previousDirection = left
                        riverType = RIVER_SOUTH_EAST
                    Else If previousDirection = right
                        riverType = RIVER_SOUTH_WEST
                    Else
                        riverType = RIVER_NORTH_SOUTH
                    End If
                Else If direction = left
                    If previousDirection = up
                        riverType = RIVER_SOUTH_WEST
                    Else If previousDirection = down
                        riverType = RIVER_NORTH_WEST
                    Else
                        riverType = RIVER_EAST_WEST
                    End If
                Else If direction = right
                    If previousDirection = up
                        riverType = RIVER_SOUTH_EAST
                    Else If previousDirection = down
                        riverType = RIVER_NORTH_EAST
                    Else
                        riverType = RIVER_EAST_WEST
                    End If
                End If
                biomes[currentX][currentY] = riverType
                If localMinX = currentX - 3
                    biomes[currentX-1][currentY] = RIVER_EAST_WEST
                    biomes[currentX-2][currentY] = RIVER_EAST_WEST
                Else If localMinX = currentX + 3
                    biomes[currentX+1][currentY] = RIVER_EAST_WEST
                    biomes[currentX+2][currentY] = RIVER_EAST_WEST
                Else If localMinY = currentY - 3
                    biomes[currentX][currentY-1] = RIVER_NORTH_SOUTH
                    biomes[currentX][currentY-2] = RIVER_NORTH_SOUTH
                Else If localMinY = currentY + 3
                    biomes[currentX][currentY+1] = RIVER_NORTH_SOUTH
                    biomes[currentX][currentY+2] = RIVER_NORTH_SOUTH
                End If
                
                If biomes[localMinX][localMinY] = DEEP_WATER
                    riverEnd = true
                End If
                riverTiles[Self.tileCounter] = New Point(currentX, currentY)
                Self.tileCounter += 1
                If Self.tileCounter = riverTiles.Length
                    riverTiles = riverTiles.Resize(riverTiles.Length + 1000)
                End If
                currentX = localMinX
                currentY = localMinY
                riverLength += 1
                previousDirection = direction
            End If
        End While
      
    End Method
'
'Creates a randomly sized lake at the given coordinates
'
    Method makeLake(centerX:Int, centerY:Int)
        
        Local lakeWidth:Int = Rnd(1,5)
        Local lakeHeight:Int = Rnd(1,5)
        'Print "Make a lake at " + centerX + ", " + centerY + ", Size: " + lakeWidth + "x" + lakeHeight
        Local tempX:Int = centerX - lakeWidth
        Local tempY:Int = centerY - lakeHeight
        
        Local xTarget:Int = tempX + (lakeWidth*2+1)
        Local yTarget:Int = tempY + (lakeHeight*2+1)
        
        biomes[centerX][centerY] = DEEP_WATER
        
        For Local i:Int = tempX Until xTarget
            For Local j:Int = tempY Until yTarget
                If i > 0 And i < mapWidth - 1 And j > 0 And j < mapHeight - 1 And Not (biomes[i][j] = DEEP_WATER)
                    If i = centerX - lakeWidth
                        If j = centerY - lakeHeight
                            biomes[i][j] = LAKE_TOP_LEFT
                        Else If j = yTarget - 1
                            biomes[i][j] = LAKE_BOTTOM_LEFT
                        Else
                            biomes[i][j] = LAKE_LEFT
                        End If
                    Else If i = xTarget - 1
                        If j = centerY - lakeHeight
                            biomes[i][j] = LAKE_TOP_RIGHT
                        Else If j = yTarget - 1
                            biomes[i][j] = LAKE_BOTTOM_RIGHT
                        Else
                            biomes[i][j] = LAKE_RIGHT
                        End If
                    Else If j = centerY - lakeHeight
                        biomes[i][j] = LAKE_TOP
                    Else If j = yTarget - 1
                        biomes[i][j] = LAKE_BOTTOM
                    Else
                        biomes[i][j] = DEEP_WATER
                    End If
                End If
            End For
        End For
        
    End Method
'
'Select position for /total/ caves and generate them.
'
    Method makeCaves(total:Int)
        Local lowX = Self.playfieldN.CameraX/40 - 50
        Local highX = Self.playfieldN.CameraX/40 + 100
        Local lowY = Self.playfieldN.CameraY/40 - 50
        Local highY = Self.playfieldN.CameraY/40 + 100
      
        Local randX:Int
        Local randY:Int
      
        Local cavesMade:Int = 0
        Local isReachable:Bool = false
      
        While cavesMade < total
            isReachable = false
            randX = Rnd(lowX, highX)
            randY = Rnd(lowY, highY)
          
          ' Cave entrances should be on the sides of mountains
            If biomes[randX][randY] = MOUNTAIN Or biomes[randX][randY] = MOUNTAIN_MOSSY Or biomes[randX][randY] = MOUNTAIN_SNOWY
          ' Check to see if the entrance is reachable
          ' Current code is insufficient, but will often be correct
                If (randX > 0 And biomes[randX-1][randY] = HEAVY_GRASS)
                    isReachable = true
                End If
                If (randX < mapWidth-2 And biomes[randX+1][randY] = HEAVY_GRASS)
                    isReachable = true
                End If
                If (randY > 0 And biomes[randX][randY-1] = HEAVY_GRASS)
                    isReachable = true
                End If
                If (randY > mapHeight-2 And biomes[randX][randY+1] = HEAVY_GRASS)
                    isReachable = true
                End If
            End If
          
            If isReachable
                Print "Cave entrance at " + randX*40 + ", " + randY*40
                biomes[randX][randY] = CAVE_ENTRANCE
                caves[cavesMade] = New Level(randX, randY, 150, 100, "Cellular")
                Self.caveX = randX
                Self.caveY = randY
                
                cavesMade += 1
            End If
        End While
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
                Self.playfieldN.CameraX = x * 40
                Self.playfieldN.CameraY = y * 40
                
                Self.startX = x
                Self.startY = y
                'Print "Starting at: " + x + ", " + y
                startPointSet = true
            End If
        End While
        Self.sprite1.Position(x*40, y*40)
        Self.playfieldN.CameraX = sprite1.PositionX - 300
        Self.playfieldN.CameraY = sprite1.PositionY - 235
        Self.bBox = New BoundingRect(x*40-5, y*40 + 20, 30, 30)
    End Method
 '
 'Overloads OnStop Method from iEngine
 '   
    Method OnStop ()
        'Print "Stopping Noise Test"
    End Method
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
		
'   		If KeyDown(KEY_LEFT)
'               Self.playfieldN.CameraX=Self.playfieldN.CameraX-10
'           End If
'       
'           If KeyDown(KEY_RIGHT)
'               Self.playfieldN.CameraX=Self.playfieldN.CameraX+10
'           End If
'       
'           If KeyDown(KEY_UP)
'               Self.playfieldN.CameraY=Self.playfieldN.CameraY-10
'           End If
'       
'           If KeyDown(KEY_DOWN)
'               Self.playfieldN.CameraY=Self.playfieldN.CameraY+10
'           End If
        If KeyDown(KEY_LEFT)
            If isWalkable(biomes[(bBox.x-5)/40][bBox.y/40]) And isWalkable(biomes[(bBox.x-5)/40][(bBox.y+bBox.height)/40])
                If sprite1.PositionX - Self.playfieldN.CameraX < 200
                    Self.playfieldN.CameraX=Self.playfieldN.CameraX-5
                End If
                
                sprite1.PositionX = sprite1.PositionX-5
                Self.bBox.x -= 5
            End If
        End If
    
        If KeyDown(KEY_RIGHT)
            If isWalkable(biomes[(bBox.x+bBox.width+5)/40][bBox.y/40]) And isWalkable(biomes[(bBox.x+bBox.width+5)/40][(bBox.y+bBox.height)/40])
                If sprite1.PositionX - Self.playfieldN.CameraX > 350
                    Self.playfieldN.CameraX=Self.playfieldN.CameraX+5
                End If
                sprite1.PositionX = sprite1.PositionX+5
                Self.bBox.x += 5
            End If
        End If
    
        If KeyDown(KEY_UP)
            If isWalkable(biomes[bBox.x/40][(bBox.y-5)/40]) And isWalkable(biomes[(bBox.x+bBox.width)/40][(bBox.y-5)/40])
                If sprite1.PositionY - Self.playfieldN.CameraY < 200
                    Self.playfieldN.CameraY=Self.playfieldN.CameraY-5
                End If
                sprite1.PositionY = sprite1.PositionY-5
                Self.bBox.y -= 5
            End If
        End If
    
        If KeyDown(KEY_DOWN)
            If isWalkable(biomes[bBox.x/40][(bBox.y+bBox.height+5)/40]) And isWalkable(biomes[(bBox.x+bBox.width)/40][(bBox.y+bBox.height+5)/40])
                If sprite1.PositionY - Self.playfieldN.CameraY > 250
                    Self.playfieldN.CameraY=Self.playfieldN.CameraY+5
                End If
                
                sprite1.PositionY = sprite1.PositionY+5
                Self.bBox.y += 5
            End If
        End If
        checkCameraBounds()
	End Method
'
'Method for checking the boundaries of the game camera. Ensures camera
'stops at the edges of the map
'
	Method checkCameraBounds()
        If Self.playfieldN.CameraX < 0
            Self.playfieldN.CameraX = 0
        End If
        If Self.playfieldN.CameraY < 0
            Self.playfieldN.CameraY = 0
        End If
        
        If Self.playfieldN.CameraX + (Self.playfieldN.Width) > Self.playfieldN.Width * 40
            Self.playfieldN.CameraX = Self.playfieldN.Width *39' - (Self.playfieldN.Width/40)
        End If
        If Self.playfieldN.CameraY + (Self.playfieldN.Height) > Self.playfieldN.Height * 40
            Self.playfieldN.CameraY = Self.playfieldN.Height *39' - (Self.playfieldN.Height/40)
        End If
	End Method
		
'
'Method to draw the textures drawn from simplex noise map
'
    Method drawNoiseMap(w:Int, h:Int)
'           Local xOffset:Int = Self.playfieldN.CameraX 
'           Local yOffset:Int = Self.playfieldN.CameraY
'           Local xTarget:Int = xOffset + 15
'           Local yTarget:Int = yOffset + 12
'   '       Local localTexture:Int = 0
'   '       Local counter:Int = 0
'   
'           For Local i:Int = xOffset Until xTarget
'               For Local j:Int = yOffset Until yTarget
'                   DrawImage(textures, (i-xOffset)*40+xOffset,(j-yOffset)*40+yOffset, biomes[i][j])
'               End For
'           End For
      
        Local xTile:Int = Self.playfieldN.CameraX / 40 - 1
        Local yTile:Int = Self.playfieldN.CameraY / 40 - 1
        Local xOffset:Int = Self.playfieldN.CameraX Mod 40
        Local yOffset:Int = Self.playfieldN.CameraY Mod 40
        
        For Local i:Int = xTile Until xTile + 18
            For Local j:Int = yTile Until yTile + 14
                If i > -1 And i < biomes.Length And j > -1 And j < biomes[i].Length
                    DrawImage(textures, i * 40 , j * 40, biomes[i][j])
                End If
            End For
        End For
        
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
        Local xTile:Int = Self.playfieldN.CameraX / 40 - 1
        Local yTile:Int = Self.playfieldN.CameraY / 40 - 1
        Local xOffset:Int = Self.playfieldN.CameraX Mod 40
        Local yOffset:Int = Self.playfieldN.CameraY Mod 40

        For Local i:Int = xTile Until xTile + 18
            For Local j:Int = yTile Until yTile + 14
                If Not (enemyPlacement[i][j] = -1)
                    DrawImage(enemies, i * 40, j * 40, enemyPlacement[i][j])
                End If
            End For
        End For
	End Method
		
'
'Method to read noise map and set the terrain type based on elevation and moisture
'
	Method determineBiomes(elevation:Float[][], moist:Float[][], width:Int, height:Int)
        Local localBiome:Int = 0
      
        For Local i:Int = 0 Until width
            For Local j:Int = 0 Until height
                If noiseMap[i][j] < -0.35
                    localBiome = DEEP_WATER
                Else If noiseMap[i][j] < -0.3
                    localBiome = SHALLOW_WATER
                Else If noiseMap[i][j] < -0.2
                    localBiome = BEACH
                    
                'Grassy Biomes Below
                
                Else If noiseMap[i][j] < 0.2
                    If moisture[i][j] < -0.6
                        localBiome = DESERT
                    Else If moisture[i][j] < -0.1
                        localBiome = LIGHT_GRASS
                    Else If moisture[i][j] < 0.2
                        localBiome = HEAVY_GRASS
                    Else If moisture[i][j] < 0.4
                        localBiome = FOREST
                    Else If moisture[i][j] < 1
                        localBiome = SWAMP
                    End If
                    
                'Mountain Biomes Below
                
                Else
                    If moisture[i][j] < -0.4
                        localBiome = MOUNTAIN
                    Else If moisture[i][j] < 0
                        localBiome = MOUNTAIN_MOSSY
                    Else
                        localBiome = MOUNTAIN_SNOWY
                    End If
                End If
                If i = 0 Or i = width-1 Or j = 0 Or j = height-1
                    localBiome = MAP_EDGE
                End If
                biomes[i][j] = localBiome
            
            End For
        End For
	End Method
'
'Process biomes into arrays
'
    Method processBiomes()
        'Print "Begin processing"
        Local localBiome:Int = 0
        
        Local deepWaterIndex:Int = 0
        Local shallowWaterIndex:Int = 0
        Local beachIndex:Int = 0
        Local lightGrassIndex:Int = 0
        Local heavyGrassIndex:Int = 0
        Local swampIndex:Int = 0
        Local forestIndex:Int = 0
        Local desertIndex:Int = 0
        Local mountainIndex:Int = 0
        
        For Local i:Int = 0 Until Self.mapWidth
            For Local j:Int = 0 Until Self.mapHeight

                localBiome = Self.biomes[i][j]
                If localBiome = DEEP_WATER
                    deepWaterTiles[deepWaterIndex] = New Point(i,j)
                    deepWaterIndex += 1
                    If deepWaterIndex = deepWaterTiles.Length
                        deepWaterTiles = deepWaterTiles.Resize(deepWaterTiles.Length+1000)
                    End If
                Else If localBiome = SHALLOW_WATER
                    shallowWaterTiles[shallowWaterIndex] = New Point(i,j)
                    shallowWaterIndex += 1
                    If shallowWaterIndex = shallowWaterTiles.Length
                        shallowWaterTiles = shallowWaterTiles.Resize(shallowWaterTiles.Length+1000)
                    End If
                Else If localBiome = BEACH
                    beachTiles[beachIndex] = New Point(i,j)
                    beachIndex += 1
                    If beachIndex = beachTiles.Length
                        beachTiles = beachTiles.Resize(beachTiles.Length+1000)
                    End If
                Else If localBiome = LIGHT_GRASS
                    lightGrassTiles[lightGrassIndex] = New Point(i,j)
                    lightGrassIndex += 1
                    If lightGrassIndex = lightGrassTiles.Length
                        lightGrassTiles = lightGrassTiles.Resize(lightGrassTiles.Length+1000)
                    End If
                Else If localBiome = HEAVY_GRASS
                    heavyGrassTiles[heavyGrassIndex] = New Point(i,j)
                    heavyGrassIndex += 1
                    If heavyGrassIndex = heavyGrassTiles.Length
                        heavyGrassTiles = heavyGrassTiles.Resize(heavyGrassTiles.Length+1000)
                    End If
                Else If localBiome = SWAMP
                    swampTiles[swampIndex] = New Point(i,j)
                    swampIndex += 1
                    If swampIndex = swampTiles.Length
                        swampTiles = swampTiles.Resize(swampTiles.Length+1000)
                    End If
                Else If localBiome = FOREST
                    forestTiles[forestIndex] = New Point(i,j)
                    forestIndex += 1
                    If forestIndex = forestTiles.Length
                        forestTiles = forestTiles.Resize(forestTiles.Length+1000)
                    End If
                Else If localBiome = DESERT
                    desertTiles[desertIndex] = New Point(i,j)
                    desertIndex += 1
                    If desertIndex = desertTiles.Length
                        desertTiles = desertTiles.Resize(desertTiles.Length+1000)
                    End If
                Else If localBiome = MOUNTAIN
                    mountainTiles[mountainIndex] = New Point(i,j)
                    mountainIndex += 1
                    If mountainIndex = mountainTiles.Length
                        mountainTiles = mountainTiles.Resize(mountainTiles.Length+1000)
                    End If
                End If
            End For
        End For
        deepWaterTiles = deepWaterTiles.Resize(deepWaterIndex)
        shallowWaterTiles = shallowWaterTiles.Resize(shallowWaterIndex)
        beachTiles = beachTiles.Resize(beachIndex)
        lightGrassTiles = lightGrassTiles.Resize(lightGrassIndex)
        heavyGrassTiles = heavyGrassTiles.Resize(heavyGrassIndex)
        swampTiles = swampTiles.Resize(swampIndex)
        forestTiles = forestTiles.Resize(forestIndex)
        desertTiles = desertTiles.Resize(desertIndex)
        mountainTiles = mountainTiles.Resize(mountainIndex)
        'Print "End processing"
    End Method
'
'Method to determine which kind of enemy to create based on the terrain
'type at its starting location
'
    Method determineEnemyType(enemies:Int[][], habitat:Int[][])
        For Local i:Int = 0 Until enemies.Length
            For Local j:Int = 0 Until enemies[0].Length
                If Not (enemies[i][j] = -1)
                    If habitat[i][j] = DEEP_WATER
                        enemies[i][j] = 0
                    Else If habitat[i][j] = SHALLOW_WATER
                        enemies[i][j] = 1
                    Else If habitat[i][j] = DESERT Or habitat[i][j] = BEACH Or habitat[i][j] = SWAMP
                        enemies[i][j] = 2
                    Else If habitat[i][j] = LIGHT_GRASS Or habitat[i][j] = LIGHT_GRASS_ROCKS Or habitat[i][j] = HEAVY_GRASS
                        enemies[i][j] = 3
                    Else If habitat[i][j] = FOREST
                        enemies[i][j] = 4
                    Else
                        enemies[i][j] = -1
                    End If
                End If
            End For
        End For
	End Method
'
'Method to decide where to place enemies on the game map.
'
	Method placeEnemies(width:Int, height:Int)
        Self.enemyPlacement = setArray(width, height)
        randomlyAssignCells(Self.enemyPlacement, 3)
        determineEnemyType(Self.enemyPlacement, Self.biomes)
    End Method
    
    Method isWalkable:Bool(tile:Int)
        If ((tile = DEEP_WATER) Or (tile > DESERT And tile < CAVE_ENTRANCE) Or (tile > BEACH_LIFE And tile < LIGHT_GRASS_ROCKS))
            
            Return False
        Else
            Return True
        End If
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
			End If
			If i = 0 Or j = 0 Or i = cells.Length - 1 Or j = cells[0].Length - 1 Or cells[i][j] = 9
				cells[i][j] = -1
			End If

		End For
	End For	
End Function

Function setArray:Int[][](i:Int, j:Int)
	Local result:Int[][] = New Int[i][]
		
	For Local index = 0 Until i
		result[index] = New Int[j]
	End For
		
	Return result
End Function