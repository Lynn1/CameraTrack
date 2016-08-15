--test for Cue
--linlin Aug 13, 2016
--latest update Aug 15,2016


--time nod
local timenod = {};
local cuename = {};
local CUECOUNT = 17;

function init()
	table.insert(timenod,0);      --1
	table.insert(cuename,"Cue 1");
	table.insert(timenod,3.50);   --2
	table.insert(cuename,"Cue 2");
	table.insert(timenod,8.80);   --3
	table.insert(cuename,"Cue 3");
	table.insert(timenod,20.07);  --4
	table.insert(cuename,"Cue 4");
	table.insert(timenod,28.03);  --5
	table.insert(cuename,"Cue 5");
	table.insert(timenod,34.04);  --6
	table.insert(cuename,"Cue 6");
	table.insert(timenod,37.97);  --7
	table.insert(cuename,"Cue 7");
	table.insert(timenod,48.73);  --8
	table.insert(cuename,"Cue 8");
	table.insert(timenod,54.03);  --9
	table.insert(cuename,"Cue 9");
	table.insert(timenod,59.30);  --10
	table.insert(cuename,"Cue 11");
	table.insert(timenod,64.63);  --11
	table.insert(cuename,"Cue 12");
	table.insert(timenod,70);     --12
	table.insert(cuename,"Cue 13");
	table.insert(timenod,73.60);  --13
	table.insert(cuename,"Cue 13.4");
	table.insert(timenod,80.73);  --14
	table.insert(cuename,"Cue 13.5");
	table.insert(timenod,83.73);  --15
	table.insert(cuename,"Cue 13.6");
	table.insert(timenod,91.30);  --16
	table.insert(cuename,"Cue 14");
	table.insert(timenod,102);    --17
	table.insert(cuename,"Cue 15");
end

function callCue()

	init();

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
	local i = 1;
	while(i<=CUECOUNT)do
		print(os.clock() - t0);
		print(timenod[i]);
		if os.clock() - t0 >= timenod[i] then
			print (cuename[i]);
			gma.cmd('Goto '..cuename[i]);
			i = i + 1;
		end
	end

	print ("End.");

end

return callCue();
