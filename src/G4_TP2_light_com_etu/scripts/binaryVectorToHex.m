function r=binaryVectorToHex(v)
r=[];
for n=1:8:length(v)
	byte=0;
	val=128;
	for m=1:8
		if v(n-1+m)
			byte = byte+val;
		end
		val=val/2;
	end
	r=[r dec2hex(byte,2)];
end
endfunction
