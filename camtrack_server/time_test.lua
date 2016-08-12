


--在一个死循环中设置一个跳出条件，但是这样的做法会占用大量CPU资源，强烈不推荐使用哦
function sleep(n)

	local t0 = os.clock();
	while os.clock() - t0 <= n do end

end

--调用系统的sleep函数，不消耗CPU，
--但是Windows系统中没有内置这个命令（如果你又安装Cygwin神马的也行）推荐在Linux系统中使用该方法
function sleep2(n)

	 os.execute("sleep " .. n)

end

--虽然Windows没有内置sleep命令，但是我们可以稍微利用下ping命令的性质
function sleep3(n)
   if n > 0 then os.execute("ping -n " .. tonumber(n + 1) .. " localhost > NUL") end
end

--使用socket库中select函数，可以传递0.1给n，使得休眠的时间精度达到毫秒级别。
function sleep4(n)
   local socket = require("socket");
   socket.select(nil, nil, n);
   socket:close();
end

function myfunc()
	local x=0;
	while(true) do
		print(x);
		x = x + 1;
		sleep2(1);
	end
end


return myfunc();
