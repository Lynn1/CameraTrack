-- Lua Test: linlin camera Tracking Test -
-- Jul 26, 2016
-- Aug 12, 2016 latest update


--~ function sleep(n)

--~ 	local t0 = os.clock();
--~ 	while os.clock() - t0 <= n do end

--~ end



function server()
	--local clock = os.clock();


	-- name for the host and port
	local host = host or "*";
	local port = port or 6000;

	local socket = require("socket");
	local s = assert(socket.bind(host, port));

	--local i, p = s:getsockname();
	--assert(i, p);
	print("Waiting connection to my address:" .. host .. ":" .. port .. "...");

	--c = assert(s:accept());
	c = s:accept();
	print("Connected. ");

	while true do
		local camsend;
		camsend = c:receive();

		if camsend == "end" then
			break;
		elseif camsend then
			print(camsend);

		local splitInfo = {};
		for word in string.gmatch(camsend, '([^,]+)') do
			table.insert(splitInfo, word);
		end

		local id = tonumber(splitInfo[1]);
		local hs = tonumber(splitInfo[2]);
		local vs = tonumber(splitInfo[3]);

		if id > 2 then
			vs = -1 * vs;
		end

		--MA2 spin the light
--~ 		print('Fixture '..id..' Attribute "Pan" at+ '.. hs);
--~ 		print('Fixture '..id..' Attribute "Tilt" at+ '.. vs);
		gma.cmd('Fixture '..id..' Attribute "Pan" at+ '.. hs);
		gma.cmd('Fixture '..id..' Attribute "Tilt" at+ '.. vs);

--~ 			gma.cmd('Fixture 1 Attribute "Pan" at+ '.. hs);
--~ 			gma.cmd('Fixture 1 Attribute "Tilt" at+ '.. vs);
--~ 			gma.cmd('Fixture 2 Attribute "Pan" at+ '.. hs);
--~ 			gma.cmd('Fixture 2 Attribute "Tilt" at+ '.. vs);
--~ 			gma.cmd('Fixture 3 Attribute "Pan" at+ '.. hs);
--~ 			gma.cmd('Fixture 3 Attribute "Tilt" at+ '.. -vs);
--~ 			gma.cmd('Fixture 4 Attribute "Pan" at+ '.. hs);
--~ 			gma.cmd('Fixture 4 Attribute "Tilt" at+ '.. -vs);
		end
		--sleep(.5);
	end

	s:close();
	print ("Out.");

end

return server();
