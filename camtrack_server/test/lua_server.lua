-- Lua Test: linlin camera Tracking Test -
-- Jul 26, 2016
-- Aug 12, 2016 latest update


--~ function sleep(n)

--~ 	local t0 = os.clock();
--~ 	while os.clock() - t0 <= n do end

--~ end

--time nod
local timenod = {};
local cuename = {};
local CUECOUNT = 21;

function inittimenod()
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
	table.insert(timenod,70.03);  --13
	table.insert(cuename,"Cue 13.4");
	table.insert(timenod,75.30);  --14
	table.insert(cuename,"Cue 13.5");
	table.insert(timenod,75.33);  --15
	table.insert(cuename,"Cue 13.6");
	table.insert(timenod,80.73);  --16
	table.insert(cuename,"Cue 13.65");
	table.insert(timenod,80.77);    --17
	table.insert(cuename,"Cue 13.66");
	table.insert(timenod,86.10);    --18
	table.insert(cuename,"Cue 13.75");
	table.insert(timenod,86.13);    --19
	table.insert(cuename,"Cue 13.76");
	table.insert(timenod,91.30);    --20
	table.insert(cuename,"Cue 14");
	table.insert(timenod,102);    --21
	table.insert(cuename,"Cue 15");
end

function playMusic()

	local host = host or "localhost";
	local port = port or 7000;
	local socket = require("socket");
	local s = assert(socket.tcp());
	print("Waiting connection from server on " .. host .. ":" .. port .. "...");
	-- if success, return 0 -> c
	local c = s:connect(host, port);
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
end


function server()

	inittimenod();

	-- name for the host and port
	local host = host or "*";
	local port = port or 6000;

	local socket = require("socket");
	local s = assert(socket.bind(host, port));

	--local i, p = s:getsockname();
	--assert(i, p);
	print("Waiting connection to my address:" .. host .. ":" .. port .. "...");

	--c = assert(s:accept());
	local c = s:accept();
	print("Connected. ");

	playMusic();
	local t0 = os.clock();
	local i = 1;

	while true do
		--Cue message
		if i<=CUECOUNT then
			if os.clock() - t0 >= timenod[i] then
				if i < 2 then
					print (cuename[i]);
					gma.cmd('Goto '..cuename[i]);
				else
					gma.cmd('Go ');
				end

				i = i + 1;
			end
		end

		--camera message
		local camsend;
		camsend = c:receive();

		if camsend == "no" then
			--no message;
		elseif camsend == "end" then
			break;
		elseif camsend then
			--print(camsend);
			local splitInfo = {};
			for word in string.gmatch(camsend, '([^,]+)') do
				table.insert(splitInfo, word);
			end

			local id = tonumber(splitInfo[1]);
			local hs = tonumber(splitInfo[2]);
			local vs = tonumber(splitInfo[3]);

--~ 			if id > 2 then
--~ 				vs = -1 * vs;
--~ 			end

			--MA2 spin the light
			print('Fixture '..id..' Attribute "Pan" at+ '.. hs);
			print('Fixture '..id..' Attribute "Tilt" at+ '.. vs);
			gma.cmd('Fixture '..id..' Attribute "Pan" at+ '.. hs);
			gma.cmd('Fixture '..id..' Attribute "Tilt" at+ '.. vs);

		end
	end

	s:close();
	print ("Out.");

end

return server();
