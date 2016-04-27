Import mojo
Import playniax.ignitionx.engine
Import player
Import animation
Import camera
Import level
Import simplexnoise
Import noisetestscene
Import point



		'Scenes
Global menu:MenuScene
Global gameplay:GameplayScene
Global noiseTest:NoiseTestScene

'
'Main Class that initializes the game
'
Class Game Extends iApp
    '
	'Overloads OnCreate Method from iApp
	'
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

'
'Scene that currently generates cave and puts a player sprite in it.
'
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
	
	'Overloads OnCreate Method from iEngine
	Method OnCreate ()
        Print "Creating Gameplay"
        Self.playfield=New iPlayfield
        Self.playfield.AttachLast()
        Self.playfield.AutoCls(0,0,0)
        Self.playfield.Width=600
        Self.playfield.Height=440
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
		
        
    End Method
    
    
    'Overloads OnRender Method from iEngine
    Method OnRender ()
        room.Draw(Self.playfield.CameraX, Self.playfield.CameraY)
    End Method

    
    'Overloads OnStart Method from iEngine
    Method OnStart ()
        Print "Starting Gameplay"
        room = New Level(0, 0, 160, 120, "Cellular")
        #Rem
        PlayMusic("tetris.mp3", 1)
        #End
    End Method

    
    'Overloads OnStop Method from iEngine
    Method OnStop ()
        Print "Stopping Gameplay"
    End Method


    Method OnUpdate ()
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
			If KeyDown(KEY_LEFT)
            Self.playfield.CameraX=Self.playfield.CameraX-1
        End If
    
        If KeyDown(KEY_RIGHT)
            Self.playfield.CameraX=Self.playfield.CameraX+1
        End If
    
        If KeyDown(KEY_UP)
            Self.playfield.CameraY=Self.playfield.CameraY-1
        End If
    
        If KeyDown(KEY_DOWN)
            Self.playfield.CameraY=Self.playfield.CameraY+1
        End If
        checkCameraBounds()
		End
'
'Method for checking the boundaries of the game camera. Ensures camera
'stops at the edges of the map
'
		Method checkCameraBounds()
		
        If Self.playfield.CameraX < 0
          Self.playfield.CameraX = 0
        End
        If Self.playfield.CameraY < 0
          Self.playfield.CameraY = 0
        End
'         Print "Playfield CameraX: " + Self.playfield.CameraX
'         Print "Camera End: " + (mapWidth - 30)
'         If Self.playfield.CameraX + (mapWidth/20) > mapWidth
'           Self.playfield.CameraX = mapWidth - (mapWidth/20)
'         End
        If Self.playfield.CameraX + (Self.playfield.Width/40) > Self.playfield.Width
          Self.playfield.CameraX = Self.playfield.Width - (Self.playfield.Width/40)
        End
        If Self.playfield.CameraY + (Self.playfield.Height/40) > Self.playfield.Height
          Self.playfield.CameraY = Self.playfield.Height - (Self.playfield.Height/40)
        End
		End
End Class


'
'Main function to start the game.
'
Function Main()
	Local g:Game = New Game
End