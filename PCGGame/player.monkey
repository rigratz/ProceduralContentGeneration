Import mojo
Import animation

'
'Representation of player character within the game
'
Class Player
    Field x:Int
	Field y:Int
	Field xVel:Int
	Field yVel:Int
	Field image:Image
	Field frameNum = 0
	Field downAnim:Animation
	Field currentAnimation:Animation
	
	'Constructor of Player
	'
	'Takes an image and an x/y coordinate pair
	Method New(i:Image, x:Int, y:Int)
        Self.image = i
		Self.x = x
		Self.y = y
		Self.xVel = 0
		Self.yVel = 0
		Self.downAnim = New Animation(i, 4, 69, 102, 200)
		Self.currentAnimation = Self.downAnim
	End Method
	
	
	'Method to update player's state
	'
	'This includes animation frames, position changes, or state changes
	Method Update()
		HandleControls()
		frameNum = currentAnimation.getFrame()
	End Method
	
	
	'Method to draw Player on screen
	'
	'
	Method Draw()
		DrawImage Self.image, x, y, frameNum
	End Method
	
	
	'Method to translate user's keyboard inputs to player controls.
	'
	'
	Method HandleControls()
		If KeyDown(KEY_LEFT)
			x = x - 10
		End If
		If KeyDown(KEY_RIGHT)
			x = x + 10
		End If
		If KeyDown(KEY_DOWN)
			y = y + 10
		End If
		If KeyDown(KEY_UP)
			y = y - 10
		End If
	End Method
End Class
