import sys
for filename in sys.argv[1:]:
	fp=open(filename)
	lines=fp.readlines()
	fp.close();
	acount=0;
	for i in lines:
		if i.find("asteroid")!=-1:
			acount+=1
	if (acount):
		for i in range(len(lines)):
			if lines[i].find("asteroid")!=-1:
				st=lines[i]
				lines[i]=lines[i].replace("<asteroid","<asteroid VarName=\"-asteroid_detail\" VarValue=\""+str(acount-1)+"\"")
				st=st.replace("asteroid","unit")
				st=st.replace("<unit","<unit VarName=\"asteroid_detail\" VarValue=\""+str(acount)+"\"")
				st=st.replace("AFieldBasePriv","Asteroid_Field")
				lines[i]+=st
	fp=open(filename+".xml","w")
	for i in lines:
		fp.write(i)
	fp.close()
	
		
	