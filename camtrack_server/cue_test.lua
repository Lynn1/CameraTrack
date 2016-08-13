--test for Cue
--linlin Aug 13, 2016


--time nod
local time1 = 2
local time2 = 3.125;
local time3 = 4.25;
local time4 = 5.375;
local time5 = 7.5;
local time6 = 8.5;

local time7 = 10;
local time8 = 15;
local time9 = 20;

function callCue()

	local host = host or "localhost";
	local port = port or 7000;
	local socket = require("socket");
	local s = assert(socket.tcp());
	print("Waiting connection from server on " .. host .. ":" .. port .. "...");
	-- if success, return 0 -> c
	c = s:connect(host, port);
	while true do
		if not c then
			print("connecting...\n");
			c = s:connect(host, port);
		else
			s:close();
			break;
		end
	end

	print ("Ready to callCue.");
	local t0 = os.clock();
	local step=0 ;
	while(true)do

		if os.clock() - t0 >= time1 and step==0 then
			print ("cue 1");
			gma.cmd('Goto Cue 1');
			step = step + 1 ;
		end

		if os.clock() - t0 >= time2 and step==1 then
			print ("cue 2");
			gma.cmd('Goto Cue 2');
			step = step + 1 ;
		end

		if os.clock() - t0 >= time3 and step==2 then
			print ("cue 3");
			gma.cmd('Goto Cue 3');
			step = step + 1 ;
		end

		if os.clock() - t0 >= time4 and step==3 then
			print ("cue 4");
			gma.cmd('Goto Cue 4');
			step = step + 1 ;
		end

		if os.clock() - t0 >= time5 and step==4 then
			print ("cue 5");
			gma.cmd('Goto Cue 5');
			step = step + 1 ;
		end

		if os.clock() - t0 >= time6 and step==5 then
			break;
		end

	end

	print ("End.");

end

return callCue();
