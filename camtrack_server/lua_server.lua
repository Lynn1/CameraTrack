-- Lua Test: linlin camera Tracking Test -
-- 2016.7.25
-- 2016.8.11 latest update


--~ function sleep(n)

--~ 	local t0 = os.clock();
--~ 	while os.clock() - t0 <= n do end

--~ end



function server()
	local clock = os.clock();


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
		local hs, vs = 0;

		hs = c:receive();
		vs = c:receive();

		if hs == "end" then
			break;
		elseif hs then
			print( "h ".. hs .. ", v " .. vs);

			--MA2 spin the light

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
