print("[Lua] Serverside Script Loaded")

local connectedPlayers = 0

AddEventHandler("onPlayerConnect",function(id)
    print ( "[Lua] Player with id " .. tostring(math.floor(id)) .. " connected! " .. tostring( connectedPlayers+1) .. " total players" )
    connectedPlayers = connectedPlayers + 1
end)

AddEventHandler("onPlayerDrop",function(id)
    print ( "[Lua] Player with id " .. tostring(math.floor(id)) .. " dropped! " .. tostring(connectedPlayers-1) .. " players remaining" )
    connectedPlayers = connectedPlayers - 1
end)
