'battleScene.monkey

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

Class BattleScene Extends iEngine
    Field enemyTimer:Int
    Field enemyTimer2:Int
    
    Field playerTimer:Int
    Field turn:String
    
    Field playerTurns:Int
    
    Field bg:Image
    'Overloads OnCreate Method from iEngine
    Method OnCreate ()
        Print "Creating Battle"
        enemyTimer = 0;
        enemyTimer2 = 0
        playerTimer = 0
        turn = "Timer"
        playerTurns = 0
        
        bg = iLoadSprite("cave_battle_bg.png",800,477,1)
    End Method
    
    
    'Overloads OnRender Method from iEngine
    Method OnRender ()
        DrawImage(bg, 400 , 240, 0)
    End Method
    
    
    'Overloads OnStart Method from iEngine
    Method OnStart ()
        Print "Starting Battle"
    End Method
    
    
    'Overloads OnStop Method from iEngine
    Method OnStop ()
        Print "Stopping Battle"
    End Method
    
    
    'Overloads OnUpdate Method from iEngine
    Method OnUpdate ()
        If playerTurns < 11
            If playerTimer > 100
                turn = "Player"
            Else If enemyTimer > 100
                turn = "Enemy1"
            Else If enemyTimer2 > 100
                turn = "Enemy2"
            Else 
                turn = "Timer"
            End If
            
            If turn = "Player"
                
    '               Local selection:Bool = false
    '               While Not (selection)
                    If KeyHit(KEY_ENTER)
                        playerTimer = 0
                        playerTurns += 1
                        If playerTurns > 10
                            Print "Battle Over. Testing successful!"
                        End If
                    End If
    '               End While
                
            Else If turn = "Enemy1"
                Print "Enemy1 took turn"
                enemyTimer = 0
            Else If turn = "Enemy2"
                Print "Enemy2 took turn"
                enemyTimer2 = 0
            Else If turn = "Timer"
            Print "Timer turn"
                playerTimer += 5
                enemyTimer += 1
                enemyTimer2 += 2
                If playerTimer > 100
                    Print "Player turn! Press enter to select"
                End If
            End If
                
            If KeyHit(KEY_ENTER)
                
            End If
          
            If KeyHit (KEY_SPACE)
                
            End If
        End If
    End Method
    
    
End Class