-- Lua Test: linlin camera tracking & Ian uwb tracking combine Test -
-- Jul 26, 2016
-- Aug 17, 2016 latest update


function sleep(n)

	local t0 = os.clock();
	while os.clock() - t0 <= n do end

end

-- initializes Cue time
local timenod = {};
local cuename = {};
local CUECOUNT = 21;

function initCue()
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


-- initializes the lights' positions
function initLight()

	light1X = 3.75;
	light1Y = 0.827;
	light1Z = .384;

	light2X = 3.75;
	light2Y = 8.327;
	light2Z = .384;

	light3X = 11.25;
	light3Y = 8.327;
	light3Z = .384;

	light4X = 11.25;
	light4Y = .827;
	light4Z = .384;

end

--calcPan calculates the pan the light requires
function calcPan(X, Y)

	local slope = (-Y)/(X);

	--local panRad = math.atan(slope);

	local panRad = math.atan2((-Y), (X));

	--print("init atan: " .. panRad);
	local pan = math.deg(panRad);
	--print(pan);

	return (pan+270)*0.9916666-270;
end

--calcTilt calculates the tilt the light requires
function calcTilt(X, Y, Z)

	local a1 = Z;
	local b1 = (X^2);
	local c1 = (Y^2);
	local d1 = b1 + c1;
	local e1 = math.sqrt(d1);
	local f1 = e1/a1;
	--local g1 = math.atan(f1);
	local g1 = math.atan2(e1, a1);
	local tilt = math.deg(g1)*1.1;
	--print(tilt);
	return tilt;

end

--checkNeg makes sure the light doesn't point in the opposite direction
function checkNeg(droneXVal, panCheck)

	if (panCheck < 0) then
		panCheck = panCheck - (panCheck * 2);
	end

	if (panCheck > 0) then
		panCheck = panCheck - (panCheck * 2);
	end

	return panCheck;

end

-- send command to play music
--client soket
function startMusic()
	-- name for the host and port
	local host = host or "localhost";
	local port = port or 7000;
	local socket = require("socket");
	local s = assert(socket.tcp());
	print("Waiting connection from music player...");
	-- if success, return 0 -> c
	local c = s:connect(host, port);
	while true do
		if not c then
			print("connecting music player...");
			c = s:connect(host, port);
		else
			print("music player connected. ");
			s:close();
			break;
		end
	end

	print ("Ready to callCue!\n");
end


-- init server soket to receive camTrack message
local cam_ser;
local cam_cli;

--server soket
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



-- init client soket to receive UWB message
--local uwb_ser;
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


function lightsLookAt(x,y,z)
	--Subtract the light coordinates so it will point correctly
	local drone1X = x - light1X;
	local drone1Y = y - light1Y;
	local drone1Z = z - light1Z;

	local drone2X = x - light2X;
	local drone2Y = y - light2Y;
	local drone2Z = z - light2Z;

	local drone3X = x - light3X;
	local drone3Y = y - light3Y;
	local drone3Z = z - light3Z;

	local drone4X = x - light4X;
	local drone4Y = y - light4Y;
	local drone4Z = z - light4Z;

	--Calculate the pan and correct for drone location
	pan1 = calcPan(drone1X, drone1Y);
	pan1 = checkNeg(drone1X, pan1);
	pan1 = pan1 * -1;

	if(drone1X > 0 and drone1Y < 0) then
		pan1 = pan1 * -1;
	end

	pan2 = calcPan(drone2X, drone2Y);
	pan2 = checkNeg(drone2X, pan2);

	if(drone2X > 0 and drone2Y > 0) then
		pan2 = pan2 * -1;
	end

	pan3 = calcPan(drone3X, drone3Y);
	pan3 = checkNeg(drone3X, pan3);
	pan3 = pan3 + 180;

	if(drone3X < 0 and drone3Y > 0) then
		pan3 = pan3 * -1;
	end

	pan4 = calcPan(drone4X, drone4Y);
	pan4 = checkNeg(drone4X, pan4);
	pan4 = pan4 * -1;

	if(drone4X > 0 and drone4Y > 0) then
		--pan4 = (pan4 * -1);
		if(pan4 < 0) then
			pan4 = pan4 + 90;
		else
			pan4 = pan4 - 270;
		end
	end


	if(drone4Y > 0) and (drone4X < 0) then
		if(pan4 < 0) then
			pan4 = pan4 + 180;
		else
			pan4 = pan4 - 180;
		end
	end


	if(drone4Y > 0) and (drone4X > 0) then
		if(pan4 < 0) then
			pan4 = pan4 + 90;
		else
			pan4 = pan4 - 270;
		end
		--pan4 = pan4 * -1;
	end


	if(drone4Y < 0) and (drone4X < 0) then
		if(pan4 < 0) then
			pan4 = pan4 + 270;
		else
			pan4 = pan4 - 90;
		end
		--pan4 = pan4 * -1;
	end


	--Calculate tilt
	tilt1 = calcTilt(drone1X, drone1Y, drone1Z) ;

	tilt2 = calcTilt(drone2X, drone2Y, drone2Z) ;

	tilt3 = calcTilt(drone3X, drone3Y, drone3Z) ;

	tilt4 = calcTilt(drone4X, drone4Y, drone4Z) ;


	--Send the commands to MA2
--~ 	print("Pan1: " .. pan1 .. " Tilt1: " .. tilt1 .. "\n");
--~ 	print("Pan2: " .. pan2 .. " Tilt2: " .. tilt2 .. "\n");
--~ 	print("Pan3: " .. pan3 .. " Tilt3: " .. tilt3 .. "\n");
--~ 	print("Pan4: " .. pan4 .. " Tilt4: " .. tilt4 .. "\n");

	gma.cmd('Fixture 1 Attribute "Pan" at '.. pan1);
	gma.cmd('Fixture 1 Attribute "Tilt" at '.. tilt1);

	gma.cmd('Fixture 2 Attribute "Pan" at '.. pan2);
	gma.cmd('Fixture 2 Attribute "Tilt" at '.. tilt2);

	gma.cmd('Fixture 3 Attribute "Pan" at '.. pan3);
	gma.cmd('Fixture 3 Attribute "Tilt" at '.. tilt3);

	gma.cmd('Fixture 4 Attribute "Pan" at '.. pan4);
	gma.cmd('Fixture 4 Attribute "Tilt" at '.. tilt4);

end


function main()

	initCue();

	initLight();

	--connect UWB
	initUWBClient();

	--connect camTrack
	initCamServer();

	--play music
	startMusic();

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
		local camInfo;
		camInfo = cam_cli:receive();
--~ 		print("cam Info: "..camInfo);


		if camInfo == "no" then
--~ 			local UWBinfo;
--~ 			UWBinfo = uwb_cli:receive();
--~ 			print(UWBinfo);
--~ 			if UWBinfo then
--~ 				--camera no message, control by UWB message
--~ 				local UWB_splitInfo = {};
--~ 				for word in string.gmatch(UWBinfo, '([^,]+)') do
--~ 					table.insert(UWB_splitInfo, word);
--~ 				end

--~ 				--The coordinates we need are in the 6th, 7th, and 8th spots of the table
--~ 				local x = UWB_splitInfo[6];
--~ 				local y = UWB_splitInfo[7];
--~ 				local z = UWB_splitInfo[8] - .4;

--~ 				print("UWB pos: ".. x .. y);
--~ 				lightsLookAt(x,y,z);
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


			--MA2 spin the light
--~ 			print('Fixture '..id..' Attribute "Pan" at+ '.. hs);
--~ 			print('Fixture '..id..' Attribute "Tilt" at+ '.. vs);
			gma.cmd('Fixture '..id..' Attribute "Pan" at+ '.. hs);
			gma.cmd('Fixture '..id..' Attribute "Tilt" at+ '.. vs);

		end

--~ 		sleep(.5);
	end

	cam_ser:close();
	uwb_cli:close();
	print ("End.");

end

return main();
