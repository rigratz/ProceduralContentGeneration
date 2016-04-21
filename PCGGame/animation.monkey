Import mojo

'Custom class for handling sprite animations
'
'
Class Animation
	Field img:Image
	Field frames:Int
	Field frame:Int
	Field width:Int
	Field height:Int
	Field frameTime:Int
	Field elapsed:Int
	Field lastTime:Int
	
	'Constructor for Animation
	'
	'Takes an image, number of frames, width/height of frame, and the time between frames
	Method New(i:Image, f:Int, w:Int, h:Int, ft:Int)
		Self.img = i
		Self.frames = f
		Self.width = w
		Self.height = h
		Self.frameTime = ft
		
		Self.elapsed = 0
		Self.frame = 0
		Self.lastTime = 0
	End Method
	
	'Method for determining the current animation frame of a spritesheet.
	'
	'Returns the current animation frame
	Method getFrame:Int()
		elapsed = elapsed + (Millisecs() - lastTime)
		lastTime = Millisecs()

		Local toReturn:Int = math.Floor(elapsed / frameTime)
		
		If toReturn > frames - 1
			toReturn = 0
			elapsed = 0
		End If
		
		Return toReturn
	End Method
End Class
