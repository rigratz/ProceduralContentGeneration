Import mojo
Import playniax.ignitionx.engine
Import player
Import animation
Import camera
Import level
Import simplexnoise
Import noisetestscene



		'Scenes
Global menu:MenuScene
Global gameplay:GameplayScene
Global noiseTest:NoiseTestScene

'
'Main Class that initializes the game
'
Class Game Extends iApp
	'Overloads OnCreate Method from iApp
	Method OnCreate()
		Seed = Millisecs()
		
' 		Local img:Image = LoadImage("char_walk_down.png",69,102,4)
' 		p1 = New Player(img, 100, 100)
' 		music = LoadSound("tetris.mp3")
' 		PlayMusic("tetris.mp3", 1)
		
		'Setup Game Scenes
		menu = New MenuScene()
		gameplay = New GameplayScene()
		noiseTest = New NoiseTestScene()
		
		'Start Game/Set UpdateRate to 60fps
		iStart(menu, 60)
	End Method
End Class

'
'Scene containing opening menu options for game.
'
Class MenuScene Extends iEngine
  'Overloads OnCreate Method from iEngine
	Method OnCreate ()
        Print "Creating Menu"
  End Method
  'Overloads OnRender Method from iEngine
  Method OnRender ()
    DrawText("Press Enter to generate Cellularly", 200, 200)
    DrawText("Press Space to generate with Noise", 200, 300)
  End Method
  'Overloads OnStart Method from iEngine
  Method OnStart ()
    Print "Starting Menu"
  End Method
  'Overloads OnStop Method from iEngine
  Method OnStop ()
    Print "Stopping Menu"
  End Method
  'Overloads OnUpdate Method from iEngine
  Method OnUpdate ()
    
   If KeyHit(KEY_ENTER)
      Print "Switch"
      iStart gameplay
   End If
      
   If KeyHit (KEY_SPACE)
      Print "Switch to Noise"
      iStart noiseTest
   End If
      
  End Method
End Class

' Class NoiseTestScene Extends iEngine
'     Field noiseMap:Float[][]
'     Field mapWidth:Int
'     Field mapHeight:Int
'     Field playfieldN:iPlayfield
'     Field backLayer:iLayer
'     Field playerLayer:iLayer
'     Field player:Hero
'     
'     Method OnCreate ()
'         Print "Creating Noise Test"
'         Self.playfieldN=New iPlayfield
'         Self.playfieldN.AttachLast()
'         Self.playfieldN.AutoCls(0,0,0)
'         Self.playfieldN.Width=600
'         Self.playfieldN.Height=460
'         Self.playfieldN.Position(0,0)
'         Self.playfieldN.ZoomPointX(200)
'         Self.playfieldN.ZoomPointY(128)
'         
'         Self.backLayer = New iLayer()
'         Self.backLayer.AttachLast(playFieldN)
'         
'         Self.playerLayer = New iLayer()
'         Self.player = New Hero()
'         Self.playerLayer.AttachLast(Self.player)
'         
'         
'         
'         mapWidth = 600
'         mapHeight = 460
'         
' '         noiseMap = n.generateNoiseMap(mapWidth, mapHeight, 8, 0.4, 0.005)
'         
'     End
' 
'     Method OnRender ()
'     	drawNoiseMap(mapWidth, mapHeight)
'     	  'Debugging Text
' '     	DrawText("CameraX: " + Self.playfieldN.CameraX, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+10)
' '     	DrawText("CameraY: " + Self.playfieldN.CameraY, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+20)
' '     	DrawText("MapX: " + Self.playfieldN.CameraX , Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+40)
' '     	DrawText("MapY: " + Self.playfieldN.CameraY, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+50)
' '       DrawText("MapXEnd: " + (Self.playfieldN.CameraX + 120), Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+70)
' '     	DrawText("MapYEnd: " + (Self.playfieldN.CameraY + 89), Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+80)
'     End
' 
'     Method OnStart ()
'         Print "Starting Noise Test"
'         Local n:SimplexNoise = New SimplexNoise()
'         noiseMap = n.generateOctavedNoiseMap(mapWidth, mapHeight, 6, 0.5, 1)
'         Local x:Int = noiseMap.Length
'         Local y:Int = noiseMap[0].Length
'         Print "X: " + x
'         Print "Y: " + y
'         Print "Top Left Corner: " + noiseMap[0][0]
'         Print "Top Right Corner: " + noiseMap[x-1][0]
'         Print "Bottom Left Corner: " + noiseMap[0][y-1]
'         Print "Bottom Right Corner: " + noiseMap[x-1][y-1]
'         'noiseMap = n.generateNoiseMap(mapWidth, mapHeight)
'     End
' 
'     Method OnStop ()
'         Print "Stopping Noise Test"
'     End
' 
'     Method OnUpdate ()
'     	If KeyHit(KEY_ENTER)
'     		Print "Switch"
'         iStart gameplay
' 			End
' 			If KeyDown(KEY_CONTROL)
'         
'             If KeyDown(KEY_A)
'                 Self.playfieldN.AlphaFade=Self.playfieldN.AlphaFade-0.01
'                 If Self.playfieldN.AlphaFade < 0 Self.playfieldN.AlphaFade=Self.playfieldN.AlphaFade+1
'             End If
'             
'             If KeyDown(KEY_Z)
'                 Self.playfieldN.ZoomX=Self.playfieldN.ZoomX-0.01
'                 Self.playfieldN.ZoomY=Self.playfieldN.ZoomY-0.01
'             End If
'         
'         Else
'         
'             If KeyDown(KEY_A)
'                 Self.playfieldN.AlphaFade=Self.playfieldN.AlphaFade+0.01
'                 If Self.playfieldN.AlphaFade > 1 Self.playfieldN.AlphaFade=Self.playfieldN.AlphaFade-1
'             End If
'             
'             If KeyDown(KEY_Z)
'                 Self.playfieldN.ZoomX=Self.playfieldN.ZoomX+0.01
'                 Self.playfieldN.ZoomY=Self.playfieldN.ZoomY+0.01
'             End If
'         
'         End If
' 			If KeyDown(KEY_LEFT)
'             Self.playfieldN.CameraX=Self.playfieldN.CameraX-1
'         End If
'     
'         If KeyDown(KEY_RIGHT)
'             Self.playfieldN.CameraX=Self.playfieldN.CameraX+1
'         End If
'     
'         If KeyDown(KEY_UP)
'             Self.playfieldN.CameraY=Self.playfieldN.CameraY-1
'         End If
'     
'         If KeyDown(KEY_DOWN)
'             Self.playfieldN.CameraY=Self.playfieldN.CameraY+1
'         End If
'         checkCameraBounds()
' 		End
' 		Method checkCameraBounds()
' 		
'         If Self.playfieldN.CameraX < 0
'           Self.playfieldN.CameraX = 0
'         End
'         If Self.playfieldN.CameraY < 0
'           Self.playfieldN.CameraY = 0
'         End
' '         Print "Playfield CameraX: " + Self.playfieldN.CameraX
' '         Print "Camera End: " + (mapWidth - 30)
' '         If Self.playfieldN.CameraX + (mapWidth/20) > mapWidth
' '           Self.playfieldN.CameraX = mapWidth - (mapWidth/20)
' '         End
'         If Self.playfieldN.CameraX + (Self.playfieldN.Width/20) > Self.playfieldN.Width
'           Self.playfieldN.CameraX = Self.playfieldN.Width - (Self.playfieldN.Width/20)
'         End
'         If Self.playfieldN.CameraY + (Self.playfieldN.Height/20) > Self.playfieldN.Height
'           Self.playfieldN.CameraY = Self.playfieldN.Height - (Self.playfieldN.Height/20)
'         End
' 		End
' 		
' 		
' 		Method drawNoiseMap(w:Int, h:Int)
'       Local xOffset:Int = Self.playfieldN.CameraX
'       Local yOffset:Int = Self.playfieldN.CameraY
'       Local xTarget:Int = xOffset + 30
'       Local yTarget:Int = yOffset + 23
'       Local localTexture:Int = 0
'       Local counter:Int = 0
'       
'       'Print "xoffset: " + xOffset
'       For Local i:Int = xOffset Until xTarget
'         For Local j:Int = yOffset Until yTarget
'           If noiseMap [i][j] < -0.35
'             'SetColor(0,0,102)
'             localTexture = 0
'            Else If noiseMap [i][j] < -0.3
'             'SetColor(0,0,255)
'             localTexture = 1
'            Else If noiseMap [i][j] < -0.2
'             'SetColor(255,255,0)
'             localTexture = 2
'            Else If noiseMap [i][j] < -0.1
'             'SetColor(51,255,51)
'             localTexture = 3
'            Else If noiseMap [i][j] < 0.3
'             'SetColor(0,153,0)
'             localTexture = 4
'            Else If noiseMap [i][j] < 0.5
'             'SetColor(204,102,0)
'             localTexture = 5
'            Else If noiseMap [i][j] < 0.6
'             'SetColor(224,224,224)
'             localTexture = 6
'            Else If noiseMap [i][j] < 1
'             'SetColor(224,224,224)
'             localTexture = 7
' '            Else If noiseMap [i][j] < 0.9
' '             SetColor(255,255,255)
'           End
'           'DrawRect((i-xOffset)*10,(j-yOffset)*10,10,10)
'           If i = 0 Or i = w-1 Or j = 0 Or j = h-1
'             localTexture = 8
'           End
'           DrawImage(textures, (i-xOffset)*20+xOffset,(j-yOffset)*20+yOffset, localTexture)
'           
' '           If Self.playfieldN.CameraX = 570
' '             counter += 1
' '           End
'         End
'         'Print "i - xOffset = " + (i-xOffset)
'       End
'       
'       'Print "Counter: " + counter
' '       DrawText("xOffset: " + xOffset, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+100)
' '     	DrawText("yOffset: " + yOffset, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+110)
' '     	DrawText("xTarget: " + xTarget, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+120)
' '     	DrawText("yTarget: " + yTarget, Self.playfieldN.CameraX+10, Self.playfieldN.CameraY+130)
' 		End
'     
' End
Class GameplayScene Extends iEngine
	Field p1:Player
	Field music:Sound
	Field cam:Camera
	Field room:Level
	Field playfield:iPlayfield
	Field playfield2:iPlayfield
	Field layer:iLayer
	Field sprite1:iLayerSprite
	Field sprite2:iLayerSprite
	
	Method OnCreate ()
        Print "Creating Gameplay"
        Self.playfield=New iPlayfield
        Self.playfield.AttachLast()
        Self.playfield.AutoCls(0,0,0)
        Self.playfield.Width=600
        Self.playfield.Height=445
        Self.playfield.Position(16,16)
        Self.playfield.ZoomPointX(200)
        Self.playfield.ZoomPointY(128)
        #Rem
        Self.playfield2=New iPlayfield
        Self.playfield2.AttachLast()
        Self.playfield2.AutoCls(48,48,80)
        Self.playfield2.width=400
        Self.playfield2.height=256
        Self.playfield2.Position(160,160)
        Self.playfield2.ZoomPointX(200)
        Self.playfield2.ZoomPointY(128)
        Self.playfield2.ZoomX(0.25)
        Self.playfield2.ZoomY(0.25)
        #End
    
        Self.layer=New iLayer
    
        ' The layer needs to be attached to both playfields:
    
    	Local img:Image = iLoadSprite("char_walk_down.png",69,102,4)
        Self.layer.AttachLast(Self.playfield)
        #Rem
        Self.layer.AttachLast(Self.playfield2)
    	#End
        Self.sprite1=New iLayerSprite
        Self.sprite1.AttachLast(Self.layer)
        Self.sprite1.ImagePointer(img)
        Self.sprite1.Position(300,275)
    	#Rem
        Self.sprite2=New iLayerSprite
        Self.sprite2.AttachLast(Self.layer)
        Self.sprite2.ImagePointer(gfxMonkey)
        Self.sprite2.Position(0,0)
        #End
        
		Self.p1 = New Player(img, 100, 100)
		
		music = LoadSound("tetris.mp3")
		
        
    End

    Method OnRender ()
    	
    	room.Draw()
		
    	#Rem
    	p1.Draw()
    	#End
    	#Rem
    	SetColor (0,80,0)
        DrawOval (200-64,128-64,128,128)
        #End
        
    End

    Method OnStart ()
        Print "Starting Gameplay"
        room = New Level(0, 0, 150, 100, "Drunk")
        #Rem
        PlayMusic("tetris.mp3", 1)
        #End
    End

    Method OnStop ()
        Print "Stopping Gameplay"
    End

    Method OnUpdate ()
    	#Rem
    	If KeyDown(KEY_CONTROL)
        
            If KeyDown(KEY_A)
                Self.playfield.AlphaFade=Self.playfield.AlphaFade-0.01
                If Self.playfield.AlphaFade < 0 Self.playfield.AlphaFade=Self.playfield.AlphaFade+1
            End If
            
            If KeyDown(KEY_Z)
                Self.playfield.ZoomX=Self.playfield.ZoomX-0.01
                Self.playfield.ZoomY=Self.playfield.ZoomY-0.01
            End If
        
        Else
        
            If KeyDown(KEY_A)
                Self.playfield.AlphaFade=Self.playfield.AlphaFade+0.01
                If Self.playfield.AlphaFade > 1 Self.playfield.AlphaFade=Self.playfield.AlphaFade-1
            End If
            
            If KeyDown(KEY_Z)
                Self.playfield.ZoomX=Self.playfield.ZoomX+0.01
                Self.playfield.ZoomY=Self.playfield.ZoomY+0.01
            End If
        
        End If
        #End
    
        If KeyDown(KEY_LEFT)
            Self.playfield.CameraX=Self.playfield.CameraX-4
        End If
    
        If KeyDown(KEY_RIGHT)
            Self.playfield.CameraX=Self.playfield.CameraX+4
        End If
    
        If KeyDown(KEY_UP)
            Self.playfield.CameraY=Self.playfield.CameraY-4
        End If
    
        If KeyDown(KEY_DOWN)
            Self.playfield.CameraY=Self.playfield.CameraY+4
        End If
    	#Rem
        Self.sprite2.rotation=Self.sprite2.rotation+1
        #End
        #Rem
        If sprite1.Frame = 3
        	sprite1.Frame = 0
        Else
        	Self.sprite1.NextFrame()
        End
        #End
        sprite1.AnimationLoop(1, 60)
        Self.sprite1.Show("LOOPING ANIMATION:")
        Self.sprite1.Show("FramePointer="+Self.sprite1.Frame)
        
		p1.Update()
    End
End

Function Main()
	Local g:Game = New Game
End