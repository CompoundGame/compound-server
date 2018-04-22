print("Script loaded - Virus Invaders")

local connectedPlayers = 0
clockset = false

AddEventHandler("onPlayerConnect",function(id)
    print ( "[Lua] Player with id " .. tostring(math.floor(id)) .. " connected!" )
    connectedPlayers = connectedPlayers + 1
end)

AddEventHandler("onPlayerDrop",function(id)
    print ( "[Lua] Player with id " .. tostring(math.floor(id)) .. " dropped!" )
    connectedPlayers = connectedPlayers - 1
    if ( connectedPlayer == 0 ) then
        clockset = false
    end
end)

RegisterServerEvent("invaders:askForClock")
AddEventHandler("invaders:askForClock",function()
    if clockset == false then
        clockset = true
        print("ASKED FOR CLOCK")
        TriggerClientEvent("invaders:setClock", -1 )
    end
end)

local killed = {}

RegisterServerEvent("invaders:commandKill")
AddEventHandler("invaders:commandKill",function( critterID )
    if killed[critterID] == nil then
        TriggerClientEvent("invaders:enqueue", -1, critterID )
        killed[critterID] = true
    end
end)

local iterator = 1

local width = 1200
local height = 800

local codeDictionary = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "F" }

function generateCode()
    local code = ""
    for i = 1, 6 do
        code = code .. codeDictionary[ math.random(1,15) ]
    end
    return code
end

extraTimer = false -- guards some network bugs

RegisterServerEvent("invaders:spawnTimer")
AddEventHandler("invaders:spawnTimer",function()
    if ( connectedPlayers >= 2 ) then
        if extraTimer then
            local tex = math.random(1,15)
            if ( tex == 12 ) then tex = tex + 1 end
            TriggerClientEvent("invaders:spawnCritter", -1, iterator, tostring(tex) .. ".png", generateCode(), math.random(300,width-300) )
            iterator = iterator+1
        else
            extraTimer = true
        end
    else
        extraTimer = false
    end
end)


--[[
RegisterServerEvent("invaders:virus")

AddEventHandler("invaders:virus", function(b1,b2,b3,b4,b5,b6,b7,b8)
    print ( "Received arguments of invaders:virus = \n" .. tostring(b1) .. ' ' ..  tostring(b2) .. ' ' ..  tostring(b3) .. ' ' ..  tostring(b4) .. ' ' .. tostring(b5) .. ' ' ..  tostring(b6) .. ' ' ..  tostring(b7) .. ' ' ..  tostring(b8) )
    TriggerClientEvent("invaders:clientTest",-1,"Boop","Mipa",100,230)
end)
]]
