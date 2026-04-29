#!/bin/env sh

export SAFECMDS=Rplus,Cpp,CppIII,opt,shl,shr,dcr,exor,bigoh,tilde,bitand,bitor,xor,rightshift,enternote,enterexample,exitexample,required,requires,effects,postconditions,postcondition,preconditions,precondition,returns,throws,default,complexity,remark,remarks,note,notes,realnote,realnotes,errors,sync,implimits,replaceable,exceptionsafety,returntype,cvalue,ctype,ctypes,dtype,ctemplate,templalias,xref,xsee,ntbs,ntmbs,ntwcs,ntcxvis,ntcxxxiis,expos,impdef,notdef,unspec,unspecbool,seebelow,unspecuniqtype,unspecalloctype,unun,change,rationale,effect,difficulty,howwide,uniquens,cv,seebelow

export TEXTCMDS=tcode,term,grammarterm,techterm,defnx,defn,Fundescx,Fundesc,state,leftshift,EnterBlock,ExitBlock,NTS,EXPO,impdefx,UNSP,xname,mname,diffdef,stage,doccite,cvqual,numconst,logop

git latexdiff --no-cleanup --flatten  --main new.tex  --append-textcmd=${TEXTCMDS} --append-safecmd=${SAFECMDS}  --add-to-config='ARRENV=itemdecl;codeblock' -o diff.pdf 917cce5e008263a04aa66db6a5bc1e6d45688f30 --
