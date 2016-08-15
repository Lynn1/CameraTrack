-- receive command and play music




function playMusic()
	local host = host or "localhost";
	local port = port or 7000;

	local socket = require("socket");
	local s = assert(socket.bind(host, port));

	print("Waiting connection to my address:" .. host .. ":" .. port .. "...");

	c = s:accept();
	print("Connected.");

	--do something
	s:close();
	print ("Ready to paly.");

--~ 	local path = [[""%ProgramFiles(x86)%\Windows Media Player\wmplayer.exe" "D:\linlin\test2.mp3""]];
	local path = [[""%ProgramFiles(x86)%\The KMPlayer\KMPlayer.exe" "D:\linlin\test2.mp3""]];

	os.execute(path);

end



return playMusic();
