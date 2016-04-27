'
'Helper class designed to keep track of point positions
'of various objects for use in preprocessing.
'

Class Point
    Field x:Int
    Field y:Int
    
    Method New(x:Int, y:Int)
        Self.x = x
        Self.y = y
    End Method
    
    Method getX()
        Return Self.x
    End Method
    
    Method getY()
        Return Self.y
    End Method
    
End Class