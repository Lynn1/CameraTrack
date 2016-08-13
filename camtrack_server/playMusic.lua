-- receive command and play music




function playMusic()
	local host = host or "*";
	local port = port or 7000;

	local socket = require("socket");
	local s = assert(socket.bind(host, port));

	print("Waiting connection to my address:" .. host .. ":" .. port .. "...");

	c = s:accept();
	print("Connected.");

	--do something
	s:close();
	print ("Ready to paly.");

	os.execute([[""C:\Program Files (x86)\The KMPlayer\KMPlayer.exe" "D:\linlin\test.mp3""]]);

end



return playMusic();
