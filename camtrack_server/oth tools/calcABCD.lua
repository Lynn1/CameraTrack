


local A,B,C,D;

local dh1  = 29;
local dv1  = -6;
local len1 = 33;

local dh2  = 33;
local dv2  = -1;
local len2 = 55;



function calcABCD()

	A = (dh2-dh1)/(len2-len1);
	B = dh1-len1*A;
	C = (dv2-dv1)/(len2-len1);
	D = dv1-len1*C;

	print("A: "..A.."    B: "..B.."    C: "..C.."    D: "..D);


end

return calcABCD();
