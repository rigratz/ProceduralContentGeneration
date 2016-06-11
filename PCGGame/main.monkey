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
Import gameplayscene


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
		'Start Game/Set UpdateRate to 60fps
		menu = New MenuScene()
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
        DrawText("Press Enter to generate a Cave", 200, 200)
        DrawText("Press Space to generate Overworld Map", 200, 300)
        DrawText("Press Z to demo battles", 200, 400)
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
            gameplay = New GameplayScene()
            iStart gameplay
        End If
      
        If KeyHit (KEY_SPACE)
            Print "Switch to Noise"
            noiseTest = New NoiseTestScene()
            iStart noiseTest
        End If
        
        If KeyHit (KEY_Z)
            Print "Switch to Battle"
            iStart New BattleScene()
        End If
      
    End Method
    
    
End Class


'
'Main function to start the game.
'
Function Main()
	Local g:Game = New Game
End