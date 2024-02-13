; file testFile.as
.define sz=7
.define len=1000
	mov r3,LIST[sz]
	add r7,r2
	prn #sz
	prn #len
LIST: .data -90,sz,5000
MAIN: mov r3 ,LENGTH
	sub r1, r4
END: stop
VALID: .data 12,4,5
LENGTH: .string "GOOGLE!"
