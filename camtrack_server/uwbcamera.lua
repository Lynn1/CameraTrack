-- Lua Test: camera tracking &  uwb tracking combine Test - linlin
-- Jul 26, 2016
-- Aug 25, 2016 latest update


function sleep(n)

	local t0 = os.clock();
	while os.clock() - t0 <= n do end

end

-- initializes the lights' positions
function initLight()

	light2X = 3.75;
	light2Y = 0.827;
	light2Z = .384;

	light3X = 3.75;
	light3Y = 8.327;
	light3Z = .384;

	light4X = 11.25;
	light4Y = 8.327;
	light4Z = .384;

	light1X = 11.25;
	light1Y = .827;
	light1Z = .384;

end

--calcPan calculates the pan the light requires
function calcPan(X, Y)
	local pan = 0;

	if math.abs(X) <= 0.05 then
		if  Y >=0 then
			pan = 90;
		else
			pan = -90;
		end
		return pan;
	else
		local slope = Y/X;

		local panRad = math.atan(slope);

		pan = math.deg(panRad);

		if X<0 then
			if Y >=0 then
				pan = 180 + pan;
			else
				pan = -180 + pan;
			end
		end

		return pan;

	end

--~ 	return (pan + 270) * 0.9916666 - 270;
end

--calcTilt calculates the tilt the light requires
function calcTilt(X, Y, Z)
		local tilt = 0; -- horizon is 0

		local xy = math.sqrt( X^2 + Y^2 );

		if xy <=0.05 then
			if Z >=0 then
				tilt = 90;
			else
				tilt = -45;
			end
			return tilt;
		else
			local slope = Z/xy;

			local tiltRad = math.atan(slope);

			tilt = math.deg(tiltRad);

			if tilt<-45 then
				tilt = -45;
			end
		end

		return tilt;

end


function lightsLookAt(xD, yD, zD)

	--Subtract the light coordinates so it will point correctly
	local drone1X = -1 *(xD - light1X);
	local drone1Y = -1 *(yD - light1Y);
	local drone1Z = zD - light1Z;

	local drone2X = xD - light2X;
	local drone2Y = yD - light2Y;
	local drone2Z = zD - light2Z;

	local drone3X = xD - light3X;
	local drone3Y = yD - light3Y;
	local drone3Z = zD - light3Z;

	local drone4X = -1 *(xD - light4X);
	local drone4Y = -1 *(yD - light4Y);
	local drone4Z = zD - light4Z;


	local pan1 = calcPan(drone1X, drone1Y);

	local pan2 = calcPan(drone2X, drone2Y);

	local pan3 = calcPan(drone3X, drone3Y);

	local pan4 = calcPan(drone4X, drone4Y);


	--Calculate tilt
	tilt1 = -97.5 + calcTilt(drone1X, drone1Y, drone1Z);

	tilt2 = 99.3 - calcTilt(drone2X, drone2Y, drone2Z);

	tilt3 = 98.5 - calcTilt(drone3X, drone3Y, drone3Z);

	tilt4 = -99 + calcTilt(drone4X, drone4Y, drone4Z);

	--Send the commands to MA2
	print("Pan1: " .. pan1 .. " Tilt1: " .. tilt1 .. "\n");
	print("Pan2: " .. pan2 .. " Tilt2: " .. tilt2 .. "\n");
	print("Pan3: " .. pan3 .. " Tilt3: " .. tilt3 .. "\n");
	print("Pan4: " .. pan4 .. " Tilt4: " .. tilt4 .. "\n");

	gma.cmd('Fixture 1 Attribute "Pan" at '.. pan1);
	gma.cmd('Fixture 2 Attribute "Pan" at '.. pan2);
	gma.cmd('Fixture 3 Attribute "Pan" at '.. pan3);
	gma.cmd('Fixture 4 Attribute "Pan" at '.. pan4);

	gma.cmd('Fixture 1 Attribute "Tilt" at '.. tilt1);
	gma.cmd('Fixture 2 Attribute "Tilt" at '.. tilt2);
	gma.cmd('Fixture 3 Attribute "Tilt" at '.. tilt3);
	gma.cmd('Fixture 4 Attribute "Tilt" at '.. tilt4);


end


-- init server socket to receive camTrack message
local cam_ser;
local cam_cli;

--server socket
function initCamServer()
	-- name for the client host and port
	local host = host or "*";
	local port = port or 6000;

	local socket = require("socket");
	cam_ser = assert(socket.bind(host, port));
	print("Waiting connection from camTrack...");

	cam_cli = cam_ser:accept();
	print("camTrack connected. ");
end



-- init client socket to receive UWB message
--local uwb_ser;
local uwb_cli;

--client socket
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

function main()

	initLight();

	--connect UWB
--~ 	initUWBClient();

	--connect camTrack
	initCamServer();

--~ 	local t0 = os.clock();
	while true do

		--camera message
		local camInfo;
		camInfo = cam_cli:receive();
		--print("cam Info: "..camInfo);


		if camInfo == "no" then
--~ 				--UWB Message
--~ 				local UWBinfo;
--~ 				UWBinfo = uwb_cli:receive();
--~ 				--print(UWBinfo);
--~ 				if UWBinfo then
--~ 					--camera no message, control by UWB message
--~ 					local UWB_splitInfo = {};
--~ 					for word in string.gmatch(UWBinfo, '([^,]+)') do
--~ 						table.insert(UWB_splitInfo, word);
--~ 					end

--~ 					--The coordinates we need are in the 6th, 7th, and 8th spots of the table
--~ 					local x = UWB_splitInfo[6];
--~ 					local y = UWB_splitInfo[7];
--~ 					local z = UWB_splitInfo[8] - .4;

--~ 					print("UWB pos: ".. x .. " " .. y .. " " .. z);
--~ 					lightsLookAt(x, y, z);
--~ 				else
--~ 					break;
--~ 				end
		elseif camInfo == "end" then
			break;

		elseif camInfo then
			local cam_splitInfo = {};
			for word in string.gmatch(camInfo, '([^,]+)') do
				table.insert(cam_splitInfo, word);
			end

			local id = tonumber(cam_splitInfo[1]);
			local hs = tonumber(cam_splitInfo[2]);
			local vs = tonumber(cam_splitInfo[3]);


--~ 				--MA2 spin the light
				print('Fixture '..id..' Attribute "Pan" at+ '.. hs);
				print('Fixture '..id..' Attribute "Tilt" at+ '.. vs);
--~ 				gma.cmd('Fixture '..id..' Attribute "Pan" at+ '.. hs);
--~ 				gma.cmd('Fixture '..id..' Attribute "Tilt" at+ '.. vs);

		end

		--sleep(.5);
	end

	cam_ser:close();
--~ 	uwb_cli:close();
	print ("End.");

end

return main();
