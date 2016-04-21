Import mojo
#Rem
#End
Class Level
	Field layout:String[][]
	Field width:Int
	Field height:Int
	Field generated:Bool
	
	'Location on World Map
	Field xCoord:Int
	Field yCoord:Int
	#Rem
	#End
	Method New(x:Int, y:Int, w:Int, h:Int, type:String)
		Self.generated = False
		Self.xCoord = x
		Self.yCoord = y
		
		Self.width = w
		Self.height = h
		Self.layout = setArray(Self.width, Self.height)
		
		If type = "Cellular"
			randomlyAssignCells(Self.layout)
			layout = generateCellularly(Self.layout)
			layout = generateCellularly(Self.layout)
			layout = generateCellularly(Self.layout)
			layout = generateCellularly(Self.layout)
			layout = generateCellularly(Self.layout)
		Else If type = "Drunk"
			fillCells(Self.layout)
			drunkWalk(Self.layout)
		End
		Self.generated = true

	End
	
	Method isGenerated:Bool()
		Return generated
	End
	
	#Rem
			setArray
			:Helper method for allocating a 2D array of a given size
	#End
	Method setArray:String[][](i:Int, j:Int)
		Local result:String[][] = New String[i][]
		
		For Local index = 0 Until i
			result[index] = New String[j]
		End
		
		Return result
	End
	#Rem
			randomlyAssignCells
			:Randomly sets values for Cellular automata generation's initial state
	#End
	Method randomlyAssignCells(design:String[][])
		Seed = Millisecs()
		Local rand:Int = 0
		
		For Local i:Int = 0 Until design.Length
			For Local j:Int = 0 Until design[0].Length
				rand = Rnd(0, 100)
				If rand < 45
					design[i][j] = "X"	
				Else
					design[i][j] = "O"
				End
				If i = 0 Or j = 0 Or i = design.Length - 1 Or j = design[0].Length - 1
					design[i][j] = "X"
				End

			End
		End
		
	End
	#Rem
			generateCellularly
			:One iteration of a cellular automata generation algorithm
	#End
	Method generateCellularly:String[][](design:String[][])
		Local adjacentWalls:Int
		Local result:String[][] = setArray(design.Length, design[0].Length)
		
		For Local i:Int = 0 Until design.Length
			For Local j:Int = 0 Until design[0].Length
				adjacentWalls = checkWalls(design, i, j)
				
				If design[i][j] = "X"
					If adjacentWalls > 3
						result[i][j] = "X"
					Else
						result[i][j] = "O"
					End
				Else If design[i][j] = "O"
					If adjacentWalls > 4
						result[i][j] = "X"
					Else
						result[i][j] = "O"
					End
				End
				
				If i = 0 Or j = 0 Or i = design.Length - 1 Or j = design[0].Length - 1
					result[i][j] = "X"
				End
			End
		End
		
		Return result
	End
	
	#Rem
	#End
	Method checkWalls:Int(design:String[][], i:Int, j:Int)
		Local total:Int = 0
		If i > 0 And design[i-1][j] = "X"
			total += 1
		End
		If i < design.Length - 1 And design[i+1][j] = "X"
			total += 1
		End
		If j > 0 And design[i][j-1] = "X"
			total += 1
		End
		If j < design[0].Length - 1 And design[i][j+1] = "X"
			total += 1
		End
		If i > 0 And j < design[0].Length - 1 And design[i-1][j+1] = "X"
			total += 1
		End
		If i > 0 And j > 0 And design[i-1][j-1] = "X"
			total += 1
		End
		If i < design.Length - 1 And j < design[0].Length - 1 And design[i+1][j+1] = "X"
			total += 1
		End
		If i < design.Length - 1 And j > 0 And design[i+1][j-1] = "X"
			total += 1
		End

		Return total 
	End
	#Rem
	#End
	Method printLevel(design:String[][])

		Local line:String
	
		For Local i:Int = 0 Until design.Length

			line = ""
			For Local j:Int = 0 Until design[1].Length
				line += design[i][j]
			End

			Print line
		End
	End
	#Rem
	#End
	Method fillCells(design:String[][])
		For Local i:Int = 0 Until design.Length
			For Local j:Int = 0 Until design[0].Length
				design[i][j] = "X"
			End
		End
	End
	#Rem
	#End
	Method drunkWalk(design:String[][])
		Local target:Float = (design.Length * design[0].Length) * 0.35
		Local cleared:Int = 1
		
		Local tempX:Int
		Local tempY:Int
		Local direction:Int
		
		Local followBias:Int = 0
		tempX = Rnd(1, design.Length - 1)
		tempY = Rnd(1, design[0].Length - 1)
		
		design[tempX][tempY] = "O"
		
		While cleared < target
			If followBias < 40
				direction = Rnd(0, 4)
			End
			If direction = 0 And tempY > 1
				tempY -= 1
			Else If direction = 1 And tempY < design[0].Length - 2
				tempY += 1
			Else If direction = 2 And tempX > 1
				tempX -= 1
			Else If direction = 3 And tempX < design.Length - 2
				tempX += 1
			End
			
			If design[tempX][tempY] = "X"
				design[tempX][tempY] = "O"
				cleared += 1
			End
			followBias = Rnd(0, 100)
		End
	End
	#Rem
	#End
	Method Draw()
		For Local i:Int = 0 Until Self.layout.Length
			For Local j:Int = 0 Until Self.layout[0].Length
				If Self.layout[i][j] = "X"
					SetColor(0,0,0)
				Else
					SetColor(255,255,255)
				End
				DrawRect(i * 10, j * 10, 10, 10)
			End
		End
	End
End