Import mojo
Import animation

Class Player
	Field x:Int
	Field y:Int
	Field xVel:Int
	Field yVel:Int
	Field image:Image
	Field frameNum = 0
	Field downAnim:Animation
	Field currentAnimation:Animation
	
	Method New(i:Image, x:Int, y:Int)
		Self.image = i
		Self.x = x
		Self.y = y
		Self.xVel = 0
		Self.yVel = 0
		Self.downAnim = New Animation(i, 4, 69, 102, 200)
		Self.currentAnimation = Self.downAnim
	End
	
	Method Update()
		HandleControls()
		frameNum = currentAnimation.getFrame()
	End
	
	Method Draw()
		DrawImage Self.image, x, y, frameNum
	End
	
	Method HandleControls()
		If KeyDown(KEY_LEFT)
			x = x - 4
		End
		If KeyDown(KEY_RIGHT)
			x = x + 4
		End
		If KeyDown(KEY_DOWN)
			y = y + 4
		End
		If KeyDown(KEY_UP)
			y = y - 4
		End
	End
End
