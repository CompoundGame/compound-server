
print("Script loaded - Virus Invaders")

math.randomseed(os.time())

local connectedPlayers = 0
clockset = false

local score = {}
score[1] = 0
score[2] = 0

playerList = {}

function getPlayerSideFromId(id)
    if playerList[1] == id then
        return 1
    elseif playerList[2] == id then
        return 2
    end
end

nicknames = {}
nicknames[1]="P1"
nicknames[2]="P2"

RegisterServerEvent("invaders:nickname")
AddEventHandler("invaders:nickname",function(playerside, nickname)
    nicknames[playerside] = nickname;
end)

function insertInPlayerList( id )
    if playerList[1] == nil then
        playerList[1] = id
    elseif playerList[2] == nil then
        playerList[2] = id
    end
end

function popFromPlayerList( id )
    if playerList[1] == id then
        playerList[1] = nil
    elseif playerList[2] == id then
        playerList[2] = nil
    end
end


AddEventHandler("onPlayerConnect",function(id)
    print ( "[Lua] Player with id " .. tostring(math.floor(id)) .. " connected!" )
    connectedPlayers = connectedPlayers + 1
    insertInPlayerList( id )
    score[1] = 0
    score[2] = 0
end)

AddEventHandler("onPlayerDrop",function(id)
    print ( "[Lua] Player with id " .. tostring(math.floor(id)) .. " dropped!" )
    connectedPlayers = connectedPlayers - 1
    popFromPlayerList(id)
    if ( connectedPlayers == 0 ) then
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
AddEventHandler("invaders:commandKill",function( critterID, playerSide )
    if killed[critterID] == nil then
        TriggerClientEvent("invaders:enqueue", -1, critterID, playerSide )
        killed[critterID] = true
        score[playerSide] = score[playerSide] + 1
        TriggerClientEvent("invaders:score", -1, nicknames[playerSide] .. " " .. tostring(score[playerSide]), playerSide )
    end
end)

local width = 1600
local height = 1000

--local codeDictionary = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "F" }

--dictionary = { "inject", "SQL", "cosine", "sine", "backdoor", "compound", "developer" }

dictionary = {"include", "iostream", "namespace", "unsigned", "int", "DEFINE", "qsort", "FILE", "fopen",
"status", "init", "clone", "submodule", "update", "add", "commit", "push", "reset", "remove", "branch", "checkout", "fetch", "pull", "remote",
"require", "usbdetection", "autolaunch", "electron", "var", "ipcrenderer", "document", "String"}
function generateCode()
    return string.upper ( dictionary[math.random(#dictionary)] )
end

extraTimer = false -- guards some network bugs

xPadding = 100

local iterator = 10
function spawnCritter()
    local tex = math.random(1,15)
    if ( tex == 12 ) then tex = tex + 1 end

    local tag
    local xPosition

    -- Player 1
    tag = generateCode()
    xPosition = math.random(xPadding,width/2-xPadding)
    if playerList[1] then
        TriggerClientEvent("invaders:spawnCritter", playerList[1], iterator, tostring(tex) .. ".png", tag, xPosition, 1 )
    end
    if playerList[2] then
        TriggerClientEvent("invaders:spawnCritter", playerList[2], iterator, tostring(tex) .. ".png", tag, xPosition, 0 )
    end
    iterator = iterator+1
    --Player 2
    tag = generateCode()
    xPosition = math.random(width/2+xPadding,width-xPadding)
    if playerList[1] then
        TriggerClientEvent("invaders:spawnCritter", playerList[1], iterator, tostring(tex) .. ".png", tag, xPosition, 0 )
    end
    if playerList[2] then
        TriggerClientEvent("invaders:spawnCritter", playerList[2], iterator, tostring(tex) .. ".png", tag, xPosition, 1 )
    end
    iterator = iterator+1
end

--local difficulties = { 18, 16, 14, 12, 10, 8, 6, 4, 2 }
local difficulties = { 12, 10, 8, 6, 4, 2, 1 }
local wave = 1
local maxWave = 7
local timeout = false
local counter = difficulties[1]
--local creaturesPerWave = { 5, 5, 5, 5, 5, 5, 5, 5, 5}
creaturesPerWave = { 3, 5, 5, 5, 5, 5, 10, 20, 30}
local creaturesSpawnedYet = 0

local gameEnded = false

function announce( strToAnnounce )
    TriggerClientEvent("invaders:announce",-1,strToAnnounce)
end


RegisterServerEvent("invaders:spawnTimer")
AddEventHandler("invaders:spawnTimer",function()
    if not gameEnded then
        if ( connectedPlayers >= 2 ) then
            if extraTimer then
                if counter <= 0 then


                    if not timeout then
                        spawnCritter()

                        if playerList[1] then
                            TriggerClientEvent("invaders:playerSide", playerList[1], 1)
                        end
                        if playerList[2] then
                            TriggerClientEvent("invaders:playerSide", playerList[2], 2)
                        end

                        creaturesSpawnedYet = creaturesSpawnedYet + 1

                        counter = difficulties[wave]

                        if creaturesSpawnedYet == creaturesPerWave[wave] then
                            -- MOVE TO NEXT WAVE
                            timeout = true
                            counter = 30
                            creaturesSpawnedYet = 0
                            announce( "WAVE " .. tostring(wave+1) )
                        end

                    else
                        timeout = false
                        if wave < maxWave then
                            if wave < maxWave-1 then
                                TriggerClientEvent("invaders:updateTimer",-1,difficulties[wave]*10)
                            end
                            wave = wave + 1
                        else
                            gameEnded = true
                            if score[1] > score[2] then
                                announce ( "Winner " .. nicknames[1] )
                            elseif score[1] < score[2] then
                                announce ( "Winner " .. nicknames[2] )
                            else
                                announce ("DRAW")
                            end
                        end
                    end

                else
                    counter = counter - 1
                end
            else
                extraTimer = true
            end
        else
            extraTimer = false
        end
    end
end)