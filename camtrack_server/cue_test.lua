--test for Cue
--linlin 13/8/2016

local t0 = os.clock();
local time1 = 10;
local time2 = 15;

function callcue()

	while(true)do

		if os.clock() - t0 == time1 then
			gma.cmd('Goto Cue 2');
		end
		if os.clock() - t0 == time2 then
			gma.cmd('Goto Cue 4');
		end
		--and so on

	end

end


return callcue();
