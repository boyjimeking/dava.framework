TIMEOUT = 10.0
TIMECLICK = 0.2

-- local variable
local step_num = 0
local function next_step()
	step_num = step_num + 1
	print("")
end

function SetPackagePath(path)
	package.path = package.path .. ";" .. path .. "Actions/?.lua;" .. path .. "Scripts/?.lua;"
	
	require "logger"
	require "coxpcall"
end

function Step (func, ...)
    print("Start step: " .. description[step_num])
    
	local status, err = copcall(func, ...)
	if status then
		autotestingSystem:OnTestStep(description[step_num], true)
		next_step()
	else
		autotestingSystem:OnTestStep(description[step_num], false, err)
	end
end

function Yield()
    --print("Yield")
    coroutine.yield()
end

function OnError(msg)
    print("Error on step " .. description[step_num])
    print(msg)
    print()
    print()
    autotestingSystem:OnError(description[step_num])
end

function Assert(func, ..)
    --local isPassed = (expression)
    --autotestingSystem:OnTestAssert(msg, isPassed)
    --if not isPassed then
        --while true do
            --Yield()
        --end
    --end
	print("Start assertion: " .. description[step_num])
    
	local status, err = copcall(func, ...)
	if status then
		autotestingSystem:OnTestStep(description[step_num], err)
		next_step()
	else
		autotestingSystem:OnTestStep(description[step_num], false, err)
    end
end

function ResumeTest()
    --print("ResumeTest")
    if coroutine.status(co) == "suspended" then
        coroutine.resume(co)
        --print("ResumeTest done")
    else
        --print("ResumeTest failed. status:", coroutine.status(co))
        StopTest()
    end
end

function CreateTest(test)
    --print("CreateTest")
    co = coroutine.create(test) -- create a coroutine with foo as the entry
    --print(type(co))                 -- display the type of object "co"
    
    --print(AutotestingSystem)
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
    --print("AutotestingSystem.Singleton_Autotesting_Instance")
    
    --print(autotestingSystem:GetTimeElapsed())
    
    --print("CreateTest done")
end

function StartTest(test)
	print()
	print()
	print("================================================================================")
    print("START TEST: " .. description[step_num])
    print("================================================================================")
    next_step()
    CreateTest(test)
    ResumeTest()
end

function StopTest()
--    print("StopTest")
    autotestingSystem:OnTestFinished()
end

function WaitForMaster()
--    print("WaitForMaster")
    autotestingSystem:WaitForMaster()
end

function WaitForHelpers(helpersCount)
--    print("WaitForHelpers")
    autotestingSystem:WaitForHelpers(helpersCount)
end

function Wait(waitTime)
    waitTime =  waitTime or TIMECLICK
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        Yield()
    end
end

function WaitControl(name, time)
    local waitTime = time or TIMEOUT
    print("WaitControl name="..name.." waitTime="..waitTime)
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
--        print("Searching "..elapsedTime)
        
        if autotestingSystem:FindControl(name) then
            print("WaitControl found "..name)
            return true
        else
            Yield()
        end
    end
    
    print("WaitControl not found "..name)
    return false
end

function TouchDownPosition(position, touchId)
    local touchId = touchId or 1
    print("TouchDownPosition position="..position.x..","..position.y.." touchId="..touchId)
    autotestingSystem:TouchDown(position, touchId)
end

function TouchDown(x, y, touchId)
    local touchId = touchId or 1
    print("TouchDown x="..x.." y="..y.." touchId="..touchId)
    autotestingSystem:TouchDown(position, touchId)
end

function TouchMovePosition(position, touchId)
    local touchId = touchId or 1
    print("TouchMovePosition position="..position.x..","..position.y.." touchId="..touchId)
    autotestingSystem:TouchMove(position, touchId)
end

function TouchMove(x, y, touchId)
    local touchId = touchId or 1
    print("TouchMove x="..x.." y="..y.." touchId="..touchId)
    autotestingSystem:TouchMove(position, touchId)
end

function TouchUp(touchId)
    print("TouchUp "..touchId)
    autotestingSystem:TouchUp(touchId)
end

function ClickPosition(position, touchId, time)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    print("ClickPosition position="..position.x..","..position.y.." touchId="..touchId.." waitTime="..waitTime)
    
    TouchDownPosition(position, touchId)
    Wait(waitTime)
    TouchUp(touchId)
end

function Click(x, y, touchId)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    print("Click x="..x.." y="..y.." touchId="..touchId)
    
    local position = Vector.Vector2(x, y)
    ClickPosition(position, touchId, waitTime)
end

function ClickControl(name, touchId, time)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
	
	Wait()
    print("ClickControl name="..name.." touchId="..touchId.." waitTime="..waitTime)
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
--        print("Searching "..elapsedTime)
        
        local control = autotestingSystem:FindControl(name)
        if control then
--            print("ClickControl found "..name)
--            print(control)
            
            -- local position = control:GetPosition(true)
            local position = Vector.Vector2()
--            print(position)
--            print("position="..position.x..","..position.y)
            
            local geomData = control:GetGeometricData()
--            print(geomData)
            local rect = geomData:GetUnrotatedRect()
--            print(rect)
            
--            print("rect.x="..rect.x)
--            print("rect.y="..rect.y)
--            print("rect.dx="..rect.dx)
--            print("rect.dy="..rect.dy)
            
            position.x = rect.x + rect.dx/2
            position.y = rect.y +rect.dy/2
--            print("position="..position.x..","..position.y)
            
            ClickPosition(position, touchId, waitTime)
            
			Wait()
            return true
        else
            Yield()
        end
    end
    
    print("ClickControl not found "..name)
    return false
end

function SetText(path, text)
    print("SetText path="..path.." text="..text)
    return autotestingSystem:SetText(path, text)
end

function CheckText(name, txt)
	error("Method not implemented")
	if autotestingSystem:FindControl(name) then
		local control = autotestingSystem:FindControl(name)
		return autotestingSystem:CheckText(control, txt)
	else
		error("Control " .. name .. " not found")
	end
end

function CheckMsgText(name, key)
	print("Check that text witk key [" .. key .. "] is present on control " .. name)
	if autotestingSystem:FindControl(name) then
		local control = autotestingSystem:FindControl(name)
		return autotestingSystem:CheckMsgText(control, key)
	else
		error("Control " .. name .. " not found")
	end
end