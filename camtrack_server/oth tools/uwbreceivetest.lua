

local uwb_cli;

--client soket
function initUWBClient()

	-- name for the server host and port
	local host = host or "192.168.1.208";
	local port = port or 10686;

	local socket = require("socket");
	uwb_cli = assert(socket.tcp());

	print("Waiting connection from UWB...");
	-- if success, return 0 -> c
	local c = uwb_cli:connect(host, port);

	while true do
		if not c then
			print("connecting UWB...");
			c = uwb_cli:connect(host, port);
		else
			print("UWB connected. ");
			--uwb_cli:close();
			break;
		end
	end
end

function sleep(n)

	local t0 = os.clock();
	while os.clock() - t0 <= n do end

end

function main()


	--connect UWB
	initUWBClient();

	local t = 10;
	local t0 = os.clock();

	while true do

		local UWBinfo;
		UWBinfo = uwb_cli:receive();
		print(UWBinfo);

--~ 		sleep(.1);

	end

	uwb_cli:close();
	print ("End.");

end

return main();
