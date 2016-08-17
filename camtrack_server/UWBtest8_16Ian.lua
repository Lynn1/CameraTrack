-- UWB Test
-- Ian Mackenzie E-GO CG


local clock = os.clock;


--sleep allows for the creation of downtime
function sleep(n)

	local t0 = clock();
	while clock() - t0 <= n do end

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


--lightInit initializes the lights' positions
function lightInit()

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


--checkNeg makes sure the light doesn't point in the opposite direction
function checkNeg(droneXVal, panCheck)

--~ 	if(tonumber(droneXVal) < 0) then
--~ 		panCheck = panCheck * -1;
--~ 	end

	if (panCheck < 0) then
		panCheck = panCheck - (panCheck * 2);
	end

	if (panCheck > 0) then
		panCheck = panCheck - (panCheck * 2);
	end

	return panCheck;

end


function mainLoop()

	--Initialization and connection of the socket
	lightInit();

	local socket = require("socket");

	local host = host or "192.168.1.208";
	local port = port or 10686;

	local s = assert(socket.tcp());

	print("Connecting...");

	local connected = s:connect(host, port);

	print("connected");

	while(true)do

		--If not connected, then continue trying to connect
		if not connected then

			local connected = s:connect(host, port);

		--Else, we can move on to receiving the UWB information
		else


			--UWBinfo contains all of the information sent by UWB, too much
			print("Receiving...");
			local UWBinfo = s:receive();
			print("Received...");
			local splitByCommaInfo = {};


			--Split UWBinfo by comma and put them into a table
			for word in string.gmatch(UWBinfo, '([^,]+)') do
				table.insert(splitByCommaInfo, word);
			end


			--The coordinates we need are in the 6th, 7th, and 8th spots of the table
			local x = splitByCommaInfo[6];
			local y = splitByCommaInfo[7];
			local z = splitByCommaInfo[8] - .4;

			print("z:" .. z);


			--Subtract the light coordinates so it will point correctly
			drone1X = x - light1X;
			drone1Y = y - light1Y;
			drone1Z = z - light1Z;

			drone2X = x - light2X;
			drone2Y = y - light2Y;
			drone2Z = z - light2Z;

			drone3X = x - light3X;
			drone3Y = y - light3Y;
			drone3Z = z - light3Z;

			drone4X = x - light4X;
			drone4Y = y - light4Y;
			drone4Z = z - light4Z;

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


--~ 			if(drone1Y > 0) and (drone1X < 0) then
--~ 				if(pan1 < 0) then
--~ 					pan1 = pan1 + 180;
--~ 				else
--~ 					pan1 = pan1 - 180;
--~ 				end
--~ 			end

--~ 			if(drone2Y > 0) and (drone2X < 0) then
--~ 				if(pan1 < 0) then
--~ 					pan2 = pan2 + 180;
--~ 				else
--~ 					pan2 = pan2 - 180;
--~ 				end
--~ 			end

--~ 			if(drone3Y > 0) and (drone3X < 0) then
--~ 				if(pan3 < 0) then
--~ 					pan3 = pan3 + 180;
--~ 				else
--~ 					pan3 = pan3 - 180;
--~ 				end
--~ 			end

			if(drone4Y > 0) and (drone4X < 0) then
				if(pan4 < 0) then
					pan4 = pan4 + 180;
				else
					pan4 = pan4 - 180;
				end
			end




--~ 			if(drone1Y > 0) and (drone1X > 0) then
--~ 				if(pan1 < 0) then
--~ 					pan1 = pan1 + 90;
--~ 				else
--~ 					pan1 = pan1 - 270;
--~ 				end
--~ 				--pan1 = pan1 * -1;
--~ 			end

--~ 			if(drone2Y > 0) and (drone2X > 0) then
--~ 				if(pan1 < 0) then
--~ 					pan2 = pan2 + 90;
--~ 				else
--~ 					pan2 = pan2 - 270;
--~ 				end
--~ 				--pan2 = pan2 * -1;
--~ 			end

--~ 			if(drone3Y > 0) and (drone3X > 0) then
--~ 				if(pan3 < 0) then
--~ 					pan3 = pan3 + 90;
--~ 				else
--~ 					pan3 = pan3 - 270;
--~ 				end
--~ 				--pan3 = pan3 * -1;
--~ 			end

			if(drone4Y > 0) and (drone4X > 0) then
				if(pan4 < 0) then
					pan4 = pan4 + 90;
				else
					pan4 = pan4 - 270;
				end
				--pan4 = pan4 * -1;
			end




--~ 			if(drone1Y < 0) and (drone1X < 0) then
--~ 				if(pan1 < 0) then
--~ 					pan1 = pan1 + 270;
--~ 				else
--~ 					pan1 = pan1 - 90;
--~ 				end
--~ 				--pan1 = pan1 * -1;
--~ 			end

--~ 			if(drone2Y < 0) and (drone2X < 0) then
--~ 				if(pan1 < 0) then
--~ 					pan2 = pan2 + 270;
--~ 				else
--~ 					pan2 = pan2 - 90;
--~ 				end
--~ 				--pan2 = pan2 * -1;
--~ 			end

--~ 			if(drone3Y < 0) and (drone3X < 0) then
--~ 				if(pan3 < 0) then
--~ 					pan3 = pan3 + 270;
--~ 				else
--~ 					pan3 = pan3 - 90;
--~ 				end
--~ 				--pan3 = pan3 * -1;
--~ 			end

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
			gma.cmd('Fixture 1 Attribute "Pan" at '.. pan1);
			gma.cmd('Fixture 1 Attribute "Tilt" at '.. tilt1);

			gma.cmd('Fixture 2 Attribute "Pan" at '.. pan2);
			gma.cmd('Fixture 2 Attribute "Tilt" at '.. tilt2);

			gma.cmd('Fixture 3 Attribute "Pan" at '.. pan3);
			gma.cmd('Fixture 3 Attribute "Tilt" at '.. tilt3);

			gma.cmd('Fixture 4 Attribute "Pan" at '.. pan4);
			gma.cmd('Fixture 4 Attribute "Tilt" at '.. tilt4);

--~ 			print("Pan1: " .. pan1 .. " Tilt1: " .. tilt1 .. "\n");
--~ 			print("Pan2: " .. pan2 .. " Tilt2: " .. tilt2 .. "\n");
--~ 			print("Pan3: " .. pan3 .. " Tilt3: " .. tilt3 .. "\n");
--~ 			print("Pan4: " .. pan4 .. " Tilt4: " .. tilt4 .. "\n");


			--Sleep for the specified amount of time
			sleep(.1);

		end
	end
end


function server()

	--Initialization of socket
	local socket = require("socket");

	local host = host or "*";

	local port1 = port1 or 6000;
	local port2 = port2 or 7000;
	local port3 = port3 or 8000;
	local port4 = port4 or 9000;

	local s1 = assert(socket.bind(host, port1));
	local s2 = assert(socket.bind(host, port2));
	local s3 = assert(socket.bind(host, port3));
	local s4 = assert(socket.bind(host, port4));

	--print("Waiting connection to my address:" .. host .. ":" .. port .. "...");

	c1 = s1:accept();
	c2 = s2:accept();
	c3 = s3:accept();
	c4 = s4:accept();

	--print("Connected. ");


	while true do
		local hs, vs = 0;

		hs = c:receive();
		vs = c:receive();

		if hs == "end" then
			break;
		elseif hs then
			print( "h ".. hs .. ", v " .. vs);

			--MA2 spin the light
			gma.cmd('Fixture 1 Attribute "Pan" at+ '.. hs);
			gma.cmd('Fixture 1 Attribute "Tilt" at+ '.. vs);

		end
	end

	s:close();
	print ("Out.");

end


return mainLoop();

