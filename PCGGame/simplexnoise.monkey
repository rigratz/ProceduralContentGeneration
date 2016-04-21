Import mojo

' An implementation of Simplex Noise generation for use in procedural
' map generation. Noise algorithm based off of Stefan Gustavson's
' "Simplex Noise Demystified" paper.

Class SimplexNoise
    Field grad3:Int[][]
    Field p:Int[]
    Field perm:Int[]
   
    'Constructor for SimplexNoise object.
    '
    '
   
    Method New ()
        grad3 = [[1,1,0],[-1,1,0],[1,-1,0],[-1,-1,0],[1,0,1],[-1,0,1],
              [1,0,-1],[-1,0,-1],[0,1,1],[0,-1,1],[0,1,-1],[0,-1,-1]]
        p = [151,160,137,91,90,15,
            131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
            190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
            88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
            77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
            102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
            135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
            5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
            223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
            129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
            251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
            49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
            138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180] 
        perm = New Int[512]
        For Local i:Int = 0 Until 512
            perm[i] = p[i & 255]
        End
    End Method
   
    ' Helper Method used to calculate a dot product.
    '
    ' Returns the dot product of first two values in array against the x and y.
   
    Method dot:Float (g:Int[], x:Float, y:Float)
        Return g[0] * x + g[1] * y
    End Method
   
    ' Calculates the SimplexNoise for an x,y coordinate.
    '
    ' Returns the noise at point (x,y)
   
    Method makeNoise:Float(x:Float, y:Float)
        'Noise from Simplex Triangle corners
        Local n0:Float
        Local n1:Float
        Local n2:Float
        'Determine Simplex Cell
        Local f2:Float = 0.5 * (math.Sqrt(3.0) - 1.0)
        Local s:Float = (x + y) * f2
        Local i:Int = math.Floor(x + s)
        Local j:Int = math.Floor(y + s)
    
        Local g2:Float = (3.0 - math.Sqrt(3.0)) / 6.0
        Local t:Float = (i + j) * g2
        Local x0:Float = i - t
        Local y0:Float = j - t
        Local x01:Float = x - x0
        Local y01:Float = y - y0
    
        'For 2D, Simplex is equilateral triangle
        Local i1:Int
        Local j1:Int
        If x01 > y01
            i1 = 1
            j1 = 0
        Else
            i1 = 0
            j1 = 1
        End If
    
        Local x1:Float = x01 - i1 + g2
        Local y1:Float = y01 - j1 + g2
        Local x2:Float = x01 - 1.0 + 2.0 * g2
        Local y2:Float = y01 - 1.0 + 2.0 * g2
    
        'Hashed gradients
        Local ii:Int = i & 255
        Local jj:Int = j & 255
        'Uses perm
        Local gi0:Int = perm[ii + perm[jj]] Mod 12
        Local gi1:Int = perm[ii + i1 + perm[jj + j1]] Mod 12
        Local gi2:Int = perm[ii + 1 + perm[jj + 1]] Mod 12
    
        'Calculate contributions from corners
        Local t0:Float = 0.5 - x01 * x01 - y01 * y01
        If t0 < 0
        n0 = 0.0
        Else
            t0 *= t0
            n0 = t0 * t0 * dot(grad3[gi0], x01, y01)
        End If
    
        Local t1:Float = 0.5 - x1 * x1 - y1 * y1
        If t1 < 0
            n1 = 0.0
        Else
            t1 *= t1
            n1 = t1 * t1 * dot(grad3[gi1], x1, y1)
        End If
        
        Local t2:Float = 0.5 - x2 * x2 - y2 * y2
        If t2 < 0
            n2 = 0.0
        Else
            t2 *= t2
            n2 = t2 * t2 * dot(grad3[gi2], x2, y2)
        End If
    
        'Returns in range -1 to 1
    
        Return 70.0 * (n0 + n1 + n2)
    End Method
   
    ' Generates a 2D array of dimension width*height and fills with calculated
    ' noise values using a single octave.
    '
    ' Returns generated 2D noise array.
   
    Method generateNoiseMap:Float[][] (width:Int, height:Int)
        Local result:Float[][] = setArray(width, height)
        Seed = Millisecs()
        Local offset:Int = Rnd(0, 1000)
        Local frequency:Float = 5.0 / width
        'Local frequency:Float = 5.0 / width
        For Local i:Int = 0 Until width 
            For Local j:Int = 0 Until height
                result[i][j] = makeNoise((i+offset)*frequency, (j+offset)*frequency)
                'result[i][j] = (result[i][j]+1) / 2
                'result[i][j] += makeOctavedNoise(1,2,1,i,j)
                'result[i][j] = makeNoise(i*frequency, j*frequency)
            End
        End
        Print "Returning Moisture Map"
        Return result
    End Method
   
    ' Generates a 2D array of dimension width*height and fills with calculated
    ' noise values using multiple weighted octaves.
    '
    ' Returns generated 2D noise array.
   
    Method generateOctavedNoiseMap:Float[][] (width:Int, height:Int, octaves:Int, roughness:Float, scale:Float)
        Local result:Float[][] = setArray(width, height)
        Seed = Millisecs()
        Local offset:Int = Rnd(0, 1000000)
        'Local frequency:Float = 5.0 / width
        Local layerFrequency:Float = scale
        Local layerWeight:Float = 1
        Local weightSum:Float = 0

        'For Local i:Int = 0 Until octaves
      
        For Local i:Int = 0 Until width 
            For Local j:Int = 0 Until height
                result[i][j] = makeOctavedNoise(5, 0.5, 0.01, i, j, offset)
                'result[i][j] = (result[i][j]+1) / 2
                'result[i][j] += makeOctavedNoise(1,2,1,i,j)
            End
        End
        
        'layerFrequency *= 2;
        'weightSum += layerWeight;
        'layerWeight *= roughness;
        
        'End
        Print "Islandizing noise..."
        result = makeIsland(result, width, height)
        Print "Returning Noise Map"
      
      
        Return result
    End Method
   
    'Uses a gradient to make the map more of an island(s)
    '
    'Returns a 2D array of the islandized noise map
    Method makeIsland:Float[][] (noiseMap:Float[][], width:Int, height:Int)
        Local island:Float[][] = setArray(width, height)
      
        Local centerX:Int = width / 2
        Local centerY:Int = height / 2
'       Local centerX:Int = Rnd(0, width)
'       Local centerY:Int = Rnd(0, height)
        Print "Island CenterX = " + centerX
        Print "Island CenterY = " + centerY
        Local xDist:Float = 0
        Local yDist:Float = 0
        Local totalDist:Float = 0
      
        For Local i:Int = 0 Until width
            For Local j:Int = 0 Until height
                xDist = (centerX - i) * (centerX - i)
                yDist = (centerY - j) * (centerY - j)
            
                totalDist = math.Sqrt(xDist + yDist) / width
                noiseMap[i][j] -= totalDist
            End
        End
        Return noiseMap
    End Method
 
 
    'Uses exponential weighed averages to make a noise map generated using /octaves/ octaves
    '
    'Returns a single noise value
    Method makeOctavedNoise:Float(octaves:Int, roughness:Float, scale:Float, x:Int, y:Int, offset:Int)
        Local noiseSum:Float = 0
        Local layerFrequency:Float = scale
        Local layerWeight:Float = 1
        Local weightSum:Float = 0
      
        'Seed = Millisecs()
        'Local offset:Int = Rnd(0, 1000000)
        For Local i:Int = 0 Until octaves
            noiseSum += makeNoise((x+offset) * layerFrequency, (y+offset) * layerFrequency) * layerWeight
            layerFrequency *= 2
            weightSum += layerWeight
            layerWeight *= roughness
        End
        Return noiseSum / weightSum
    End Method
   
    'Helper method for allocating a 2D float array of size i*j
    '
    'Returns an empty 2D array of size i*j
    Method setArray:Float[][](i:Int, j:Int)
        Local result:Float[][] = New Float[i][]
		
		For Local index = 0 Until i
			result[index] = New Float[j]
		End
		
		Return result
    End Method
End Class
