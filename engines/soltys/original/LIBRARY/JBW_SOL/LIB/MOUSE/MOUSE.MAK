MODEL=s
INC=C:\BORLANDC\INCLUDE;D:\JBW\INC

C=bcc -c -I$(INC) -G -r -k -O -d -f- -m$(MODEL) -Z -K -N-
ASM=tasm /la /zi /ml /w2

..\wind.lib:	wind.obj move.obj windhelp.obj scrsaver.obj
 tlib ..\wind /C +-wind +-move +-windhelp +-scrsaver

