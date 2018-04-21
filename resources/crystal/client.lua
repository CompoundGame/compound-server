print("Script loaded - green circle");

local speed = 10

local myObject = CreateObject()
SetPosition( myObject, 100, 100 )

local x = 100
local y = 100

--[[
W - 22
A - 0
S - 18
D - 3
]]

-- Also generate a circle

for t = 0, 2*3.14, 3.14/4 do
    local newStuff = CreateObject()
    SetPosition ( newStuff, math.cos(t)*150 + 400, math.sin(t)*150+400 )
end

AddEventHandler( "onKeyPress", function( keyCode )

    if keyCode == 22 then -- W
        y = y - speed
    elseif keyCode == 18 then -- S
        y = y + speed
    elseif keyCode == 0 then -- A
        x = x - speed
    elseif keyCode == 3 then -- D
        x = x + speed
    end

    SetPosition( myObject, x, y )
end
)
