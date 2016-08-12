


--��һ����ѭ��������һ����������������������������ռ�ô���CPU��Դ��ǿ�Ҳ��Ƽ�ʹ��Ŷ
function sleep(n)

	local t0 = os.clock();
	while os.clock() - t0 <= n do end

end

--����ϵͳ��sleep������������CPU��
--����Windowsϵͳ��û������������������ְ�װCygwin�����Ҳ�У��Ƽ���Linuxϵͳ��ʹ�ø÷���
function sleep2(n)

	 os.execute("sleep " .. n)

end

--��ȻWindowsû������sleep����������ǿ�����΢������ping���������
function sleep3(n)
   if n > 0 then os.execute("ping -n " .. tonumber(n + 1) .. " localhost > NUL") end
end

--ʹ��socket����select���������Դ���0.1��n��ʹ�����ߵ�ʱ�侫�ȴﵽ���뼶��
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
