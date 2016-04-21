Import mojo

Class Camera
	Field camX:Int
	Field camY:Int
	Method New()
		Self.camX = 0
		Self.camY = 0
	End
	
	Method updateCamera(x:Int, y:Int)
		Self.camX = x
		Self.camY = y
	End
End
	