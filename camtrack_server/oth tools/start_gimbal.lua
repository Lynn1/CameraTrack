local rs232 = require("luars232")

--IMPORTANT: 在设备管理器中查看插入设备后识别的串口号，port_name 填写该识别的串口号

local port_name = "COM4"

local H_ANGLE = 00;
local V_ANGLE = 300;

function sleep(n)

	local t0 = os.clock();
	while os.clock() - t0 <= n do end

end


local function connectToDevice(hs, vs)
local out = io.stderr

	-- open port
	local e, p = rs232.open(port_name)
	if e ~= rs232.RS232_ERR_NOERROR then
		-- handle error
		out:write(string.format("can't open serial port '%s', error: '%s'\n",
				port_name, rs232.error_tostring(e)))
		return
	end

	-- set port settings
	assert(p:set_baud_rate(rs232.RS232_BAUD_115200) == rs232.RS232_ERR_NOERROR);
	assert(p:set_data_bits(rs232.RS232_DATA_8) == rs232.RS232_ERR_NOERROR);
	assert(p:set_parity(rs232.RS232_PARITY_NONE) == rs232.RS232_ERR_NOERROR);
	assert(p:set_stop_bits(rs232.RS232_STOP_1) == rs232.RS232_ERR_NOERROR);
	assert(p:set_flow_control(rs232.RS232_FLOW_OFF)  == rs232.RS232_ERR_NOERROR);

	out:write(string.format("OK, port open with values '%s'\n", tostring(p)))
--Set remote cradle turning angle format: "$WICTL,H_ANGLE,V_ANGLE*00\r\n", H_ANGLE = horizontal angle * 10,
--V_ANGLE = vertical angle * 10 (i.e. if horizontal angle of 30.4 degrees and vertical angle
-- of 92.0 degrees are required, send "$WICTL,304,920*00\r\n" to complete.)

--Please note that the horizontal cradle has a 270 degrees limit and vertical cradle has a 180 degrees limit.
--commands that exceed this limit will not be executed.


--~ 	err, len_written = p:write("$WISAR,1,"..hs..","..vs.."*00\r\n", 100);
	err, len_written = p:write("$WISAA,1,"..hs..","..vs.."*00\r\n", 100);
	--p:write("$WISAR,1,"..hs..","..vs.."*00\r\n", 100);

	--p:write("$WICTL,00,00*00\r\n", 100);
	--p:write("$WICTL,"..hs..","..vs.."*00\r\n", 100);

	--p:write("$WISAA,0,00,00*00\r\n", 100);
	--p:write("$WISAR,0,00,00*00\r\n", 100);


	assert(e == rs232.RS232_ERR_NOERROR);

	local read_len = 20 -- read one byte
	local timeout = 100 -- in miliseconds
	local err, data_read, size = p:read(read_len, timeout)
	assert(e == rs232.RS232_ERR_NOERROR)

	if data_read and size ~= 0 then
		print("device return msg:\n"..data_read);
	else
		print("device not responding");
	end

	assert(p:close() == rs232.RS232_ERR_NOERROR)

end


--~ -- receive command and play music
--~ --server
--~ function waitToGimbal()

--~ 	local host = host or "localhost";
--~ 	local port = port or 7000;

--~ 	local socket = require("socket");
--~ 	local s = assert(socket.bind(host, port));

--~ 	print("Waiting connection to my address:" .. host .. ":" .. port .. "...");

--~ 	c = s:accept();
--~ 	print("Connected.\n start to gimbal.");

--~ 	--close socket
--~ 	s:close();

--~ 	connetToDevice();

--~ end


function spinAt(h,v)

	--spin at h
	connectToDevice(h,V_ANGLE);
	H_ANGLE = h;

	sleep(0.3);

	--spin at v
	connectToDevice(H_ANGLE,v);
	V_ANGLE = v;

	sleep(0.3);
end



function main()

	 H_s = 0;
	 V_s = 90;

	 H_e = 180;
	 V_e = 45;

	h = H_s;
	v = V_s;

--~ 	connectToDevice(0,90);

	while v>= V_e do
	 connectToDevice(h,v);
	 v = v - 15 ;
	end

	while h<= H_e do
	 connectToDevice(h,v);
	 h = h +20 ;
	end

--~ 	--start state 0,0--Fixed
--~ 	local h = 00;
--~ 	local v = 300;
--~ 	spinAt(h,v);

	----------------------------
--~ 	--spin at H,V
--~ 	h = 00;
--~ 	v = 00;
--~ 	spinAt(h,v);


--~ 	--spin at H,V
--~ 	h = 1800;
--~ 	v = 00;
--~ 	spinAt(h,v);

--~ 	--spin at H,V
--~ 	h = 1000;
--~ 	v = 900;
--~ 	spinAt(h,v);

--~ 	--spin at H,V
--~ 	h = 2700;
--~ 	v = 400;
--~ 	spinAt(h,v);

--~ 	--spin at H,V
--~ 	h = 500;
--~ 	v = 800;
--~ 	spinAt(h,v);

--~ 	--spin at H,V
--~ 	h = 900;
--~ 	v = 450;
--~ 	spinAt(h,v);
	----------------------------


--~ 	-- back to 0,0--Fixed
--~ 	h = 00;
--~ 	v = 00;
--~ 	spinAt(h,v);

end

return main();
--return waitToGimbal();
