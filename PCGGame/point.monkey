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
    
    '
    'Getter method for Point's X coordinate
    '
    Method getX()
        Return Self.x
    End Method
    
    '
    'Getter method for Point's Y coordinate
    '
    Method getY()
        Return Self.y
    End Method
    
End Class