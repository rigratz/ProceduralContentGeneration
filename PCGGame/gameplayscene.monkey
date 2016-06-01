'gameplayscene.monkey
Import mojo
Import playniax.ignitionx.engine
Import player
Import animation
Import camera
Import level
Import simplexnoise
Import noisetestscene
Import point
Import pathfinder
Import math
Import battleScene

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
	
	Field bBox:BoundingRect
	
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
        'DrawRect(Self.bBox.x, Self.bBox.y, Self.bBox.width, Self.bBox.height)
    End Method

    
    'Overloads OnStart Method from iEngine
    Method OnStart ()
        'Print "Starting Gameplay"
        room = New Level(0, 0, 160, 120, "Cellular")
'           Local aStar:AStarSearch = New AStarSearch(room.layout, 20000, false)
'           'Print "Find Path..."
'           Local treasurePath:Path = aStar.findPath(room.entranceX, room.entranceY, room.treasureX, room.treasureY)
'           
'           If Not (treasurePath = Null)
'               room.setTreasure2()
'               Local x1:Int = room.entranceX
'               Local x2:Int = room.treasureX
'               Local y1:Int = room.entranceY 
'               Local y2:Int = room.treasureY
'               
'               Local tx:Int = room.treasure2X
'               Local ty:Int = room.treasure2Y
'               
'               Local distFS:Int = math.Abs(x1-tx) + math.Abs(y1-tx)
'               Local distFT:Int = math.Abs(x2-tx) + math.Abs(y2-tx)
'               Local closestX:Int
'               Local closestY:Int
'               
'               If distFS < distFT
'                   closestX = x1
'                   closestY = y1 
'               Else
'                   closestX = x2
'                   closestY = y2
'               End If
'               Local tp2:Path = aStar.findPath(closestX, closestY, room.treasure2X, room.treasure2Y)
'               'treasurePath.printPath()
'               If Not (tp2 = Null)
'               
'                   room.debugPath(treasurePath)
'                   room.debugPath(tp2)
'               Else
'                   Print "Failed on second treasure"
'               End If
'           Else
'               Print "No path was found, bad room!"
'           End
        
        Self.sprite1.Position(room.entranceX*40, room.entranceY*40 - 80)
        
        Self.bBox = New BoundingRect(room.entranceX*40-5, room.entranceY*40 - 60, 30, 30)
        Self.playfield.CameraX = room.entranceX*40 - 300
        Self.playfield.CameraY = room.entranceY*40 - 200
        #Rem
        PlayMusic("tetris.mp3", 1)
        #End
    End Method

    
    'Overloads OnStop Method from iEngine
    Method OnStop ()
        'Print "Stopping Gameplay"
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
            If Not (room.layout[(bBox.x-5)/40][bBox.y/40] = 0 Or room.layout[(bBox.x-5)/40][(bBox.y+bBox.height)/40] = 0)
                If sprite1.PositionX - Self.playfield.CameraX < 200
                    Self.playfield.CameraX=Self.playfield.CameraX-5
                End If
                
                sprite1.PositionX = sprite1.PositionX-5
                Self.bBox.x -= 5
            End If
        End If
    
        If KeyDown(KEY_RIGHT)
            If Not (room.layout[(bBox.x+bBox.width+5)/40][bBox.y/40] = 0 Or room.layout[(bBox.x+bBox.width+5)/40][(bBox.y+bBox.height)/40] = 0)
                If sprite1.PositionX - Self.playfield.CameraX > 350
                    Self.playfield.CameraX=Self.playfield.CameraX+5
                End If
                sprite1.PositionX = sprite1.PositionX+5
                Self.bBox.x += 5
            End If
        End If
    
        If KeyDown(KEY_UP)
            If Not (room.layout[bBox.x/40][(bBox.y-5)/40] = 0 Or room.layout[(bBox.x+bBox.width)/40][(bBox.y-5)/40] = 0)
                If sprite1.PositionY - Self.playfield.CameraY < 200
                    Self.playfield.CameraY=Self.playfield.CameraY-5
                End If
                sprite1.PositionY = sprite1.PositionY-5
                Self.bBox.y -= 5
            End If
        End If
    
        If KeyDown(KEY_DOWN)
            If Not (room.layout[bBox.x/40][(bBox.y+bBox.height+5)/40] = 0 Or room.layout[(bBox.x+bBox.width)/40][(bBox.y+bBox.height+5)/40] = 0)
                If sprite1.PositionY - Self.playfield.CameraY > 250
                    Self.playfield.CameraY=Self.playfield.CameraY+5
                End If
                
                sprite1.PositionY = sprite1.PositionY+5
                Self.bBox.y += 5
            End If
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
        If Self.playfield.CameraX + (Self.playfield.Width) > 160 * 40
          Self.playfield.CameraX = 160*40 - Self.playfield.Width' - (Self.playfield.Width/40)
        End
        If Self.playfield.CameraY + (Self.playfield.Height) > 120 * 40
          Self.playfield.CameraY = 120*40 - Self.playfield.Height' - (Self.playfield.Height/40)
        End
	End Method
End Class

Class BoundingRect
    Field x:Int
    Field y:Int
    Field width:Int
    Field height:Int
    
    Field top:Int
    Field bottom:Int
    Field left:Int
    Field right:Int
    
    Method New(x:Int, y:Int, w:Int, h:Int)
        Self.x = x
        Self.y = y
        Self.width = w
        Self.height = h
        
        Self.top = y
        Self.bottom = y + h
        Self.left = x
        Self.right = x + w
    End Method
    
End Class
